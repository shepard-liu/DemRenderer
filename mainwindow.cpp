#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include <QFileDialog>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow) {
    ui->setupUi(this);

    // Renderer
    connect(ui->mActionAutoFitElevation, &QAction::triggered, ui->centralwidget,
            &Renderer::onSetAutoFitElevation);
    connect(ui->mActionResetCamera, &QAction::triggered, ui->centralwidget,
            &Renderer::onResetCameraControl);
    connect(ui->mActionEnableOrthoImageTexture, &QAction::triggered, ui->centralwidget,
            &Renderer::onEnableTextureRender);
    // UI
    connect(ui->mActionOpen, &QAction::triggered, this, &MainWindow::onActionOpenTriggered);
    connect(ui->mActionOrthographic, &QAction::triggered, this,
            &MainWindow::onActionOrthoProjTriggered);
    connect(ui->mActionPerspective, &QAction::triggered, this, &MainWindow::onActionPerspProjTriggered);
    connect(ui->mActionRandomizeGradient, &QAction::triggered, this,
            &MainWindow::onActionRandomizeGradientTriggered);
    connect(ui->mActionOpenOrthoImage,  &QAction::triggered, this,
            &MainWindow::onActionOpenOrthoImageTriggered);
    connect(ui->mActionIncElevScale, &QAction::triggered, this,
            &MainWindow::onActionIncElevScaleTriggered);
    connect(ui->mActionDecElevScale, &QAction::triggered, this,
            &MainWindow::onActionDecElevScaleTriggered);
    connect(ui->mActionResetElevScale, &QAction::triggered, this,
            &MainWindow::onActionResetElevScaleTriggered);
}

MainWindow::~MainWindow() {
    delete ui;
}


void MainWindow::onActionOpenTriggered() {
    QString filepath = QFileDialog::getOpenFileName(this,
                       "请选择要打开的DEM文件(文本格式)", Helpers::applicationDir, "DEM (*.asc)");
    if(filepath.size() == 0)return;

    mDem = DigitalElevationModel::loadFromFile(filepath, DigitalElevationModel::FromText);
    ui->centralwidget->setupRenderer(&mDem);

    ui->mActionRandomizeGradient->setEnabled(true);
    ui->mActionAutoFitElevation->setEnabled(true);
    ui->mActionIncElevScale->setEnabled(true);
    ui->mActionDecElevScale->setEnabled(true);
    ui->mActionOpenOrthoImage->setEnabled(true);

    ui->mActionEnableOrthoImageTexture->setEnabled(false);
    ui->mActionEnableOrthoImageTexture->setChecked(false);
}

void MainWindow::onActionOrthoProjTriggered(bool checked) {
    ui->mActionPerspective->setChecked(!ui->mActionPerspective->isChecked());
    ui->centralwidget->switchProjectionType(checked ? Renderer::Orthographic : Renderer::Perspective);
}

void MainWindow::onActionPerspProjTriggered(bool checked) {
    ui->mActionOrthographic->setChecked(!ui->mActionOrthographic->isChecked());
    ui->centralwidget->switchProjectionType(checked ? Renderer::Perspective : Renderer::Orthographic);
}

void MainWindow::onActionRandomizeGradientTriggered() {
    ui->centralwidget->setupRenderer(&mDem, mTextureImage.isNull() ? nullptr : &mTextureImage, true);
    ui->mActionEnableOrthoImageTexture->setChecked(false);
    ui->centralwidget->onEnableTextureRender(false);
}

void MainWindow::onActionOpenOrthoImageTriggered() {
    QString filepath = QFileDialog::getOpenFileName(this,
                       "请选择要打开的纹理图像", Helpers::applicationDir, "image (*.jpg)");
    if(filepath.size() == 0)return;

    mTextureImage = QImage(filepath);

    ui->centralwidget->setupRenderer(&mDem, &mTextureImage);
    ui->mActionEnableOrthoImageTexture->setEnabled(true);
    ui->mActionEnableOrthoImageTexture->setChecked(true);
}

void MainWindow::onActionIncElevScaleTriggered() {
    ui->centralwidget->setElevationScale(ui->centralwidget->elevationScale() + 0.10);
}

void MainWindow::onActionDecElevScaleTriggered() {
    ui->centralwidget->setElevationScale(ui->centralwidget->elevationScale() - 0.10);
}

void MainWindow::onActionResetElevScaleTriggered() {
    ui->centralwidget->setElevationScale(1.0);
}

bool MainWindow::eventFilter(QObject *watched, QEvent *event) {
    if(event->type() == QEvent::KeyPress) {
        return true;
    } else if(event->type() == (
                  QEvent::MouseButtonPress |
                  QEvent::MouseButtonRelease |
                  QEvent::MouseMove |
                  QEvent::Wheel)) {
        return true;
    }

    return QObject::eventFilter(watched, event);
}

