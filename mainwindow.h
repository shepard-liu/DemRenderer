#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "digitalelevationmodel.h"
#include "renderer.h"
#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onActionOpenTriggered();
    void onActionOrthoProjTriggered(bool checked);
    void onActionPerspProjTriggered(bool checked);
    void onActionRandomizeGradient();

signals:
    void setupRenderer(const DigitalElevationModel* pDem, bool useRandomizedGradient = false);
    void setRendererProjectionType(Renderer::ProjectionType type);

private:
    Ui::MainWindow *ui;

    DigitalElevationModel mDem{};

private:
    // QObject interface
public:
    bool eventFilter(QObject *watched, QEvent *event) override;
};
#endif // MAINWINDOW_H
