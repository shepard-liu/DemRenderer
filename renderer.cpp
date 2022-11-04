#include "renderer.h"
#include <QOpenGLShaderProgram>
#include <helpers.h>
#include <QSurfaceFormat>
#include <limits>
#include <algorithm>
#include <QRandomGenerator>

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
    glClearColor(0.0f, 0.0f, 1.0f, 0.0f);
    mProgram = new QOpenGLShaderProgram(this);

    // 加载着色器
    mProgram->addShaderFromSourceCode(QOpenGLShader::Vertex,
                                      Helpers::readFile("./dem.vsh"));
    mProgram->addShaderFromSourceCode(QOpenGLShader::Fragment,
                                      Helpers::readFile("./dem.fsh"));

    mProgram->link();

    // 获取着色器变量位置
    mPositionAttr = mProgram->attributeLocation("aPosition");
    mColorAttr =  mProgram->attributeLocation("aColor");
    mMatrixUnif = mProgram->uniformLocation("uMatrix");

    Q_ASSERT(mPositionAttr != -1);
    Q_ASSERT(mColorAttr != -1);
    Q_ASSERT(mMatrixUnif != -1);

    glEnable(GL_DEPTH_TEST);
}

void Renderer::resizeGL(int w, int h) {

}

void Renderer::paintGL() {
    if(muDemCols * muDemRows == 0) return;

    glClear(GL_COLOR_BUFFER_BIT);
    glClear(GL_DEPTH_BUFFER_BIT);

    mProgram->bind();

    // 计算模型视图变换矩阵
    QMatrix4x4 matrix;

    float elevSpan = mfMaxElev - mfMinElev;

    // 设定远近裁剪面相对于 DEM xyz 跨度
    matrix.perspective(60.0f,
                       width() / float(height()),
                       elevSpan * 0.001f,
                       std::max({elevSpan, float(muDemCols), float(muDemRows)}) * 100.0f);
    // 定位DEM到中心位置
//    matrix.translate(-muDemCols / 2.0f, -muDemRows / 2.0f, -(mfMaxElev + elevSpan));
    matrix.translate(mfModelTranslateX, mfModelTranslateY, mfModelTranslateZ);

    mProgram->setUniformValue(mMatrixUnif, matrix);

    // 绑定缓冲区对象
    glBindBuffer(GL_ARRAY_BUFFER, mVboIds[0]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mEboIds[0]);

    // 解释顶点属性
    glVertexAttribPointer(mPositionAttr, 3, GL_FLOAT,
                          GL_FALSE, (3 + 4) * sizeof(GLfloat), 0);
    glVertexAttribPointer(mColorAttr, 4, GL_FLOAT,
                          GL_FALSE, (3 + 4) * sizeof(GLfloat), (const void *)(3 * sizeof(GLfloat)));

    glEnableVertexAttribArray(mPositionAttr);
    glEnableVertexAttribArray(mColorAttr);

    // 渲染
    for(quint64 i = 0; i < muDemRows - 1; ++i) {
        glDrawElements(GL_TRIANGLE_STRIP, muDemCols * 2, GL_UNSIGNED_INT,
                       (const void *)(i * muDemCols * 2 * sizeof(GLuint)));
    }
//    glDrawElements(GL_TRIANGLE_STRIP, muDemCols * 2, GL_UNSIGNED_INT, 0);

    glDisableVertexAttribArray(mPositionAttr);
    glDisableVertexAttribArray(mColorAttr);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    mProgram->release();
}

void Renderer::onSetupRenderer(const DigitalElevationModel *pDem) {
    if(!pDem || pDem->isEmpty()) {
        return;
    }

    muDemCols = pDem->getCols();
    muDemRows = pDem->getRows();

    auto * pData = pDem->getData().data();

    std::vector<float> vertexAttribs{};
    vertexAttribs.reserve(muDemCols * muDemRows * (3 + 4));
    std::vector<std::vector<GLuint>> indexArrays(muDemRows - 1);

    // 纹理映射

    // 搜索DEM高程跨度
    float maxElev = -std::numeric_limits<float>::max(), minElev = -maxElev;

    for(quint64 i = 0; i < muDemCols * muDemRows; ++i) {
        if(pData[i] < minElev) minElev = pData[i];
        if(pData[i] > maxElev) maxElev = pData[i];
    }

    mfMaxElev = maxElev, mfMinElev = minElev;

    // 生成顶点数据
    for(quint64 y = 0; y < muDemRows; ++y) {
        auto &currentIndexArray = indexArrays[y];

        for(quint64 x = 0; x < muDemCols; ++x) {
            quint64 index = x + y * muDemCols;

            vertexAttribs.push_back(x);
            vertexAttribs.push_back(y);
            vertexAttribs.push_back(pData[index] * 0.1);
//            vertexAttribs.push_back(float(QRandomGenerator::global()->generateDouble()) *
//                                    100);
//            vertexAttribs.push_back(0.0f);

            // 插值出顶点渐变颜色
            auto vertexColor = Helpers::linearGradient(mGradientAltitude,
                               (pData[index] - minElev) / (maxElev - minElev));

//            auto color = float(QRandomGenerator::global()->generateDouble());
//            auto vertexColor = std::vector<float> {color, color, color, 1.0f };
            for(auto component : vertexColor) {
                vertexAttribs.push_back(component);
            }

            Q_ASSERT(vertexColor[3] == 1);
            // 生成索引数组
            if(y != muDemRows - 1) { // 若非最后一列
                currentIndexArray.push_back(index + muDemCols);
                currentIndexArray.push_back(index);
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

    // 解绑
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    // 设置视点
    mfModelTranslateZ =  -(maxElev + (maxElev - minElev)) ;
}

void Renderer::cleanUpBuffers() {
    if(mVboIds.size())
        glDeleteBuffers(mVboIds.size(), mVboIds.data());
    if(mEboIds.size())
        glDeleteBuffers(mEboIds.size(), mEboIds.data());

}

void Renderer::keyPressEvent(QKeyEvent *event) {
    qDebug() << event->key();

    switch (event->key()) {
    case Qt::Key::Key_W:
        mfModelTranslateY += 10.0f;
        break;
    case Qt::Key::Key_S:
        mfModelTranslateY -= 10.0f;
        break;
    case Qt::Key::Key_A:
        mfModelTranslateX -= 10.0f;
        break;
    case Qt::Key::Key_D:
        mfModelTranslateX += 10.0f;
        break;
    case Qt::Key::Key_Q:
        mfModelTranslateZ -= 10.0f;
        break;
    case Qt::Key::Key_E:
        mfModelTranslateZ += 10.0f;
        break;
    default:
        break;
    }

    update();

}












