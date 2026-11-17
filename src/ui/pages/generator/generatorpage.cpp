#include "ui/pages/generator/generatorpage.h"
#include "ui_generatorpage.h"

#include "core/siggenclient.h"
#include "ui/pages/generator/calibrationpage.h"
#include "ui/pages/generator/connectionpage.h"
#include "ui/pages/generator/generatorpages.h"
#include "ui/pages/generator/homepage.h"
#include "ui/pages/generator/modulationpage.h"
#include "ui/pages/generator/noisepage.h"
#include "ui/pages/generator/referencepage.h"
#include "ui/pages/generator/sweeppage.h"
#include "ui/pages/generator/systeminfopage.h"

#include <QPushButton>

GeneratorPage::GeneratorPage(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::GeneratorPage)
{
    ui->setupUi(this);

    m_client = new SigGenClient(this);
    connect(m_client, &SigGenClient::commandSent, this, &GeneratorPage::handleCommandSent);
    connect(m_client, &SigGenClient::responseLine, this, &GeneratorPage::handleResponse);
    connect(m_client, &SigGenClient::connectionChanged,
            this, &GeneratorPage::handleConnectionChanged);
    connect(m_client, &SigGenClient::logMessage, this, &GeneratorPage::logMessage);

    // ---------------------------------------------------------- connection
    connect(ui->connectionPage, &ConnectionPage::logMessage,
            this, &GeneratorPage::logMessage);
    connect(ui->connectionPage, &ConnectionPage::serialConnectRequested, this,
            [this](const QString &portName, int baudRate) {
        emit logMessage(QString("正在打开串口 %1（%2, 8N1）...")
                            .arg(portName)
                            .arg(baudRate));
        if (!m_client->openSerial(portName, baudRate))
            emit logMessage(QString("串口 %1（%2, 8N1）打开失败。")
                                .arg(portName)
                                .arg(baudRate));
    });
    connect(ui->connectionPage, &ConnectionPage::tcpConnectRequested, this,
            [this](const QString &host, quint16 port) {
        emit logMessage(QString("正在连接 %1:%2 ...").arg(host).arg(port));
        m_client->openTcp(host, port);
    });
    connect(ui->connectionPage, &ConnectionPage::disconnectRequested,
            m_client, &SigGenClient::close);
    connect(ui->connectionPage, &ConnectionPage::runRequested,
            this, &GeneratorPage::runDevice);

    // ---------------------------------------------------------- home page
    connect(ui->homePage, &HomePage::frequencyApplyRequested, this, [this](quint64 hz) {
        send(SigGenProtocol::setFrequency(hz), QStringLiteral("设置频率"));
    });
    connect(ui->homePage, &HomePage::amplitudeApplyRequested, this, [this](double dbm) {
        send(SigGenProtocol::setAmplitude(dbm), QStringLiteral("设置功率"));
    });
    connect(ui->homePage, &HomePage::rfToggleRequested, this, [this](bool enabled) {
        send(enabled ? SigGenProtocol::rfOn() : SigGenProtocol::rfOff(),
             enabled ? QStringLiteral("射频开") : QStringLiteral("射频关"));
    });
    connect(ui->homePage, &HomePage::navigateRequested,
            this, &GeneratorPage::showStackPage);
    connect(ui->homePage, &HomePage::statusQueryRequested,
            this, &GeneratorPage::queryStatus);
    connect(ui->homePage, &HomePage::temperatureQueryRequested, this, [this]() {
        send(SigGenProtocol::queryTemperature(), QStringLiteral("读取温度"));
    });
    connect(ui->homePage, &HomePage::lockQueryRequested, this, [this]() {
        send(SigGenProtocol::queryLockState(), QStringLiteral("读取锁定状态"));
    });
    connect(ui->homePage, &HomePage::referenceQueryRequested, this, [this]() {
        send(SigGenProtocol::queryReferenceState(), QStringLiteral("读取参考状态"));
    });
    connect(ui->homePage, &HomePage::fanQueryRequested, this, [this]() {
        send(SigGenProtocol::queryFanState(), QStringLiteral("读取风扇状态"));
    });

    // ------------------------------------------------------ reference page
    connect(ui->referencePage, &ReferencePage::backRequested,
            this, &GeneratorPage::goHome);
    connect(ui->referencePage, &ReferencePage::applyRequested, this, [this](int channel) {
        send(SigGenProtocol::setReference(channel), QStringLiteral("设置参考源"));
    });
    connect(ui->referencePage, &ReferencePage::selectionChanged,
            ui->homePage, &HomePage::setReferenceSummary);

    // ----------------------------------------------------- modulation page
    connect(ui->modulationPage, &ModulationPage::backRequested,
            this, &GeneratorPage::goHome);
    connect(ui->modulationPage, &ModulationPage::logMessage,
            this, &GeneratorPage::logMessage);
    connect(ui->modulationPage, &ModulationPage::modulationRequested,
            this, &GeneratorPage::applyModulation);
    connect(ui->modulationPage, &ModulationPage::typeApplyRequested, this, [this]() {
        send(SigGenProtocol::setModulationTypeAsk(), QStringLiteral("设置 ASK 调制"));
    });
    connect(ui->modulationPage, &ModulationPage::frequencyApplyRequested, this,
            [this](int index) {
        send(SigGenProtocol::setModulationFrequency(index), QStringLiteral("设置调制频率"));
    });
    connect(ui->modulationPage, &ModulationPage::dutyApplyRequested, this,
            [this](int duty) {
        send(SigGenProtocol::setModulationDuty(duty), QStringLiteral("设置调制占空比"));
    });

    // ---------------------------------------------------------- sweep page
    connect(ui->sweepPage, &SweepPage::backRequested,
            this, &GeneratorPage::goHome);
    connect(ui->sweepPage, &SweepPage::modeChanged,
            ui->homePage, &HomePage::setSweepSummary);
    connect(ui->sweepPage, &SweepPage::frequencySweepRequested, this,
            [this](quint64 start, quint64 stop, quint64 step, int interval) {
        send(SigGenProtocol::setSweepStartFrequency(start), QStringLiteral("扫描起始频率"));
        send(SigGenProtocol::setSweepStopFrequency(stop), QStringLiteral("扫描终止频率"));
        send(SigGenProtocol::setSweepStepFrequency(step), QStringLiteral("扫描频率步进"));
        send(SigGenProtocol::setSweepInterval(interval), QStringLiteral("扫描间隔"));
    });
    connect(ui->sweepPage, &SweepPage::powerSweepRequested, this,
            [this](double start, double stop, double step, int interval) {
        send(SigGenProtocol::setSweepStartAmplitude(start), QStringLiteral("扫描起始功率"));
        send(SigGenProtocol::setSweepStopAmplitude(stop), QStringLiteral("扫描终止功率"));
        send(SigGenProtocol::setSweepStepAmplitude(step), QStringLiteral("扫描功率步进"));
        send(SigGenProtocol::setSweepInterval(interval), QStringLiteral("扫描间隔"));
    });

    // -------------------------------------------------------- static pages
    connect(ui->noisePage, &NoisePage::backRequested,
            this, &GeneratorPage::goHome);
    connect(ui->systemInfoPage, &SystemInfoPage::backRequested,
            this, &GeneratorPage::goHome);
    connect(ui->calibrationPage, &CalibrationPage::backRequested,
            this, &GeneratorPage::goHome);

    // -------------------------------------------------------------- sidebar
    connect(ui->homeButton, &QPushButton::clicked, this, &GeneratorPage::goHome);
    connect(ui->connectPageButton, &QPushButton::clicked,
            this, &GeneratorPage::showConnectPage);

    ui->homePage->setReferenceSummary(ui->referencePage->currentText());
    ui->homePage->setSweepSummary(ui->sweepPage->currentModeText());

    setPagesEnabled(false);
    showConnectPage();
    ui->connectionPage->refreshPorts();
}

