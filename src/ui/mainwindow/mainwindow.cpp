#include "ui/mainwindow/mainwindow.h"
#include "ui_mainwindow.h"

#include "app/appinfo.h"
#include "ui/pages/generator/generatorpage.h"
#include "ui/pages/lanconfig/lanconfigpage.h"
#include "ui/theme.h"

#include <QAction>
#include <QDateTime>
#include <QFontDatabase>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QStatusBar>
#include <QTextCursor>
#include <QtGlobal>

#ifdef Q_OS_WIN
#  ifndef NOMINMAX
#    define NOMINMAX
#  endif
#  include <windows.h>
#endif

namespace {

#ifdef Q_OS_WIN
void enableDarkTitleBar(HWND window)
{
    if (!window)
        return;

    HMODULE dwmapi = LoadLibraryW(L"dwmapi.dll");
    if (!dwmapi)
        return;

    using DwmSetWindowAttributeFn = HRESULT (WINAPI *)(HWND, DWORD, LPCVOID, DWORD);
    const auto setAttribute = reinterpret_cast<DwmSetWindowAttributeFn>(
        GetProcAddress(dwmapi, "DwmSetWindowAttribute"));

    if (setAttribute) {
        BOOL dark = TRUE;
        // Attribute 20 is used by Windows 10 2004+ and Windows 11; older
        // Windows 10 builds recognise attribute 19.
        if (FAILED(setAttribute(window, 20, &dark, sizeof(dark))))
            setAttribute(window, 19, &dark, sizeof(dark));
    }

    FreeLibrary(dwmapi);
}
#endif

} // namespace

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowTitle(AppInfo::productName());
    setWindowIcon(QIcon(":/icons/ico.png"));

#ifdef Q_OS_WIN
    enableDarkTitleBar(reinterpret_cast<HWND>(winId()));
#endif
    connect(ui->generatorPage, &GeneratorPage::logMessage,this, &MainWindow::appendLog);
    connect(ui->lanConfigPage, &LanConfigPage::logMessage,this, &MainWindow::appendLog);
    connect(ui->generatorPage, &GeneratorPage::connectionStateChanged,this, &MainWindow::setConnectionState);

    ui->logView->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));

    connect(ui->clearLogButton, &QPushButton::clicked,ui->logView, &QPlainTextEdit::clear);
    connect(ui->copyLogButton, &QPushButton::clicked, this, [this]() 
    {
        ui->logView->selectAll();
        ui->logView->copy();
        ui->logView->moveCursor(QTextCursor::End);
    });

    resizeDocks({ui->logDock}, {190}, Qt::Vertical);

    connect(ui->actionQuit, &QAction::triggered, this, &QWidget::close);
    connect(ui->actionAbout, &QAction::triggered, this, [this]() {
        QMessageBox::about(this, QStringLiteral("关于 %1").arg(AppInfo::shortName()),
            QStringLiteral(
                "<b>%1</b><br><br>"
                "• USB 虚拟串口（可调波特率，默认 115200, 8N1）与 LAN TCP 两种控制方式<br>"
                "• 频率、功率、射频开关、参考源、ASK 调制与扫频参数设置<br>"
                "• 锁定/参考/温度/风扇状态查询<br>"
                "• USR-M0 网口模块搜索、参数读取/保存与设备管理<br><br>"
                "版本 %2")
                .arg(AppInfo::productDescription(), AppInfo::version()));
    });

    statusBar()->showMessage(QStringLiteral("就绪"));
    appendLog(QStringLiteral("程序启动。信号源串口默认 115200, 8, N, 1，"
                             "可在设备连接页修改波特率；LAN 设备默认 192.168.0.7，"
                             "配置端口 1500。"));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::appendLog(const QString &message)
{
    const QString timestamp = QDateTime::currentDateTime().toString(QStringLiteral("HH:mm:ss.zzz"));
    ui->logView->appendPlainText(QString("[%1] %2").arg(timestamp, message));
}

void MainWindow::setConnectionState(bool connected)
{
    ui->headerStatusLabel->setText(connected ? QStringLiteral("已连接")
                                             : QStringLiteral("未连接"));
    ui->statusDot->setProperty("connected", connected);
    Theme::repolish(ui->statusDot);
}
