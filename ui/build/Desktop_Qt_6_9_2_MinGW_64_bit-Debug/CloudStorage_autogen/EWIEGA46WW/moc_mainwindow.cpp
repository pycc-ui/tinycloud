/****************************************************************************
** Meta object code from reading C++ file 'mainwindow.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.9.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../mainwindow.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'mainwindow.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 69
#error "This file was generated using the moc from 6.9.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

#ifndef Q_CONSTINIT
#define Q_CONSTINIT
#endif

QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
QT_WARNING_DISABLE_GCC("-Wuseless-cast")
namespace {
struct qt_meta_tag_ZN10MainWindowE_t {};
} // unnamed namespace

template <> constexpr inline auto MainWindow::qt_create_metaobjectdata<qt_meta_tag_ZN10MainWindowE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "MainWindow",
        "minimizeWindow",
        "",
        "toggleFullscreen",
        "closeWindow",
        "init_listWidget",
        "on_uploadButton_clicked",
        "onItemDoubleClicked",
        "QListWidgetItem*",
        "item",
        "addFileItem",
        "fileName",
        "fileSize",
        "is_dir",
        "on_parentdirButton_clicked",
        "on_newdirButton_clicked",
        "showContextMenu",
        "pos",
        "renameFile",
        "filename",
        "deleteFile",
        "deleteDir",
        "dirname",
        "renameDir",
        "downloadFile",
        "filename_remote",
        "on_pushButton_clicked",
        "on_set_ipButton_clicked",
        "on_set_portButton_clicked",
        "on_pushButton_2_clicked"
    };

    QtMocHelpers::UintData qt_methods {
        // Slot 'minimizeWindow'
        QtMocHelpers::SlotData<void()>(1, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'toggleFullscreen'
        QtMocHelpers::SlotData<void()>(3, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'closeWindow'
        QtMocHelpers::SlotData<void()>(4, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'init_listWidget'
        QtMocHelpers::SlotData<void()>(5, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'on_uploadButton_clicked'
        QtMocHelpers::SlotData<void()>(6, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onItemDoubleClicked'
        QtMocHelpers::SlotData<void(QListWidgetItem *)>(7, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { 0x80000000 | 8, 9 },
        }}),
        // Slot 'addFileItem'
        QtMocHelpers::SlotData<void(const QString &, qint64, bool)>(10, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QString, 11 }, { QMetaType::LongLong, 12 }, { QMetaType::Bool, 13 },
        }}),
        // Slot 'on_parentdirButton_clicked'
        QtMocHelpers::SlotData<void()>(14, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'on_newdirButton_clicked'
        QtMocHelpers::SlotData<void()>(15, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'showContextMenu'
        QtMocHelpers::SlotData<void(const QPoint &)>(16, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QPoint, 17 },
        }}),
        // Slot 'renameFile'
        QtMocHelpers::SlotData<void(const QString &)>(18, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QString, 19 },
        }}),
        // Slot 'deleteFile'
        QtMocHelpers::SlotData<void(const QString &)>(20, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QString, 19 },
        }}),
        // Slot 'deleteDir'
        QtMocHelpers::SlotData<void(const QString &)>(21, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QString, 22 },
        }}),
        // Slot 'renameDir'
        QtMocHelpers::SlotData<void(const QString &)>(23, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QString, 22 },
        }}),
        // Slot 'downloadFile'
        QtMocHelpers::SlotData<void(const QString &)>(24, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QString, 25 },
        }}),
        // Slot 'on_pushButton_clicked'
        QtMocHelpers::SlotData<void()>(26, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'on_set_ipButton_clicked'
        QtMocHelpers::SlotData<void()>(27, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'on_set_portButton_clicked'
        QtMocHelpers::SlotData<void()>(28, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'on_pushButton_2_clicked'
        QtMocHelpers::SlotData<void()>(29, 2, QMC::AccessPrivate, QMetaType::Void),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<MainWindow, qt_meta_tag_ZN10MainWindowE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject MainWindow::staticMetaObject = { {
    QMetaObject::SuperData::link<QMainWindow::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN10MainWindowE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN10MainWindowE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN10MainWindowE_t>.metaTypes,
    nullptr
} };

void MainWindow::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<MainWindow *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->minimizeWindow(); break;
        case 1: _t->toggleFullscreen(); break;
        case 2: _t->closeWindow(); break;
        case 3: _t->init_listWidget(); break;
        case 4: _t->on_uploadButton_clicked(); break;
        case 5: _t->onItemDoubleClicked((*reinterpret_cast< std::add_pointer_t<QListWidgetItem*>>(_a[1]))); break;
        case 6: _t->addFileItem((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<qint64>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<bool>>(_a[3]))); break;
        case 7: _t->on_parentdirButton_clicked(); break;
        case 8: _t->on_newdirButton_clicked(); break;
        case 9: _t->showContextMenu((*reinterpret_cast< std::add_pointer_t<QPoint>>(_a[1]))); break;
        case 10: _t->renameFile((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 11: _t->deleteFile((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 12: _t->deleteDir((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 13: _t->renameDir((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 14: _t->downloadFile((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 15: _t->on_pushButton_clicked(); break;
        case 16: _t->on_set_ipButton_clicked(); break;
        case 17: _t->on_set_portButton_clicked(); break;
        case 18: _t->on_pushButton_2_clicked(); break;
        default: ;
        }
    }
}

const QMetaObject *MainWindow::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *MainWindow::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN10MainWindowE_t>.strings))
        return static_cast<void*>(this);
    return QMainWindow::qt_metacast(_clname);
}

int MainWindow::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 19)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 19;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 19)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 19;
    }
    return _id;
}
QT_WARNING_POP
