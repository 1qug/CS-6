#include "core/protocol/usrm0protocol.h"

#include <QStringList>

namespace {

QByteArray paddedCredential(const QString &value)
{
    QByteArray bytes = value.toUtf8().left(6);
    bytes.resize(6);
    return bytes;
}

void writeLe16(QByteArray &data, int offset, quint16 value)
{
    data[offset] = char(value & 0xFF);
    data[offset + 1] = char((value >> 8) & 0xFF);
}

void writeLe32(QByteArray &data, int offset, quint32 value)
{
    data[offset] = char(value & 0xFF);
    data[offset + 1] = char((value >> 8) & 0xFF);
    data[offset + 2] = char((value >> 16) & 0xFF);
    data[offset + 3] = char((value >> 24) & 0xFF);
}

quint16 readLe16(const QByteArray &data, int offset)
{
    return quint16(quint8(data[offset]))
         | quint16(quint8(data[offset + 1])) << 8;
}

quint32 readLe32(const QByteArray &data, int offset)
{
    return quint32(quint8(data[offset]))
         | quint32(quint8(data[offset + 1])) << 8
         | quint32(quint8(data[offset + 2])) << 16
         | quint32(quint8(data[offset + 3])) << 24;
}

quint32 ipToLeValue(const QHostAddress &address)
{
    return address.toIPv4Address();
}

QHostAddress leValueToIp(const QByteArray &data, int offset)
{
    return QHostAddress(readLe32(data, offset));
}

void writeIpLe(QByteArray &data, int offset, const QHostAddress &address)
{
    writeLe32(data, offset, ipToLeValue(address));
}

QByteArray normalized(const QByteArray &raw, int size)
{
    QByteArray result = raw.left(size);
    result.resize(size);
    return result;
}

QString nullTerminatedString(const QByteArray &data, int offset, int length)
{
    const QByteArray slice = data.mid(offset, length);
    int end = slice.indexOf('\0');
    if (end < 0)
        end = slice.size();
    return QString::fromLatin1(slice.constData(), end).trimmed();
}

void writeFixedString(QByteArray &data, int offset, int length, const QString &text)
{
    const QByteArray bytes = text.toLatin1().left(length);
    for (int i = 0; i < length; ++i)
        data[offset + i] = i < bytes.size() ? bytes.at(i) : '\0';
}

} // namespace

