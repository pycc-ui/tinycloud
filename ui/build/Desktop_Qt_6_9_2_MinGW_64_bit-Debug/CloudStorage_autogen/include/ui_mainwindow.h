/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 6.9.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QIcon>
#include <QtWidgets/QApplication>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QListView>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QStackedWidget>
#include <QtWidgets/QToolButton>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QAction *actioncontacts;
    QWidget *centralwidget;
    QGridLayout *gridLayout;
    QWidget *widget_2;
    QHBoxLayout *horizontalLayout;
    QSpacerItem *horizontalSpacer_2;
    QToolButton *minimizeButton;
    QToolButton *fullscreenButton;
    QToolButton *closeButton;
    QWidget *widget;
    QVBoxLayout *verticalLayout;
    QToolButton *user_toolButton;
    QToolButton *file_toolButton;
    QSpacerItem *verticalSpacer;
    QToolButton *setting_toolButton;
    QStackedWidget *stackedWidget;
    QWidget *userwindow;
    QWidget *layoutWidget;
    QFormLayout *formLayout_2;
    QLabel *label_4;
    QLineEdit *nameEdit;
    QPushButton *pushButton_2;
    QWidget *filewindow;
    QPushButton *uploadButton;
    QListView *listView;
    QListWidget *listWidget;
    QPushButton *parentdirButton;
    QPushButton *newdirButton;
    QWidget *settingwindow;
    QWidget *layoutWidget1;
    QFormLayout *formLayout;
    QLabel *label;
    QLineEdit *ip_LineEdit;
    QPushButton *set_ipButton;
    QLabel *label_3;
    QLineEdit *port_LineEdit;
    QPushButton *set_portButton;
    QLabel *label_2;
    QLineEdit *downloadPath_LineEdit;
    QPushButton *pushButton;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName("MainWindow");
        MainWindow->resize(840, 640);
        QSizePolicy sizePolicy(QSizePolicy::Policy::Preferred, QSizePolicy::Policy::Preferred);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(MainWindow->sizePolicy().hasHeightForWidth());
        MainWindow->setSizePolicy(sizePolicy);
        MainWindow->setMinimumSize(QSize(840, 640));
        QFont font;
        font.setPointSize(20);
        MainWindow->setFont(font);
        MainWindow->setCursor(QCursor(Qt::CursorShape::ArrowCursor));
        MainWindow->setStyleSheet(QString::fromUtf8("QMainWindow{\n"
"  background:  rgb(245, 245, 245);\n"
"}\n"
"\n"
"QWidget#centralwidget {\n"
"    background-color: white;\n"
"    border-radius: 8px;\n"
"    border: 1px solid rgb(170,170,170);\n"
"}"));
        actioncontacts = new QAction(MainWindow);
        actioncontacts->setObjectName("actioncontacts");
        QIcon icon(QIcon::fromTheme(QIcon::ThemeIcon::AddressBookNew));
        actioncontacts->setIcon(icon);
        actioncontacts->setMenuRole(QAction::MenuRole::NoRole);
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName("centralwidget");
        centralwidget->setMinimumSize(QSize(0, 0));
        centralwidget->setStyleSheet(QString::fromUtf8(""));
        gridLayout = new QGridLayout(centralwidget);
        gridLayout->setSpacing(0);
        gridLayout->setObjectName("gridLayout");
        gridLayout->setContentsMargins(0, 0, 0, 0);
        widget_2 = new QWidget(centralwidget);
        widget_2->setObjectName("widget_2");
        widget_2->setMinimumSize(QSize(0, 30));
        widget_2->setMaximumSize(QSize(16777215, 30));
        widget_2->setStyleSheet(QString::fromUtf8("\n"
"QToolButton{\n"
"	background-color: rgba( 0, 0, 0,0);\n"
"	border: none;\n"
"   min-width: 30px;\n"
"    min-height: 30px;\n"
"    max-width: 30px;\n"
"    max-height: 30px;\n"
"\n"
"}\n"
"QToolButton#closeButton{\n"
"	border-top-right-radius: 8px;\n"
"	\n"
"}\n"
"QToolButton#closeButton::hover{\n"
"	background-color: rgb(255, 0, 0);\n"
"	\n"
"}\n"
"QToolButton#fullscreenButton::hover{\n"
"	background-color: rgb(217, 217, 217);\n"
"}\n"
"QToolButton#minimizeButton::hover{\n"
"	background-color: rgb(217, 217, 217);\n"
"}\n"
""));
        horizontalLayout = new QHBoxLayout(widget_2);
        horizontalLayout->setSpacing(0);
        horizontalLayout->setObjectName("horizontalLayout");
        horizontalLayout->setContentsMargins(0, 0, 0, 0);
        horizontalSpacer_2 = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        horizontalLayout->addItem(horizontalSpacer_2);

        minimizeButton = new QToolButton(widget_2);
        minimizeButton->setObjectName("minimizeButton");
        QIcon icon1;
        icon1.addFile(QString::fromUtf8(":/img/minus.svg"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        minimizeButton->setIcon(icon1);
        minimizeButton->setIconSize(QSize(15, 15));

        horizontalLayout->addWidget(minimizeButton);

        fullscreenButton = new QToolButton(widget_2);
        fullscreenButton->setObjectName("fullscreenButton");
        QIcon icon2;
        icon2.addFile(QString::fromUtf8(":/img/full-screen.svg"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        icon2.addFile(QString::fromUtf8(":/img/oExit-fullscreen.svg"), QSize(), QIcon::Mode::Normal, QIcon::State::On);
        fullscreenButton->setIcon(icon2);
        fullscreenButton->setIconSize(QSize(15, 15));

        horizontalLayout->addWidget(fullscreenButton);

        closeButton = new QToolButton(widget_2);
        closeButton->setObjectName("closeButton");
        closeButton->setMinimumSize(QSize(30, 30));
        QIcon icon3;
        icon3.addFile(QString::fromUtf8(":/img/close.svg"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        closeButton->setIcon(icon3);
        closeButton->setIconSize(QSize(15, 15));

        horizontalLayout->addWidget(closeButton);


        gridLayout->addWidget(widget_2, 0, 1, 1, 1);

        widget = new QWidget(centralwidget);
        widget->setObjectName("widget");
        widget->setMinimumSize(QSize(60, 0));
        widget->setMaximumSize(QSize(60, 16777215));
        QFont font1;
        font1.setPointSize(10);
        font1.setStrikeOut(false);
        widget->setFont(font1);
        widget->setStyleSheet(QString::fromUtf8("QWidget{\n"
"    border-right: 1px solid rgb(217, 217, 217);\n"
"}\n"
"\n"
"QToolButton{\n"
"    background-color: rgba(0, 0, 0, 0);\n"
"    border: none;\n"
"    min-width: 40px;\n"
"    min-height: 40px;\n"
"    max-width: 40px;\n"
"    max-height: 40px;\n"
"    border-radius: 8px; /* \346\267\273\345\212\240\345\234\206\350\247\222 */\n"
"}\n"
"\n"
"QToolButton:hover{\n"
"    background-color: rgb(217, 217, 217);\n"
"    border-radius: 8px; /* \346\202\254\345\201\234\346\227\266\344\271\237\344\277\235\346\214\201\345\234\206\350\247\222 */\n"
"}\n"
"\n"
"QToolButton:pressed{\n"
"    background-color: rgb(180, 180, 180); /* \346\267\273\345\212\240\346\214\211\344\270\213\346\225\210\346\236\234 */\n"
"    border-radius: 8px;\n"
"}\n"
"QToolButton:checked{\n"
"    border-radius: 8px;\n"
"    \n"
"}"));
        verticalLayout = new QVBoxLayout(widget);
        verticalLayout->setSpacing(16);
        verticalLayout->setObjectName("verticalLayout");
        user_toolButton = new QToolButton(widget);
        user_toolButton->setObjectName("user_toolButton");
        user_toolButton->setMinimumSize(QSize(40, 40));
        user_toolButton->setMaximumSize(QSize(40, 40));
        QIcon icon4;
        icon4.addFile(QString::fromUtf8(":/img/user.svg"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        icon4.addFile(QString::fromUtf8(":/img/user-in.svg"), QSize(), QIcon::Mode::Normal, QIcon::State::On);
        user_toolButton->setIcon(icon4);
        user_toolButton->setIconSize(QSize(25, 25));

        verticalLayout->addWidget(user_toolButton);

        file_toolButton = new QToolButton(widget);
        file_toolButton->setObjectName("file_toolButton");
        file_toolButton->setMinimumSize(QSize(40, 40));
        file_toolButton->setMaximumSize(QSize(40, 40));
        QIcon icon5;
        icon5.addFile(QString::fromUtf8(":/img/file.svg"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        icon5.addFile(QString::fromUtf8(":/img/file-in.svg"), QSize(), QIcon::Mode::Normal, QIcon::State::On);
        file_toolButton->setIcon(icon5);
        file_toolButton->setIconSize(QSize(25, 25));

        verticalLayout->addWidget(file_toolButton);

        verticalSpacer = new QSpacerItem(50, 30, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);

        verticalLayout->addItem(verticalSpacer);

        setting_toolButton = new QToolButton(widget);
        setting_toolButton->setObjectName("setting_toolButton");
        setting_toolButton->setMinimumSize(QSize(40, 40));
        setting_toolButton->setMaximumSize(QSize(40, 40));
        QIcon icon6;
        icon6.addFile(QString::fromUtf8(":/img/setting.svg"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        icon6.addFile(QString::fromUtf8(":/img/setting-in.svg"), QSize(), QIcon::Mode::Normal, QIcon::State::On);
        setting_toolButton->setIcon(icon6);
        setting_toolButton->setIconSize(QSize(25, 25));

        verticalLayout->addWidget(setting_toolButton);


        gridLayout->addWidget(widget, 0, 0, 2, 1);

        stackedWidget = new QStackedWidget(centralwidget);
        stackedWidget->setObjectName("stackedWidget");
        stackedWidget->setEnabled(true);
        stackedWidget->setMinimumSize(QSize(740, 540));
        stackedWidget->setStyleSheet(QString::fromUtf8("QWidget\n"
"{\n"
" border-radius: 0px;\n"
"background-color: rgba( 0, 0, 0,0);\n"
"}\n"
"\n"
"QWidget#stackedWidget\n"
"{\n"
"	background-color: rgba( 0, 0, 0,0);\n"
"    min-width: 740px;\n"
"    min-height: 540px;\n"
"    max-width: 65535px;\n"
"    max-height: 65535px;\n"
"}\n"
""));
        userwindow = new QWidget();
        userwindow->setObjectName("userwindow");
        userwindow->setToolTipDuration(0);
        userwindow->setStyleSheet(QString::fromUtf8("QLineEdit\n"
"{\n"
"background-color:rgb(234, 234, 234)\n"
"}\n"
"QPushButton{\n"
"    background-color: rgba(0, 0, 0, 0);\n"
"    border: none;\n"
"    min-width: 75px;\n"
"    min-height: 25px;\n"
"    max-width: 75px;\n"
"    max-height: 25px;\n"
"    border-radius: 8px; /* \346\267\273\345\212\240\345\234\206\350\247\222 */\n"
"}\n"
"\n"
"QPushButton:hover{\n"
"    background-color: rgb(217, 217, 217);\n"
"    border-radius: 8px; /* \346\202\254\345\201\234\346\227\266\344\271\237\344\277\235\346\214\201\345\234\206\350\247\222 */\n"
"}\n"
"\n"
"QPushButton:pressed{\n"
"    background-color: rgb(180, 180, 180); /* \346\267\273\345\212\240\346\214\211\344\270\213\346\225\210\346\236\234 */\n"
"    border-radius: 8px;\n"
"}\n"
"QPushButton:checked{\n"
"    border-radius: 8px;\n"
"    \n"
"}"));
        layoutWidget = new QWidget(userwindow);
        layoutWidget->setObjectName("layoutWidget");
        layoutWidget->setGeometry(QRect(140, 220, 291, 151));
        formLayout_2 = new QFormLayout(layoutWidget);
        formLayout_2->setObjectName("formLayout_2");
        formLayout_2->setContentsMargins(0, 0, 0, 0);
        label_4 = new QLabel(layoutWidget);
        label_4->setObjectName("label_4");

        formLayout_2->setWidget(0, QFormLayout::ItemRole::LabelRole, label_4);

        nameEdit = new QLineEdit(layoutWidget);
        nameEdit->setObjectName("nameEdit");

        formLayout_2->setWidget(0, QFormLayout::ItemRole::FieldRole, nameEdit);

        pushButton_2 = new QPushButton(layoutWidget);
        pushButton_2->setObjectName("pushButton_2");

        formLayout_2->setWidget(1, QFormLayout::ItemRole::FieldRole, pushButton_2);

        stackedWidget->addWidget(userwindow);
        filewindow = new QWidget();
        filewindow->setObjectName("filewindow");
        filewindow->setToolTipDuration(3);
        uploadButton = new QPushButton(filewindow);
        uploadButton->setObjectName("uploadButton");
        uploadButton->setGeometry(QRect(650, 550, 75, 25));
        uploadButton->setStyleSheet(QString::fromUtf8("QPushButton{\n"
"    background-color: rgba(0, 0, 0, 0);\n"
"    border: none;\n"
"    min-width: 75px;\n"
"    min-height: 25px;\n"
"    max-width: 75px;\n"
"    max-height: 25px;\n"
"    border-radius: 8px; /* \346\267\273\345\212\240\345\234\206\350\247\222 */\n"
"}\n"
"\n"
"QPushButton:hover{\n"
"    background-color: rgb(217, 217, 217);\n"
"    border-radius: 8px; /* \346\202\254\345\201\234\346\227\266\344\271\237\344\277\235\346\214\201\345\234\206\350\247\222 */\n"
"}\n"
"\n"
"QPushButton:pressed{\n"
"    background-color: rgb(180, 180, 180); /* \346\267\273\345\212\240\346\214\211\344\270\213\346\225\210\346\236\234 */\n"
"    border-radius: 8px;\n"
"}\n"
"QPushButton:checked{\n"
"    border-radius: 8px;\n"
"    \n"
"}"));
        listView = new QListView(filewindow);
        listView->setObjectName("listView");
        listView->setGeometry(QRect(100, 100, 281, 211));
        listWidget = new QListWidget(filewindow);
        listWidget->setObjectName("listWidget");
        listWidget->setGeometry(QRect(30, 50, 701, 491));
        listWidget->setStyleSheet(QString::fromUtf8("QListWidget#listWidget\n"
"{\n"
"background-color:rgb(240, 240, 240)\n"
"}"));
        parentdirButton = new QPushButton(filewindow);
        parentdirButton->setObjectName("parentdirButton");
        parentdirButton->setGeometry(QRect(30, 10, 75, 25));
        parentdirButton->setStyleSheet(QString::fromUtf8("QPushButton{\n"
"    background-color: rgba(0, 0, 0, 0);\n"
"    border: none;\n"
"    min-width: 75px;\n"
"    min-height: 25px;\n"
"    max-width: 75px;\n"
"    max-height: 25px;\n"
"    border-radius: 8px; /* \346\267\273\345\212\240\345\234\206\350\247\222 */\n"
"}\n"
"\n"
"QPushButton:hover{\n"
"    background-color: rgb(217, 217, 217);\n"
"    border-radius: 8px; /* \346\202\254\345\201\234\346\227\266\344\271\237\344\277\235\346\214\201\345\234\206\350\247\222 */\n"
"}\n"
"\n"
"QPushButton:pressed{\n"
"    background-color: rgb(180, 180, 180); /* \346\267\273\345\212\240\346\214\211\344\270\213\346\225\210\346\236\234 */\n"
"    border-radius: 8px;\n"
"}\n"
"QPushButton:checked{\n"
"    border-radius: 8px;\n"
"    \n"
"}"));
        newdirButton = new QPushButton(filewindow);
        newdirButton->setObjectName("newdirButton");
        newdirButton->setGeometry(QRect(610, 10, 75, 25));
        newdirButton->setStyleSheet(QString::fromUtf8("QPushButton{\n"
"    background-color: rgba(0, 0, 0, 0);\n"
"    border: none;\n"
"    min-width: 75px;\n"
"    min-height: 25px;\n"
"    max-width: 75px;\n"
"    max-height: 25px;\n"
"    border-radius: 8px; /* \346\267\273\345\212\240\345\234\206\350\247\222 */\n"
"}\n"
"\n"
"QPushButton:hover{\n"
"    background-color: rgb(217, 217, 217);\n"
"    border-radius: 8px; /* \346\202\254\345\201\234\346\227\266\344\271\237\344\277\235\346\214\201\345\234\206\350\247\222 */\n"
"}\n"
"\n"
"QPushButton:pressed{\n"
"    background-color: rgb(180, 180, 180); /* \346\267\273\345\212\240\346\214\211\344\270\213\346\225\210\346\236\234 */\n"
"    border-radius: 8px;\n"
"}\n"
"QPushButton:checked{\n"
"    border-radius: 8px;\n"
"    \n"
"}"));
        stackedWidget->addWidget(filewindow);
        settingwindow = new QWidget();
        settingwindow->setObjectName("settingwindow");
        settingwindow->setToolTipDuration(6);
        settingwindow->setStyleSheet(QString::fromUtf8("QLineEdit\n"
"{\n"
"background-color:rgb(234, 234, 234)\n"
"}QPushButton{\n"
"    background-color: rgba(0, 0, 0, 0);\n"
"    border: none;\n"
"    min-width: 75px;\n"
"    min-height: 25px;\n"
"    max-width: 200px;\n"
"    max-height: 25px;\n"
"    border-radius: 8px; /* \346\267\273\345\212\240\345\234\206\350\247\222 */\n"
"}\n"
"\n"
"QPushButton:hover{\n"
"    background-color: rgb(217, 217, 217);\n"
"    border-radius: 8px; /* \346\202\254\345\201\234\346\227\266\344\271\237\344\277\235\346\214\201\345\234\206\350\247\222 */\n"
"}\n"
"\n"
"QPushButton:pressed{\n"
"    background-color: rgb(180, 180, 180); /* \346\267\273\345\212\240\346\214\211\344\270\213\346\225\210\346\236\234 */\n"
"    border-radius: 8px;\n"
"}\n"
"QPushButton:checked{\n"
"    border-radius: 8px;\n"
"    \n"
"}"));
        layoutWidget1 = new QWidget(settingwindow);
        layoutWidget1->setObjectName("layoutWidget1");
        layoutWidget1->setGeometry(QRect(110, 100, 561, 381));
        formLayout = new QFormLayout(layoutWidget1);
        formLayout->setObjectName("formLayout");
        formLayout->setContentsMargins(0, 0, 0, 0);
        label = new QLabel(layoutWidget1);
        label->setObjectName("label");

        formLayout->setWidget(0, QFormLayout::ItemRole::LabelRole, label);

        ip_LineEdit = new QLineEdit(layoutWidget1);
        ip_LineEdit->setObjectName("ip_LineEdit");
        ip_LineEdit->setEnabled(false);

        formLayout->setWidget(0, QFormLayout::ItemRole::FieldRole, ip_LineEdit);

        set_ipButton = new QPushButton(layoutWidget1);
        set_ipButton->setObjectName("set_ipButton");

        formLayout->setWidget(1, QFormLayout::ItemRole::FieldRole, set_ipButton);

        label_3 = new QLabel(layoutWidget1);
        label_3->setObjectName("label_3");

        formLayout->setWidget(2, QFormLayout::ItemRole::LabelRole, label_3);

        port_LineEdit = new QLineEdit(layoutWidget1);
        port_LineEdit->setObjectName("port_LineEdit");
        port_LineEdit->setEnabled(false);

        formLayout->setWidget(2, QFormLayout::ItemRole::FieldRole, port_LineEdit);

        set_portButton = new QPushButton(layoutWidget1);
        set_portButton->setObjectName("set_portButton");

        formLayout->setWidget(3, QFormLayout::ItemRole::FieldRole, set_portButton);

        label_2 = new QLabel(layoutWidget1);
        label_2->setObjectName("label_2");

        formLayout->setWidget(4, QFormLayout::ItemRole::LabelRole, label_2);

        downloadPath_LineEdit = new QLineEdit(layoutWidget1);
        downloadPath_LineEdit->setObjectName("downloadPath_LineEdit");
        downloadPath_LineEdit->setEnabled(false);

        formLayout->setWidget(4, QFormLayout::ItemRole::FieldRole, downloadPath_LineEdit);

        pushButton = new QPushButton(layoutWidget1);
        pushButton->setObjectName("pushButton");

        formLayout->setWidget(5, QFormLayout::ItemRole::FieldRole, pushButton);

        stackedWidget->addWidget(settingwindow);

        gridLayout->addWidget(stackedWidget, 1, 1, 1, 1);

        MainWindow->setCentralWidget(centralwidget);

        retranslateUi(MainWindow);

        stackedWidget->setCurrentIndex(1);


        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "MainWindow", nullptr));
        actioncontacts->setText(QCoreApplication::translate("MainWindow", "contacts", nullptr));
        minimizeButton->setText(QString());
        fullscreenButton->setText(QString());
        closeButton->setText(QString());
        user_toolButton->setText(QString());
        file_toolButton->setText(QString());
        setting_toolButton->setText(QString());
        label_4->setText(QCoreApplication::translate("MainWindow", "username", nullptr));
        pushButton_2->setText(QCoreApplication::translate("MainWindow", "\344\277\256\346\224\271\345\257\206\347\240\201", nullptr));
        uploadButton->setText(QCoreApplication::translate("MainWindow", "\344\270\212\344\274\240", nullptr));
        parentdirButton->setText(QCoreApplication::translate("MainWindow", "..", nullptr));
        newdirButton->setText(QCoreApplication::translate("MainWindow", "\346\226\260\345\273\272\346\226\207\344\273\266\345\244\271", nullptr));
        label->setText(QCoreApplication::translate("MainWindow", "ip", nullptr));
        set_ipButton->setText(QCoreApplication::translate("MainWindow", "\350\256\276\345\256\232\346\234\215\345\212\241\345\231\250IP", nullptr));
        label_3->setText(QCoreApplication::translate("MainWindow", "port", nullptr));
        set_portButton->setText(QCoreApplication::translate("MainWindow", "\350\256\276\345\256\232\346\234\215\345\212\241\345\231\250\347\253\257\345\217\243", nullptr));
        label_2->setText(QCoreApplication::translate("MainWindow", "downloadpath", nullptr));
        pushButton->setText(QCoreApplication::translate("MainWindow", "\351\200\211\346\213\251\344\270\213\350\275\275\346\226\207\344\273\266\345\244\271", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
