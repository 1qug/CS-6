#include "core/protocol/siggenprotocol.h"

namespace SigGenProtocol {

QString rfOn()
{
    return QStringLiteral("CONFigure:RFOut 1\r\n");
}

QString rfOff()
{
    return QStringLiteral("CONFigure:RFOut 0\r\n");
}

QString setFrequency(quint64 hz)
{
    return QString("CONFigure:FREQuency %1\r\n").arg(hz);
}

QString setAmplitude(double dbm)
{
    return QString("CONFigure:POWer %1\r\n").arg(dbm, 0, 'f', 1);
}

QString setReference(int channel)
{
    return QString("CONFigure:REFchannel %1\r\n").arg(channel);
}

QString setModulation(bool enabled)
{
    return enabled ? QStringLiteral("CONFigure:RFModulation 1\r\n")
                  : QStringLiteral("CONFigure:RFModulation 0\r\n");
}

QString setModulationTypeAsk()
{
    return QStringLiteral("CONFigure:RFModulation:TYPE 3\r\n");
}

QString setModulationFrequency(int index)
{
    return QString("CONFigure:RFModulation:FREQuency %1\r\n").arg(index);
}

QString setModulationDuty(int percent)
{
    return QString("CONFigure:RFModulation:DUTY %1\r\n").arg(percent);
}

QString setSweepStartFrequency(quint64 hz)
{
    return QString("CONFigure:SCANFrequency:STARt %1\r\n").arg(hz);
}

QString setSweepStopFrequency(quint64 hz)
{
    return QString("CONFigure:SCANFrequency:STOP %1\r\n").arg(hz);
}

QString setSweepStepFrequency(quint64 hz)
{
    return QString("CONFigure:SCANFrequency:STEP %1\r\n").arg(hz);
}

QString setSweepStartAmplitude(double dbm)
{
    return QString("CONFigure:SCANPower:STARt %1\r\n").arg(dbm, 0, 'f', 1);
}

QString setSweepStopAmplitude(double dbm)
{
    return QString("CONFigure:SCANPower:STOP %1\r\n").arg(dbm, 0, 'f', 1);
}

QString setSweepStepAmplitude(double dbm)
{
    return QString("CONFigure:SCANPower:STEP %1\r\n").arg(dbm, 0, 'f', 1);
}

QString setSweepInterval(int milliseconds)
{
    return QString("CONFigure:SCANInterval %1\r\n").arg(milliseconds);
}

QString queryState()
{
    return QStringLiteral("MEASure:STATe?\r\n");
}

QString queryTemperature()
{
    return QStringLiteral("MEASure:TEMPerature?\r\n");
}

QString queryLockState()
{
    return QStringLiteral("MEASure:LOCKstate?\r\n");
}

QString queryReferenceState()
{
    return QStringLiteral("MEASure:REFstate?\r\n");
}

QString queryFanState()
{
    return QStringLiteral("MEASure:FANstate?\r\n");
}

bool parseStatusLine(const QString &line, DeviceStatus *status)
{
    if (!status)
        return false;

    const QString text = line.trimmed();
    const int colon = text.indexOf(':');
    if (colon < 0)
        return false;

    const QString key = text.left(colon).trimmed().toLower();
    const QString value = text.mid(colon + 1).trimmed();

    if (key == QLatin1String("lock state")) {
        status->lockState = value;
    } else if (key == QLatin1String("ref state")) {
        status->referenceState = value;
    } else if (key == QLatin1String("temperatue")
               || key == QLatin1String("temperature")) {
        status->temperature = value;
    } else if (key == QLatin1String("fan state")) {
        status->fanState = value;
    } else {
        return false;
    }
    return true;
}

} // namespace SigGenProtocol
