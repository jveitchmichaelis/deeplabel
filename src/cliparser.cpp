#include "cliparser.h"

CliParser::CliParser(QObject *parent) : QObject(parent)
{
    SetupOptions();
}

void CliParser::SetupOptions(){

    exportFormatOption = new QCommandLineOption({"f", "format"}, "export format", "kitti, darknet, gcp, voc, coco, mot, birdsai, tfrecord");
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
    exportVideoFilename = new QCommandLineOption("video-filename", "video output filename", "filename", "out.mp4");
    exportVideoFourcc = new QCommandLineOption("fourcc", "video codec fourcc", "codec", "h264");
    exportVideoFps = new QCommandLineOption("fps", "video framerate", "fps", "10");
    exportVideoColourmap = new QCommandLineOption("colourmap", "video colourmap", "colourmap", "Inferno");
    exportVideoSize = new QCommandLineOption("videosize", "video size: width, height", "w,h", "1280,720");
    exportVideoDisplayBoxes = new QCommandLineOption("display-boxes", "display boxes in output", "on, off", "on");
    exportVideoDisplayNames = new QCommandLineOption("display-names", "display class names", "on, off", "on");

    importImages = new QCommandLineOption("images", "import image path/folder", "images");
    importTFRecordMask = new QCommandLineOption("records", "mask for TF Records (* wildcard)", "images");
    importAnnotations = new QCommandLineOption("annotations", "import annotation path/folder", "annotations");
    importUnlabelledImages = new QCommandLineOption("import-unlabelled", "import images without labels");
    importOverwrite = new QCommandLineOption("overwrite", "overwrite existing databases");

    detectChannels = new QCommandLineOption("detect-channels", "number of channels", "detect-channels", "3");
    detectTarget = new QCommandLineOption("detect-target", "Detection target", "CPU, CUDA", "CPU");
    detectFramework = new QCommandLineOption("detect-framework", "Detection framework", "detect-framework", "Darknet");
    detectConvertDepth = new QCommandLineOption("convert-depth", "Convert 16-bit to 8-bit");
    detectGrayToRGB = new QCommandLineOption("convert-gray", "Convert RGB to grayscale");
    detectNMSThresh = new QCommandLineOption("nms-thresh", "NMS Threshold", "threshold", "0.7");
    detectConfThresh = new QCommandLineOption("conf-thresh", "Confidence Threshold", "threshold", "0.5");
    detectConfig = new QCommandLineOption("config", "Path to model config file", "path");
    detectWeights = new QCommandLineOption("weights", "Path to model weights file", "path");

    configSilence = new QCommandLineOption({"q","quiet"}, "no log messages");

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
    parser.addOption(*exportVideoFourcc);
    parser.addOption(*exportVideoFps);
    parser.addOption(*exportVideoFilename);
    parser.addOption(*exportVideoColourmap);
    parser.addOption(*exportVideoSize);
    parser.addOption(*exportVideoDisplayBoxes);
    parser.addOption(*exportVideoDisplayNames);
    parser.addOption(*exportUnlabelledImages);
    parser.addOption(*importImages);
    parser.addOption(*importAnnotations);
    parser.addOption(*importUnlabelledImages);
    parser.addOption(*importOverwrite);
    parser.addOption(*importTFRecordMask);

    parser.addOption(*detectChannels);
    parser.addOption(*detectTarget);
    parser.addOption(*detectFramework);
    parser.addOption(*detectConvertDepth);
    parser.addOption(*detectGrayToRGB);
    parser.addOption(*detectNMSThresh);
    parser.addOption(*detectConfThresh);
    parser.addOption(*detectConfig);
    parser.addOption(*detectWeights);

    parser.addOption(*configSilence);

}

bool CliParser::Run(){

    parser.process(*QCoreApplication::instance());

    bool res = false;
    auto mode = parser.positionalArguments().at(0);

    for(auto &option : parser.unknownOptionNames()){
        qCritical() << "Unknown option: " << option;
    }

    if(parser.isSet(*configSilence)){
        qSetMessagePattern("");
    }

    if(mode == "export"){
        res = handleExport();
    }else if(mode == "import"){
        res = handleImport();
    }else if(mode == "merge"){
        res = handleMerge();
    }else if(mode == "detect"){
        res = handleDetect();
    }

    return res;
}

bool CliParser::handleDetect(){

    DetectorOpenCV detector;

    qInfo() << "Using weights: " << parser.value("weights");
    qInfo() << "Using config: " << parser.value("config");

    detector.setChannels(parser.value("detect-channels").toInt());
    detector.setTarget(parser.value("detect-target"));
    detector.setFramework(parser.value("detect-framework"));
    detector.setConvertGrayscale(parser.isSet("convert-gray"));
    detector.setConvertDepth(parser.isSet("convert-depth"));
    detector.setNMSThreshold(parser.value("nms-thresh").toDouble());
    detector.setConfidenceThreshold(parser.value("conf-thresh").toDouble());
    detector.loadNetwork(parser.value("names").toStdString(),
                         parser.value("config").toStdString(),
                         parser.value("weights").toStdString());

    LabelProject project;
    project.loadDatabase(parser.value("input"));

    detector.runOnProject(&project);

    return true;
}

bool CliParser::handleMerge(){
    qCritical() << "Not implemented yet";
    return false;
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
    }else if(parser.value("format") == "pascalvoc"){

        PascalVOCImporter importer(&project);
        importer.moveToThread(import_thread);
        importer.setImportUnlabelled(import_unlabelled);
        importer.import(parser.value("images"), parser.value("annotations"));
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
    }else if(parser.value(*exportFormatOption) == "video"){
        exporter = new VideoExporter(&project);
        auto filename = parser.value(*exportVideoFilename);
        auto fourcc = parser.value(*exportVideoFourcc);

        bool ok = false;
        auto fps = parser.value(*exportVideoFps).toDouble(&ok);
        if(!ok){
            qCritical() << "Invalid FPS, you specified: " << parser.value(*exportVideoFps);
            return false;
        }

        // Check video size
        auto videosize = parser.value(*exportVideoSize).split(",");
        if(videosize.size() != 2){
            qCritical() << "Video size should be two comma separated values: w,h";
            return false;
        }

        auto width = parser.value(*exportVideoSize).split(",").at(0).toInt(&ok);
        if(!ok){
            qCritical() << "Invalid video width, you specified: " << parser.value(*exportVideoSize).split(",").at(0);
            return false;
        }

        auto height = parser.value(*exportVideoSize).split(",").at(1).toInt(&ok);
        if(!ok){
            qCritical() << "Invalid video height, you specified: " << parser.value(*exportVideoSize).split(",").at(1);
            return false;
        }

        auto colourmap = parser.value(*exportVideoColourmap);

        static_cast<VideoExporter*>(exporter)->videoConfig(filename, fourcc, fps, {width, height}, colourmap);
        static_cast<VideoExporter*>(exporter)->labelConfig(parser.value(*exportVideoDisplayNames) == "on",
                                                           parser.value(*exportVideoDisplayBoxes) == "on");


        if(!parser.isSet(*exportOutputFolder) || parser.value(*exportOutputFolder) == ""){
            qCritical() << "Please specify an output folder";
            return false;
        }else{
            exporter->setOutputFolder(parser.value(*exportOutputFolder), true);
        }

        exporter->process();
        return true;

    }else{
        qCritical() << "Invalid exporter type specified";
        return false;
    }

    if(exporter != nullptr){
        exporter->moveToThread(export_thread);

        // No progress bar
        exporter->disableProgress(true);
        exporter->setExportUnlabelled(parser.isSet(*exportUnlabelledImages));

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
