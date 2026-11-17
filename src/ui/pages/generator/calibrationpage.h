#pragma once

#include <QWidget>

namespace Ui { class CalibrationPage; }

// Parameter calibration page, reserved for future hardware support.
class CalibrationPage : public QWidget
{
    Q_OBJECT

public:
    explicit CalibrationPage(QWidget *parent = nullptr);
    ~CalibrationPage() override;

signals:
    void backRequested();

private:
    Ui::CalibrationPage *ui = nullptr;
};
