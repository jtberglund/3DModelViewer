#pragma once

#include "qtabwidget.h"
#include <memory>
#include <vector>

using std::string;
using std::shared_ptr;
using std::unique_ptr;

class ModelViewer;
class QOpenGLContext;

class TabPane : public QTabWidget {
    Q_OBJECT

public:
    TabPane(QWidget* parent = 0);
    ~TabPane();

    // Create a new tab with the specified file
    // Returns index of the tab
    int addTab(string fileName);
    void setWireFrameView(bool enabled);

public slots:
    void closeTab(int index);

private:
    // Holds all of our views
    std::vector<shared_ptr<ModelViewer>> _viewers;

    void addViewer();
};

