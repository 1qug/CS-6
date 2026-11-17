#include "transport/serialtransport.h"

SerialTransport::SerialTransport(QObject *parent)
    : ITransport(parent)
{
    connect(&m_port, &QSerialPort::readyRead, this, [this]() {
        emit dataReceived(m_port.readAll());
    });
    connect(&m_port, &QSerialPort::errorOccurred, this,
            [this](QSerialPort::SerialPortError error) {
        if (error == QSerialPort::NoError)
            return;
        emit errorOccurred(m_port.errorString());
    });
}

bool SerialTransport::open(const QString &target, quint16 port)
{
    Q_UNUSED(port)

    if (m_port.isOpen())
        m_port.close();

    m_port.setPortName(target);
    m_port.setDataBits(QSerialPort::Data8);
    m_port.setParity(QSerialPort::NoParity);
    m_port.setStopBits(QSerialPort::OneStop);
    m_port.setFlowControl(QSerialPort::NoFlowControl);

    if (!m_port.setBaudRate(m_baudRate)) {
        emit errorOccurred(QString("串口 %1 不支持波特率 %2。")
                               .arg(target)
                               .arg(m_baudRate));
        return false;
    }

    if (!m_port.open(QIODevice::ReadWrite)) {
        emit errorOccurred(QString("串口 %1 打开失败：%2").arg(target, m_port.errorString()));
        return false;
    }

    emit opened();
    return true;
}

void SerialTransport::close()
{
    if (!m_port.isOpen())
        return;
    m_port.close();
    emit closed();
}

bool SerialTransport::isOpen() const
{
    return m_port.isOpen();
}

qint64 SerialTransport::write(const QByteArray &data)
{
    return m_port.write(data);
}

QString SerialTransport::description() const
{
    return QString("串口 %1 (%2, 8N1)").arg(m_port.portName()).arg(m_baudRate);
}
