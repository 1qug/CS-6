#pragma once

#include <QByteArray>
#include <QHostAddress>
#include <QString>

// Data structures and packet helpers for the USR-M0 / USR-K6 style network
// configuration protocol (UDP port 1500, frame: FF | length | command | MAC |
// username | password | parameters | checksum).
namespace UsrM0Protocol {

constexpr quint16 ConfigPort = 1500;
constexpr quint16 AtPort = 48899;

struct Device {
    QHostAddress address;
    QByteArray mac; // 6 bytes
    QString name;
    QString version;

    QString macString() const;
    QString addressString() const { return address.toString(); }
};

struct BasicParams {
    bool staticIp = true;
    QHostAddress staticIpAddress;
    QHostAddress gateway;
    QHostAddress subnet;
    QString deviceName;
    quint16 httpPort = 80;
    quint16 locationUrlPort = 0;
    QByteArray raw; // 67-byte basic parameter block (unknown bytes preserved)

    static BasicParams defaults();
};

struct PortParams {
    quint32 baudRate = 115200;
    quint8 dataBits = 8;
    quint8 stopBits = 1;
    quint8 parity = 1;      // 1=NONE, 2=ODD, 3=EVEN, 4=MARK, 5=SPACE
    quint8 flowControl = 0; // preserved from the device
    quint8 workMode = 1;    // 0=UDP client, 1=TCP client, 2=UDP server, 3=TCP server, 4=HTTPD
    quint16 localPort = 20108;
    quint16 remotePort = 8234;
    QHostAddress remoteAddress;
    QByteArray raw; // 63-byte port parameter block (unknown bytes preserved)

    static PortParams defaults();
};

QByteArray buildSearchPacket();
QByteArray buildReadPacket(const QByteArray &mac, const QString &user, const QString &pass);
QByteArray buildSimplePacket(quint8 command, const QByteArray &mac,
                             const QString &user, const QString &pass);
QByteArray buildWritePacket(quint8 command, const QByteArray &mac,
                            const QString &user, const QString &pass,
                            const QByteArray &params);

bool parseSearchResponse(const QByteArray &datagram, Device *device);
bool parseAsciiSearchResponse(const QByteArray &datagram, Device *device);

// Parses "FF 01 <cmd> <status>" acknowledgements. status 'K' = OK,
// 'P' = wrong password, 'E' = checksum/command error.
bool isAck(const QByteArray &datagram, quint8 *command, bool *ok,
           QString *message = nullptr);

BasicParams parseBasicParams(const QByteArray &raw);
QByteArray encodeBasicParams(const BasicParams &params);
PortParams parsePortParams(const QByteArray &raw);
QByteArray encodePortParams(const PortParams &params);

quint8 checksum(const QByteArray &data, int fromIndex, int toIndex);

} // namespace UsrM0Protocol
