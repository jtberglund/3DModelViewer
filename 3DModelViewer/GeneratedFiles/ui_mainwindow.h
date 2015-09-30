/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 5.5.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QToolBar>
#include "TabPane.h"

QT_BEGIN_NAMESPACE

class Ui_MainWindowClass
{
public:
    QAction *actionNew;
    QAction *actionExit;
    QAction *actionView_wireframe;
    QAction *actionLighting;
    QAction *actionToggleTexturing;
    TabPane *tabPane;
    QMenuBar *menuBar;
    QMenu *menuFile;
    QToolBar *mainToolBar;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *MainWindowClass)
    {
        if (MainWindowClass->objectName().isEmpty())
            MainWindowClass->setObjectName(QStringLiteral("MainWindowClass"));
        MainWindowClass->resize(600, 453);
        actionNew = new QAction(MainWindowClass);
        actionNew->setObjectName(QStringLiteral("actionNew"));
        actionExit = new QAction(MainWindowClass);
        actionExit->setObjectName(QStringLiteral("actionExit"));
        actionView_wireframe = new QAction(MainWindowClass);
        actionView_wireframe->setObjectName(QStringLiteral("actionView_wireframe"));
        actionView_wireframe->setCheckable(true);
        QIcon icon;
        icon.addFile(QStringLiteral(":/MainWindow/Resources/icons/mesh.svg"), QSize(), QIcon::Normal, QIcon::Off);
        actionView_wireframe->setIcon(icon);
        actionLighting = new QAction(MainWindowClass);
        actionLighting->setObjectName(QStringLiteral("actionLighting"));
        actionLighting->setCheckable(true);
        actionLighting->setChecked(true);
        QIcon icon1;
        icon1.addFile(QStringLiteral("Resources/icons/light-bulb24.svg"), QSize(), QIcon::Normal, QIcon::Off);
        actionLighting->setIcon(icon1);
        actionToggleTexturing = new QAction(MainWindowClass);
        actionToggleTexturing->setObjectName(QStringLiteral("actionToggleTexturing"));
        actionToggleTexturing->setCheckable(true);
        actionToggleTexturing->setChecked(true);
        QIcon icon2;
        icon2.addFile(QStringLiteral("Resources/icons/image.svg"), QSize(), QIcon::Normal, QIcon::Off);
        actionToggleTexturing->setIcon(icon2);
        tabPane = new TabPane(MainWindowClass);
        tabPane->setObjectName(QStringLiteral("tabPane"));
        MainWindowClass->setCentralWidget(tabPane);
        menuBar = new QMenuBar(MainWindowClass);
        menuBar->setObjectName(QStringLiteral("menuBar"));
        menuBar->setGeometry(QRect(0, 0, 600, 21));
        menuFile = new QMenu(menuBar);
        menuFile->setObjectName(QStringLiteral("menuFile"));
        MainWindowClass->setMenuBar(menuBar);
        mainToolBar = new QToolBar(MainWindowClass);
        mainToolBar->setObjectName(QStringLiteral("mainToolBar"));
        mainToolBar->setAcceptDrops(false);
        MainWindowClass->addToolBar(Qt::TopToolBarArea, mainToolBar);
        statusBar = new QStatusBar(MainWindowClass);
        statusBar->setObjectName(QStringLiteral("statusBar"));
        MainWindowClass->setStatusBar(statusBar);

        menuBar->addAction(menuFile->menuAction());
        menuFile->addAction(actionNew);
        menuFile->addSeparator();
        menuFile->addAction(actionExit);
        mainToolBar->addAction(actionView_wireframe);
        mainToolBar->addAction(actionToggleTexturing);
        mainToolBar->addAction(actionLighting);

        retranslateUi(MainWindowClass);

        QMetaObject::connectSlotsByName(MainWindowClass);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindowClass)
    {
        MainWindowClass->setWindowTitle(QApplication::translate("MainWindowClass", "3D Model Viewer", 0));
        actionNew->setText(QApplication::translate("MainWindowClass", "New", 0));
#ifndef QT_NO_STATUSTIP
        actionNew->setStatusTip(QApplication::translate("MainWindowClass", "Open a new file", 0));
#endif // QT_NO_STATUSTIP
        actionNew->setShortcut(QApplication::translate("MainWindowClass", "Ctrl+N", 0));
        actionExit->setText(QApplication::translate("MainWindowClass", "Exit", 0));
#ifndef QT_NO_TOOLTIP
        actionExit->setToolTip(QApplication::translate("MainWindowClass", "Exit", 0));
#endif // QT_NO_TOOLTIP
#ifndef QT_NO_STATUSTIP
        actionExit->setStatusTip(QApplication::translate("MainWindowClass", "Exit the application", 0));
#endif // QT_NO_STATUSTIP
        actionExit->setShortcut(QApplication::translate("MainWindowClass", "Esc", 0));
        actionView_wireframe->setText(QApplication::translate("MainWindowClass", "View wireframe", 0));
#ifndef QT_NO_TOOLTIP
        actionView_wireframe->setToolTip(QApplication::translate("MainWindowClass", "View this model as a wireframe", 0));
#endif // QT_NO_TOOLTIP
        actionView_wireframe->setShortcut(QApplication::translate("MainWindowClass", "Ctrl+M", 0));
        actionLighting->setText(QApplication::translate("MainWindowClass", "Toggle lighting", 0));
#ifndef QT_NO_TOOLTIP
        actionLighting->setToolTip(QApplication::translate("MainWindowClass", "Toggle Lighting", 0));
#endif // QT_NO_TOOLTIP
        actionLighting->setShortcut(QApplication::translate("MainWindowClass", "Ctrl+L", 0));
        actionToggleTexturing->setText(QApplication::translate("MainWindowClass", "Toggle Texturing", 0));
#ifndef QT_NO_TOOLTIP
        actionToggleTexturing->setToolTip(QApplication::translate("MainWindowClass", "Toggle Texturing", 0));
#endif // QT_NO_TOOLTIP
        actionToggleTexturing->setShortcut(QApplication::translate("MainWindowClass", "Ctrl+T", 0));
        menuFile->setTitle(QApplication::translate("MainWindowClass", "File", 0));
    } // retranslateUi

};

namespace Ui {
    class MainWindowClass: public Ui_MainWindowClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
