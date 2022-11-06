#include "renderer.h"
#include <QOpenGLShaderProgram>
#include <helpers.h>
#include <QSurfaceFormat>
#include <limits>
#include <algorithm>
#include <QRandomGenerator>
#include <QApplication>

Renderer::Renderer(QWidget *parent): QOpenGLWidget(parent) {
    QSurfaceFormat format;
    format.setSamples(16);
    setFormat(format);
    setFocusPolicy(Qt::StrongFocus);
}

Renderer::~Renderer() {
    cleanUpBuffers();
    delete mProgram;
}

void Renderer::initializeGL() {
    initializeOpenGLFunctions();
    glClearColor(0.52f, 0.807f, 0.922f, 0.0f);
    mProgram = new QOpenGLShaderProgram(this);

    qDebug() << Helpers::applicationDir;
    // 加载着色器
    mProgram->addShaderFromSourceCode(QOpenGLShader::Vertex,
                                      Helpers::readFile(Helpers::applicationDir + "/dem.vsh"));
    mProgram->addShaderFromSourceCode(QOpenGLShader::Fragment,
                                      Helpers::readFile(Helpers::applicationDir + "/dem.fsh"));
    mProgram->link();

    // 获取着色器变量位置
    mPositionAttr = mProgram->attributeLocation("aPosition");
    mColorAttr =  mProgram->attributeLocation("aColor");
    mTexCoordAttr = mProgram->attributeLocation("aTexCoord");
    mMatrixUnif = mProgram->uniformLocation("uMatrix");
    mEnableTexUnif = mProgram->uniformLocation("uEnableTex");
    mSamplerUnif = mProgram->uniformLocation("uSampler");

    Q_ASSERT(mPositionAttr != -1);
    Q_ASSERT(mColorAttr != -1);
//    Q_ASSERT(mTexCoordAttr != -1);
    Q_ASSERT(mMatrixUnif != -1);
//    Q_ASSERT(mEnableTexUnif != -1);
//    Q_ASSERT(mSamplerUnif != -1);

    // 消隐
    glEnable(GL_DEPTH_TEST);
}

void Renderer::resizeGL(int w, int h) {
    updateMvpMatrix();
}

void Renderer::paintGL() {
    if(!ready()) return;

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    mProgram->bind();

    // 传入MVP矩阵
    mProgram->setUniformValue(mMatrixUnif, mMvpMatrix);
    // 传入是否启用纹理
    mProgram->setUniformValue(mEnableTexUnif, mbRenderTexture);

    // 绑定缓冲区对象
    glBindBuffer(GL_ARRAY_BUFFER, mVboIds[0]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mEboIds[0]);

    if(mbRenderTexture) {
        mProgram->setUniformValue(mSamplerUnif, 0);
    }

    // 解释顶点属性
    GLuint bytesPerVertex = (3 + 4 + 2) * sizeof(GLfloat);
    glVertexAttribPointer(mPositionAttr,    3, GL_FLOAT, GL_FALSE, bytesPerVertex,
                          0);
    glVertexAttribPointer(mColorAttr,       4, GL_FLOAT, GL_FALSE, bytesPerVertex,
                          (const void *)(3 * sizeof(GLfloat)));
    glVertexAttribPointer(mTexCoordAttr,    2, GL_FLOAT, GL_FALSE, bytesPerVertex,
                          (const void *)(7 * sizeof(GLfloat)));

    glEnableVertexAttribArray(mPositionAttr);
    glEnableVertexAttribArray(mColorAttr);
    glEnableVertexAttribArray(mTexCoordAttr);
    // 渲染
    if(mbRenderTexture && mpTexture) {
        mpTexture->bind(0);
    }
    for(quint64 i = 0; i < muDemRows - 1; ++i) {
        glDrawElements(GL_TRIANGLE_STRIP, muDemCols * 2, GL_UNSIGNED_INT,
                       (const void *)(i * muDemCols * 2 * sizeof(GLuint)));
    }

    glDisableVertexAttribArray(mPositionAttr);
    glDisableVertexAttribArray(mColorAttr);
    glDisableVertexAttribArray(mTexCoordAttr);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    mProgram->release();
}

