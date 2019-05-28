#include "imagedisplay.h"
#include "ui_imagedisplay.h"

std::unordered_map<std::string ,int> ImageDisplay::colour_hashmap{
    { "Cividis", cv::COLORMAP_CIVIDIS },
    { "Inferno", cv::COLORMAP_INFERNO },
    { "Magma", cv::COLORMAP_MAGMA },
    { "Hot", cv::COLORMAP_HOT },
    { "Bone", cv::COLORMAP_BONE },
    { "Plasma", cv::COLORMAP_PLASMA },
    { "Jet", cv::COLORMAP_JET },
    { "Rainbow", cv::COLORMAP_RAINBOW },
    { "Ocean", cv::COLORMAP_OCEAN },
    { "Viridis", cv::COLORMAP_VIRIDIS }
};

ImageDisplay::ImageDisplay(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ImageDisplay)
{
    ui->setupUi(this);

    imageLabel = new ImageLabel;
    scrollArea = new QScrollArea(this);
    ui->mainLayout->addWidget(scrollArea);

    scrollArea->setWidget(imageLabel);

    scrollArea->setAlignment(Qt::AlignCenter);
    scrollArea->setBackgroundRole(QPalette::Shadow);

    connect(ui->fitToWindowButton, SIGNAL(clicked()), this, SLOT(updateDisplay()));
    connect(ui->resetZoomButton, SIGNAL(clicked()), this, SLOT(resetZoom()));
    connect(ui->zoomInButton, SIGNAL(clicked()), this, SLOT(zoomIn()));
    connect(ui->zoomOutButton, SIGNAL(clicked()), this, SLOT(zoomOut()));
    connect(ui->zoomSpinBox, SIGNAL(valueChanged(int)), this, SLOT(scaleImage()));

    QtAwesome* awesome = new QtAwesome( qApp );
    awesome->initFontAwesome();

    ui->zoomInButton->setIcon(awesome->icon(fa::searchplus));
    ui->zoomOutButton->setIcon(awesome->icon(fa::searchminus));
    ui->resetZoomButton->setIcon(awesome->icon(fa::home));

    updateDisplay();
}

ImageDisplay::~ImageDisplay()
{
    delete ui;
}

void ImageDisplay::setImagePath(QString path){
    if(path != ""){
        current_imagepath = path;
        loadPixmap();
    }
}

void ImageDisplay::convert16(cv::Mat &source){

    double minval, maxval;
    cv::minMaxIdx(source, &minval, &maxval);
    double range = maxval-minval;
    double scale_factor = 255.0/range;

    source.convertTo(source, CV_32FC1);
    source -= minval;
    source *= scale_factor;
    source.convertTo(source, CV_8UC1);

    return;
}

void ImageDisplay::loadPixmap(){

    pixmap.load(current_imagepath);

    auto image = cv::imread(current_imagepath.toStdString(), cv::IMREAD_UNCHANGED|cv::IMREAD_ANYDEPTH);

    if(image.empty()){
        qDebug() << "Failed to load image " << current_imagepath;
        return;
    }

    display_image = image.clone();

    if(image.elemSize() == 2){

        convert16(display_image);

        if(display_image.channels() == 1 && apply_colourmap){
            cv::applyColorMap(display_image, display_image, colour_map);
        }

        QTemporaryDir dir;
        if (dir.isValid()) {
            cv::imwrite(dir.path().toStdString()+"/temp.png", display_image);
            pixmap.load(dir.path()+"/temp.png");
        }

    }else{
        // Default to single channel 8-bit image
        format = QImage::Format_Grayscale8;

        if(display_image.channels() == 3){
            cv::cvtColor(display_image, display_image, cv::COLOR_BGR2RGB);
            format = QImage::Format_RGB888;
        }else if (display_image.channels() == 4){
            cv::cvtColor(display_image, display_image, cv::COLOR_BGRA2RGBA);
            format = QImage::Format_RGBA8888;
        }else if(display_image.channels() == 1 && apply_colourmap){
            cv::applyColorMap(display_image, display_image, colour_map);
            //cv::cvtColor(display_image, display_image, cv::COLOR_BGR2RGB);
            format = QImage::Format_RGB888;
        }

        pixmap.fromImage(QImage(display_image.data, display_image.cols, display_image.rows, display_image.step, format));
    }

    imageLabel->setImage(display_image);
    updateDisplay();
    emit image_loaded();
}

void ImageDisplay::setColourMap(QString map){

    auto map_str = map.toStdString();
    if(colour_hashmap.count(map_str)){
        colour_map = colour_hashmap[map_str];
        loadPixmap();
    }

}

void ImageDisplay::toggleColourMap(bool enable){
    apply_colourmap = enable;
    loadPixmap();
}

void ImageDisplay::adjustScrollBar(QScrollBar *scrollBar, double factor)
{
    scrollBar->setValue(int(factor * scrollBar->value()
                            + ((factor - 1) * scrollBar->pageStep()/2)));
}

void ImageDisplay::scaleImage(void)
{
    if(pixmap.isNull()) return;

    image_scale_factor = static_cast<double>(ui->zoomSpinBox->value()) / 100.0;

    imageLabel->zoom(image_scale_factor);

    adjustScrollBar(scrollArea->horizontalScrollBar(), image_scale_factor);
    adjustScrollBar(scrollArea->verticalScrollBar(), image_scale_factor);

    ui->zoomInButton->setEnabled(image_scale_factor <= 3.0);
    ui->zoomOutButton->setEnabled(image_scale_factor >= 0.33);
}

void ImageDisplay::zoomIn()
{
    if(fit_to_window) return;
    ui->zoomSpinBox->setValue(ui->zoomSpinBox->value()*1.2);
}

void ImageDisplay::zoomOut()
{
    if(fit_to_window) return;
    ui->zoomSpinBox->setValue(ui->zoomSpinBox->value()*0.8);
}

void ImageDisplay::resetZoom(){

    ui->zoomSpinBox->setValue(100);
}

void ImageDisplay::updateDisplay()
{

    fit_to_window = ui->fitToWindowButton->isChecked();

    // Decide whether to use scrollbars
    scrollArea->setWidgetResizable(fit_to_window);

    // Auto-scale the image to the size of the scrollarea,
    // or leave it full-size
    imageLabel->setScaledContents(fit_to_window);

    if(!pixmap.isNull()){
        imageLabel->setPixmap(pixmap);
    }
}


