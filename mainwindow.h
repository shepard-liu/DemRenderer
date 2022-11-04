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

signals:
    void setupRenderer(const DigitalElevationModel* pDem);

private:
    Ui::MainWindow *ui;

    DigitalElevationModel mDem{};

    // QObject interface
public:
    bool eventFilter(QObject *watched, QEvent *event) override;
};
#endif // MAINWINDOW_H
