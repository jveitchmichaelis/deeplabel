#ifndef IMAGEDISPLAY_H
#define IMAGEDISPLAY_H

#include <QWidget>
#include <QScrollArea>
#include <QScrollBar>
#include <QTemporaryDir>
#include <QApplication>

#include <QtAwesome.h>
#include <opencv2/opencv.hpp>
#include <imagelabel.h>

#include <unordered_map>

namespace Ui {
class ImageDisplay;
}

class ImageDisplay : public QWidget
{
    Q_OBJECT

public:
    explicit ImageDisplay(QWidget *parent = nullptr);
    ~ImageDisplay();

public slots:
    void setImagePath(QString path);
    ImageLabel* getImageLabel(void){return imageLabel;}
    int getBitDepth(void){return bit_depth;}
    void setColourMap(QString map);

    void toggleColourMap(bool enable);
    cv::Mat getOriginalImage();
private:
    cv::Mat display_image;
    cv::Mat original_image;
    QPixmap pixmap;
    Ui::ImageDisplay *ui;
    QString current_imagepath;
    void convert16(cv::Mat &source, double minval=-1, double maxval=-1);
    ImageLabel* imageLabel;
    QScrollArea* scrollArea;
    bool fit_to_window = false;
    double image_scale_factor = 1.0;
    bool apply_colourmap = true;
    int colour_map = cv::COLORMAP_MAGMA;
    QImage::Format format = QImage::Format_Grayscale8;
    int bit_depth = 8;

    static std::unordered_map<std::string, int> colour_hashmap;

private slots:
    void loadPixmap();
    void updateDisplay();
    void resetZoom();
    void scaleImage();

    void adjustScrollBar(QScrollBar *scrollBar, double factor);
    void zoomIn();
    void zoomOut();

signals:
    void image_loaded();
};

#endif // IMAGEDISPLAY_H
