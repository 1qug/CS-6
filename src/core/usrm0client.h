#pragma once

#include "core/protocol/usrm0protocol.h"

#include <QHostAddress>
#include <QList>
#include <QObject>
#include <QQueue>
#include <QSet>
#include <QTimer>
#include <QUdpSocket>

Q_DECLARE_METATYPE(UsrM0Protocol::Device)
Q_DECLARE_METATYPE(UsrM0Protocol::BasicParams)
Q_DECLARE_METATYPE(UsrM0Protocol::PortParams)

class UsrM0Client : public QObject
{
    Q_OBJECT

public:
    explicit UsrM0Client(QObject *parent = nullptr);

    void search(const QList<QHostAddress> &broadcasts, int timeoutMs = 1600);
    void readConfig(const UsrM0Protocol::Device &device,
                    const QString &user, const QString &pass);
    void saveConfig(const UsrM0Protocol::Device &device,
                    const UsrM0Protocol::BasicParams &basic,
                    const UsrM0Protocol::PortParams &port,
                    const QString &user, const QString &pass,
                    bool restartAfterSave);
    void restartDevice(const UsrM0Protocol::Device &device,
                       const QString &user, const QString &pass);
    void factoryReset(const UsrM0Protocol::Device &device,
                      const QString &user, const QString &pass);

    bool isBusy() const { return m_state != State::Idle; }

signals:
    void deviceFound(const UsrM0Protocol::Device &device);
    void searchFinished(int deviceCount);
    void configRead(const UsrM0Protocol::Device &device,
                    const UsrM0Protocol::BasicParams &basic,
                    const UsrM0Protocol::PortParams &port);
    void operationFinished(const QString &action, bool ok, const QString &detail);
    void logMessage(const QString &message);

private slots:
    void onReadyRead();
    void finishSearch();
    void finishRead();
    void onAckTimeout();

private:
    enum class State { Idle, Searching, Reading, Writing };

    struct Step {
        QByteArray frame;
        quint8 command = 0;
        QString label;
    };

    void sendTo(const QByteArray &payload, const QHostAddress &address, quint16 port);
    void startNextStep();
    void failOperation(const QString &detail);

    QUdpSocket *m_socket = nullptr;
    QTimer m_searchTimer;
    QTimer m_readTimer;
    QTimer m_ackTimer;

    State m_state = State::Idle;
    QSet<QString> m_seenMacs;

    UsrM0Protocol::Device m_selectedDevice;
    QString m_user;
    QString m_password;
    QByteArray m_readBuffer;

    QQueue<Step> m_steps;
    quint8 m_currentCommand = 0;
    QString m_currentLabel;
    QString m_operation;
    bool m_restartAfterSave = false;
};
