#include "cliparser.h"

CliParser::CliParser(QObject *parent) : QObject(parent)
{
    SetupOptions();
}

void CliParser::SetupOptions(){

    exportFormatOption = new QCommandLineOption({"f", "format"}, "export format", "[kitti, darknet, gcp, voc, coco, mot, birdsai, tfrecord]");
    exportOutputFolder = new QCommandLineOption({"o", "output"}, "output folder", "folder path");
    exportInputFile = new QCommandLineOption({"i", "input"}, "label database", "file path");
    exportValidationSplit = new QCommandLineOption({"s", "split"}, "validation split percentage", "percentage", "20");
    exportNoSubfolders = new QCommandLineOption("no-subfolders", "export directly to specified folder");
    exportFilePrefix = new QCommandLineOption("prefix", "filename prefix", "prefix", "");
    exportNamesFile = new QCommandLineOption({"n", "names"}, "names file", "file path");
    exportGCPBucket = new QCommandLineOption("bucket", "GCP bucket");
    exportGCPLocal = new QCommandLineOption("local", "use local paths for GCP export");
    exportPascalVOCLabelMap = new QCommandLineOption("export-map", "export label.pbtxt file");
    exportShuffleImages = new QCommandLineOption("shuffle", "shuffle images when splitting");
    exportAppendLabels = new QCommandLineOption("append-labels", "append to label files");
    exportUnlabelledImages = new QCommandLineOption("export-unlabelled", "export images without labels");
    importImages = new QCommandLineOption("images", "import image path/folder", "images");
    importTFRecordMask = new QCommandLineOption("records", "mask for TF Records (* wildcard)", "images");
    importAnnotations = new QCommandLineOption("annotations", "import annotation path/folder", "annotations");
    importUnlabelledImages = new QCommandLineOption("import-unlabelled", "import images without labels");
    importOverwrite = new QCommandLineOption("overwrite", "overwrite existing databases");

    parser.addHelpOption();
    parser.addVersionOption();
    parser.setOptionsAfterPositionalArgumentsMode(QCommandLineParser::ParseAsOptions);

    parser.addPositionalArgument("mode", "[export, import]");
    parser.addOption(*exportFormatOption);
    parser.addOption(*exportOutputFolder);
    parser.addOption(*exportInputFile);
    parser.addOption(*exportValidationSplit);
    parser.addOption(*exportNoSubfolders);
    parser.addOption(*exportFilePrefix);
    parser.addOption(*exportNamesFile);
    parser.addOption(*exportGCPBucket);
    parser.addOption(*exportGCPLocal);
    parser.addOption(*exportPascalVOCLabelMap);
    parser.addOption(*exportShuffleImages);
    parser.addOption(*exportAppendLabels);
    parser.addOption(*exportUnlabelledImages);
    parser.addOption(*importImages);
    parser.addOption(*importAnnotations);
    parser.addOption(*importUnlabelledImages);
    parser.addOption(*importOverwrite);
    parser.addOption(*importTFRecordMask);

}

bool CliParser::Run(){

    parser.process(*QCoreApplication::instance());

    bool res = false;
    auto mode = parser.positionalArguments().at(0);

    for(auto &option : parser.unknownOptionNames()){
        qCritical() << "Unknown option: " << option;
    }

    if(mode == "export"){
        res = handleExport();
    }else if(mode == "import"){
        res = handleImport();
    }

    return res;
}

