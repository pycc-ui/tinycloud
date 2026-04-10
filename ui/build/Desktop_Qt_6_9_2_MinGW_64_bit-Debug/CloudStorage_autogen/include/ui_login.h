/********************************************************************************
** Form generated from reading UI file 'login.ui'
**
** Created by: Qt User Interface Compiler version 6.9.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_LOGIN_H
#define UI_LOGIN_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_login
{
public:
    QLabel *label_2;
    QWidget *verticalLayoutWidget;
    QVBoxLayout *verticalLayout;
    QLineEdit *nameEdit;
    QLineEdit *pswEdit;
    QLabel *promptLabel;
    QPushButton *pushButton;
    QPushButton *pushButton_2;
    QLabel *label;

    void setupUi(QWidget *login)
    {
        if (login->objectName().isEmpty())
            login->setObjectName("login");
        login->resize(400, 300);
        label_2 = new QLabel(login);
        label_2->setObjectName("label_2");
        label_2->setGeometry(QRect(80, 110, 24, 28));
        verticalLayoutWidget = new QWidget(login);
        verticalLayoutWidget->setObjectName("verticalLayoutWidget");
        verticalLayoutWidget->setGeometry(QRect(120, 90, 160, 130));
        verticalLayout = new QVBoxLayout(verticalLayoutWidget);
        verticalLayout->setObjectName("verticalLayout");
        verticalLayout->setContentsMargins(0, 0, 0, 0);
        nameEdit = new QLineEdit(verticalLayoutWidget);
        nameEdit->setObjectName("nameEdit");

        verticalLayout->addWidget(nameEdit);

        pswEdit = new QLineEdit(verticalLayoutWidget);
        pswEdit->setObjectName("pswEdit");

        verticalLayout->addWidget(pswEdit);

        promptLabel = new QLabel(verticalLayoutWidget);
        promptLabel->setObjectName("promptLabel");
        promptLabel->setEnabled(false);

        verticalLayout->addWidget(promptLabel);

        pushButton = new QPushButton(verticalLayoutWidget);
        pushButton->setObjectName("pushButton");

        verticalLayout->addWidget(pushButton);

        pushButton_2 = new QPushButton(verticalLayoutWidget);
        pushButton_2->setObjectName("pushButton_2");

        verticalLayout->addWidget(pushButton_2);

        label = new QLabel(login);
        label->setObjectName("label");
        label->setGeometry(QRect(80, 90, 31, 21));

        retranslateUi(login);

        QMetaObject::connectSlotsByName(login);
    } // setupUi

    void retranslateUi(QWidget *login)
    {
        login->setWindowTitle(QCoreApplication::translate("login", "Form", nullptr));
        label_2->setText(QCoreApplication::translate("login", "\345\257\206\347\240\201", nullptr));
        nameEdit->setText(QCoreApplication::translate("login", "111", nullptr));
        pswEdit->setText(QCoreApplication::translate("login", "111", nullptr));
        promptLabel->setText(QString());
        pushButton->setText(QCoreApplication::translate("login", "\347\231\273\345\275\225", nullptr));
        pushButton_2->setText(QCoreApplication::translate("login", "\346\263\250\345\206\214", nullptr));
        label->setText(QCoreApplication::translate("login", "\350\264\246\345\217\267", nullptr));
    } // retranslateUi

};

namespace Ui {
    class login: public Ui_login {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_LOGIN_H