void Renderer::setupRenderer(const DigitalElevationModel *pDem, const QImage* pTexture,
                             bool useRandomizedGradient) {
    if(!pDem || pDem->isEmpty()) {
        return;
    }
    mbRenderTexture = pTexture != nullptr ? true : false;

    muDemCols = pDem->getCols();
    muDemRows = pDem->getRows();
    mfBboxXSpan = muDemCols * pDem->getCellSize();
    mfBboxYSpan = muDemRows * pDem->getCellSize();

    auto * pData = pDem->getData().data();

    std::vector<float> vertexAttribs{};
    vertexAttribs.reserve(muDemCols * muDemRows * (3 + 4 + 2));
    std::vector<std::vector<GLuint>> indexArrays(muDemRows - 1);

    // 搜索DEM高程跨度
    float maxElev = -std::numeric_limits<float>::max(), minElev = -maxElev;

    for(quint64 i = 0; i < muDemCols * muDemRows; ++i) {
        if(pData[i] < minElev) minElev = pData[i];
        if(pData[i] > maxElev) maxElev = pData[i];
    }

    mfMaxElev = maxElev, mfMinElev = minElev;

    // 计算渲染参数
    float elevSpan = maxElev - minElev;
    mfBboxMinEdge = std::min({elevSpan, mfBboxXSpan, mfBboxYSpan});
    mfBboxMaxEdge = std::max({elevSpan, mfBboxXSpan, mfBboxYSpan});
    mfBboxDiagonal = sqrt(elevSpan * elevSpan +  mfBboxXSpan * mfBboxXSpan + mfBboxYSpan * mfBboxYSpan);
    mfDemGridDiagonal = sqrt(mfBboxXSpan * mfBboxXSpan + mfBboxYSpan * mfBboxYSpan);

    // 找到DEM格网中心位置
    auto geoCenter = pDem->getGeoCoord(muDemRows / 2.0, muDemCols / 2.0).toVector2D();
    mDemXYCenter = QVector2D(geoCenter.y(), geoCenter.x());

    // 确定要渲染的渐变
    std::vector<Helpers::ColorStop> gradient = mDefaultGradient;
    if(useRandomizedGradient) {
        gradient = std::vector<Helpers::ColorStop>();
        int nStops = QRandomGenerator::global()->bounded(2, 6);
        for(int i = 0; i <= nStops; ++i) {
            gradient.push_back(Helpers::ColorStop(i / float(nStops),
                                                  QRandomGenerator::global()->bounded(0, 256),
                                                  QRandomGenerator::global()->bounded(0, 256),
                                                  QRandomGenerator::global()->bounded(0, 256),
                                                  1.0f
                                                 ));
        }
    }

    // 生成顶点数据
    for(quint64 y = 0; y < muDemRows; ++y) {
        auto &currentIndexArray = indexArrays[y];

        for(quint64 x = 0; x < muDemCols; ++x) {
            quint64 index = x + y * muDemCols;

            QVector3D geoCoord = pDem->getGeoCoord(y, x);

            /**
             * 交换XY轴输入顶点
             *
             * 地理坐标系为北东高坐标（左手系），而OpenGL为右手。
             * 直接输入会导致XY翻转，DEM平面被沿XY轴角平分线对称。
             * 交换后输入，世界坐标系的Y轴为DEM地理参考的X轴，世界坐标系的X轴为地理参考的Y轴。
             */
            vertexAttribs.insert(vertexAttribs.end(), {
                geoCoord.y(),
                geoCoord.x(),
                pData[index],
            });

            // 插值出顶点渐变颜色
            auto vertexColor = Helpers::linearGradient(gradient,
                               (pData[index] - minElev) / (maxElev - minElev));
            vertexAttribs.insert(vertexAttribs.end(), vertexColor.begin(), vertexColor.end());

            // 纹理映射
            vertexAttribs.insert(vertexAttribs.end(), {
                mbRenderTexture ? x / float(muDemCols)              : 0,
                mbRenderTexture ? -(y / float(muDemRows)) + 1.0f    : 0,
            });

            // 生成索引数组
            if(y != muDemRows - 1) { // 若非最后一列
                currentIndexArray.insert(currentIndexArray.end(), {
                    GLuint(index + muDemCols),
                    GLuint(index)
                });
            }
        }
    }

    // 初始化渲染
    cleanUpBuffers();
    mVboIds = std::vector<GLuint>(1, 0);
    mEboIds = std::vector<GLuint>(1, 0);

    glGenBuffers(1, mVboIds.data());
    glGenBuffers(1, mEboIds.data());

    // 缓存VBO数据
    glBindBuffer(GL_ARRAY_BUFFER, mVboIds[0]);
    glBufferData(GL_ARRAY_BUFFER, vertexAttribs.size() * sizeof(GLfloat),
                 vertexAttribs.data(), GL_STATIC_DRAW);

    // 缓存EBO数据
    auto flatIndexArray = std::accumulate(indexArrays.begin(),
                                          indexArrays.end(),
                                          std::vector<GLuint> {},
    [](std::vector<GLuint>& a, std::vector<GLuint>& b) {
        a.insert(a.end(), b.begin(), b.end());
        return a;
    });
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mEboIds[0]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 flatIndexArray.size() * sizeof(GLuint), flatIndexArray.data(), GL_STATIC_DRAW);

    // 载入纹理图像
    if(mbRenderTexture) {
        mpTexture = new QOpenGLTexture((*pTexture).mirrored());
        mpTexture->setMagnificationFilter(QOpenGLTexture::Linear);
        mpTexture->setMinificationFilter(QOpenGLTexture::LinearMipMapLinear);
        mpTexture->setSize(pTexture->width(), pTexture->height());
        mpTexture->setFormat(QOpenGLTexture::TextureFormat::RGBFormat);
        mpTexture->allocateStorage(QOpenGLTexture::PixelFormat::RGB, QOpenGLTexture::PixelType::UInt8);
    }

    // 解绑
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    onResetCameraControl();

    updateMvpMatrix();
    update();
}

