#ifndef RENDERER_H
#define RENDERER_H

#include "orbitcontrols.h"
#include <QKeyEvent>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLWidget>
#include "digitalelevationmodel.h"
#include "helpers.h"


class Renderer : public QOpenGLWidget, protected QOpenGLFunctions {
    Q_OBJECT

public:
    enum ProjectionType {
        Orthographic = 0x1,
        Perspective = 0x2,
    };

    // 和包围盒最长边长度一起用于确定近裁剪面
    const float NEAR_PLANE_SCALE = 0.001f;
    // 和包围盒最短边长度一起用于确定近裁剪面
    const float FAR_PLANE_SCALE = 100.0f;

public:
    explicit Renderer(QWidget* parent);
    ~Renderer();

    // QOpenGLWidget interface
protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;

public slots:
    void onSetupRenderer(const DigitalElevationModel* pDem, bool useRandomizedGradient = false);
    void onSwitchProjectionType(Renderer::ProjectionType type);
    void onResetCameraControl();
    void onSetAutoFitElevation(bool enabled);

private:
    void cleanUpBuffers();
    void updateMvpMatrix();
    bool ready();

private:
    QOpenGLShaderProgram* mProgram = nullptr;
    // 当前投影类型
    ProjectionType mCurrentProjType = ProjectionType::Perspective;
    // 正射缩放倍率
    float mfOrthoZoom{1.0};
    // 模型视图投影变换矩阵
    QMatrix4x4 mMvpMatrix{};
    // 环绕式相机控制器
    OrbitControls mOrbitCameraCtrl{};
    // 是否自适应高程显示
    bool mbAutoFitElevation{false};

    // DEM渲染元数据
    quint64 muDemCols{};
    quint64 muDemRows{};
    float mfMinElev{};
    float mfMaxElev{};
    float mfBboxMinEdge{};   // DEM包围盒最小边
    float mfBboxMaxEdge{};   // DEM包围盒最大边
    float mfBboxDiagonal{}; // DEM包围盒斜对角线


    // VAO在低版本OpenGL ES不支持，这里不使用
    // VBOs
    std::vector<GLuint> mVboIds{};
    // EBOs
    std::vector<GLuint> mEboIds{};

    // attribute变量aPosition
    GLint mPositionAttr{-1};
    // attribute变量aColor
    GLint mColorAttr{-1};
    // uniform变量uMatrix
    GLint mMatrixUnif{-1};

    // 线性渐变插值转折点
    const std::vector<Helpers::ColorStop> mDefaultGradient{
        Helpers::ColorStop(0.0f, 34, 191, 195, 1.0f),
        Helpers::ColorStop(0.18f, 123, 227, 62, 1.0f),
        Helpers::ColorStop(1.0f, 253, 95, 10, 1.0f),
    };

    // 鼠标状态
    bool mbLeftDown{false};    // 左键按下
    bool mbRightDown{false};   // 右键按下
    QPoint mMouseDownPos{};

    float mCameraPhiOnMouseDown{};
    float mCameraThetaOnMouseDown{};
    QVector3D mCameraCenterOnMouseDown{};
    QMatrix4x4 mCameraInvRotationMatrixOnMouseDown{};

    // QWidget interface
protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
};

#endif // RENDERER_H
