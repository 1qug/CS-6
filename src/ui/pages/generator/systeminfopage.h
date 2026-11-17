#pragma once

#include <QWidget>

namespace Ui { class SystemInfoPage; }

// Read-only product and interface information page.
class SystemInfoPage : public QWidget
{
    Q_OBJECT

public:
    explicit SystemInfoPage(QWidget *parent = nullptr);
    ~SystemInfoPage() override;

signals:
    void backRequested();

private:
    Ui::SystemInfoPage *ui = nullptr;
};
