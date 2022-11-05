#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include <QFileDialog>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow) {
    ui->setupUi(this);

    // Renderer
    connect(this, &MainWindow::setupRenderer, ui->centralwidget, &Renderer::onSetupRenderer);
    connect(this, &MainWindow::setRendererProjectionType, ui->centralwidget,
            &Renderer::onSwitchProjectionType);
    connect(ui->mActionAutoFitElevation, &QAction::triggered, ui->centralwidget,
            &Renderer::onSetAutoFitElevation);
    // UI
    connect(ui->mActionOpen, &QAction::triggered, this, &MainWindow::onActionOpenTriggered);
    connect(ui->mActionOrthographic, &QAction::triggered, this,
            &MainWindow::onActionOrthoProjTriggered);
    connect(ui->mActionPerspective, &QAction::triggered, this, &MainWindow::onActionPerspProjTriggered);
    connect(ui->mActionRandomizeGradient, &QAction::triggered, this,
            &MainWindow::onActionRandomizeGradient);

}

MainWindow::~MainWindow() {
    delete ui;
}


void MainWindow::onActionOpenTriggered() {
    QString filepath = QFileDialog::getOpenFileName(this,
                       "请选择要打开的DEM文件(文本格式)", Helpers::applicationDir, "DEM (*.asc)");

    mDem = DigitalElevationModel::loadFromFile(filepath, DigitalElevationModel::FromText);
    emit setupRenderer(&mDem);

    ui->mActionRandomizeGradient->setEnabled(true);
    ui->mActionAutoFitElevation->setEnabled(true);
    ui->mActionOpenOrthoImage->setEnabled(true);
}

void MainWindow::onActionOrthoProjTriggered(bool checked) {
    ui->mActionPerspective->setChecked(!ui->mActionPerspective->isChecked());
    emit setRendererProjectionType(checked ? Renderer::Orthographic : Renderer::Perspective);
}

void MainWindow::onActionPerspProjTriggered(bool checked) {
    ui->mActionOrthographic->setChecked(!ui->mActionOrthographic->isChecked());
    emit setRendererProjectionType(checked ? Renderer::Perspective : Renderer::Orthographic);
}

void MainWindow::onActionRandomizeGradient() {
    emit setupRenderer(&mDem, true);
}

bool MainWindow::eventFilter(QObject *watched, QEvent *event) {
    if(event->type() == QEvent::KeyPress) {
        return true;
    } else if(event->type() ==
              (QEvent::MouseButtonPress |
               QEvent::MouseButtonRelease |
               QEvent::MouseMove |
               QEvent::Wheel)) {
        return true;
    }

    return QObject::eventFilter(watched, event);
}