void Renderer::switchProjectionType(ProjectionType type) {
    mCurrentProjType = type;
    onResetCameraControl();
}

float Renderer::elevationScale() const {
    return mfElevScale;
}

void Renderer::setElevationScale(float newScale) {
    mfElevScale = newScale;
    if(mfElevScale < 0.05f)mfElevScale = 0.05f;
    updateMvpMatrix();
    update();
}

void Renderer::onResetCameraControl() {
    mOrbitCameraCtrl.setCenter(QVector3D(mDemXYCenter, 0.0f));
    mOrbitCameraCtrl.setTheta(Helpers::Pi / 3);
    mOrbitCameraCtrl.setPhi(0);
    if(mCurrentProjType == ProjectionType::Perspective) {
        mOrbitCameraCtrl.setRadius(mfDemGridDiagonal);
    } else if(mCurrentProjType == ProjectionType::Orthographic) {
        mOrbitCameraCtrl.setRadius(mfBboxMaxEdge * sqrt(3.0f));
        mfOrthoZoom = 1.0f;
    }
    updateMvpMatrix();
    update();
}

void Renderer::onSetAutoFitElevation() {
    // 自适应高程将高程缩放至(0~DEM格网平面对角线的一半)
    mfElevScale = (mfDemGridDiagonal / 2.0f) / (mfMaxElev - mfMinElev);
    updateMvpMatrix();
    update();
}

void Renderer::onEnableTextureRender(bool enabled) {
    mbRenderTexture = enabled;
    updateMvpMatrix();
    update();
}

void Renderer::cleanUpBuffers() {
    if(mVboIds.size())
        glDeleteBuffers(mVboIds.size(), mVboIds.data());
    if(mEboIds.size())
        glDeleteBuffers(mEboIds.size(), mEboIds.data());
    if(mpTexture) {
        delete mpTexture;
        mpTexture = nullptr;
    }
}

