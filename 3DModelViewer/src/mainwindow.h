#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtWidgets/QMainWindow>
#include "ui_mainwindow.h"
#include "ModelViewer.h"
#include <memory>

using std::shared_ptr;
using std::string;

class TabPane;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindowClass _ui;
    TabPane* _tabPane;

    //shared_ptr<ModelViewer> _viewer;
    string _file;

private slots:
    void addNew();
    void exitApp();
    void toggleWireFrame(bool checked);

};

#endif // MAINWINDOW_H
