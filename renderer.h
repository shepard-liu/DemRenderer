#ifndef RENDERER_H
#define RENDERER_H

#include <QKeyEvent>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLWidget>
#include "digitalelevationmodel.h"
#include "helpers.h"

class Renderer : public QOpenGLWidget, protected QOpenGLFunctions {
    Q_OBJECT
public:
    explicit Renderer(QWidget* parent);
    ~Renderer();

    // QOpenGLWidget interface
protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;

public slots:
    void onSetupRenderer(const DigitalElevationModel* pDem);

private:
    void cleanUpBuffers();

private:
    QOpenGLShaderProgram* mProgram = nullptr;
    // 相机
    QMatrix4x4 mCameraMatrix{};

    // DEM渲染元数据
    quint64 muDemCols{};
    quint64 muDemRows{};
    float mfMinElev{};
    float mfMaxElev{};
    quint64 muTriangles{};  // 三角形计数

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
    const std::vector<Helpers::ColorStop> mGradientAltitude{
        Helpers::ColorStop(0.0f, 34, 191, 195, 1.0f),
        Helpers::ColorStop(0.18f, 123, 227, 62, 1.0f),
        Helpers::ColorStop(1.0f, 253, 146, 45, 1.0f),
    };

    // 模型变换
    float mfModelTranslateX = 0.0f;
    float mfModelTranslateY = 0.0f;
    float mfModelTranslateZ = 0.0f;


    // QWidget interface
protected:
    void keyPressEvent(QKeyEvent *event)override;
};

#endif // RENDERER_H
