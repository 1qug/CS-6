#include "core/usrm0client.h"

UsrM0Client::UsrM0Client(QObject *parent)
    : QObject(parent)
{
    qRegisterMetaType<UsrM0Protocol::Device>("UsrM0Protocol::Device");
    qRegisterMetaType<UsrM0Protocol::BasicParams>("UsrM0Protocol::BasicParams");
    qRegisterMetaType<UsrM0Protocol::PortParams>("UsrM0Protocol::PortParams");

    m_socket = new QUdpSocket(this);
    if (!m_socket->bind(QHostAddress::AnyIPv4, 0,
                        QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint)) {
        emit logMessage(QString("UDP 套接字绑定失败：%1").arg(m_socket->errorString()));
    }
    connect(m_socket, &QUdpSocket::readyRead, this, &UsrM0Client::onReadyRead);

    m_searchTimer.setSingleShot(true);
    connect(&m_searchTimer, &QTimer::timeout, this, &UsrM0Client::finishSearch);

    m_readTimer.setSingleShot(true);
    connect(&m_readTimer, &QTimer::timeout, this, &UsrM0Client::finishRead);

    m_ackTimer.setSingleShot(true);
    connect(&m_ackTimer, &QTimer::timeout, this, &UsrM0Client::onAckTimeout);
}

void UsrM0Client::search(const QList<QHostAddress> &broadcasts, int timeoutMs)
{
    if (isBusy()) {
        emit logMessage("设备正在通信中，请稍后再搜索。");
        return;
    }

    m_state = State::Searching;
    m_seenMacs.clear();

    const QByteArray binary = UsrM0Protocol::buildSearchPacket();
    const QByteArray ascii = QByteArrayLiteral("WWW.USR.CN");

    QList<QHostAddress> targets = broadcasts;
    targets.append(QHostAddress(QHostAddress::Broadcast));

    for (const QHostAddress &target : targets) {
        sendTo(binary, target, UsrM0Protocol::ConfigPort);
        sendTo(ascii, target, UsrM0Protocol::AtPort);
    }

    emit logMessage(QString("开始搜索设备（UDP %1/%2）...")
                    .arg(UsrM0Protocol::ConfigPort)
                    .arg(UsrM0Protocol::AtPort));
    m_searchTimer.start(timeoutMs);
}

void UsrM0Client::readConfig(const UsrM0Protocol::Device &device,
                             const QString &user, const QString &pass)
{
    if (isBusy()) {
        emit logMessage("设备正在通信中，请稍后再读取参数。");
        return;
    }

    m_state = State::Reading;
    m_selectedDevice = device;
    m_user = user;
    m_password = pass;
    m_operation = QStringLiteral("读取参数");
    m_readBuffer.clear();

    const QByteArray frame = UsrM0Protocol::buildReadPacket(device.mac, user, pass);
    sendTo(frame, device.address, UsrM0Protocol::ConfigPort);
    emit logMessage(QString("读取设备 %1 参数：%2")
                    .arg(device.addressString(), QString::fromLatin1(frame.toHex(' ')).toUpper()));
    m_readTimer.start(2500);
}

void UsrM0Client::saveConfig(const UsrM0Protocol::Device &device,
                             const UsrM0Protocol::BasicParams &basic,
                             const UsrM0Protocol::PortParams &port,
                             const QString &user, const QString &pass,
                             bool restartAfterSave)
{
    if (isBusy()) {
        emit logMessage("设备正在通信中，请稍后再保存参数。");
        return;
    }

    m_state = State::Writing;
    m_selectedDevice = device;
    m_user = user;
    m_password = pass;
    m_operation = QStringLiteral("保存参数");
    m_restartAfterSave = restartAfterSave;
    m_steps.clear();

    m_steps.enqueue({UsrM0Protocol::buildWritePacket(0x05, device.mac, user, pass,
                                                     UsrM0Protocol::encodeBasicParams(basic)),
                     0x05, QStringLiteral("基础参数")});
    m_steps.enqueue({UsrM0Protocol::buildWritePacket(0x06, device.mac, user, pass,
                                                     UsrM0Protocol::encodePortParams(port)),
                     0x06, QStringLiteral("端口参数")});
    m_steps.enqueue({UsrM0Protocol::buildSimplePacket(0x04, device.mac, user, pass),
                     0x04, QStringLiteral("保存参数")});

    emit logMessage(QString("开始保存 %1 的参数...").arg(device.addressString()));
    startNextStep();
}

