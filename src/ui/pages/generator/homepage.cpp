#include "ui/pages/generator/homepage.h"
#include "ui_homepage.h"

#include "ui/pages/generator/generatorpages.h"

#include <QAbstractButton>
#include <QDoubleSpinBox>
#include <QPushButton>
#include <QSignalBlocker>

namespace {

QString formatMHz(double value)
{
    QString text = QString::number(value, 'f', 6);
    while (text.contains(QLatin1Char('.')) && text.endsWith(QLatin1Char('0')))
        text.chop(1);
    if (text.endsWith(QLatin1Char('.')))
        text.chop(1);
    return text + QStringLiteral(" MHz");
}

} // namespace

HomePage::HomePage(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::HomePage)
{
    ui->setupUi(this);
    ui->rfToggle->setCursor(Qt::PointingHandCursor);

    connect(ui->frequencySpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),this, &HomePage::refreshParameterStrip);
    connect(ui->amplitudeSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),this, &HomePage::refreshParameterStrip);

    connect(ui->frequencyApplyButton, &QPushButton::clicked, this, [this]() {
        const quint64 hz = quint64(qRound64(ui->frequencySpin->value() * 1000000.0));
        emit frequencyApplyRequested(hz);
    });
    connect(ui->amplitudeApplyButton, &QPushButton::clicked, this, [this]() {
        emit amplitudeApplyRequested(ui->amplitudeSpin->value());
    });
    connect(ui->rfToggle, &QAbstractButton::toggled, this, [this](bool enabled) {
        ui->rfStateLabel->setText(enabled ? QStringLiteral("开启") : QStringLiteral("关闭"));
        emit rfToggleRequested(enabled);
    });

    connect(ui->enterReferenceButton, &QPushButton::clicked, this, [this]() {
        emit navigateRequested(GeneratorPages::Reference);
    });
    connect(ui->enterModulationButton, &QPushButton::clicked, this, [this]() {
        emit navigateRequested(GeneratorPages::Modulation);
    });
    connect(ui->enterSweepButton, &QPushButton::clicked, this, [this]() {
        emit navigateRequested(GeneratorPages::Sweep);
    });
    connect(ui->enterNoiseButton, &QPushButton::clicked, this, [this]() {
        emit navigateRequested(GeneratorPages::Noise);
    });
    connect(ui->enterSystemInfoButton, &QPushButton::clicked, this, [this]() {
        emit navigateRequested(GeneratorPages::SystemInfo);
    });
    connect(ui->enterCalibrationButton, &QPushButton::clicked, this, [this]() {
        emit navigateRequested(GeneratorPages::Calibration);
    });

    connect(ui->stateButton, &QPushButton::clicked,this, &HomePage::statusQueryRequested);
    connect(ui->temperatureButton, &QPushButton::clicked,this, &HomePage::temperatureQueryRequested);
    connect(ui->lockButton, &QPushButton::clicked,this, &HomePage::lockQueryRequested);
    connect(ui->referenceButton, &QPushButton::clicked,this, &HomePage::referenceQueryRequested);
    connect(ui->fanButton, &QPushButton::clicked,this, &HomePage::fanQueryRequested);

    refreshParameterStrip();
}

HomePage::~HomePage()
{
    delete ui;
}

void HomePage::setReferenceSummary(const QString &text)
{
    ui->referenceSummaryLabel->setText(text);
}

void HomePage::setModulationSummary(const QString &text)
{
    ui->modulationSummaryLabel->setText(text);
}

void HomePage::setSweepSummary(const QString &text)
{
    ui->sweepSummaryLabel->setText(text);
}

void HomePage::setLastCommand(const QString &command, const QString &note)
{
    ui->lastCommandLabel->setText(QString("最近指令：%1%2").arg(
        note.isEmpty() ? QString() : note + QStringLiteral("  "), command));
}

void HomePage::resetRfState()
{
    QSignalBlocker blocker(ui->rfToggle);
    ui->rfToggle->setChecked(false);
    ui->rfStateLabel->setText(QStringLiteral("关闭"));
}

void HomePage::refreshParameterStrip()
{
    ui->freqValueLabel->setText(formatMHz(ui->frequencySpin->value()));
    ui->ampValueLabel->setText(
        QString::number(ui->amplitudeSpin->value(), 'f', 1) + QStringLiteral(" dBm"));
}
