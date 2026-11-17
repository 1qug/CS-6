#pragma once

#include "transport/itransport.h"

#include <QTcpSocket>

class TcpTransport : public ITransport
{
    Q_OBJECT

public:
    explicit TcpTransport(QObject *parent = nullptr);

    bool open(const QString &target, quint16 port = 0) override;
    void close() override;
    bool isOpen() const override;
    qint64 write(const QByteArray &data) override;
    QString description() const override;

private:
    QTcpSocket m_socket;
};
