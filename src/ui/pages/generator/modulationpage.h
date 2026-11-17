#pragma once

#include <QWidget>

namespace Ui { class ModulationPage; }

// ASK modulation settings page.
class ModulationPage : public QWidget
{
    Q_OBJECT

public:
    explicit ModulationPage(QWidget *parent = nullptr);
    ~ModulationPage() override;

signals:
    void backRequested();
    void logMessage(const QString &message);
    void modulationRequested(bool enabled);
    void typeApplyRequested();
    void frequencyApplyRequested(int index);
    void dutyApplyRequested(int percent);

private:
    Ui::ModulationPage *ui = nullptr;
};
