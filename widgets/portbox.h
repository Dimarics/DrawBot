#ifndef PORTBOX_H
#define PORTBOX_H

#include <QComboBox>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>

class PortBox : public QComboBox
{
    Q_OBJECT
public:
    PortBox(QWidget* = nullptr);
    bool bind();
    void dislink();
    QSerialPort *serialPort() const;

protected:
    void timerEvent(QTimerEvent*) override;

private:
    QSerialPort *m_serialPort;
    bool equal(const QList<QSerialPortInfo>&, const QList<QSerialPortInfo>&);

public slots:
    void write(const QString&);

signals:
    void connectionLost();
};

#endif // PORTBOX_H
