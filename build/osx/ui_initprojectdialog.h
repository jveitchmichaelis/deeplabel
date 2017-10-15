/********************************************************************************
** Form generated from reading UI file 'initprojectdialog.ui'
**
** Created by: Qt User Interface Compiler version 5.9.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_INITPROJECTDIALOG_H
#define UI_INITPROJECTDIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QDialog>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QPushButton>

QT_BEGIN_NAMESPACE

class Ui_InitProjectDialog
{
public:
    QGridLayout *gridLayout;
    QPushButton *newProjectPushButton;
    QPushButton *openProjectPushButton;

    void setupUi(QDialog *InitProjectDialog)
    {
        if (InitProjectDialog->objectName().isEmpty())
            InitProjectDialog->setObjectName(QStringLiteral("InitProjectDialog"));
        InitProjectDialog->resize(282, 78);
        gridLayout = new QGridLayout(InitProjectDialog);
        gridLayout->setObjectName(QStringLiteral("gridLayout"));
        newProjectPushButton = new QPushButton(InitProjectDialog);
        newProjectPushButton->setObjectName(QStringLiteral("newProjectPushButton"));

        gridLayout->addWidget(newProjectPushButton, 0, 0, 1, 1);

        openProjectPushButton = new QPushButton(InitProjectDialog);
        openProjectPushButton->setObjectName(QStringLiteral("openProjectPushButton"));

        gridLayout->addWidget(openProjectPushButton, 1, 0, 1, 1);


        retranslateUi(InitProjectDialog);

        QMetaObject::connectSlotsByName(InitProjectDialog);
    } // setupUi

    void retranslateUi(QDialog *InitProjectDialog)
    {
        InitProjectDialog->setWindowTitle(QApplication::translate("InitProjectDialog", "Dialog", Q_NULLPTR));
        newProjectPushButton->setText(QApplication::translate("InitProjectDialog", "New Project", Q_NULLPTR));
        openProjectPushButton->setText(QApplication::translate("InitProjectDialog", "Open Existing Project", Q_NULLPTR));
    } // retranslateUi

};

namespace Ui {
    class InitProjectDialog: public Ui_InitProjectDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_INITPROJECTDIALOG_H
