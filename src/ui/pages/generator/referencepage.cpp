#include "ui/pages/generator/referencepage.h"
#include "ui_referencepage.h"

#include <QComboBox>
#include <QPushButton>

ReferencePage::ReferencePage(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ReferencePage)
{
    ui->setupUi(this);

    for (int i = 0; i < ui->referenceCombo->count(); ++i)
        ui->referenceCombo->setItemData(i, i);

    connect(ui->backReferenceButton, &QPushButton::clicked,
            this, &ReferencePage::backRequested);
    connect(ui->referenceApplyButton, &QPushButton::clicked, this, [this]() {
        emit applyRequested(ui->referenceCombo->currentData().toInt());
    });
    connect(ui->referenceCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [this]() { emit selectionChanged(ui->referenceCombo->currentText()); });
}

ReferencePage::~ReferencePage()
{
    delete ui;
}

QString ReferencePage::currentText() const
{
    return ui->referenceCombo->currentText();
}
