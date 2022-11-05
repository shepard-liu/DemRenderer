#include "mainwindow.h"
#include <QApplication>
#include "helpers.h"

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    Helpers::init();
    MainWindow w;
    w.show();
    return a.exec();
}
