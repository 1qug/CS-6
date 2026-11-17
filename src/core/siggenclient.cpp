#include "core/siggenclient.h"

#include "transport/itransport.h"
#include "transport/serialtransport.h"
#include "transport/tcptransport.h"

SigGenClient::SigGenClient(QObject *parent)
    : QObject(parent)
{
    m_timer.setSingleShot(true);
    m_timer.setInterval(600);
    connect(&m_timer, &QTimer::timeout, this, &SigGenClient::processNext);
}

bool SigGenClient::openSerial(const QString &portName, int baudRate)
{
    close();

    auto *transport = new SerialTransport(this);
    transport->setBaudRate(baudRate);
    setTransport(transport);
    if (!transport->open(portName)) {
        setTransport(nullptr);
        transport->deleteLater();
        return false;
    }
    return true;
}

bool SigGenClient::openTcp(const QString &host, quint16 port)
{
    close();

    auto *transport = new TcpTransport(this);
    setTransport(transport);
    return transport->open(host, port);
}

void SigGenClient::close()
{
    if (!m_transport)
        return;

    ITransport *transport = m_transport;
    transport->close();      // emits closed() -> connectionChanged(false)
    setTransport(nullptr);
    transport->deleteLater();
}

void SigGenClient::setTransport(ITransport *transport)
{
    if (m_transport)
        disconnect(m_transport, nullptr, this, nullptr);

    m_transport = transport;
    m_queue.clear();
    m_rxBuffer.clear();
    m_sending = false;
    m_timer.stop();

    if (!m_transport)
        return;

    connect(m_transport, &ITransport::dataReceived, this, &SigGenClient::handleData);
    connect(m_transport, &ITransport::opened, this, [this]() {
        emit connectionChanged(true, m_transport->description());
    });
    connect(m_transport, &ITransport::closed, this, [this]() {
        m_timer.stop();
        m_sending = false;
        m_queue.clear();
        emit connectionChanged(false, m_transport->description());
    });
    connect(m_transport, &ITransport::errorOccurred, this, [this](const QString &message) {
        emit logMessage(QString("链路错误：%1").arg(message));
    });
}

bool SigGenClient::isConnected() const
{
    return m_transport && m_transport->isOpen();
}

void SigGenClient::sendCommand(const QString &command, const QString &note)
{
    if (!isConnected()) {
        emit logMessage("未连接设备，指令未发送。");
        return;
    }

    m_queue.enqueue({command, note});
    if (!m_sending)
        processNext();
}

void SigGenClient::clearPending()
{
    m_queue.clear();
}

void SigGenClient::processNext()
{
    if (!isConnected()) {
        m_sending = false;
        return;
    }

    if (m_queue.isEmpty()) {
        m_sending = false;
        return;
    }

    const PendingCommand pending = m_queue.dequeue();
    m_sending = true;

    emit commandSent(pending.command, pending.note);

    // Protocol commands already end with CR LF; append the terminator only for
    // a bare payload so every command written to the device ends with 0x0D0A.
    QByteArray payload = pending.command.toUtf8();
    if (!payload.endsWith("\r\n"))
        payload.append("\r\n");
    m_transport->write(payload);

    m_timer.start();
}

void SigGenClient::handleData(const QByteArray &data)
{
    m_rxBuffer.append(data);

    // Accept CRLF as well as lone CR/LF line endings.
    m_rxBuffer.replace("\r\n", "\n");
    m_rxBuffer.replace('\r', '\n');

    int newline = m_rxBuffer.indexOf('\n');
    while (newline >= 0) {
        QByteArray rawLine = m_rxBuffer.left(newline);
        m_rxBuffer.remove(0, newline + 1);
        if (rawLine.endsWith('\r'))
            rawLine.chop(1);
        const QString line = QString::fromUtf8(rawLine).trimmed();
        if (!line.isEmpty())
            emit responseLine(line);
        newline = m_rxBuffer.indexOf('\n');
    }

    if (m_rxBuffer.size() > 4096)
        m_rxBuffer.clear();
}