void Renderer::updateMvpMatrix() {
    mMvpMatrix = QMatrix4x4{};

    if(mCurrentProjType == ProjectionType::Perspective) {
        // 透视投影矩阵：
        mMvpMatrix.perspective(60.0f,
                               width() / float(height()),
                               mfBboxMinEdge * NEAR_PLANE_SCALE,
                               mfBboxMaxEdge * FAR_PLANE_SCALE);
    } else if(mCurrentProjType == ProjectionType::Orthographic) {
        float desiredBoxHeight = mfBboxMaxEdge * mfOrthoZoom;
        float aspectRatio = width() / float(height());
        // 正射投影矩阵：
        mMvpMatrix.ortho(-desiredBoxHeight * aspectRatio / 2.0f,
                         desiredBoxHeight * aspectRatio / 2.0f,
                         -desiredBoxHeight / 2.0f,
                         desiredBoxHeight / 2.0f,
                         0.0f,
                         mfBboxDiagonal * 100.0f);
    }

    // 右乘视图矩阵
    mMvpMatrix *= mOrbitCameraCtrl.computeViewMatrix();

    // 右乘模型矩阵
    mMvpMatrix.scale(1.0f, 1.0f, mfElevScale);
}

bool Renderer::ready() {
    return muDemCols != 0 && muDemRows != 0;
}

void Renderer::mousePressEvent(QMouseEvent *event) {
    QOpenGLWidget::mousePressEvent(event);

    if(!ready()) return;

    switch (event->button()) {
    case Qt::MouseButton::LeftButton:
        mbLeftDown = true;
        break;
    case Qt::MouseButton::RightButton:
        mbRightDown = true;
        break;
    default:
        break;
    }

    mMouseDownPos = event->pos();

    // 记录当前相机参数
    mCameraPhiOnMouseDown = mOrbitCameraCtrl.phi();
    mCameraThetaOnMouseDown = mOrbitCameraCtrl.theta();
    mCameraCenterOnMouseDown = mOrbitCameraCtrl.center();
    mCameraInvRotationMatrixOnMouseDown = mOrbitCameraCtrl.computeRotationMatrix().transposed();
}

void Renderer::mouseReleaseEvent(QMouseEvent *event) {
    QOpenGLWidget::mouseReleaseEvent(event);

    if(!ready()) return;

    mbLeftDown = mbRightDown = false;
}

void Renderer::mouseMoveEvent(QMouseEvent *event) {
    QOpenGLWidget::mouseMoveEvent(event);

    if(!ready()) return;

    QPoint currentPos = event->pos();

    float deltaX = currentPos.x() - mMouseDownPos.x(),
          deltaY = currentPos.y() - mMouseDownPos.y();

    if(mbLeftDown) {
        mOrbitCameraCtrl.setPhi(mCameraPhiOnMouseDown - deltaX / width() * Helpers::Pi);
        mOrbitCameraCtrl.setTheta(mCameraThetaOnMouseDown - deltaY / height() * Helpers::Pi);
    } else if (mbRightDown) {
        QVector4D normalPlaneDeltaVector = QVector4D(
                                               -deltaX / width() * mfBboxDiagonal / 2,
                                               deltaY / height() * mfBboxDiagonal / 2,
                                               0.0f,
                                               1.0f);
        QVector4D centerDeltaVector = mCameraInvRotationMatrixOnMouseDown.transposed() *
                                      normalPlaneDeltaVector;

        mOrbitCameraCtrl.setCenter(mCameraCenterOnMouseDown + centerDeltaVector.toVector3D());
    }

    updateMvpMatrix();
    update();
}


void Renderer::wheelEvent(QWheelEvent *event) {
    if(!ready()) return;

    float positive = event->angleDelta().y() > 0 ? 1.0f : -1.0f;

    if(mCurrentProjType == ProjectionType::Perspective) {
        mOrbitCameraCtrl.move(0.0f, 0.0f, mfBboxDiagonal * 0.05 * (-positive));
    } else if(mCurrentProjType == ProjectionType::Orthographic) {
        mfOrthoZoom += 0.05 * (-positive);
        if(mfOrthoZoom < 0.1f)mfOrthoZoom = 0.1f;
        if(mfOrthoZoom > 2.0f)mfOrthoZoom = 2.0f;
    }

    updateMvpMatrix();
    update();
}








