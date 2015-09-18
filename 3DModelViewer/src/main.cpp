#include "mainwindow.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[]) {
    // Set default format for all QOpenGLWidget instances
    //QSurfaceFormat format;
    //format.setDepthBufferSize(24);
    //format.setMajorVersion(3);
    //format.setMinorVersion(3);
    //format.setSamples(4);
    //QSurfaceFormat::setDefaultFormat(format);

    QApplication app(argc, argv);
    app.setAttribute(Qt::AA_ShareOpenGLContexts);
    MainWindow window;
    window.show();
    return app.exec();
}
