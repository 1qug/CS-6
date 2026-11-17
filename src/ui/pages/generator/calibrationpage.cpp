#include "ui/pages/generator/calibrationpage.h"
#include "ui_calibrationpage.h"

#include <QPushButton>

CalibrationPage::CalibrationPage(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::CalibrationPage)
{
    ui->setupUi(this);

    connect(ui->backCalibrationButton, &QPushButton::clicked,this, &CalibrationPage::backRequested);
}

CalibrationPage::~CalibrationPage()
{
    delete ui;
}
