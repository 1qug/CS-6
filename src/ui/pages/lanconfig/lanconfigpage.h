#pragma once

#include "core/protocol/usrm0protocol.h"

#include <QHostAddress>
#include <QList>
#include <QWidget>

namespace Ui { class LanConfigPage; }

class UsrM0Client;

// USR-M0 network module configuration page. Protocol and UDP state live in
// UsrM0Client; this class only maps the form to that service.
class LanConfigPage : public QWidget
{
    Q_OBJECT

public:
    explicit LanConfigPage(QWidget *parent = nullptr);

signals:
    void logMessage(const QString &message);

private slots:
    void refreshInterfaces();
    void searchDevices();
    void showFirmwareUpgradeHelp();
    void showSerialConfigHint();
    void handleDeviceFound(const UsrM0Protocol::Device &device);
    void handleSearchFinished(int deviceCount);
    void handleSelectionChanged();
    void readParameters();
    void saveParameters();
    void restartDevice();
    void factoryReset();
    void openWebPage();
    void exportDevices();

    void handleConfigRead(const UsrM0Protocol::Device &device,
                          const UsrM0Protocol::BasicParams &basic,
                          const UsrM0Protocol::PortParams &port);
    void handleOperationFinished(const QString &action, bool ok, const QString &detail);

private:
    struct InterfaceInfo {
        QString name;
        QHostAddress address;
        QHostAddress broadcast;
    };

    QList<QHostAddress> searchTargets() const;
    int selectedDeviceIndex() const;
    UsrM0Protocol::Device selectedDevice() const;

    bool collectBasicParams(UsrM0Protocol::BasicParams *basic);
    bool collectPortParams(UsrM0Protocol::PortParams *port);
    bool validateCredentials();

    void fillForm(const UsrM0Protocol::BasicParams &basic,
                  const UsrM0Protocol::PortParams &port);
    void setFormEnabled(bool enabled);
    void setBusy(bool busy);

    Ui::LanConfigPage *ui = nullptr;
    UsrM0Client *m_client = nullptr;
    QList<InterfaceInfo> m_interfaces;
    QList<UsrM0Protocol::Device> m_devices;
    UsrM0Protocol::BasicParams m_basic = UsrM0Protocol::BasicParams::defaults();
    UsrM0Protocol::PortParams m_port = UsrM0Protocol::PortParams::defaults();
    QString m_loadedMac;
    QList<QWidget *> m_formWidgets;
};
