#include "svgconvertor.h"
#include "ui_svgconvertor.h"

#include <QFileInfo>
#include <QPdfDocument>
#include <QPainter>

SvgConvertor::SvgConvertor(const QString &path) : ui(new Ui::SvgConvertor), m_maxSize(10000),
    m_imagePath(path)
{
    ui->setupUi(this);
    if (QFileInfo(path).suffix() == "pdf")
    {
        /*QPrinter printer(QPrinter::HighResolution);
        //printer.setPageSize(QPageSize::A3);
        printer.setOutputFormat(QPrinter::PdfFormat);
        printer.setOutputFileName(path);
        m_image = QImage(printer.paperRect(QPrinter::DevicePixel).size().toSize(), QImage::Format_ARGB32);*/
        QPdfDocument document(this);
        document.load(path);
        const QSize size = document.pagePointSize(0).toSize() * 4;
        //QPdfDocumentRenderOptions options;
        //options.setRenderFlags(QPdfDocumentRenderOptions::RenderFlag::Grayscale |
        //                       QPdfDocumentRenderOptions::RenderFlag::PathAliased);
        //options.setScaledSize(size * 20);
        m_image = QImage(size, QImage::Format_ARGB32);
        m_image.fill(Qt::white);
        QPainter painter(&m_image);
        painter.drawImage(0, 0, document.render(0, size));
    }
    else
        m_image = QImage(path).convertToFormat(QImage::Format_ARGB32);
    //if (m_image.width() > m_maxSize || m_image.height() > m_maxSize) m_image = m_image.scaled(m_maxSize, m_maxSize, Qt::KeepAspectRatio);
    setAttribute(Qt::WA_DeleteOnClose);
    connect(ui->filter, &QComboBox::currentTextChanged, this, [this](const QString &text)
    {
        if (text == "Canny")
            ui->filterWidget->setCurrentWidget(ui->cannyPage);
        else if (text == "Threshold")
            ui->filterWidget->setCurrentWidget(ui->thresholdPage);
        if (ui->done->isEnabled()) findContours();
    });
    connect(ui->convert, &QAbstractButton::clicked, this, [this]()
    {
        if (!ui->done->isEnabled())
        {
            ui->done->setEnabled(true);
            ui->convert->setText("Сравнить");
            findContours();
        }
    });
    connect(ui->interference, &QSlider::valueChanged, this, &SvgConvertor::findContours);

    // Canny
    connect(ui->level, &QSpinBox::valueChanged, this, &SvgConvertor::findContours);
    connect(ui->minimum, &QSlider::valueChanged, this, &SvgConvertor::findContours);
    connect(ui->maximum, &QSlider::valueChanged, this, &SvgConvertor::findContours);

    // Threshold
    connect(ui->thresh, &QSlider::valueChanged, this, &SvgConvertor::findContours);
    connect(ui->resolution, &QSlider::valueChanged, this, &SvgConvertor::findContours);
    connect(ui->fill, &QCheckBox::stateChanged, this, &SvgConvertor::findContours);
    connect(ui->streak, &QCheckBox::stateChanged, this, &SvgConvertor::findContours);
    connect(ui->texture, &QComboBox::currentIndexChanged, this, &SvgConvertor::findContours);
    connect(ui->scale, &QDoubleSpinBox::valueChanged, this, &SvgConvertor::findContours);
    connect(ui->angle, &QSpinBox::valueChanged, this, &SvgConvertor::findContours);

    connect(ui->convert, &QAbstractButton::pressed, this, [this]()
    { if (ui->done->isEnabled()) ui->image->setPixmap(QPixmap::fromImage(m_image).scaled(ui->image->size(), Qt::KeepAspectRatio)); });
    connect(ui->convert, &QAbstractButton::released, this, [this]()
    { if (ui->done->isEnabled()) ui->image->setPixmap(m_contourViewer); });
    connect(ui->done, &QAbstractButton::pressed, this, [this]()
    { emit done(m_contours); });
}

