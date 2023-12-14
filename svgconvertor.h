#ifndef SVGCONVERTOR_H
#define SVGCONVERTOR_H

#include <QWidget>
#include <opencv2/opencv.hpp>

namespace Ui {
class SvgConvertor;
}

class SvgConvertor : public QWidget
{
    Q_OBJECT

public:
    SvgConvertor(const QString&);
    ~SvgConvertor();

protected:
    void resizeEvent(QResizeEvent*) override;

private:
    enum FindField
    {
        Top,
        Right,
        Bottom,
        Left
    };
    Ui::SvgConvertor *ui;
    const qreal m_maxSize;
    QString m_imagePath;
    QImage m_image;
    qreal m_imageWidth;
    qreal m_imageHeight;
    QPixmap m_contourViewer;
    QList<QList<QPointF>> m_contours;

    void drawContours();
    bool checkPixel(const cv::Mat&, const QPoint&);
    bool findPoint(const cv::Mat&, const int, const int, const int, const int, const int);

private slots:
    void findContours();
    void canny(cv::Mat&);
    void threshold(cv::Mat&);
    void getContour(cv::Mat&);

signals:
    void done(QList<QList<QPointF>>);
};

#endif // SVGCONVERTOR_H