bool CliParser::handleImport(){

    QString database = parser.value("input");

    LabelProject project;
    if(QFileInfo(database).exists()){
        if(!parser.isSet("overwrite")){
            qCritical() << "Database exists, will not ovewrite.";
            return false;
        }
    }else{
        project.createDatabase(database);
    }

    project.loadDatabase(database);

    QThread* import_thread = new QThread;

    bool import_unlabelled = parser.isSet(*importUnlabelledImages);

    if(parser.value("format") == "darknet"){

        if(!QFileInfo(parser.value("images")).exists()){
            qCritical() << "Image list doesn't exist";
            return false;
        }

        if(!QFileInfo(parser.value("names")).exists()){
            qCritical() << "Names file doesn't exist";
            return false;
        }

        DarknetImporter importer(&project);
        importer.moveToThread(import_thread);
        importer.setImportUnlabelled(import_unlabelled);
        importer.import(parser.value("images"), parser.value("names"));
    }else if(parser.value("format") == "coco"){

        if(!QFileInfo(parser.value("annotations")).exists()){
            qCritical() << "Image sequence folder doesn't exist";
            return false;
        }

        CocoImporter importer(&project);
        importer.moveToThread(import_thread);
        importer.setImportUnlabelled(import_unlabelled);
        importer.import(parser.value("annotations"), parser.value("images"));
    }else if(parser.value("format") == "mot"){

        if(!QDir(parser.value("images")).exists()){
            qCritical() << "Image sequence folder doesn't exist";
            return false;
        }

        if(!QFileInfo(parser.value("names")).exists()){
            qCritical() << "Names file doesn't exist";
            return false;
        }

        MOTImporter importer(&project);
        importer.moveToThread(import_thread);
        importer.setImportUnlabelled(import_unlabelled);
        importer.loadClasses(parser.value("names"));
        importer.import(parser.value("images"));
    }else if(parser.value("format") == "birdsai"){

        if(!QDir(parser.value("annotations")).exists()){
            qCritical() << "Annotations folder doesn't exist";
            return false;
        }

        if(!QDir(parser.value("images")).exists()){
            qCritical() << "Image sequence folder doesn't exist";
            return false;
        }

        BirdsAIImporter importer(&project);
        importer.moveToThread(import_thread);
        importer.setImportUnlabelled(import_unlabelled);
        importer.loadClasses(parser.value("names"));
        importer.import(parser.value("images"),
                        parser.value("annotations"));
    }else if(parser.value("format") == "tfrecord"){

        TFRecordImporter importer(&project);
        importer.moveToThread(import_thread);
        importer.setImportUnlabelled(import_unlabelled);

        importer.import_records(parser.value("records"));
    }

    return true;
}

bool CliParser::handleExport(){

    QThread* export_thread = new QThread;
    BaseExporter* exporter = nullptr;

    LabelProject project;
    QString database = parser.value("input");

    if(!QFileInfo(database).exists()){
        qCritical() << "Database file does not exist.";
        return false;
    }

    if(!parser.isSet("input") || parser.value("input") == ""){
        qCritical() << "No input file specified";
        return false;
    }else{
        qInfo() << "Attemting to load: " << parser.value("input");
        project.loadDatabase(parser.value("input"));
    }

    if(parser.value(*exportFormatOption) == "kitti"){
        exporter = new KittiExporter(&project);
    }else if(parser.value(*exportFormatOption) == "darknet"){
        exporter = new DarknetExporter(&project);
        if(parser.isSet(*exportNamesFile)){
            static_cast<DarknetExporter*>(exporter)->generateLabelIds(parser.value(*exportNamesFile));
        }else{
            qCritical() << "No names file specifed.";
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
        if(parser.isSet(*exportGCPLocal)){
            bool local = true;
            static_cast<GCPExporter*>(exporter)->setBucket("", local);
        }else if(parser.isSet(*exportGCPBucket)){
            static_cast<GCPExporter*>(exporter)->setBucket(parser.value(*exportGCPBucket));
        }else{
            qCritical() << "Using bucket and no path specifed.";
            return false;
        }
    }else if(parser.value(*exportFormatOption) == "tfrecord"){
        exporter = new TFRecordExporter(&project);
        if(parser.isSet(*exportNamesFile)){
            static_cast<TFRecordExporter*>(exporter)->generateLabelIds(parser.value(*exportNamesFile));
        }else{
            qCritical() << "No names file specifed.";
            return false;
        }
    }else{
        qCritical() << "Invalid exporter type specified";
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
            qCritical() << "Please specify an output folder";
            return false;
        }else{
            exporter->setOutputFolder(parser.value(*exportOutputFolder), parser.isSet(*exportNoSubfolders));
        }

        exporter->process();

    }else{
        qFatal("Failed to instantiate exporter");
        return false;
    }

    return true;
}
