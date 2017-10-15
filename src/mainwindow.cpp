#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(ui->actionNew_Project, SIGNAL(triggered(bool)), this, SLOT(newProject()));
    connect(ui->actionOpen_Project, SIGNAL(triggered(bool)), this, SLOT(openProject()));

    project = new LabelProject(this);
}

void MainWindow::openProject()
{
    QString openDir = QStandardPaths::displayName(QStandardPaths::DesktopLocation);
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open Project"),
                                                    openDir,
                                                    tr("Label database (*.lbldb)"));

    if(fileName != ""){
        project->loadDatabase(fileName);
    }

    return;
}

void MainWindow::newProject()
{
    QString openDir = QStandardPaths::displayName(QStandardPaths::DesktopLocation);
    QString fileName = QFileDialog::getSaveFileName(this, tr("New Project"),
                                                    openDir,
                                                    tr("Label database (*.lbldb)"));

    if(fileName != ""){
        project->createDatabase(fileName);
    }

    return;
}

MainWindow::~MainWindow()
{
    delete ui;
}
