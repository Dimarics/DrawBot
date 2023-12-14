#include "volume.h"

#include <QHBoxLayout>
#include <QEvent>

Volume::Volume(QWidget *parent) : QWidget(parent)
{
    QHBoxLayout *layout = new QHBoxLayout;
    layout->setSpacing(15);
    setLayout(layout);

    m_spinBox = new QSpinBox(this);
    m_spinBox->setMinimumSize(85, 25);
    m_spinBox->setAlignment(Qt::AlignHCenter);
    m_spinBox->setButtonSymbols(QAbstractSpinBox::NoButtons);
    layout->addWidget(m_spinBox);

    m_slider = new QSlider(this);
    m_slider->setOrientation(Qt::Horizontal);
    m_slider->installEventFilter(this);
    m_slider->setMinimumHeight(15);
    m_slider->setMaximumHeight(15);
    layout->addWidget(m_slider);

    connect(m_spinBox, &QSpinBox::valueChanged, this, [this](const int value)
    {
        m_slider->blockSignals(true);
        m_slider->setValue(value);
        m_slider->blockSignals(false);
        emit valueChanged(m_spinBox->value());
    });
    connect(m_slider, &QSlider::valueChanged, this, [this](const int value)
    {
        m_spinBox->blockSignals(true);
        m_spinBox->setValue(value);
        m_spinBox->blockSignals(false);
        emit valueChanged(m_spinBox->value());
    });

    connect(m_spinBox, &QAbstractSpinBox::editingFinished, this, [this]()
    {
        m_spinBox->blockSignals(true);
        m_spinBox->clearFocus();
        m_spinBox->blockSignals(false);
        emit sendValue(m_spinBox->value());
    });

}

void Volume::setMinimum(const int value)
{
    m_spinBox->setMinimum(value);
    m_slider->setMinimum(value);
}

void Volume::setMaximum(const int value)
{
    m_spinBox->setMaximum(value);
    m_slider->setMaximum(value);
}

void Volume::setValue(const int value, const bool emit_signal)
{
    m_spinBox->setValue(value);
    if (emit_signal) emit sendValue(m_spinBox->value());
}

bool Volume::eventFilter(QObject *target, QEvent *event)
{
    if (target == m_slider)
    {
        switch (event->type())
        {
        case QEvent::Wheel:
        case QEvent::KeyPress:
            return true;
            break;

        case QEvent::MouseButtonRelease:
            emit sendValue(m_slider->value());
            break;

        default:
            break;
        }
    }
    return QWidget::eventFilter(target, event);
}

int Volume::value() const
{
    return m_spinBox->value();
}

int Volume::minimum() const
{
    return m_spinBox->minimum();
}

int Volume::maximum() const
{
    return m_spinBox->maximum();
}