namespace UsrM0Protocol {

QString Device::macString() const
{
    QStringList parts;
    for (char byte : mac)
        parts << QString("%1").arg(quint8(byte), 2, 16, QChar('0')).toUpper();
    return parts.join(':');
}

BasicParams BasicParams::defaults()
{
    BasicParams params;
    params.staticIp = true;
    params.staticIpAddress = QHostAddress(QStringLiteral("192.168.0.7"));
    params.gateway = QHostAddress(QStringLiteral("192.168.0.1"));
    params.subnet = QHostAddress(QStringLiteral("255.255.255.0"));
    params.deviceName = QStringLiteral("USR-M0");
    params.httpPort = 80;
    params.raw = QByteArray(67, '\0');
    params = parseBasicParams(encodeBasicParams(params));
    return params;
}

PortParams PortParams::defaults()
{
    PortParams params;
    params.baudRate = 115200;
    params.dataBits = 8;
    params.stopBits = 1;
    params.parity = 1;
    params.workMode = 1;
    params.localPort = 20108;
    params.remotePort = 8234;
    params.remoteAddress = QHostAddress(QStringLiteral("192.168.0.201"));
    params.raw = QByteArray(63, '\0');
    params = parsePortParams(encodePortParams(params));
    return params;
}

quint8 checksum(const QByteArray &data, int fromIndex, int toIndex)
{
    quint8 sum = 0;
    for (int i = fromIndex; i <= toIndex && i < data.size(); ++i)
        sum = quint8(sum + quint8(data.at(i)));
    return sum;
}

QByteArray buildSearchPacket()
{
    return QByteArray::fromHex("FF010102");
}

QByteArray buildReadPacket(const QByteArray &mac, const QString &user, const QString &pass)
{
    return buildSimplePacket(0x03, mac, user, pass);
}

QByteArray buildSimplePacket(quint8 command, const QByteArray &mac,
                             const QString &user, const QString &pass)
{
    return buildWritePacket(command, mac, user, pass, QByteArray());
}

QByteArray buildWritePacket(quint8 command, const QByteArray &mac,
                            const QString &user, const QString &pass,
                            const QByteArray &params)
{
    QByteArray frame;
    frame.append(char(0xFF));

    QByteArray macBytes = mac.left(6);
    macBytes.resize(6);

    const quint8 length = quint8(1 + 6 + 6 + 6 + params.size());
    frame.append(char(length));
    frame.append(char(command));
    frame.append(macBytes);
    frame.append(paddedCredential(user));
    frame.append(paddedCredential(pass));
    frame.append(params);
    frame.append(char(checksum(frame, 1, frame.size() - 1)));
    return frame;
}

bool parseSearchResponse(const QByteArray &datagram, Device *device)
{
    if (!device || datagram.size() < 19)
        return false;
    if (quint8(datagram.at(0)) != 0xFF || quint8(datagram.at(2)) != 0x01)
        return false;

    const quint8 b0 = quint8(datagram.at(5));
    const quint8 b1 = quint8(datagram.at(6));
    const quint8 b2 = quint8(datagram.at(7));
    const quint8 b3 = quint8(datagram.at(8));

    device->address = QHostAddress(QString("%1.%2.%3.%4").arg(b0).arg(b1).arg(b2).arg(b3));
    device->mac = datagram.mid(9, 6);

    const quint16 version = quint16(quint8(datagram.at(15)) << 8)
                          | quint8(datagram.at(16));
    device->version = QString("0x%1").arg(version, 4, 16, QChar('0')).toUpper();

    QByteArray nameBytes = datagram.mid(19, 16);
    const int nul = nameBytes.indexOf('\0');
    if (nul >= 0)
        nameBytes.truncate(nul);
    device->name = QString::fromLatin1(nameBytes).trimmed();
    if (device->name.isEmpty())
        device->name = QStringLiteral("USR-M0");

    return !device->address.isNull();
}

bool parseAsciiSearchResponse(const QByteArray &datagram, Device *device)
{
    if (!device)
        return false;

    const QString text = QString::fromLatin1(datagram).trimmed();
    const QStringList parts = text.split(',');
    if (parts.size() < 2)
        return false;

    QHostAddress address;
    if (!address.setAddress(parts.at(0).trimmed()))
        return false;

    QString macText = parts.at(1).trimmed();
    macText.remove(':');
    macText.remove('-');
    if (macText.size() != 12)
        return false;
    bool macOk = false;
    const qulonglong macValue = macText.toULongLong(&macOk, 16);
    if (!macOk)
        return false;

    device->address = address;
    device->mac.clear();
    for (int shift = 40; shift >= 0; shift -= 8)
        device->mac.append(char((macValue >> shift) & 0xFF));
    device->name = parts.size() > 2 ? parts.at(2).trimmed() : QString();
    device->version = parts.size() > 3 ? parts.at(3).trimmed() : QString();
    if (device->name.isEmpty())
        device->name = QStringLiteral("USR-M0");
    return true;
}

bool isAck(const QByteArray &datagram, quint8 *command, bool *ok, QString *message)
{
    // Acknowledgements are exactly four bytes: FF 01 <command> <status>.
    if (datagram.size() != 4)
        return false;
    if (quint8(datagram.at(0)) != 0xFF || quint8(datagram.at(1)) != 0x01)
        return false;

    const quint8 cmd = quint8(datagram.at(2));
    const quint8 status = quint8(datagram.at(3));
    if (command)
        *command = cmd;

    const bool success = (status == 'K');
    if (ok)
        *ok = success;

    if (message) {
        switch (status) {
        case 'K':
            *message = QStringLiteral("成功");
            break;
        case 'P':
            *message = QStringLiteral("用户名或密码错误");
            break;
        case 'E':
            *message = QStringLiteral("指令或校验错误");
            break;
        default:
            *message = QString("设备返回状态 0x%1").arg(status, 2, 16, QChar('0')).toUpper();
            break;
        }
    }
    return true;
}

BasicParams parseBasicParams(const QByteArray &raw)
{
    BasicParams params;
    const QByteArray block = normalized(raw, 67);

    params.raw = block;
    params.staticIp = (quint8(block.at(3)) & 0x80) != 0;
    params.locationUrlPort = readLe16(block, 4);
    params.httpPort = readLe16(block, 6);
    params.staticIpAddress = leValueToIp(block, 9);
    params.gateway = leValueToIp(block, 13);
    params.subnet = leValueToIp(block, 17);
    params.deviceName = nullTerminatedString(block, 21, 16);

    if (params.staticIpAddress.isNull())
        params.staticIpAddress = QHostAddress(QStringLiteral("192.168.0.7"));
    if (params.gateway.isNull())
        params.gateway = QHostAddress(QStringLiteral("192.168.0.1"));
    if (params.subnet.isNull())
        params.subnet = QHostAddress(QStringLiteral("255.255.255.0"));
    return params;
}

QByteArray encodeBasicParams(const BasicParams &params)
{
    QByteArray block = normalized(params.raw, 67);

    block[3] = char((quint8(block.at(3)) & ~0x80)
                    | (params.staticIp ? 0x80 : 0x00));
    writeLe16(block, 4, params.locationUrlPort);
    writeLe16(block, 6, params.httpPort);
    writeIpLe(block, 9, params.staticIpAddress);
    writeIpLe(block, 13, params.gateway);
    writeIpLe(block, 17, params.subnet);
    writeFixedString(block, 21, 16, params.deviceName);
    return block;
}

PortParams parsePortParams(const QByteArray &raw)
{
    PortParams params;
    const QByteArray block = normalized(raw, 63);

    params.raw = block;
    params.baudRate = readLe32(block, 0);
    params.dataBits = quint8(block.at(4));
    params.stopBits = quint8(block.at(5));
    params.parity = quint8(block.at(6));
    params.workMode = quint8(block.at(7));
    params.localPort = readLe16(block, 12);
    params.remotePort = readLe16(block, 14);
    params.remoteAddress = leValueToIp(block, 46);
    if (params.remoteAddress.isNull()) {
        const QString host = nullTerminatedString(block, 16, 30);
        QHostAddress address;
        if (address.setAddress(host))
            params.remoteAddress = address;
    }
    if (params.remoteAddress.isNull())
        params.remoteAddress = QHostAddress(QStringLiteral("192.168.0.201"));
    if (params.baudRate < 300 || params.baudRate > 4000000)
        params.baudRate = 115200;
    if (params.workMode > 4)
        params.workMode = 1;
    return params;
}

QByteArray encodePortParams(const PortParams &params)
{
    QByteArray block = normalized(params.raw, 63);

    writeLe32(block, 0, params.baudRate);
    block[4] = char(params.dataBits);
    block[5] = char(params.stopBits);
    block[6] = char(params.parity);
    block[7] = char(params.workMode);
    writeLe16(block, 12, params.localPort);
    writeLe16(block, 14, params.remotePort);
    writeFixedString(block, 16, 30, params.remoteAddress.toString());
    writeIpLe(block, 46, params.remoteAddress);
    return block;
}

} // namespace UsrM0Protocol
