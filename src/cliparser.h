#ifndef CLIPARSER_H
#define CLIPARSER_H

#include <QObject>
#include <QCommandLineParser>
#include <exporter.h>
#include <baseimporter.h>

class CliParser : public QObject
{
    Q_OBJECT
public:
    explicit CliParser(QObject *parent = nullptr);
    bool Run();

private:
    void SetupOptions();
    bool handleExport();

    QCommandLineParser parser;

    QCommandLineOption *exportFormatOption;
    QCommandLineOption *exportOutputFolder;
    QCommandLineOption *exportInputFile;
    QCommandLineOption *exportValidationSplit;
    QCommandLineOption *exportNoSubfolders;
    QCommandLineOption *exportFilePrefix;
    QCommandLineOption *exportNamesFile;
    QCommandLineOption *exportGCPBucket;
    QCommandLineOption *exportPascalVOCLabelMap;
    QCommandLineOption *exportShuffleImages;
    QCommandLineOption *exportAppendLabels;
    QCommandLineOption *exportUnlabelledImages;

signals:

};

#endif // CLIPARSER_H
