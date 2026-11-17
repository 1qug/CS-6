#pragma once

#include "transport/itransport.h"

#include <QSerialPort>

class SerialTransport : public ITransport
{
    Q_OBJECT

public:
    explicit SerialTransport(QObject *parent = nullptr);

    // Must be set before open(); the default matches the device manual.
    void setBaudRate(int baudRate) { m_baudRate = baudRate; }
    int baudRate() const { return m_baudRate; }

    bool open(const QString &target, quint16 port = 0) override;
    void close() override;
    bool isOpen() const override;
    qint64 write(const QByteArray &data) override;
    QString description() const override;

private:
    QSerialPort m_port;
    int m_baudRate = 115200;
};