SvgConvertor::~SvgConvertor()
{
    delete ui;
}

void SvgConvertor::resizeEvent(QResizeEvent*)
{
    if (!ui->done->isEnabled())
    {
        ui->image->setPixmap(QPixmap::fromImage(m_image).scaled(ui->image->size(), Qt::KeepAspectRatio));
    }
    else
    {
        drawContours();
    }
}

void SvgConvertor::findContours()
{
    if (!ui->done->isEnabled()) return;
    const qreal k = ui->resolution->value() / 100.0;
    QImage image(m_image.width() > m_maxSize || m_image.height() > m_maxSize ?
                     m_image.scaled(m_maxSize * k, m_maxSize * k, Qt::KeepAspectRatio) :
                     m_image.scaled(m_image.size() * k, Qt::KeepAspectRatio));
    cv::Mat cv_image(image.height(), image.width(), CV_8UC4, image.bits(), image.bytesPerLine());
    m_imageWidth = image.width();
    m_imageHeight = image.height();
    
    cvtColor(cv_image, cv_image, cv::COLOR_BGR2GRAY);
    //cv::bitwise_not(cv_image, cv_image);

    if (ui->filter->currentText() == "Canny")
        canny(cv_image);
    else if (ui->filter->currentText() == "Threshold")
        threshold(cv_image);

    QList<QList<QPointF>> contours;
    const qreal size = (m_imageWidth > m_imageHeight ? m_imageWidth : m_imageHeight) / 10;

    for (QList<QPointF> &contour : m_contours)
    {
        qreal length = 0;
        for (qsizetype i = 1; i < contour.size(); ++i)
            length += (contour.at(i) - contour.at(i - 1)).manhattanLength();
        if (!(length < size / 500.f * (500.f - ui->interference->value())))
            contours << contour;
    }
    m_contours = contours;
    /*for (qsizetype i = 0; i < m_contours.size(); ++i)
    {
        qreal length = 0;
        for (qsizetype j = 1; j < m_contours.at(i).size(); ++j)
            length += (m_contours.at(i).at(j) - m_contours.at(i).at(j - 1)).manhattanLength();
        if (length < size /  ui->interference->value())
            indexes << i;
    }
    if (!indexes.isEmpty())
    {
        for (qsizetype i : indexes)
            m_contours.removeAt(i);
        m_contours.squeeze();
    }*/

    drawContours();
}

void SvgConvertor::canny(cv::Mat &cv_image)
{
    Canny(cv_image, cv_image, ui->minimum->value(), ui->maximum->value(), ui->level->value() * 2 + 1);
    cv_image.convertTo(cv_image, CV_8U);
    getContour(cv_image);
}

