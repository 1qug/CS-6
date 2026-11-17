#pragma once

#include <QWidget>

namespace Ui { class SweepPage; }

// Frequency / power sweep parameter page.
class SweepPage : public QWidget
{
    Q_OBJECT

public:
    explicit SweepPage(QWidget *parent = nullptr);
    ~SweepPage() override;

    QString currentModeText() const;

signals:
    void backRequested();
    void frequencySweepRequested(quint64 startHz, quint64 stopHz,
                                 quint64 stepHz, int intervalMs);
    void powerSweepRequested(double startDbm, double stopDbm,
                             double stepDbm, int intervalMs);
    void modeChanged(const QString &text);

private:
    Ui::SweepPage *ui = nullptr;
};
