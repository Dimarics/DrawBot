#include "drawingmode.h"
#include "ui_drawingmode.h"
#include "widgets/window.h"
#include "svgconvertor.h"

#include <QFileDialog>
#include <QFontDatabase>
#include <QNetworkAccessManager>
#include <QUdpSocket>

DrawingMode::DrawingMode(QWidget *parent) : QWidget(parent), ui(new Ui::DrawingMode),
    laser_active(false), m_pointsPath("/"), m_font("Segoe UI", 12), m_tool(Pen)
{
    ui->setupUi(this);
    ui->textPageLayout->setAlignment(ui->setText, Qt::AlignHCenter);
    ui->gridLayout_2->setAlignment(ui->drawTest, Qt::AlignHCenter);
    ui->progressBar->hide();
    ui->fontBox->addItems(QFontDatabase::families());

    connect(ui->connectMode, &QComboBox::currentTextChanged, this, [this](const QString &mode)
    {
        if (mode == "COM порт")
        {
            ui->download->close();
            ui->connectButton->show();
            ui->portBox->show();
        }
        else if (mode == "Wi-Fi")
        {
            ui->download->show();
            ui->connectButton->close();
            ui->portBox->close();
        }
    });
    emit ui->connectMode->currentTextChanged(ui->connectMode->currentText());

    // Wi-Fi
    QUdpSocket *udp = new QUdpSocket(this);
    /*connect(ui->lineEdit, &QLineEdit::returnPressed, this, [this, udp]
    {
        QByteArray data = ui->lineEdit->text().toUtf8();
        udp->writeDatagram(data, data.size(), QHostAddress("192.168.4.1"), 8888);
    });*/
    connect(udp, &QUdpSocket::readyRead, this, [udp]
    {
       QByteArray datagram(udp->pendingDatagramSize(), Qt::Initialization());
       udp->readDatagram(datagram.data(), datagram.size());
       qDebug() << datagram;
    });

    // Порт
    connect(ui->connectButton, &QPushButton::clicked, this, [this]()
    {
        if (ui->connectButton->isChecked())
        {
            bool succes = ui->portBox->bind();
            ui->connectButton->setChecked(succes);
            qDebug() << (succes ? "connect" : "connectoin failed");
        }
        else
        {
            ui->portBox->dislink();
        }
    });
    connect(ui->portBox, &PortBox::connectionLost, this, [this]
    {
        ui->connectButton->setChecked(false);
        stop();
    });
    connect(ui->portBox->serialPort(), &QSerialPort::readyRead, this, &DrawingMode::readPort);

    // панель меню
    connect(ui->open, &QAbstractButton::clicked, this, [this]()
    {
        QString image_path = QFileDialog::getOpenFileName(this, "Открыть изображение", ".",
                                                          "Файлы изображений (*.png *.jpg *.bmp);;"
                                                          "ПДФ файлы(*pdf)");
        if (image_path.isEmpty()) return;
        SvgConvertor *svgConvertor = new SvgConvertor(image_path);
        Window *window = new Window(svgConvertor, "SVG конвертер", true);
        window->setSize(900, 600);
        connect(window, &Window::finished, window, &QWidget::close);
        connect(svgConvertor, &SvgConvertor::done, window, &QWidget::close);
        connect(svgConvertor, &SvgConvertor::done, ui->paintWidget, &PaintWidget::addImage);
        window->show();
    });
    connect(ui->zoomIn, &QAbstractButton::clicked, this, [this]()
    {
        ui->paintWidget->scale(1.25);
    });
    connect(ui->zoomOut, &QAbstractButton::clicked, this, [this]()
    {
        ui->paintWidget->scale(0.8);
    });

    // файл
    connect(ui->saveAs, &QAbstractButton::clicked, this, [this]()
    {
        QFileDialog fileDialog(this);
        fileDialog.setFileMode(QFileDialog::AnyFile);
        QFile file(fileDialog.getSaveFileName(this, "Сохранить файл", m_pointsPath, "Точки (*.pt)"));
        if (!file.open(QIODevice::WriteOnly)) return;
        QDataStream output(&file);
        file.close();
    });
    connect(ui->remove, &QAbstractButton::clicked, ui->paintWidget, &PaintWidget::removeItem);
    connect(ui->create, &QAbstractButton::clicked, ui->paintWidget->scene(), &QGraphicsScene::clear);

    // Фигуры
    connect(ui->addRect, &QPushButton::clicked, this, [this]()
    {
        QList<QList<QPointF>> contour
        {
            QList<QPointF>{ QPointF(0, 0), QPointF(1, 0), QPointF(1, 1), QPointF(0, 1),  QPointF(0, 0)}
        };
        ui->paintWidget->addImage(contour);
    });
    connect(ui->addCircle, &QPushButton::clicked, this, [this]()
    { ui->paintWidget->addImage(QList<QList<QPointF>>{circle()}); });
    connect(ui->addPentagon, &QPushButton::clicked, this, [this]()
    { ui->paintWidget->addImage(QList<QList<QPointF>>{circle(1, QPointF(1, 1), 5)}); });
    connect(ui->drawTest, &QPushButton::clicked, this, [this]()
    {
        QList<QList<QPointF>> contours
        {
            // граница
            QList<QPointF>{ QPointF(0, 0), QPointF(240, 0), QPointF(240, 128), QPointF(0, 128),  QPointF(0, 0) },
            // круг 10
            circle(7.5, QPointF(7.5, 7.5)), QList<QPointF> { QPointF(20, 3), QPointF(20, 13) },
            QList<QPointF> { QPointF(23, 13), QPointF(28, 13), QPointF(28, 3), QPointF(23, 3), QPointF(23, 13) },
            // круг 20
            circle(15.5, QPointF(31, 31)),
                    QList<QPointF>
            {
                QPointF(52, 23), QPointF(57, 23), QPointF(57, 28),
                        QPointF(52, 28), QPointF(52, 33), QPointF(57, 33)
            },
            QList<QPointF> { QPointF(59, 33), QPointF(64, 33), QPointF(64, 23), QPointF(59, 23), QPointF(59, 33) },
            // круг 30
            circle(23.5, QPointF(67, 60), 30),
                    QList<QPointF> { QPointF(96, 52), QPointF(101, 52), QPointF(96, 57), QPointF(101, 57), QPointF(96, 62)},
            QList<QPointF> { QPointF(103, 62), QPointF(108, 62), QPointF(108, 52), QPointF(103, 52), QPointF(103, 62)},
            // крест 20 верхний
            QList<QPointF> { QPointF(120, 32), QPointF(120, 0)},
            QList<QPointF> { QPointF(104, 16), QPointF(136, 16)},
            QList<QPointF> // 0
            {
                QPointF(135.5, 19), QPointF(130.5, 19), QPointF(130.5, 29),
                        QPointF(135.5, 29), QPointF(135.5, 19)
            },
            QList<QPointF> // 2
            {
                QPointF(123.5, 19), QPointF(128.5, 19), QPointF(128.5, 24),
                        QPointF(123.5, 24), QPointF(123.5, 29), QPointF(128.5, 29)
            },
            // крест 20 левый
            QList<QPointF> { QPointF(15.5, 48.5), QPointF(15.5, 79.5)},
            QList<QPointF> { QPointF(0, 64), QPointF(31, 64)},
            QList<QPointF> // 0
            {
                QPointF(30.5, 67), QPointF(25.5, 67), QPointF(25.5, 77),
                        QPointF(30.5, 77), QPointF(30.5, 67)
            },
            QList<QPointF> // 2
            {
                QPointF(18.5, 67), QPointF(23.5, 67), QPointF(23.5, 72),
                        QPointF(18.5, 72), QPointF(18.5, 77), QPointF(23.5, 77)
            },
            // змейка горизонтально-прямоугольная
            QList<QPointF>
            {
                QPointF(40, 125), QPointF(40, 110), QPointF(41.6, 110), QPointF(41.6, 125),
                        QPointF(45, 125), QPointF(45, 110), QPointF(49.7, 110), QPointF(49.7, 125),
                        QPointF(56.1, 125), QPointF(56.1, 110), QPointF(64.1, 110), QPointF(64.1, 125),
                        QPointF(73.8, 125), QPointF(73.8, 110), QPointF(84.8, 110), QPointF(84.8, 125),
                        QPointF(97.5, 125), QPointF(97.5, 110), QPointF(111.8, 110), QPointF(111.8, 125),
                        QPointF(127, 125), QPointF(127, 110)
            },
            // зигзаг
            QList<QPointF>
            {
                QPointF(130, 97.5), QPointF(138, 112.5), QPointF(146, 97.5), QPointF(154, 112.5),
                        QPointF(162, 97.5), QPointF(170, 112.5), QPointF(178, 97.5),
                        QPointF(193, 89.5), QPointF(178, 81.5), QPointF(193, 73.5), QPointF(178, 65.5),
                        QPointF(193, 57.5), QPointF(178, 49.5)
            },
            // 10
            QList<QPointF> { QPointF(195, 40), QPointF(195, 30) },
            QList<QPointF> { QPointF(198, 30), QPointF(198, 40), QPointF(203, 40), QPointF(203, 30), QPointF(198, 30)},
            // 8
            QList<QPointF>
            {
                QPointF(198, 63), QPointF(198, 68), QPointF(203, 68), QPointF(203, 63),
                        QPointF(198, 63), QPointF(198, 58), QPointF(203, 58), QPointF(203, 63),
            },
            // 6
            QList<QPointF>
            {
                QPointF(203, 81), QPointF(198, 81), QPointF(198, 91),
                        QPointF(203, 91), QPointF(203, 86), QPointF(198, 86),
            },
            // 4
            QList<QPointF> { QPointF(198, 98), QPointF(198, 103), QPointF(203, 103)},
            QList<QPointF> { QPointF(203, 98), QPointF(203, 108) },
            // змейка вертикально-прямоугольная
            QList<QPointF>
            {
                QPointF(227, 115.5), QPointF(212, 115.5), QPointF(212, 113.9), QPointF(227, 113.9),
                        QPointF(227, 110.5), QPointF(212, 110.5), QPointF(212, 105.8), QPointF(227, 105.8),
                        QPointF(227, 99.4), QPointF(212, 99.4), QPointF(212, 91.4), QPointF(227, 91.4),
                        QPointF(227, 81.7), QPointF(212, 81.7), QPointF(212, 70.7), QPointF(227, 70.7),
                        QPointF(227, 58), QPointF(212, 58), QPointF(212, 43.7), QPointF(227, 43.7),
                        QPointF(227, 27.7), QPointF(212, 27.7)
            },
            // диагональ
            QList<QPointF> { QPointF(240, 0), QPointF(0, 128) },
        };
        ui->paintWidget->addImage(contours);
    });

    // текст
    connect(ui->fontBox, &QComboBox::currentTextChanged, this, [this](const QString &family)
    {
        m_font.setFamily(family);
        ui->textEdit->setFont(m_font);
    });
    connect(ui->setBold, &QCheckBox::stateChanged, this, [this](const int state)
    {
        m_font.setBold(state == Qt::Checked);
        ui->textEdit->setFont(m_font);
    });
    connect(ui->setItalic, &QCheckBox::stateChanged, this, [this](const int state)
    {
        m_font.setItalic(state == Qt::Checked);
        ui->textEdit->setFont(m_font);
    });
    connect(ui->setUnderline, &QCheckBox::stateChanged, this, [this](const int state)
    {
        m_font.setUnderline(state == Qt::Checked);
        ui->textEdit->setFont(m_font);
    });
    connect(ui->setStrikeOut, &QCheckBox::stateChanged, this, [this](const int state)
    {
        m_font.setStrikeOut(state == Qt::Checked);
        ui->textEdit->setFont(m_font);
    });
    connect(ui->setText, &QAbstractButton::clicked, this, [this]()
    {
        if (ui->textEdit->toPlainText().isEmpty()) return;
        QFont font(m_font);
        font.setStyleStrategy(QFont::NoAntialias);
        font.setPointSize(500);

        QFontMetrics font_metrics(font);
        QSize text_size = font_metrics.size(Qt::TextExpandTabs, ui->textEdit->toPlainText());
        QImage image(text_size, QImage::Format_ARGB32);
        image.fill(Qt::white);

        QPainter painter(&image);
        painter.setFont(font);
        painter.drawText(QRect(0, 0, text_size.width(), text_size.height()), ui->textEdit->toPlainText());
        //ui->label->setPixmap(QPixmap::fromImage(image));

        cv::Mat contour(image.height(), image.width(), CV_8UC4, image.bits(), image.bytesPerLine());
        cvtColor(contour, contour, cv::COLOR_BGR2GRAY);
        //Canny(contour, contour, 150, 300, 3);
        contour.convertTo(contour, CV_8U);
        cv::bitwise_not(contour, contour);

        QList<QList<QPointF>> contours;
        std::vector<std::vector<cv::Point>> cv_contours;
        cv::findContours(contour, cv_contours, cv::RETR_LIST, cv::CHAIN_APPROX_SIMPLE);

        for (const std::vector<cv::Point> &spline : cv_contours)
        {
            contours << QList<QPointF>();
            for (const cv::Point &point : spline)
                contours.last() << QPointF(point.x, point.y);
            contours.last() << contours.last().first();
        }
        ui->paintWidget->addImage(contours);
    });

    // движение
    connect(ui->start, &QPushButton::toggled, this, [this](const bool pressed)
    {
        if (pressed)
        {
            ui->start->setText("Пауза");
            ui->start->setToolTip("Пауза");
            ui->start->setIcon(QPixmap(":/images/pause.png"));
        }
        else
        {
            ui->start->setText("Старт");
            ui->start->setToolTip(m_currentCommand == QString() ? "Старт" : "Продолжить");
            ui->start->setIcon(QPixmap(":/images/start.png"));
        }
    });
    connect(ui->start, &QPushButton::clicked, this, [this]
    {
        if (m_currentCommand != QString())
            ui->portBox->write("M0");
        if (ui->start->isChecked())
        {
            if (m_currentCommand == QString())
            {
                m_commands.enqueue("G28");
                for (QList<QPointF> &contour : ui->paintWidget->points())
                {
                    switch (m_tool)
                    {
                    case Pen:
                        m_commands.enqueue("G00 X" + QString::number(contour.first().x(), 'f', 2) +
                                           " Y" + QString::number(contour.first().y(), 'f', 2));
                        m_commands.enqueue("M03");
                        for(QPointF &point : contour)
                        {
                            m_commands.enqueue("G01 X" + QString::number(point.x(), 'f', 2) +
                                               " Y" + QString::number(point.y(), 'f', 2));
                        }
                        m_commands.enqueue("M05");
                        break;

                    case Laser:
                        m_commands.enqueue("G00 X" + QString::number(contour.first().x(), 'f', 2) +
                                           " Y" + QString::number(contour.first().y(), 'f', 2));
                        m_commands.enqueue("M13");
                        for(QPointF &point : contour)
                        {
                            m_commands.enqueue("G01 X" + QString::number(point.x(), 'f', 2) +
                                               " Y" + QString::number(point.y(), 'f', 2));
                        }
                        m_commands.enqueue("M15");
                        break;
                    }
                };
                //m_commands.enqueue("G28");
                if (m_commands.size() == 1)
                {
                    ui->start->setChecked(false);
                    return;
                }
                qDebug() << m_commands.size();
                ui->progressBar->setMaximum(m_commands.size());
                ui->progressBar->setValue(0);
                ui->progressBar->show();
            }
            move();
        }
    });
    connect(ui->stop, &QPushButton::clicked, this, &DrawingMode::stop);

    ui->fontBox->setCurrentText("Segoe UI");
}

