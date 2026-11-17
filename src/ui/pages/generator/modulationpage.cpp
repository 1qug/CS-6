#include "ui/pages/generator/modulationpage.h"
#include "ui_modulationpage.h"

#include "app/appinfo.h"

#include <QComboBox>
#include <QPushButton>
#include <QStandardItemModel>

ModulationPage::ModulationPage(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ModulationPage)
{
    ui->setupUi(this);

    for (int i = 0; i < ui->modFrequencyCombo->count(); ++i)
        ui->modFrequencyCombo->setItemData(i, i);
    for (int i = 0; i < ui->modDutyCombo->count(); ++i)
        ui->modDutyCombo->setItemData(i, 10 * (i + 1));

    if (auto *model = qobject_cast<QStandardItemModel *>(ui->modTypeCombo->model())) {
        for (int i = 0; i < model->rowCount(); ++i) {
            if (i != 3)
                model->item(i)->setEnabled(false);
        }
    }
    if (auto *model = qobject_cast<QStandardItemModel *>(ui->modChannelCombo->model())) {
        if (model->rowCount() > 1)
            model->item(1)->setEnabled(false);
    }

    connect(ui->backModulationButton, &QPushButton::clicked,this, &ModulationPage::backRequested);
    connect(ui->modOnButton, &QPushButton::clicked,this, [this]() { emit modulationRequested(true); });
    connect(ui->modOffButton, &QPushButton::clicked,this, [this]() { emit modulationRequested(false); });
    connect(ui->askButton, &QPushButton::clicked, this, [this]() {
        if (ui->modTypeCombo->currentIndex() != 3) {
            emit logMessage(QStringLiteral("%1 仅支持 ASK 调制。").arg(AppInfo::shortName()));
            ui->modTypeCombo->setCurrentIndex(3);
            return;
        }
        emit typeApplyRequested();
    });
    connect(ui->modFrequencyApplyButton, &QPushButton::clicked, this, [this]() {
        emit frequencyApplyRequested(ui->modFrequencyCombo->currentData().toInt());
    });
    connect(ui->modDutyApplyButton, &QPushButton::clicked, this, [this]() {
        emit dutyApplyRequested(ui->modDutyCombo->currentData().toInt());
    });
}

ModulationPage::~ModulationPage()
{
    delete ui;
}
