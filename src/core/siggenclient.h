#pragma once

#include <QByteArray>
#include <QObject>
#include <QQueue>
#include <QString>
#include <QTimer>

class ITransport;

// Serialises commands for the signal generator. The device requires at least
// 500 ms between two commands, so the queue inserts a 600 ms gap.
class SigGenClient : public QObject
{
    Q_OBJECT

public:
    explicit SigGenClient(QObject *parent = nullptr);

    // The client owns the transport it creates; callers never manage links.
    bool openSerial(const QString &portName, int baudRate = 115200);
    bool openTcp(const QString &host, quint16 port);
    void close();
    bool isConnected() const;

    // Injection point for tests or alternative links. Does not take ownership.
    void setTransport(ITransport *transport);

    void sendCommand(const QString &command, const QString &note = QString());
    void clearPending();
    int pendingCount() const { return m_queue.size(); }

signals:
    void commandSent(const QString &command, const QString &note);
    void responseLine(const QString &line);
    void logMessage(const QString &message);
    void connectionChanged(bool connected, const QString &description);

private slots:
    void processNext();

private:
    void handleData(const QByteArray &data);

    struct PendingCommand {
        QString command;
        QString note;
    };

    ITransport *m_transport = nullptr;
    QQueue<PendingCommand> m_queue;
    QTimer m_timer;
    QByteArray m_rxBuffer;
    bool m_sending = false;
};
