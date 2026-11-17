#pragma once

#include <QWidget>

namespace Ui { class ConnectionPage; }

// Link selection, connection handling and the run entry point. The page owns
// no transport; it only emits connection requests for GeneratorPage.
class ConnectionPage : public QWidget
{
    Q_OBJECT

public:
    explicit ConnectionPage(QWidget *parent = nullptr);
    ~ConnectionPage() override;

    void setConnected(bool connected, const QString &description);

public slots:
    void refreshPorts();

signals:
    void logMessage(const QString &message);
    void serialConnectRequested(const QString &portName, int baudRate);
    void tcpConnectRequested(const QString &host, quint16 port);
    void disconnectRequested();
    void runRequested();

private slots:
    void updateMode();
    void updateSerialHint();
    void handleConnectClicked();

private:
    int selectedBaudRate() const;

    Ui::ConnectionPage *ui = nullptr;
    bool m_connected = false;
};
