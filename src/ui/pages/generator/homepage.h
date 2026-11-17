#pragma once

#include <QWidget>

namespace Ui { class HomePage; }

// Control home page: large frequency/power readout, basic settings, function
// card navigation and device status queries.
class HomePage : public QWidget
{
    Q_OBJECT

public:
    explicit HomePage(QWidget *parent = nullptr);
    ~HomePage() override;

    void setReferenceSummary(const QString &text);
    void setModulationSummary(const QString &text);
    void setSweepSummary(const QString &text);
    void setLastCommand(const QString &command, const QString &note = QString());
    void resetRfState();

signals:
    void frequencyApplyRequested(quint64 hz);
    void amplitudeApplyRequested(double dbm);
    void rfToggleRequested(bool enabled);
    void navigateRequested(int page);
    void statusQueryRequested();
    void temperatureQueryRequested();
    void lockQueryRequested();
    void referenceQueryRequested();
    void fanQueryRequested();

private slots:
    void refreshParameterStrip();

private:
    Ui::HomePage *ui = nullptr;
};
