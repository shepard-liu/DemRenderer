#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include <QFileDialog>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow) {
    ui->setupUi(this);
    connect(ui->mActionOpen, &QAction::triggered, this,
            &MainWindow::onActionOpenTriggered);
    connect(this, &MainWindow::setupRenderer, ui->centralwidget,
            &Renderer::onSetupRenderer);
}

MainWindow::~MainWindow() {
    delete ui;
}


void MainWindow::onActionOpenTriggered() {

    QString filepath = QFileDialog::getOpenFileName(this,
                       "请选择要打开的DEM文件(文本格式)", ".", "DEM (*.asc)");

    mDem = DigitalElevationModel::loadFromFile(filepath,
            DigitalElevationModel::FromText);
    emit setupRenderer(&mDem);
}

bool MainWindow::eventFilter(QObject *watched, QEvent *event) {
    if(event->type() == QEvent::KeyPress) {
        qDebug() << "Key event";
        return true;
    } else {
        return QObject::eventFilter(watched, event);
    }
}

