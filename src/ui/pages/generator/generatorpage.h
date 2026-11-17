#pragma once

#include "core/protocol/siggenprotocol.h"

#include <QStringList>
#include <QWidget>

namespace Ui { class GeneratorPage; }

class SigGenClient;

// Signal generator workspace. This class is the composition point for the
// individual page widgets: it owns the protocol service, routes page signals
// to commands and renders responses into the sidebar and pages.
class GeneratorPage : public QWidget
{
    Q_OBJECT

public:
    explicit GeneratorPage(QWidget *parent = nullptr);
    ~GeneratorPage() override;

signals:
    void logMessage(const QString &message);
    void connectionStateChanged(bool connected);

private slots:
    void goHome();
    void runDevice();
    void showConnectPage();
    void queryStatus();
    void showStackPage(int page);
    void applyModulation(bool enabled);

    void handleCommandSent(const QString &command, const QString &note);
    void handleResponse(const QString &line);
    void handleConnectionChanged(bool connected, const QString &description);

private:
    void send(const QString &command, const QString &note);
    void setPagesEnabled(bool enabled);

    Ui::GeneratorPage *ui = nullptr;
    SigGenClient *m_client = nullptr;
    SigGenProtocol::DeviceStatus m_status;
    QStringList m_responseLines;
};
