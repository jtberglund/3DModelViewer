#include "mainwindow.h"
#include "QMenu"
#include "QFileDialog"
#include "QErrorMessage"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    _ui.setupUi(this);

    connect(_ui.actionNew, SIGNAL(triggered()), this, SLOT(addNew()));
    connect(_ui.actionExit, SIGNAL(triggered()), this, SLOT(exitApp()));

}

MainWindow::~MainWindow() {}

void MainWindow::addNew() {
    _ui.viewer->loadFile("Resources\\jeep\\jeep1.ms3d");
    return;
    _file = QFileDialog::getOpenFileName(
        this,                  // Parent object
        tr("Open file"),       // Caption
        QDir::homePath(),      // Directory
        tr("All Files (*.*);;" // File filter
           "Wavefront (*.obj)")
    ).toStdString();

    // Load the file into the viewer
    if(!_ui.viewer->loadFile(_file)) {
        // If we can't load the file, create an error popup
        QErrorMessage errorBox;
        errorBox.showMessage("Error: Invalid file tpye");
        errorBox.exec();
        //msgBox.critical(0, "Error", "Invalid file");
        //msgBox.setGeometry(500, 500, 500, 200);
        //msgBox.setFixedSize(500, 200);
        //msgBox.exec();
    }

    // Set window title to reflect new file
    string title = "3D Model Viewer - ";
    title.append(_file);
    setWindowTitle(title.c_str());
}

void MainWindow::exitApp() {
    close();
}