void SvgConvertor::threshold(cv::Mat &cv_image)
{
    //cv::adaptiveThreshold(cv_image, cv_image, 255, cv::THRESH_BINARY, cv::ADAPTIVE_THRESH_GAUSSIAN_C, ui->level->value() * 2 + 1, ui->minimum->value());
    //cv::GaussianBlur(cv_image, cv_image, cv::Size(ui->level->value() * 2 + 1, ui->level->value() * 2 + 1), 0);
    cv::threshold(cv_image, cv_image, ui->thresh->value(), 255, cv::THRESH_BINARY_INV);

    //imshow("", cv_image);
    if (ui->fill->checkState() == Qt::Checked)
    {
        getContour(cv_image);
    }
    else
    {
        std::vector<std::vector<cv::Point>> cv_contours;
        cv::findContours(cv_image, cv_contours, cv::RETR_LIST, cv::CHAIN_APPROX_SIMPLE);
        m_contours.clear();
        for (const std::vector<cv::Point> &contour : cv_contours)
        {
            if (contour.size() > 1)
            {
                m_contours << QList<QPointF>();
                for(const cv::Point &point : contour)
                    m_contours.last() << QPointF(point.x, point.y);
                m_contours.last() << QPointF(contour.at(0).x, contour.at(0).y);
                //std::vector<cv::Point> approximations;
                //cv::approxPolyDP(spline, approximations, (m_imageWidth > m_imageHeight ? m_imageHeight : m_imageWidth) * 0.005, false);
                //m_contours << QList<QPoint>();
                //for(const cv::Point &point : approximations)
                //m_contours.last() << QPoint(point.x, point.y);
            }
        }
        if (ui->streak->checkState() == Qt::Checked)
        {
            const qreal side = 2 * (m_imageWidth > m_imageHeight ? m_imageWidth : m_imageHeight);
            for (int y = -side; y < side; y += side / 300.f * ui->scale->value())
            {
                auto streak = [&](const qreal angle)
                {
                    QList<QPointF> line;
                    for (int x = -side; x < side; ++x)
                    {
                        const qreal rx = x * qCos(angle) - y * qSin(angle);
                        const qreal ry = x * qSin(angle) + y * qCos(angle);
                        if (rx > 0 && rx < cv_image.cols && ry > 0 && ry < cv_image.rows)
                        {
                            if (cv_image.at<bool>(ry, rx))
                                line << QPointF(rx, ry);
                            else if (line.size() == 1)
                                line.clear();
                            else if (line.size() > 1)
                            {
                                m_contours << QList<QPointF>{line.first(), line.last()};
                                line.clear();
                            }
                        }
                    }
                };
                streak(qDegreesToRadians(ui->angle->value()));
                if (ui->texture->currentText() == "Сетка")
                    streak(qDegreesToRadians(90 + ui->angle->value()));
            }
        }
    }
}