GeneratorPage::~GeneratorPage()
{
    delete ui;
}

void GeneratorPage::goHome()
{
    showStackPage(m_client->isConnected() ? GeneratorPages::Home
                                          : GeneratorPages::Connect);
}

void GeneratorPage::runDevice()
{
    if (m_client->isConnected())
        showStackPage(GeneratorPages::Home);
}

void GeneratorPage::showConnectPage()
{
    showStackPage(GeneratorPages::Connect);
}

void GeneratorPage::showStackPage(int page)
{
    ui->generatorStack->setCurrentIndex(page);
}

void GeneratorPage::queryStatus()
{
    ui->lockLabel->setText(QStringLiteral("Lock State:\n查询中..."));
    ui->referenceStateLabel->setText(QStringLiteral("Ref State:\n查询中..."));
    ui->temperatureLabel->setText(QStringLiteral("Temperature:\n查询中..."));
    ui->fanLabel->setText(QStringLiteral("Fan State:\n查询中..."));
    send(SigGenProtocol::queryState(), QStringLiteral("状态查询"));
}

void GeneratorPage::applyModulation(bool enabled)
{
    send(SigGenProtocol::setModulation(enabled),
         enabled ? QStringLiteral("调制开") : QStringLiteral("调制关"));
    ui->homePage->setModulationSummary(
        enabled ? QStringLiteral("内部 ASK / 开") : QStringLiteral("内部 ASK / 关"));
}

