/****************************************************************************
** Meta object code from reading C++ file 'TabPane.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.5.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../src/TabPane.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'TabPane.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.5.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
struct qt_meta_stringdata_TabPane_t {
    QByteArrayData data[8];
    char stringdata0[83];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_TabPane_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_TabPane_t qt_meta_stringdata_TabPane = {
    {
QT_MOC_LITERAL(0, 0, 7), // "TabPane"
QT_MOC_LITERAL(1, 8, 8), // "closeTab"
QT_MOC_LITERAL(2, 17, 0), // ""
QT_MOC_LITERAL(3, 18, 5), // "index"
QT_MOC_LITERAL(4, 24, 14), // "enableLighting"
QT_MOC_LITERAL(5, 39, 7), // "enabled"
QT_MOC_LITERAL(6, 47, 19), // "enableWireFrameView"
QT_MOC_LITERAL(7, 67, 15) // "enableTexturing"

    },
    "TabPane\0closeTab\0\0index\0enableLighting\0"
    "enabled\0enableWireFrameView\0enableTexturing"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_TabPane[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       4,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    1,   34,    2, 0x0a /* Public */,
       4,    1,   37,    2, 0x0a /* Public */,
       6,    1,   40,    2, 0x0a /* Public */,
       7,    1,   43,    2, 0x0a /* Public */,

 // slots: parameters
    QMetaType::Void, QMetaType::Int,    3,
    QMetaType::Void, QMetaType::Bool,    5,
    QMetaType::Void, QMetaType::Bool,    5,
    QMetaType::Void, QMetaType::Bool,    5,

       0        // eod
};

void TabPane::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        TabPane *_t = static_cast<TabPane *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->closeTab((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 1: _t->enableLighting((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 2: _t->enableWireFrameView((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 3: _t->enableTexturing((*reinterpret_cast< bool(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObject TabPane::staticMetaObject = {
    { &QTabWidget::staticMetaObject, qt_meta_stringdata_TabPane.data,
      qt_meta_data_TabPane,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *TabPane::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *TabPane::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_TabPane.stringdata0))
        return static_cast<void*>(const_cast< TabPane*>(this));
    return QTabWidget::qt_metacast(_clname);
}

int TabPane::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QTabWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 4)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 4;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 4)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 4;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
