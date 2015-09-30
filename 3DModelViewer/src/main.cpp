#include "mainwindow.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    app.setAttribute(Qt::AA_ShareOpenGLContexts);
    MainWindow window;
    window.show();
    return app.exec();
}
