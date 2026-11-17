#pragma once

#include <QByteArray>
#include <QObject>
#include <QString>

// Common byte-stream interface used by both USB (virtual COM) and LAN (TCP)
// control paths. The signal generator only cares about bytes, not the link.
class ITransport : public QObject
{
    Q_OBJECT

public:
    explicit ITransport(QObject *parent = nullptr) : QObject(parent) {}
    ~ITransport() override = default;

    virtual bool open(const QString &target, quint16 port = 0) = 0;
    virtual void close() = 0;
    virtual bool isOpen() const = 0;
    virtual qint64 write(const QByteArray &data) = 0;
    virtual QString description() const = 0;

signals:
    void opened();
    void closed();
    void dataReceived(const QByteArray &data);
    void errorOccurred(const QString &message);
};
