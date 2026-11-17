#include "ui/pages/lanconfig/lanconfigpage.h"
#include "ui_lanconfigpage.h"

#include "core/usrm0client.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDesktopServices>
#include <QFile>
#include <QFileDialog>
#include <QHeaderView>
#include <QLineEdit>
#include <QMessageBox>
#include <QNetworkAddressEntry>
#include <QNetworkInterface>
#include <QPushButton>
#include <QSpinBox>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QUrl>

LanConfigPage::LanConfigPage(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::LanConfigPage)
{
    ui->setupUi(this);

    m_client = new UsrM0Client(this);
    connect(m_client, &UsrM0Client::logMessage, this, &LanConfigPage::logMessage);
    connect(m_client, &UsrM0Client::deviceFound, this, &LanConfigPage::handleDeviceFound);
    connect(m_client, &UsrM0Client::searchFinished,
            this, &LanConfigPage::handleSearchFinished);
    connect(m_client, &UsrM0Client::configRead, this, &LanConfigPage::handleConfigRead);
    connect(m_client, &UsrM0Client::operationFinished,
            this, &LanConfigPage::handleOperationFinished);

    ui->ipTypeCombo->setItemData(0, true);
    ui->ipTypeCombo->setItemData(1, false);
    for (int i = 0; i < ui->dataBitsCombo->count(); ++i)
        ui->dataBitsCombo->setItemData(i, 5 + i);
    for (int i = 0; i < ui->parityCombo->count(); ++i)
        ui->parityCombo->setItemData(i, i + 1);
    for (int i = 0; i < ui->stopBitsCombo->count(); ++i)
        ui->stopBitsCombo->setItemData(i, i + 1);

    // Protocol values: TCP Client=1, TCP Server=3, UDP Client=0,
    // UDP Server=2, HTTPD Client=4.
    const int workModes[] = {1, 3, 0, 2, 4};
    for (int i = 0; i < ui->workModeCombo->count() && i < 5; ++i)
        ui->workModeCombo->setItemData(i, workModes[i]);

    ui->deviceTable->verticalHeader()->setVisible(false);
    ui->deviceTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    m_formWidgets << ui->ipTypeCombo << ui->ipEdit << ui->gatewayEdit
                  << ui->subnetEdit << ui->nameEdit << ui->httpPortSpin
                  << ui->baudCombo << ui->dataBitsCombo << ui->parityCombo
                  << ui->stopBitsCombo << ui->workModeCombo
                  << ui->localPortSpin << ui->remotePortSpin << ui->remoteIpEdit;

    connect(ui->refreshInterfaceButton, &QPushButton::clicked,
            this, &LanConfigPage::refreshInterfaces);
    connect(ui->searchButton, &QPushButton::clicked,
            this, &LanConfigPage::searchDevices);
    connect(ui->readButton, &QPushButton::clicked,
            this, &LanConfigPage::readParameters);
    connect(ui->saveButton, &QPushButton::clicked,
            this, &LanConfigPage::saveParameters);
    connect(ui->restartButton, &QPushButton::clicked,
            this, &LanConfigPage::restartDevice);
    connect(ui->resetButton, &QPushButton::clicked,
            this, &LanConfigPage::factoryReset);
    connect(ui->webButton, &QPushButton::clicked,
            this, &LanConfigPage::openWebPage);
    connect(ui->exportButton, &QPushButton::clicked,
            this, &LanConfigPage::exportDevices);
    connect(ui->firmwareButton, &QPushButton::clicked,
            this, &LanConfigPage::showFirmwareUpgradeHelp);
    connect(ui->serialButton, &QPushButton::clicked,
            this, &LanConfigPage::showSerialConfigHint);
    connect(ui->deviceTable, &QTableWidget::itemSelectionChanged,
            this, &LanConfigPage::handleSelectionChanged);

    refreshInterfaces();
    setFormEnabled(false);
}

