#include "cliparser.h"

CliParser::CliParser(QObject *parent) : QObject(parent)
{
    SetupOptions();
}

void CliParser::SetupOptions(){

    exportFormatOption = new QCommandLineOption({"f", "format"}, "export format", "[kitti, darknet, gcp, voc, coco]");
    exportOutputFolder = new QCommandLineOption({"o", "output"}, "output folder", "folder path");
    exportInputFile = new QCommandLineOption({"i", "input"}, "input label database", "file path");
    exportValidationSplit = new QCommandLineOption({"s", "split"}, "validation split percentage", "percentage", "20");
    exportNoSubfolders = new QCommandLineOption("no-subfolders", "export directly to specified folder");
    exportFilePrefix = new QCommandLineOption("prefix", "filename prefix", "prefix", "");
    exportNamesFile = new QCommandLineOption({"n", "names"}, "names file", "file path");
    exportGCPBucket = new QCommandLineOption("bucket", "GCP bucket");
    exportPascalVOCLabelMap = new QCommandLineOption("export-map", "export label.pbtxt file");
    exportShuffleImages = new QCommandLineOption("shuffle", "shuffle images when splitting");
    exportAppendLabels = new QCommandLineOption("append-labels", "append to label files");
    exportUnlabelledImages = new QCommandLineOption("export-unlabelled", "export images without labels");

    parser.addHelpOption();
    parser.addVersionOption();
    parser.setOptionsAfterPositionalArgumentsMode(QCommandLineParser::ParseAsOptions);

    parser.addPositionalArgument("mode", "[export]");
    parser.addOption(*exportFormatOption);
    parser.addOption(*exportOutputFolder);
    parser.addOption(*exportInputFile);
    parser.addOption(*exportValidationSplit);
    parser.addOption(*exportNoSubfolders);
    parser.addOption(*exportFilePrefix);
    parser.addOption(*exportNamesFile);
    parser.addOption(*exportGCPBucket);
    parser.addOption(*exportPascalVOCLabelMap);
    parser.addOption(*exportShuffleImages);
    parser.addOption(*exportAppendLabels);
    parser.addOption(*exportUnlabelledImages);
}

bool CliParser::Run(){

    parser.process(*QCoreApplication::instance());

    auto res = handleExport();

    return res;
}

bool CliParser::handleExport(){

    QThread* export_thread = new QThread;
    BaseExporter* exporter = nullptr;

    LabelProject project;
    if(!parser.isSet("input") || parser.value("input") == ""){
        qDebug() << "No input file specified";
        return false;
    }else{
        qDebug() << "Attemting to load: " << parser.value("input");
        project.loadDatabase(parser.value("input"));
    }

    if(parser.value(*exportFormatOption) == "kitti"){
        exporter = new KittiExporter(&project);
    }else if(parser.value(*exportFormatOption) == "darknet"){
        exporter = new DarknetExporter(&project);
        if(parser.isSet(*exportNamesFile)){
            static_cast<DarknetExporter*>(exporter)->generateLabelIds(parser.value(*exportNamesFile));
        }else{
            qDebug() << "No names file specifed.";
            return false;
        }

    }else if(parser.value(*exportFormatOption) == "voc"){
        exporter = new PascalVocExporter(&project);
        if(parser.isSet(*exportPascalVOCLabelMap)){
            static_cast<PascalVocExporter*>(exporter)->setExportMap(true);
        }
        exporter->process();
    }else if(parser.value(*exportFormatOption) == "coco"){
        exporter = new CocoExporter(&project);
    }else if(parser.value(*exportFormatOption) == "gcp"){
        exporter = new GCPExporter(&project);
        if(parser.isSet(*exportGCPBucket)){
            static_cast<GCPExporter*>(exporter)->setBucket(parser.value(*exportGCPBucket));
        }else{
            qDebug() << "No bucket path specifed.";
            return false;
        }

    }else{
        qDebug() << "Invalid exporter type specified";
        return false;
    }

    if(exporter != nullptr){
        exporter->moveToThread(export_thread);

        // No progress bar
        exporter->disableProgress(true);

        if(parser.isSet(*exportFilePrefix)){
            exporter->setFilenamePrefix(parser.value(*exportFilePrefix));
        }

        exporter->setAppendLabels(parser.isSet(*exportAppendLabels));

        if(parser.isSet(*exportValidationSplit)){
            exporter->setValidationSplit(true);
            exporter->splitData(parser.value(*exportValidationSplit).toFloat(), parser.isSet(*exportShuffleImages));
        }else{
            exporter->setValidationSplit(false);
            exporter->splitData(0, parser.isSet(*exportShuffleImages));
        }

        exporter->setExportUnlabelled(parser.isSet(*exportUnlabelledImages));

        if(!parser.isSet(*exportOutputFolder) || parser.value(*exportOutputFolder) == ""){
            qDebug() << "Please specify an output folder";
            return false;
        }else{
            exporter->setOutputFolder(parser.value(*exportOutputFolder));
        }

        exporter->process();

    }else{
        qDebug() << "Failed to instantiate exporter";
        return false;
    }

    return true;
}