void SvgConvertor::getContour(cv::Mat &cv_image)
{

    QList<QList<QPoint>> contours;
    for (int i = 1; i < cv_image.cols && contours.isEmpty(); ++i)
        for (int j = 0; j < cv_image.rows && contours.isEmpty(); ++j)
            if (cv_image.at<bool>(j, i) == 1)
                contours << QList<QPoint>{QPoint(i, j)};

    bool finish = false;
    const int limit = cv_image.cols > cv_image.rows ? cv_image.cols : cv_image.rows;
    FindField field = Top;
    while (!finish)
    {
        bool find = false;
        for (int r = 1; !find && !finish; ++r)
        {
            if (r > limit)
            {
                finish = true;
                break;
            }
            for (quint8 count = 0; count < 4 && !find; ++count)
            {
                switch (field)
                {
                case Top:
                    for (int i = 0; i <= r * 2 && !find; ++i)
                    {
                        QPoint point(contours.last().last() + QPoint(i - r, -r));
                        find = checkPixel(cv_image, point);
                        if (find)
                        {
                            if (r > 1)
                                contours << QList<QPoint>();
                            else
                            {
                                if (checkPixel(cv_image, point + QPoint(1, 0)))
                                    cv_image.at<bool>(point.y(), point.x() + 1) = 0;
                                if (checkPixel(cv_image, point + QPoint(0, 1)))
                                    cv_image.at<bool>(point.y() + 1, point.x()) = 0;
                            }
                            contours.last() << point;
                        }
                    }
                    if (!find) field = Right;
                    break;

                case Right:
                    for (int i = 0; i <= r * 2 && !find; ++i)
                    {
                        QPoint point(contours.last().last() + QPoint(r, i - r));
                        find = checkPixel(cv_image, point);
                        if (find)
                        {
                            if (r > 1)
                                contours << QList<QPoint>();
                            else
                            {
                                if (checkPixel(cv_image, point + QPoint(-1, 0)))
                                    cv_image.at<bool>(point.y(), point.x() - 1) = 0;
                                if (checkPixel(cv_image, point + QPoint(0, 1)))
                                    cv_image.at<bool>(point.y() + 1, point.x()) = 0;
                            }
                            contours.last() << point;
                        }
                    }
                    if (!find) field = Bottom;
                    break;

                case Bottom:
                    for (int i = 0; i <= r * 2 && !find; ++i)
                    {
                        QPoint point(contours.last().last() + QPoint(r - i, r));
                        find = checkPixel(cv_image, point);
                        if (find)
                        {
                            if (r > 1)
                                contours << QList<QPoint>();
                            else
                            {
                                if (checkPixel(cv_image, point + QPoint(-1, 0)))
                                    cv_image.at<bool>(point.y(), point.x() - 1) = 0;
                                if (checkPixel(cv_image, point + QPoint(0, -1)))
                                    cv_image.at<bool>(point.y() - 1, point.x()) = 0;
                            }
                            contours.last() << point;
                        }
                    }
                    if (!find) field = Left;
                    break;

                case Left:
                    for (int i = 0; i <= r * 2 && !find; ++i)
                    {
                        QPoint point(contours.last().last() + QPoint(-r, r - i));
                        find = checkPixel(cv_image, point);
                        if (find)
                        {
                            if (r > 1)
                                contours << QList<QPoint>();
                            else
                            {
                                if (checkPixel(cv_image, point + QPoint(1, 0)))
                                    cv_image.at<bool>(point.y(), point.x() + 1) = 0;
                                if (checkPixel(cv_image, point + QPoint(0, -1)))
                                    cv_image.at<bool>(point.y() - 1, point.x()) = 0;
                            }
                            contours.last() << point;
                        }
                    }
                    if (!find) field = Top;
                    break;
                }
            }
        }
        if (find) cv_image.at<bool>(contours.last().last().y(), contours.last().last().x()) = 0;
    }

    // аппроксимация
    m_contours.clear();
    for (QList<QPoint> &contour : contours)
    {
        if (contour.size() > 1)
        {
            m_contours << QList<QPointF>{contour.first()};
            for (qsizetype i = 1; i < contour.size() - 1; ++i)
            {
                const QPoint last(contour.at(i) - contour.at(i - 1));
                const QPoint next(contour.at(i + 1) - contour.at(i));
                if ((last.x() < 0 && last.y() == last.x() && next.x() < 0 && next.y() == next.x()) ||
                        (last.x() == 0 && last.y() < 0 && next.x() == 0 && next.y() < 0) ||
                        (last.x() > 0 && last.y() == -last.x() && next.x() > 0 && next.y() == -next.x()) ||
                        (last.x() > 0 && last.y() == 0 && next.x() > 0 && next.y() == 0) ||
                        (last.x() > 0 && last.y() == last.x() && next.x() > 0 && next.y() == next.x()) ||
                        (last.x() == 0 && last.y() > 0 && next.x() == 0 && next.y() > 0) ||
                        (last.x() < 0 && last.y() == -last.x() && next.x() < 0 && next.y() == -next.x()) ||
                        (last.x() < 0 && last.y() == 0 && next.x() < 0 && next.y() == 0))
                {
                    m_contours.last() << contour.at(i);
                }
            }
            m_contours.last() << contour.last();
        }
    }
}

void SvgConvertor::drawContours()
{
    m_contourViewer = QPixmap(m_image.size()).scaled(ui->image->size(), Qt::KeepAspectRatio);
    
    QImage image(m_contourViewer.size(), QImage::Format_Grayscale8);
    image.fill(Qt::white);
    
    QPainter painter(&image);
    painter.setRenderHint(QPainter::Antialiasing);
    
    for (QList<QPointF> &lines : m_contours)
    {
        QList<QPointF> contour;
        for (QPointF &point : lines)
        {
            contour << QPointF(point) * qreal(image.width()) /
                       (m_image.width() > m_maxSize ? m_maxSize : qreal(m_image.width())) *
                       100.0 / ui->resolution->value();
        }
        painter.drawPolyline(contour);
    }
    m_contourViewer = QPixmap::fromImage(image);
    ui->image->setPixmap(m_contourViewer);
}

bool SvgConvertor::checkPixel(const cv::Mat &image, const QPoint &point)
{
    if (point.x() > 0 && point.x() < image.cols && point.y() > 0 && point.y() < image.rows && image.at<bool>(point.y(), point.x()) == 1)
        return true;
    return false;
}