void GeneratorPage::handleCommandSent(const QString &command, const QString &note)
{
    // Commands carry the CR LF terminator on the wire; keep the preview clean.
    const QString text = command.trimmed();
    ui->commandPreview->setText(text);
    ui->homePage->setLastCommand(text, note);

    emit logMessage(QString("TX  %1%2").arg(
        note.isEmpty() ? QString() : QString("[%1] ").arg(note), text));
}

void GeneratorPage::handleResponse(const QString &line)
{
    emit logMessage(QString("RX  %1").arg(line));

    m_responseLines.prepend(line);
    while (m_responseLines.size() > 5)
        m_responseLines.removeLast();
    ui->responsePreview->setText(m_responseLines.join(QLatin1Char('\n')));

    if (SigGenProtocol::parseStatusLine(line, &m_status)) {
        QString lock = m_status.lockState;
        if (lock.contains(QLatin1String("unlock"), Qt::CaseInsensitive))
            lock = QStringLiteral("Unlocked");
        else if (lock.contains(QLatin1String("locked"), Qt::CaseInsensitive))
            lock = QStringLiteral("Locked");
        ui->lockLabel->setText(QStringLiteral("Lock State:\n") + lock);

        const QString ref = m_status.referenceState;
        if (ref.contains(QLatin1String("warmed"), Qt::CaseInsensitive))
            ui->referenceStateLabel->setText(QStringLiteral("Ref State:\nWarmed"));
        else if (ref.contains(QLatin1String("warm"), Qt::CaseInsensitive))
            ui->referenceStateLabel->setText(QStringLiteral("Ref State:\nWarm"));
        else if (ref.contains(QLatin1String("heat"), Qt::CaseInsensitive))
            ui->referenceStateLabel->setText(QStringLiteral("Ref State:\nHeating"));
        else
            ui->referenceStateLabel->setText(QStringLiteral("Ref State:\n") + ref);

        ui->temperatureLabel->setText(QStringLiteral("Temperature:\n") + m_status.temperature);
        ui->fanLabel->setText(QStringLiteral("Fan State:\n") + m_status.fanState);
    }
}

void GeneratorPage::handleConnectionChanged(bool connected, const QString &description)
{
    ui->connectionPage->setConnected(connected, description);
    setPagesEnabled(connected);
    emit connectionStateChanged(connected);

    if (!connected) {
        ui->homePage->resetRfState();
        showConnectPage();
        emit logMessage(QStringLiteral("设备连接已断开。"));
    } else {
        emit logMessage(QString("设备连接成功：%1").arg(description));
        queryStatus();
    }
}

void GeneratorPage::send(const QString &command, const QString &note)
{
    m_client->sendCommand(command, note);
}

void GeneratorPage::setPagesEnabled(bool enabled)
{
    ui->homePage->setEnabled(enabled);
    ui->referencePage->setEnabled(enabled);
    ui->modulationPage->setEnabled(enabled);
    ui->sweepPage->setEnabled(enabled);
    ui->noisePage->setEnabled(enabled);
    ui->systemInfoPage->setEnabled(enabled);
    ui->calibrationPage->setEnabled(enabled);
}
