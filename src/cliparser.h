#ifndef CLIPARSER_H
#define CLIPARSER_H

#include <QObject>
#include <QCommandLineParser>
#include <exporter.h>
#include <importer.h>
#include <detection/detectoropencv.h>


class CliParser : public QObject
{
    Q_OBJECT
public:
    explicit CliParser(QObject *parent = nullptr);
    bool Run();

private:
    void SetupOptions();
    bool handleExport();
    bool handleImport();
    bool handleMerge();
    bool handleDetect();

    LabelProject getProject(bool &ok);

    QCommandLineParser parser;

    QCommandLineOption *exportFormatOption;
    QCommandLineOption *exportOutputFolder;
    QCommandLineOption *exportInputFile;
    QCommandLineOption *exportValidationSplit;
    QCommandLineOption *exportNoSubfolders;
    QCommandLineOption *exportFilePrefix;
    QCommandLineOption *exportNamesFile;
    QCommandLineOption *exportGCPBucket;
    QCommandLineOption *exportGCPLocal;
    QCommandLineOption *exportPascalVOCLabelMap;
    QCommandLineOption *exportShuffleImages;
    QCommandLineOption *exportAppendLabels;
    QCommandLineOption *exportUnlabelledImages;

    QCommandLineOption *exportVideoFilename;
    QCommandLineOption *exportVideoFourcc;
    QCommandLineOption *exportVideoFps;
    QCommandLineOption *exportVideoColourmap;
    QCommandLineOption *exportVideoSize;
    QCommandLineOption *exportVideoDisplayNames;
    QCommandLineOption *exportVideoDisplayBoxes;

    QCommandLineOption *importImages;
    QCommandLineOption *importAnnotations;
    QCommandLineOption *importUnlabelledImages;
    QCommandLineOption *importOverwrite;
    QCommandLineOption *importTFRecordMask;

    QCommandLineOption *detectChannels;
    QCommandLineOption *detectTarget;
    QCommandLineOption *detectFramework;
    QCommandLineOption *detectConvertDepth;
    QCommandLineOption *detectGrayToRGB;
    QCommandLineOption *detectNMSThresh;
    QCommandLineOption *detectConfThresh;
    QCommandLineOption *detectConfig;
    QCommandLineOption *detectWeights;

    QCommandLineOption *configSilence;


signals:

};

#endif // CLIPARSER_H
