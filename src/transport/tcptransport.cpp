#include "transport/tcptransport.h"

TcpTransport::TcpTransport(QObject *parent)
    : ITransport(parent)
{
    connect(&m_socket, &QTcpSocket::connected, this, [this]() {
        emit opened();
    });
    connect(&m_socket, &QTcpSocket::disconnected, this, [this]() {
        emit closed();
    });
    connect(&m_socket, &QTcpSocket::readyRead, this, [this]() {
        emit dataReceived(m_socket.readAll());
    });
    connect(&m_socket, &QTcpSocket::errorOccurred, this,
            [this](QAbstractSocket::SocketError) {
        emit errorOccurred(m_socket.errorString());
    });
}

bool TcpTransport::open(const QString &target, quint16 port)
{
    if (m_socket.state() != QAbstractSocket::UnconnectedState)
        m_socket.abort();

    m_socket.connectToHost(target, port);
    return true;
}

void TcpTransport::close()
{
    if (m_socket.state() != QAbstractSocket::UnconnectedState)
        m_socket.abort();
}

bool TcpTransport::isOpen() const
{
    return m_socket.state() == QAbstractSocket::ConnectedState;
}

qint64 TcpTransport::write(const QByteArray &data)
{
    return m_socket.write(data);
}

QString TcpTransport::description() const
{
    return QString("TCP %1:%2").arg(m_socket.peerName()).arg(m_socket.peerPort());
}
