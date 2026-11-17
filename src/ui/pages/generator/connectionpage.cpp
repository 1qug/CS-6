#include "ui/pages/generator/connectionpage.h"
#include "ui_connectionpage.h"

#include "ui/theme.h"

#include <QComboBox>
#include <QIntValidator>
#include <QLineEdit>
#include <QPushButton>
#include <QSerialPortInfo>
#include <QSpinBox>

ConnectionPage::ConnectionPage(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ConnectionPage)
{
    ui->setupUi(this);

    connect(ui->refreshPortsButton, &QPushButton::clicked,
            this, &ConnectionPage::refreshPorts);
    connect(ui->connectButton, &QPushButton::clicked,
            this, &ConnectionPage::handleConnectClicked);
    connect(ui->modeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ConnectionPage::updateMode);
    connect(ui->runButton, &QPushButton::clicked,
            this, &ConnectionPage::runRequested);

    // The combo is editable so the operator can enter a non-standard rate as
    // well as pick a common one. QSerialPort accepts arbitrary values.
    ui->baudCombo->setInsertPolicy(QComboBox::NoInsert);
    const int commonBaudRates[] = {
        1200, 2400, 4800, 9600, 19200, 38400, 57600,
        115200, 230400, 460800, 921600, 1500000, 2000000, 3000000
    };
    for (const int baudRate : commonBaudRates)
        ui->baudCombo->addItem(QString::number(baudRate), baudRate);
    ui->baudCombo->setCurrentText(QStringLiteral("115200"));
    ui->baudCombo->lineEdit()->setValidator(new QIntValidator(50, 4000000, this));
    ui->baudCombo->lineEdit()->setPlaceholderText(QStringLiteral("自定义波特率"));
    connect(ui->baudCombo, &QComboBox::currentTextChanged,
            this, &ConnectionPage::updateSerialHint);

    updateSerialHint();
    updateMode();
    setConnected(false, QString());
}

ConnectionPage::~ConnectionPage()
{
    delete ui;
}

void ConnectionPage::refreshPorts()
{
    const QString current = ui->portCombo->currentData().toString();
    ui->portCombo->clear();

    const auto ports = QSerialPortInfo::availablePorts();
    for (const QSerialPortInfo &port : ports) {
        const QString description = port.description().isEmpty()
                ? QString() : QString(" - %1").arg(port.description());
        ui->portCombo->addItem(port.portName() + description, port.portName());
    }

    if (ui->portCombo->count() == 0) {
        ui->portCombo->addItem(QStringLiteral("未发现串口"), QString());
        emit logMessage(QStringLiteral("未发现可用串口，请检查 FT232RL 驱动是否安装。"));
    } else {
        const int index = ui->portCombo->findData(current);
        if (index >= 0)
            ui->portCombo->setCurrentIndex(index);
    }
}

void ConnectionPage::updateMode()
{
    const bool serial = ui->modeCombo->currentIndex() == 0;
    ui->labelSerial->setVisible(serial);
    ui->portCombo->setVisible(serial);
    ui->refreshPortsButton->setVisible(serial);
    ui->labelBaudRate->setVisible(serial);
    ui->baudCombo->setVisible(serial);
    ui->labelSerialParams->setVisible(serial);
    ui->labelHost->setVisible(!serial);
    ui->hostEdit->setVisible(!serial);
    ui->labelTcpPort->setVisible(!serial);
    ui->tcpPortSpin->setVisible(!serial);
}

void ConnectionPage::updateSerialHint()
{
    const int baudRate = selectedBaudRate();
    ui->labelSerialParams->setText(
        baudRate > 0
            ? QStringLiteral("串口参数：%1, 8, N, 1").arg(baudRate)
            : QStringLiteral("串口参数：自定义, 8, N, 1"));
}

int ConnectionPage::selectedBaudRate() const
{
    bool ok = false;
    const int baudRate = ui->baudCombo->currentText().trimmed().toInt(&ok);
    return ok ? baudRate : 0;
}

void ConnectionPage::handleConnectClicked()
{
    if (m_connected) {
        emit disconnectRequested();
        return;
    }

    if (ui->modeCombo->currentIndex() == 0) {
        const QString portName = ui->portCombo->currentData().toString();
        if (portName.isEmpty()) {
            emit logMessage(QStringLiteral("请选择有效的串口。"));
            return;
        }
        const int baudRate = selectedBaudRate();
        if (baudRate < 50 || baudRate > 4000000) {
            emit logMessage(QStringLiteral("请输入有效波特率（50 ~ 4000000）。"));
            return;
        }
        emit serialConnectRequested(portName, baudRate);
    } else {
        const QString host = ui->hostEdit->text().trimmed();
        if (host.isEmpty()) {
            emit logMessage(QStringLiteral("请输入设备 IP 地址。"));
            return;
        }
        emit tcpConnectRequested(host, quint16(ui->tcpPortSpin->value()));
    }
}

void ConnectionPage::setConnected(bool connected, const QString &description)
{
    m_connected = connected;

    ui->connectButton->setText(connected ? QStringLiteral("断开连接")
                                         : QStringLiteral("连接设备"));
    ui->connectButton->setEnabled(true);
    ui->connectionLabel->setText(connected
                                 ? QString("已连接：%1").arg(description)
                                 : QStringLiteral("未连接"));
    ui->connectionLabel->setProperty("connected", connected);
    Theme::repolish(ui->connectionLabel);

    ui->modeCombo->setEnabled(!connected);
    ui->portCombo->setEnabled(!connected);
    ui->refreshPortsButton->setEnabled(!connected);
    ui->baudCombo->setEnabled(!connected);
    ui->hostEdit->setEnabled(!connected);
    ui->tcpPortSpin->setEnabled(!connected);

    ui->runButton->setEnabled(connected);
}