void LanConfigPage::refreshInterfaces()
{
    const QHostAddress previous = ui->interfaceCombo->count() > 0
            ? m_interfaces.value(ui->interfaceCombo->currentData().toInt()).address
            : QHostAddress();

    m_interfaces.clear();
    ui->interfaceCombo->clear();

    const auto interfaces = QNetworkInterface::allInterfaces();
    for (const QNetworkInterface &iface : interfaces) {
        if (!(iface.flags() & QNetworkInterface::IsUp)
                || (iface.flags() & QNetworkInterface::IsLoopBack)) {
            continue;
        }

        const auto entries = iface.addressEntries();
        for (const QNetworkAddressEntry &entry : entries) {
            if (entry.ip().protocol() != QAbstractSocket::IPv4Protocol)
                continue;
            if (entry.ip().isNull())
                continue;

            quint32 mask = entry.netmask().toIPv4Address();
            if (mask == 0)
                mask = 0xFFFFFFFFu;
            const quint32 broadcast = entry.ip().toIPv4Address() | ~mask;

            InterfaceInfo info;
            info.name = iface.humanReadableName();
            info.address = entry.ip();
            info.broadcast = QHostAddress(broadcast);
            m_interfaces.append(info);

            ui->interfaceCombo->addItem(
                QString("%1  (%2)").arg(info.name, info.address.toString()),
                m_interfaces.size() - 1);
        }
    }

    if (m_interfaces.isEmpty()) {
        InterfaceInfo info;
        info.name = QStringLiteral("全部网卡");
        info.address = QHostAddress();
        info.broadcast = QHostAddress(QHostAddress::Broadcast);
        m_interfaces.append(info);
        ui->interfaceCombo->addItem(QStringLiteral("全部网卡 (255.255.255.255)"), 0);
    }

    for (int i = 0; i < m_interfaces.size(); ++i) {
        if (!previous.isNull() && m_interfaces.at(i).address == previous) {
            ui->interfaceCombo->setCurrentIndex(i);
            break;
        }
    }
}

QList<QHostAddress> LanConfigPage::searchTargets() const
{
    const int index = ui->interfaceCombo->currentData().toInt();
    if (index >= 0 && index < m_interfaces.size())
        return {m_interfaces.at(index).broadcast};
    return {QHostAddress(QHostAddress::Broadcast)};
}

void LanConfigPage::searchDevices()
{
    if (m_client->isBusy())
        return;

    m_devices.clear();
    ui->deviceTable->setRowCount(0);
    m_loadedMac.clear();
    setFormEnabled(false);
    setBusy(true);
    ui->statusLabel->setText(QStringLiteral("搜索中..."));

    m_client->search(searchTargets());
}

void LanConfigPage::showFirmwareUpgradeHelp()
{
    QMessageBox::information(this, QStringLiteral("固件升级"),
        QStringLiteral("固件升级需要使用原厂 USR-M0 工具及对应固件文件。"));
}

void LanConfigPage::showSerialConfigHint()
{
    QMessageBox::information(this, QStringLiteral("串口配置"),
        QStringLiteral("通过串口配置需将模块 CFG 引脚接地，串口固定为 9600, 8, N, 1。\n"
                       "当前版本提供网络配置接口。"));
}

void LanConfigPage::handleDeviceFound(const UsrM0Protocol::Device &device)
{
    const int index = m_devices.size();
    m_devices.append(device);

    const int row = ui->deviceTable->rowCount();
    ui->deviceTable->insertRow(row);

    auto *ipItem = new QTableWidgetItem(device.addressString());
    ipItem->setData(Qt::UserRole, index);
    ui->deviceTable->setItem(row, 0, ipItem);
    ui->deviceTable->setItem(row, 1, new QTableWidgetItem(device.macString()));
    ui->deviceTable->setItem(row, 2, new QTableWidgetItem(device.name));
    ui->deviceTable->setItem(row, 3, new QTableWidgetItem(device.version));

    ui->statusLabel->setText(QStringLiteral("发现 %1 台设备...").arg(m_devices.size()));
}

void LanConfigPage::handleSearchFinished(int deviceCount)
{
    setBusy(false);
    ui->statusLabel->setText(QStringLiteral("搜索完成，共发现 %1 台设备。").arg(deviceCount));

    if (deviceCount > 0 && ui->deviceTable->rowCount() > 0
            && ui->deviceTable->currentRow() < 0) {
        ui->deviceTable->selectRow(0);
    }
}