QList<QPointF> DrawingMode::circle(const qreal radius, const QPointF &offset, const quint16 resolution) const
{
    const qreal angle = 360.f / resolution;
    QList<QPointF> contour;
    for (int i = 0; i < resolution; ++i)
    {
        contour << offset +
                   QPointF(radius * qCos(qDegreesToRadians(angle * i - 90)),
                           radius * qSin(qDegreesToRadians(angle * i - 90)));
    }
    contour << contour.first();
    return contour;
}
//-------------------------------------------------
void DrawingMode::setTool(Tool tool)
{
    m_tool = tool;
}

void DrawingMode::readPort()
{
    QString input = ui->portBox->serialPort()->readAll();
    if (input == QString()) return;
    qDebug() << "in:" << input.trimmed().trimmed();

    QStringList commands = input.trimmed().split(' ');
    if (commands.size() < 2) return;

    if (commands.at(1) == "ok")
    {
        if (commands.at(0) == m_currentCommand.split(' ').at(0))
        {
            ui->progressBar->setValue(ui->progressBar->value() + 1);
            move();
        }
        /*else if (commands.at(0) == "M0")
        {
            if (laser_active && ui->start->isChecked())
                ui->portBox->write("M13");
            else if (!ui->start->isChecked())
                ui->portBox->write("M15");
        }
        else if (commands.at(0) == "G112")
            ui->portBox->write("M15");
        if (commands.at(0) == "M13")
            laser_active = true;
        else if (ui->start->isChecked() && commands.at(0) == "M15")
            laser_active = false;*/
    }
}
//-------------------------------------------------

// Движение
void DrawingMode::move()
{
    if (!ui->start->isChecked()) return;
    else if (m_commands.size() == 0)
    {
        m_commands.clear();
        m_currentCommand = QString();
        ui->start->setChecked(false);
        ui->progressBar->hide();
        return;
    }
    m_currentCommand = m_commands.dequeue();
    ui->portBox->write(m_currentCommand + '\n');
}

void DrawingMode::stop()
{
    m_commands.clear();
    m_currentCommand = QString();
    ui->start->setChecked(false);
    ui->progressBar->hide();
    ui->portBox->write("M112\n");
}
