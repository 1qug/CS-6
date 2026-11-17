#include "ui/pages/generator/systeminfopage.h"
#include "ui_systeminfopage.h"

#include <QPushButton>

SystemInfoPage::SystemInfoPage(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::SystemInfoPage)
{
    ui->setupUi(this);

    connect(ui->backSystemInfoButton, &QPushButton::clicked,
            this, &SystemInfoPage::backRequested);
}

SystemInfoPage::~SystemInfoPage()
{
    delete ui;
}
