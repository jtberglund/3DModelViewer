#include "TabPane.h"
#include "ModelViewer.h"
#include "Utils.h"
#include "QOpenGLContext"

TabPane::TabPane(QWidget* parent) :
  QTabWidget(parent)
{
    setTabsClosable(true);
    connect(this, SIGNAL(tabCloseRequested(int)), this, SLOT(closeTab(int)));

    // Create initial view
    addViewer();
}

TabPane::~TabPane() {}

int TabPane::addTab(string fileName) {

    if(currentIndex() != -1 || _viewers.empty()) {
        addViewer();
    }

    if(fileName.length() == 0 || !_viewers[_viewers.size() - 1]->loadFile(fileName)) {
        return -1; // File could not be loaded
    }

    string label = Utils::getFileNameFromPath(fileName);
    int ret = QTabWidget::addTab(_viewers[_viewers.size() - 1].get(), label.c_str());
    setCurrentIndex(count() - 1); // set the view to the new tab

    return ret;
}

void TabPane::addViewer() {
    shared_ptr<ModelViewer> viewer = shared_ptr<ModelViewer>(new ModelViewer(this));
    _viewers.push_back(viewer);
}

void TabPane::setWireFrameView(bool enabled) {
    if(currentIndex() == -1)
        return;

    // Toggle wireframe mode
    if(enabled)
        _viewers[currentIndex()]->setViewMode(ModelViewer::ViewMode::WireFrame);
    else
        _viewers[currentIndex()]->setViewMode(ModelViewer::ViewMode::ModelView);
}

void TabPane::closeTab(int index) {
    _viewers.erase(_viewers.begin() + index);
    removeTab(index);
}