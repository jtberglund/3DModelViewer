#include "mainwindow.h"
#include "TabPane.h"
#include "ModelViewer.h"
#include "QMenu"
#include "QFileDialog"
#include "QErrorMessage"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    _ui.setupUi(this);

    connect(_ui.actionNew, SIGNAL(triggered()), this, SLOT(addNew()));
    connect(_ui.actionExit, SIGNAL(triggered()), this, SLOT(exitApp()));
    connect(_ui.actionView_wireframe, SIGNAL(triggered(bool)), this, SLOT(toggleWireFrame(bool)));
}

MainWindow::~MainWindow() {}

void MainWindow::addNew() {

    // TEMP SHORTCUT
    //_ui.tabPane->addTab("Resources\\spider\\spider.obj");
    //_ui.tabPane->addTab("Resources\\jeep\\jeep1.ms3d");
    //return;

    _file = QFileDialog::getOpenFileName(
        this,                  // Parent object
        tr("Open file"),       // Caption
        QDir::homePath(),      // Directory
        tr("All Files (*.*);;" // File filter
           "Wavefront (*.obj)")
    ).toStdString();

    if(_file.length() == 0)
        return; // no file picked

    // Load the file into the viewer
    if(_ui.tabPane->addTab(_file) == -1) {
        // If we can't load the file, create an error popup
        QErrorMessage errorBox;
        errorBox.showMessage("Error: Invalid file tpye");
        errorBox.exec();
        return;
    }

    // Set window title to reflect new file
    string title = "3D Model Viewer - ";
    title.append(_file);
    setWindowTitle(title.c_str());

    // Add a TabPane that contains our diferent viewports and widgets
    //_tabPane = new TabPane(this);
    //setCentralWidget(_tabPane);
}

void MainWindow::exitApp() {
    close();
}

void MainWindow::toggleWireFrame(bool checked) {
    _ui.tabPane->setWireFrameView(checked);
}