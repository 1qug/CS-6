#pragma once

#include <QString>

// SCPI command set for the CS-6. The short legacy codes are mapped to their
// SCPI equivalents as documented in doc/指令表.doc. Every command is returned
// as a complete line terminated with CR LF (0x0D 0x0A), as required by
// doc/YSF-6000B射频信号发生器软件开发手册V1.1.pdf section 3.1.
namespace SigGenProtocol {

struct DeviceStatus {
    QString lockState;
    QString referenceState;
    QString temperature;
    QString fanState;
};

QString rfOn();
QString rfOff();

QString setFrequency(quint64 hz);
QString setAmplitude(double dbm);

QString setReference(int channel);

QString setModulation(bool enabled);
QString setModulationTypeAsk();
QString setModulationFrequency(int index);
QString setModulationDuty(int percent);

QString setSweepStartFrequency(quint64 hz);
QString setSweepStopFrequency(quint64 hz);
QString setSweepStepFrequency(quint64 hz);
QString setSweepStartAmplitude(double dbm);
QString setSweepStopAmplitude(double dbm);
QString setSweepStepAmplitude(double dbm);
QString setSweepInterval(int milliseconds);

QString queryState();
QString queryTemperature();
QString queryLockState();
QString queryReferenceState();
QString queryFanState();

// Updates |status| from one response line (e.g. "Lock state:Locked").
bool parseStatusLine(const QString &line, DeviceStatus *status);

} // namespace SigGenProtocol
