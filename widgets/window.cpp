#include "window.h"
#include "ui_window.h"

#include <QGraphicsDropShadowEffect>
#include <QMouseEvent>
#include <QPainter>

Window::Window(QWidget *widget, const QString &title, const bool dialog) :
    QWidget(nullptr, Qt::Window | Qt::FramelessWindowHint), ui(new Ui::Window), m_widget(widget)
{
    setAttribute(Qt::WA_TranslucentBackground);
    ui->setupUi(this);
    //ui->quitButton->setIcon(QIcon(":/images/cross.svg"));
    setIcon();
    for(QObject *child : children())
        child->installEventFilter(this);
    setTitle(title);
    m_widget->setParent(ui->centralWidget);
    //ui->widgetLayout->addWidget(m_widget);
    setMinimumSize(m_widget->minimumSize() + QSize(20, 46));

    if (dialog)
    {
        setAttribute(Qt::WA_DeleteOnClose);
        setWindowModality(Qt::ApplicationModal);
        ui->screenModeButton->close();
        ui->hideButton->close();
        connect(ui->quitButton, &QPushButton::clicked, this, &Window::finished);
    }
    else
    {
        connect(ui->quitButton, &QPushButton::clicked, qApp, &QCoreApplication::quit);
        connect(ui->screenModeButton, &QPushButton::toggled, this, [this](const bool maximized)
        {
            if (maximized)
            {
                m_normalGeometry = geometry();
                QRect rect = QApplication::primaryScreen()->availableGeometry();
                setGeometry(rect.x() - 10, rect.y() - 10, rect.width() + 20, rect.height() + 20);
                ui->screenModeButton->setToolTip("Свернуть в окно");
            }
            else setGeometry(m_normalGeometry);
            ui->screenModeButton->setToolTip("Развернуть");
        });
        connect(ui->hideButton, &QPushButton::clicked, this, &QWidget::showMinimized);
    }
}

Window::~Window()
{
    delete ui;
}

void Window::setTitle(const QString &title)
{
    ui->title->setText(title);
    //setWindowTitle(title);
}

void Window::setIcon()
{
    ui->icon->setPixmap(QPixmap(":/images/windowIcon.png"));//.scaled(16, 16, Qt::KeepAspectRatio));
}

void Window::setSize(const QSize &size)
{
    resize(size + QSize(ui->leftField->width() + ui->rightField->width(),
                        ui->topField->height() + ui->windowPanel->height() + ui->bottomField->height()));
}

void Window::setSize(const int width, const int height)
{
    resize(width + ui->leftField->width() + ui->rightField->width(),
           height + ui->topField->height() + ui->windowPanel->height() + ui->bottomField->height());
}

void Window::showMaximum(const bool full)
{
    ui->screenModeButton->setChecked(full);
}

bool Window::eventFilter(QObject* target, QEvent* event)
{
    static QPointF mouse_pos;
    static bool mouse_pressed = false;
    QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);

    switch (event->type())
    {
    case QEvent::MouseButtonPress:
        if (mouseEvent->button() == Qt::LeftButton)
        {
            mouse_pressed = true;
            mouse_pos = mouseEvent->pos();
            mouse_pos += target == ui->windowPanel ? QPointF(10, 10) :
                                                     target == ui->rightTopField ? QPointF(-10, 0) :
                                                     target == ui->rightField ? QPointF(-10, 0) :
                                                     target == ui->rightBottomField ? QPointF(-10, -10) :
                                                     target == ui->bottomField ? QPointF(0, -10) :
                                                     target == ui->leftBottomField ? QPointF(0, -10) : QPointF();
        }
        break;

    case QEvent::MouseMove:
        if (mouse_pressed)
        {
            QPointF pos = mouseEvent->globalPosition() - mouse_pos;
            if (target == ui->windowPanel)
            {
                if (ui->screenModeButton->isChecked())
                {
                    mouse_pos.setX(mouse_pos.x() * (m_normalGeometry.width()) / width());
                    pos = mouseEvent->globalPosition() - mouse_pos;
                    ui->screenModeButton->setChecked(false);
                }
                move(pos.x(), pos.y());
            }
            else if (target == ui->leftField && width() + x() - pos.x() >= minimumWidth())
                setGeometry(pos.x(), y(), width() + x() - pos.x(), height());
            else if (target == ui->leftTopField)
            {
                const int _width = width() + x() - pos.x();
                const int _height = height() + y() - pos.y();

                if (_width >= minimumWidth())
                    setGeometry(pos.x(), y(), _width, height());
                if (_height >= minimumHeight())
                    setGeometry(x(), pos.y(), width(), _height);
            }
            else if (target == ui->topField && height() + y() - pos.y() >= minimumHeight())
                setGeometry(x(), pos.y(), width(), height() + y() - pos.y());
            else if (target == ui->rightTopField)
            {
                const int _width = pos.x() - x();
                const int _height = height() + y() - pos.y();

                if (_width >= minimumWidth())
                    resize(_width, height());
                if (_height >= minimumHeight())
                    setGeometry(x(), pos.y(), width(), _height);
            }
            else if (target == ui->rightField && pos.x() - x() >= minimumWidth())
                resize(pos.x() - x(), height());
            else if (target == ui->rightBottomField)
            {
                const int _width = pos.x() - x();
                const int _height = pos.y() - y();

                if (_width >= minimumWidth())
                    resize(_width, height());
                if (_height >= minimumHeight())
                    resize(width(), _height);
            }
            else if (target == ui->bottomField && pos.y() - y() >= minimumHeight())
                resize(width(), pos.y() - y());
            else if (target == ui->leftBottomField)
            {
                const int _width = width() + x() - pos.x();
                const int _height = pos.y() - y();

                if (_width >= minimumWidth())
                    setGeometry(pos.x(), y(), _width, height());
                if (_height >= minimumHeight())
                    setGeometry(x(), y(), width(), _height);
            }
        }
        break;

        case QEvent::Resize:
        if (target == ui->centralWidget) m_widget->resize(ui->centralWidget->size());
        break;

    case QEvent::MouseButtonRelease:
        if (mouseEvent->button() == Qt::LeftButton)
            mouse_pressed = false;
        break;

    default:
        break;
    }
    return QWidget::eventFilter(target, event);
}
