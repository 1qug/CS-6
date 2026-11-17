#include "ui/pages/generator/sweeppage.h"
#include "ui_sweeppage.h"

#include <QComboBox>
#include <QDoubleSpinBox>
#include <QPushButton>
#include <QSpinBox>
#include <QStackedWidget>

SweepPage::SweepPage(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::SweepPage)
{
    ui->setupUi(this);

    connect(ui->backSweepButton, &QPushButton::clicked,
            this, &SweepPage::backRequested);
    connect(ui->sweepModeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            ui->sweepStack, &QStackedWidget::setCurrentIndex);
    connect(ui->sweepModeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [this]() { emit modeChanged(ui->sweepModeCombo->currentText()); });
    connect(ui->sweepApplyButton, &QPushButton::clicked, this, [this]() {
        if (ui->sweepModeCombo->currentIndex() == 0) {
            emit frequencySweepRequested(
                quint64(qRound64(ui->freqSweepStart->value() * 1000000.0)),
                quint64(qRound64(ui->freqSweepStop->value() * 1000000.0)),
                quint64(ui->freqSweepStep->value()),
                ui->freqSweepInterval->value());
        } else {
            emit powerSweepRequested(ui->powerSweepStart->value(),
                                     ui->powerSweepStop->value(),
                                     ui->powerSweepStep->value(),
                                     ui->powerSweepInterval->value());
        }
    });
}

SweepPage::~SweepPage()
{
    delete ui;
}

QString SweepPage::currentModeText() const
{
    return ui->sweepModeCombo->currentText();
}
