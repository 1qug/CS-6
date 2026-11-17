#pragma once

#include <QWidget>

namespace Ui { class ReferencePage; }

// Reference source selection page.
class ReferencePage : public QWidget
{
    Q_OBJECT

public:
    explicit ReferencePage(QWidget *parent = nullptr);
    ~ReferencePage() override;

    QString currentText() const;

signals:
    void backRequested();
    void applyRequested(int channel);
    void selectionChanged(const QString &text);

private:
    Ui::ReferencePage *ui = nullptr;
};
