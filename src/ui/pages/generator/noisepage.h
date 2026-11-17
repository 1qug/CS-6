#pragma once

#include <QWidget>

namespace Ui { class NoisePage; }

// Noise settings page. The current hardware does not support this feature;
// the page only documents that limitation.
class NoisePage : public QWidget
{
    Q_OBJECT

public:
    explicit NoisePage(QWidget *parent = nullptr);
    ~NoisePage() override;

signals:
    void backRequested();

private:
    Ui::NoisePage *ui = nullptr;
};
