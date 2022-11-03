#ifndef RENDERER_H
#define RENDERER_H

#include <QOpenGLWidget>

class Renderer : public QOpenGLWidget
{
public:
    explicit Renderer(QWidget* parent);
};

#endif // RENDERER_H