void UsrM0Client::restartDevice(const UsrM0Protocol::Device &device,
                                const QString &user, const QString &pass)
{
    if (isBusy())
        return;

    const QByteArray frame = UsrM0Protocol::buildSimplePacket(0x02, device.mac, user, pass);
    sendTo(frame, device.address, UsrM0Protocol::ConfigPort);
    emit logMessage(QString("已发送重启指令：%1")
                    .arg(QString::fromLatin1(frame.toHex(' ')).toUpper()));
    emit operationFinished(QStringLiteral("重启设备"), true,
                           QStringLiteral("指令已发送，设备可能不会返回应答"));
}

void UsrM0Client::factoryReset(const UsrM0Protocol::Device &device,
                               const QString &user, const QString &pass)
{
    if (isBusy())
        return;

    const QByteArray frame = UsrM0Protocol::buildSimplePacket(0x0B, device.mac, user, pass);
    sendTo(frame, device.address, UsrM0Protocol::ConfigPort);
    emit logMessage(QString("已发送恢复出厂设置指令：%1")
                    .arg(QString::fromLatin1(frame.toHex(' ')).toUpper()));
    emit operationFinished(QStringLiteral("恢复出厂设置"), true,
                           QStringLiteral("指令已发送，设备重启后参数将恢复默认值"));
}

void UsrM0Client::onReadyRead()
{
    while (m_socket->hasPendingDatagrams()) {
        QByteArray datagram;
        datagram.resize(int(m_socket->pendingDatagramSize()));
        QHostAddress sender;
        quint16 senderPort = 0;
        m_socket->readDatagram(datagram.data(), datagram.size(), &sender, &senderPort);

        if (m_state == State::Searching) {
            UsrM0Protocol::Device device;
            if (UsrM0Protocol::parseSearchResponse(datagram, &device)
                    || UsrM0Protocol::parseAsciiSearchResponse(datagram, &device)) {
                const QString key = device.macString();
                if (!key.isEmpty() && !m_seenMacs.contains(key)) {
                    m_seenMacs.insert(key);
                    emit logMessage(QString("发现设备：%1  %2  %3")
                                    .arg(device.addressString(), key, device.name));
                    emit deviceFound(device);
                }
            }
            continue;
        }

        if (m_state == State::Reading) {
            // The read command answers with the raw parameter blocks, but a
            // device may also return a short acknowledgement first.
            quint8 ackCommand = 0;
            bool ackOk = false;
            QString ackMessage;
            if (UsrM0Protocol::isAck(datagram, &ackCommand, &ackOk, &ackMessage)) {
                if (!ackOk) {
                    failOperation(QStringLiteral("读取参数失败：") + ackMessage);
                    return;
                }
                continue;
            }

            if (sender == m_selectedDevice.address
                    || m_selectedDevice.address.isNull()) {
                m_readBuffer.append(datagram);
                emit logMessage(QString("收到参数数据 %1 字节（累计 %2）")
                                .arg(datagram.size())
                                .arg(m_readBuffer.size()));
                if (m_readBuffer.size() >= 130)
                    finishRead();
            }
            continue;
        }

        if (m_state == State::Writing) {
            if (sender != m_selectedDevice.address)
                continue;

            quint8 ackCommand = 0;
            bool ackOk = false;
            QString ackMessage;
            if (!UsrM0Protocol::isAck(datagram, &ackCommand, &ackOk, &ackMessage))
                continue;
            if (ackCommand != m_currentCommand)
                continue;

            m_ackTimer.stop();
            emit logMessage(QString("%1：%2").arg(m_currentLabel, ackMessage));
            if (!ackOk) {
                failOperation(QString("%1失败：%2").arg(m_currentLabel, ackMessage));
                return;
            }

            m_currentCommand = 0;
            QTimer::singleShot(150, this, &UsrM0Client::startNextStep);
        }
    }
}

