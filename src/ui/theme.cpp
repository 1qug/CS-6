#include "ui/theme.h"

#include <QApplication>
#include <QColor>
#include <QFile>
#include <QFont>
#include <QPalette>
#include <QStyle>
#include <QStyleFactory>
#include <QWidget>
#include <QtGlobal>

namespace {

constexpr auto kStyleSheetResource = ":/styles/app.qss";

QString loadStyleSheet()
{
    QFile file(QString::fromLatin1(kStyleSheetResource));
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning("CS-6: cannot open embedded stylesheet %s", kStyleSheetResource);
        return QString();
    }
    return QString::fromUtf8(file.readAll());
}

} // namespace

namespace Theme {

void apply(QApplication &app)
{
    // The native Windows 11 style ignores several stylesheet rules (tab and
    // menu colours in particular), so use Fusion for a consistent theme.
    if (QStyle *fusion = QStyleFactory::create(QStringLiteral("Fusion")))
        app.setStyle(fusion);

    // Keep the native-drawn parts (spin-box arrows, menu check marks, dialog
    // chrome) in step with the dark stylesheet.
    QPalette palette;
    palette.setColor(QPalette::Window, QColor(QStringLiteral("#0C1014")));
    palette.setColor(QPalette::WindowText, QColor(QStringLiteral("#E6ECF2")));
    palette.setColor(QPalette::Base, QColor(QStringLiteral("#0B0F13")));
    palette.setColor(QPalette::AlternateBase, QColor(QStringLiteral("#11171D")));
    palette.setColor(QPalette::ToolTipBase, QColor(QStringLiteral("#171E25")));
    palette.setColor(QPalette::ToolTipText, QColor(QStringLiteral("#E6ECF2")));
    palette.setColor(QPalette::Text, QColor(QStringLiteral("#E6ECF2")));
    palette.setColor(QPalette::Button, QColor(QStringLiteral("#171E25")));
    palette.setColor(QPalette::ButtonText, QColor(QStringLiteral("#E6ECF2")));
    palette.setColor(QPalette::BrightText, QColor(QStringLiteral("#FFFFFF")));
    palette.setColor(QPalette::Highlight, QColor(QStringLiteral("#B9852F")));
    palette.setColor(QPalette::HighlightedText, QColor(QStringLiteral("#0B0F13")));
    palette.setColor(QPalette::Link, QColor(QStringLiteral("#E7B75F")));
    palette.setColor(QPalette::PlaceholderText, QColor(QStringLiteral("#5F6B78")));
    palette.setColor(QPalette::Disabled, QPalette::WindowText,
                     QColor(QStringLiteral("#68727E")));
    palette.setColor(QPalette::Disabled, QPalette::Text,
                     QColor(QStringLiteral("#68727E")));
    palette.setColor(QPalette::Disabled, QPalette::ButtonText,
                     QColor(QStringLiteral("#68727E")));
    palette.setColor(QPalette::Disabled, QPalette::Button,
                     QColor(QStringLiteral("#12171C")));
    palette.setColor(QPalette::Disabled, QPalette::Base,
                     QColor(QStringLiteral("#101419")));
    app.setPalette(palette);

    QFont font(QStringLiteral("Microsoft YaHei"));
    font.setPointSize(9);
    app.setFont(font);

    const QString styleSheet = loadStyleSheet();
    if (!styleSheet.isEmpty())
        app.setStyleSheet(styleSheet);
}

void repolish(QWidget *widget)
{
    if (!widget)
        return;

    widget->style()->unpolish(widget);
    widget->style()->polish(widget);
    widget->update();
}

} // namespace Theme
