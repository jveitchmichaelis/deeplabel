#include "mainwindow.h"
#include "cliparser.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QCoreApplication::setApplicationName("deeplabel");
    QCoreApplication::setApplicationVersion("0.15");

    QApplication a(argc, argv);

    if(argc == 1){
        MainWindow w;
        w.show();
        return a.exec();
    }else{
        CliParser cli;
        auto res = cli.Run();
        if(!res){
            return 1;
        }
        return 0;
    }
}