int LanConfigPage::selectedDeviceIndex() const
{
    const int row = ui->deviceTable->currentRow();
    if (row < 0 || row >= ui->deviceTable->rowCount())
        return -1;
    QTableWidgetItem *item = ui->deviceTable->item(row, 0);
    if (!item)
        return -1;
    const int index = item->data(Qt::UserRole).toInt();
    return (index >= 0 && index < m_devices.size()) ? index : -1;
}

UsrM0Protocol::Device LanConfigPage::selectedDevice() const
{
    const int index = selectedDeviceIndex();
    return index >= 0 ? m_devices.at(index) : UsrM0Protocol::Device();
}

void LanConfigPage::handleSelectionChanged()
{
    const int index = selectedDeviceIndex();
    if (index < 0)
        return;

    const UsrM0Protocol::Device device = m_devices.at(index);
    if (m_loadedMac.compare(device.macString(), Qt::CaseInsensitive) != 0
            && !m_client->isBusy()) {
        readParameters();
    }
}

bool LanConfigPage::validateCredentials()
{
    if (ui->userEdit->text().isEmpty() || ui->passwordEdit->text().isEmpty()) {
        QMessageBox::warning(this, QStringLiteral("参数不完整"),
                             QStringLiteral("请填写模块用户名和密码（默认 admin/admin）。"));
        return false;
    }
    if (ui->userEdit->text().toUtf8().size() > 6
            || ui->passwordEdit->text().toUtf8().size() > 6) {
        QMessageBox::warning(this, QStringLiteral("参数不合法"),
                             QStringLiteral("用户名和密码最多 6 个字节。"));
        return false;
    }
    return true;
}

void LanConfigPage::readParameters()
{
    if (m_client->isBusy())
        return;

    const int index = selectedDeviceIndex();
    if (index < 0) {
        ui->statusLabel->setText(QStringLiteral("请先搜索并选择设备。"));
        return;
    }
    if (!validateCredentials())
        return;

    const UsrM0Protocol::Device device = m_devices.at(index);
    ui->statusLabel->setText(QStringLiteral("正在读取 %1 的参数...").arg(device.addressString()));
    setFormEnabled(false);
    setBusy(true);
    m_client->readConfig(device, ui->userEdit->text(), ui->passwordEdit->text());
}

bool LanConfigPage::collectBasicParams(UsrM0Protocol::BasicParams *basic)
{
    QHostAddress ip;
    QHostAddress gateway;
    QHostAddress subnet;

    if (!ip.setAddress(ui->ipEdit->text().trimmed())
            || !gateway.setAddress(ui->gatewayEdit->text().trimmed())
            || !subnet.setAddress(ui->subnetEdit->text().trimmed())) {
        QMessageBox::warning(this, QStringLiteral("参数不合法"),
                             QStringLiteral("模块 IP、网关、子网掩码必须是有效的 IPv4 地址。"));
        return false;
    }

    UsrM0Protocol::BasicParams result = m_basic;
    result.staticIp = ui->ipTypeCombo->currentData().toBool();
    result.staticIpAddress = ip;
    result.gateway = gateway;
    result.subnet = subnet;
    result.deviceName = ui->nameEdit->text().trimmed();
    result.httpPort = quint16(ui->httpPortSpin->value());
    *basic = result;
    return true;
}

bool LanConfigPage::collectPortParams(UsrM0Protocol::PortParams *port)
{
    bool baudOk = false;
    const quint32 baud = ui->baudCombo->currentText().trimmed().toUInt(&baudOk);
    if (!baudOk || baud < 600 || baud > 4000000) {
        QMessageBox::warning(this, QStringLiteral("参数不合法"),
                             QStringLiteral("波特率必须在 600 ~ 4000000 之间。"));
        return false;
    }

    QHostAddress remote;
    if (!remote.setAddress(ui->remoteIpEdit->text().trimmed())) {
        QMessageBox::warning(this, QStringLiteral("参数不合法"),
                             QStringLiteral("目标 IP 必须是有效的 IPv4 地址。"));
        return false;
    }

    UsrM0Protocol::PortParams result = m_port;
    result.baudRate = baud;
    result.dataBits = quint8(ui->dataBitsCombo->currentData().toInt());
    result.parity = quint8(ui->parityCombo->currentData().toInt());
    result.stopBits = quint8(ui->stopBitsCombo->currentData().toInt());
    result.workMode = quint8(ui->workModeCombo->currentData().toInt());
    result.localPort = quint16(ui->localPortSpin->value());
    result.remotePort = quint16(ui->remotePortSpin->value());
    result.remoteAddress = remote;
    *port = result;
    return true;
}

