#include "mainwindow.h"
#include "cliparser.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QCoreApplication::setApplicationName("deeplabel");
    QCoreApplication::setApplicationVersion("0.15");

    if(argc == 1){
        QApplication a(argc, argv);
        MainWindow w;
        w.show();
        return a.exec();
    }else{
        QCoreApplication a(argc, argv, 0);
        CliParser cli;
        auto res = cli.Run();
        if(!res){
            return 1;
        }
        return 0;
    }
}
