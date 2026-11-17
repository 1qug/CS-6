#include "ui/pages/generator/noisepage.h"
#include "ui_noisepage.h"

#include <QPushButton>

NoisePage::NoisePage(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::NoisePage)
{
    ui->setupUi(this);

    connect(ui->backNoiseButton, &QPushButton::clicked,
            this, &NoisePage::backRequested);
}

NoisePage::~NoisePage()
{
    delete ui;
}
