#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "digitalelevationmodel.h"
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
    void onActionRandomizeGradientTriggered();
    void onActionOpenOrthoImageTriggered();
    void onActionIncElevScaleTriggered();
    void onActionDecElevScaleTriggered();
    void onActionResetElevScaleTriggered();

private:
    Ui::MainWindow *ui;

    DigitalElevationModel mDem{};
    QImage mTextureImage{};

private:
    // QObject interface
public:
    bool eventFilter(QObject *watched, QEvent *event) override;
};
#endif // MAINWINDOW_H