void UsrM0Client::finishSearch()
{
    const int count = m_seenMacs.size();
    m_state = State::Idle;
    emit logMessage(QString("搜索完成，共发现 %1 台设备。").arg(count));
    emit searchFinished(count);
}

void UsrM0Client::finishRead()
{
    m_readTimer.stop();

    if (m_state != State::Reading)
        return;

    if (m_readBuffer.size() < 130) {
        failOperation(QString("读取参数失败：返回数据不足（%1 字节，至少需要 130 字节）")
                      .arg(m_readBuffer.size()));
        return;
    }

    const UsrM0Protocol::BasicParams basic =
            UsrM0Protocol::parseBasicParams(m_readBuffer.left(67));
    const UsrM0Protocol::PortParams port =
            UsrM0Protocol::parsePortParams(m_readBuffer.mid(67, 63));

    m_state = State::Idle;
    emit logMessage("参数读取成功（基础参数 67 字节，端口参数 63 字节）。");
    emit configRead(m_selectedDevice, basic, port);
    emit operationFinished(QStringLiteral("读取参数"), true,
                           QStringLiteral("参数读取成功"));
}

void UsrM0Client::onAckTimeout()
{
    if (m_state != State::Writing)
        return;
    failOperation(QString("%1 超时：设备无应答。").arg(m_currentLabel));
}

void UsrM0Client::sendTo(const QByteArray &payload, const QHostAddress &address, quint16 port)
{
    const qint64 written = m_socket->writeDatagram(payload, address, port);
    if (written < 0) {
        emit logMessage(QString("UDP 发送失败（%1:%2）：%3")
                        .arg(address.toString()).arg(port)
                        .arg(m_socket->errorString()));
    }
}

void UsrM0Client::startNextStep()
{
    if (m_state != State::Writing)
        return;

    if (m_steps.isEmpty()) {
        const bool restart = m_restartAfterSave;
        m_restartAfterSave = false;
        if (restart) {
            // Restart is fire-and-forget: the module may reboot before it can
            // acknowledge, so a missing ack must not be reported as a failure.
            const QByteArray frame = UsrM0Protocol::buildSimplePacket(
                0x02, m_selectedDevice.mac, m_user, m_password);
            sendTo(frame, m_selectedDevice.address, UsrM0Protocol::ConfigPort);
            emit logMessage(QString("已发送重启指令：%1")
                            .arg(QString::fromLatin1(frame.toHex(' ')).toUpper()));
        }
        m_state = State::Idle;
        emit logMessage(QString("%1 完成。").arg(m_operation));
        emit operationFinished(m_operation, true,
                               restart ? QStringLiteral("参数已保存，重启指令已发送")
                                       : QStringLiteral("参数已写入并保存"));
        return;
    }

    const Step step = m_steps.dequeue();
    m_currentCommand = step.command;
    m_currentLabel = step.label;

    sendTo(step.frame, m_selectedDevice.address, UsrM0Protocol::ConfigPort);
    emit logMessage(QString("→ %1：%2").arg(step.label,
                    QString::fromLatin1(step.frame.toHex(' ')).toUpper()));
    m_ackTimer.start(1500);
}

void UsrM0Client::failOperation(const QString &detail)
{
    m_ackTimer.stop();
    m_readTimer.stop();
    m_steps.clear();
    m_currentCommand = 0;
    m_restartAfterSave = false;
    m_state = State::Idle;
    emit logMessage(detail);
    emit operationFinished(m_operation.isEmpty() ? QStringLiteral("操作") : m_operation,
                           false, detail);
}
