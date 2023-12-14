#include "portbox.h"
#include <QTimer>

PortBox::PortBox(QWidget *parent) : QComboBox(parent), m_serialPort(new QSerialPort(this))
{
    startTimer(200);
    connect(this, &QComboBox::currentIndexChanged, this, [this]()
    {
        dislink();
        emit connectionLost();
    });
}

bool PortBox::bind()
{
    m_serialPort->close();
    m_serialPort->setPort(currentData(Qt::UserRole).value<QSerialPortInfo>());
    return m_serialPort->open(QIODeviceBase::ReadWrite);
}

void PortBox::dislink()
{
    m_serialPort->close();
    emit connectionLost();
}

void PortBox::write(const QString &data)
{
    if (!m_serialPort->isOpen())
    {
        qDebug() << "blocked: " + data;
        return;
    }
    m_serialPort->write(data.toUtf8());
    qDebug() << "out: " + data;
}

QSerialPort *PortBox::serialPort() const
{
    return m_serialPort;
}

void PortBox::timerEvent(QTimerEvent*)
{
    static QList<QSerialPortInfo> port_list;

    if (equal(port_list, QSerialPortInfo::availablePorts())) return;
    port_list = QSerialPortInfo::availablePorts();
    bool lost = true;
    blockSignals(true);
    clear();
    for (QSerialPortInfo &port_info : port_list)
    {
        addItem(port_info.portName(), QVariant::fromValue(port_info));
        if (m_serialPort->isOpen() && port_info.portName() == m_serialPort->portName())
        {
            lost = false;
            setCurrentText(m_serialPort->portName());
            //bind();
        }
    }
    blockSignals(false);
    if (lost) dislink();
}

bool PortBox::equal(const QList<QSerialPortInfo> &last_list, const QList<QSerialPortInfo> &new_list)
{
    if (last_list.size() != new_list.size())
        return false;
    for (quint8 i = 0; i < last_list.size(); ++i)
        if (last_list.at(i).portName() != new_list.at(i).portName())
            return false;
    return true;
}