void LanConfigPage::saveParameters()
{
    if (m_client->isBusy())
        return;

    const int index = selectedDeviceIndex();
    if (index < 0) {
        ui->statusLabel->setText(QStringLiteral("请先搜索并选择设备。"));
        return;
    }

    const UsrM0Protocol::Device device = m_devices.at(index);
    if (m_loadedMac.compare(device.macString(), Qt::CaseInsensitive) != 0) {
        QMessageBox::warning(this, QStringLiteral("尚未读取参数"),
                             QStringLiteral("为保证未公开参数不被破坏，请先读取该设备参数再保存。"));
        return;
    }
    if (!validateCredentials())
        return;

    UsrM0Protocol::BasicParams basic;
    UsrM0Protocol::PortParams port;
    if (!collectBasicParams(&basic) || !collectPortParams(&port))
        return;

    const auto answer = QMessageBox::question(
        this, QStringLiteral("确认保存"),
        QStringLiteral("将参数写入 %1 并保存？\n设备重启后新的 IP/端口生效。")
            .arg(device.addressString()),
        QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    if (answer != QMessageBox::Yes)
        return;

    ui->statusLabel->setText(QStringLiteral("正在保存参数..."));
    setFormEnabled(false);
    setBusy(true);
    m_client->saveConfig(device, basic, port,
                         ui->userEdit->text(), ui->passwordEdit->text(),
                         ui->restartAfterSaveCheck->isChecked());
}

void LanConfigPage::restartDevice()
{
    const int index = selectedDeviceIndex();
    if (index < 0) {
        ui->statusLabel->setText(QStringLiteral("请先搜索并选择设备。"));
        return;
    }
    if (!validateCredentials())
        return;

    const UsrM0Protocol::Device device = m_devices.at(index);
    const auto answer = QMessageBox::question(
        this, QStringLiteral("重启设备"),
        QStringLiteral("确定重启设备 %1？").arg(device.addressString()),
        QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    if (answer != QMessageBox::Yes)
        return;

    m_client->restartDevice(device, ui->userEdit->text(), ui->passwordEdit->text());
}

void LanConfigPage::factoryReset()
{
    const int index = selectedDeviceIndex();
    if (index < 0) {
        ui->statusLabel->setText(QStringLiteral("请先搜索并选择设备。"));
        return;
    }
    if (!validateCredentials())
        return;

    const UsrM0Protocol::Device device = m_devices.at(index);
    const auto answer = QMessageBox::warning(
        this, QStringLiteral("恢复出厂设置"),
        QStringLiteral("确定将设备 %1 恢复出厂设置？\nIP、串口、端口参数都会恢复默认值。")
            .arg(device.addressString()),
        QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    if (answer != QMessageBox::Yes)
        return;

    m_client->factoryReset(device, ui->userEdit->text(), ui->passwordEdit->text());
}

void LanConfigPage::openWebPage()
{
    const UsrM0Protocol::Device device = selectedDevice();
    if (device.address.isNull()) {
        ui->statusLabel->setText(QStringLiteral("请先搜索并选择设备。"));
        return;
    }
    QDesktopServices::openUrl(QUrl(QString("http://%1").arg(device.addressString())));
}

void LanConfigPage::exportDevices()
{
    if (m_devices.isEmpty()) {
        ui->statusLabel->setText(QStringLiteral("没有可导出的设备。"));
        return;
    }

    const QString fileName = QFileDialog::getSaveFileName(
        this, QStringLiteral("导出设备列表"),
        QStringLiteral("usr-m0-devices.csv"),
        QStringLiteral("CSV 文件 (*.csv)"));
    if (fileName.isEmpty())
        return;

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, QStringLiteral("导出失败"), file.errorString());
        return;
    }

    QByteArray content;
    content.append("\xEF\xBB\xBF");
    content.append("IP,MAC,Name,Version\n");
    for (const UsrM0Protocol::Device &device : m_devices) {
        content.append(QString("%1,%2,%3,%4\n")
                       .arg(device.addressString(), device.macString(),
                            device.name, device.version).toUtf8());
    }
    file.write(content);
    file.close();

    emit logMessage(QString("设备列表已导出：%1").arg(fileName));
    ui->statusLabel->setText(QStringLiteral("设备列表已导出。"));
}

void LanConfigPage::handleConfigRead(const UsrM0Protocol::Device &device,
                                     const UsrM0Protocol::BasicParams &basic,
                                     const UsrM0Protocol::PortParams &port)
{
    m_basic = basic;
    m_port = port;
    m_loadedMac = device.macString();
    fillForm(basic, port);
    setFormEnabled(true);
    ui->statusLabel->setText(QStringLiteral("参数读取成功：%1").arg(device.addressString()));
}

void LanConfigPage::handleOperationFinished(const QString &action, bool ok,
                                            const QString &detail)
{
    setBusy(false);
    ui->statusLabel->setText(QString("%1：%2").arg(action, detail));
    if (!ok) {
        emit logMessage(QString("%1失败：%2").arg(action, detail));
        if (!m_loadedMac.isEmpty())
            setFormEnabled(true);
    } else {
        emit logMessage(QString("%1成功：%2").arg(action, detail));
    }
}

void LanConfigPage::fillForm(const UsrM0Protocol::BasicParams &basic,
                             const UsrM0Protocol::PortParams &port)
{
    const int ipTypeIndex = ui->ipTypeCombo->findData(basic.staticIp);
    if (ipTypeIndex >= 0)
        ui->ipTypeCombo->setCurrentIndex(ipTypeIndex);
    ui->ipEdit->setText(basic.staticIpAddress.toString());
    ui->gatewayEdit->setText(basic.gateway.toString());
    ui->subnetEdit->setText(basic.subnet.toString());
    ui->nameEdit->setText(basic.deviceName);
    ui->httpPortSpin->setValue(basic.httpPort);

    const QString baud = QString::number(port.baudRate);
    int baudIndex = ui->baudCombo->findText(baud);
    if (baudIndex < 0) {
        ui->baudCombo->addItem(baud);
        baudIndex = ui->baudCombo->count() - 1;
    }
    ui->baudCombo->setCurrentIndex(baudIndex);

    const int dataIndex = ui->dataBitsCombo->findData(port.dataBits);
    if (dataIndex >= 0)
        ui->dataBitsCombo->setCurrentIndex(dataIndex);
    const int parityIndex = ui->parityCombo->findData(port.parity);
    if (parityIndex >= 0)
        ui->parityCombo->setCurrentIndex(parityIndex);
    const int stopIndex = ui->stopBitsCombo->findData(port.stopBits);
    if (stopIndex >= 0)
        ui->stopBitsCombo->setCurrentIndex(stopIndex);
    const int modeIndex = ui->workModeCombo->findData(port.workMode);
    if (modeIndex >= 0)
        ui->workModeCombo->setCurrentIndex(modeIndex);

    ui->localPortSpin->setValue(port.localPort);
    ui->remotePortSpin->setValue(port.remotePort);
    ui->remoteIpEdit->setText(port.remoteAddress.toString());
}

void LanConfigPage::setFormEnabled(bool enabled)
{
    for (QWidget *widget : m_formWidgets)
        widget->setEnabled(enabled);
    ui->saveButton->setEnabled(enabled && !m_loadedMac.isEmpty());
}

void LanConfigPage::setBusy(bool busy)
{
    ui->interfaceCombo->setEnabled(!busy);
    ui->searchButton->setEnabled(!busy);
    ui->readButton->setEnabled(!busy);
    ui->saveButton->setEnabled(!busy && !m_loadedMac.isEmpty());
    ui->restartButton->setEnabled(!busy);
    ui->resetButton->setEnabled(!busy);
    ui->webButton->setEnabled(!busy);
    ui->exportButton->setEnabled(!busy);
    ui->restartAfterSaveCheck->setEnabled(!busy);
    ui->firmwareButton->setEnabled(!busy);
    ui->serialButton->setEnabled(!busy);
}
