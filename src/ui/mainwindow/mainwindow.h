#pragma once

#include <QMainWindow>

namespace Ui { class MainWindow; }

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

public slots:
    void appendLog(const QString &message);
    void setConnectionState(bool connected);

private:
    Ui::MainWindow *ui = nullptr;
};
