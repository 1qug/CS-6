# CS-6 射频信号发生器上位机

本程序是 **CS-6 便携式射频信号发生器** 的 PC 端控制软件，同时集成
USR-M0 网口模块的配置功能。产品界面、窗口标题、可执行文件和工程名称
统一使用 **CS-6 射频信号发生器**。

信号源指令集依据 `doc/` 中的原始协议资料实现：

- `doc/YSF-6000B射频信号发生器软件开发手册V1.1.pdf`（指令与状态格式）
- `doc/YSF-6000B射频信号发生器使用说明书V1.1.pdf`
- `doc/CS-6型便携式射频信号发生器设计方案-V1.0.doc`
- `doc/USR-M0 V2.2.6.3/`（网口模块配置工具及协议资料）

## 功能

### 信号源控制

- **两种链路**：USB 虚拟串口（FT232RL，8, N, 1；波特率可在设备连接页
  选择标准值或直接输入自定义值，默认 115200）或 LAN TCP。
- **基础控制**：输出频率（24 MHz ~ 6.5 GHz，1 Hz 步进）、输出功率
  （-95 ~ +15 dBm，0.5 dBm 步进）、射频开关（`CONFigure:RFOut 1` /
  `CONFigure:RFOut 0`）、参考源（内部 / 外部 1、2、5、10、20、50、100 MHz，
  `CONFigure:REFchannel 0` ~ `CONFigure:REFchannel 7`）。
- **调制模式**：界面提供 AM/FM/PM/ASK/FSK/PSK 与内部/外部通道选项，
  按本机规格仅内部 ASK 可选（`CONFigure:RFModulation:TYPE 3`）；调制开关
  `CONFigure:RFModulation 1/0`、调制频率 `CONFigure:RFModulation:FREQuency 0`~`3`、
  占空比 `CONFigure:RFModulation:DUTY 10`~`90`。
- **频率 / 功率扫描**：`CONFigure:SCANFrequency:STARt/STOP/STEP`、
  `CONFigure:SCANPower:STARt/STOP/STEP`、`CONFigure:SCANInterval`。
- **状态查询**：`MEASure:STATe?`（锁定、参考、温度、风扇），以及
  `MEASure:TEMPerature?`、`MEASure:LOCKstate?`、`MEASure:REFstate?`、
  `MEASure:FANstate?` 单项查询；连接后自动查询一次状态。
- **指令格式**：每条 SCPI 指令以 CR LF（`0x0D0A`）结尾。
- **指令节流**：所有指令排队发送，间隔 600 ms，满足手册“>500 ms”的要求。
- **指令预览与日志**：左侧显示最近指令和回传，底部日志记录 TX/RX。

#### 信号源页面结构

- **设备连接页**：选择串口/TCP、连接设备、设备运行
- **控制主页**：频率/功率大号显示、频率与功率设置、6 个功能卡片、设备状态查询
- **功能子页**：参考设置、调制模式、扫描模式、噪声设置、系统信息、参数校准
- 每个子页都有“返回主页”；左侧“首页”和“设备连接”按钮可随时切换
- 连接成功后“设备运行”可用，点击进入控制主页
- 设备断开时自动回到设备连接页

### 网口模块配置

对应 USR-M0 工具中与信号源组网相关的功能：

- **搜索设备**：UDP 广播（配置端口 1500，兼容 48899 的 `WWW.USR.CN` 搜索），
  列出设备 IP、MAC、名称、版本。
- **读取参数**：`FF 13 03 ...`，返回基础参数（67 字节）与端口参数（63 字节）。
- **保存参数**：`FF 13 05 ...`（基础参数）、`FF 13 06 ...`（端口参数）、
  `FF 13 04 ...`（保存），可选保存后重启（`FF 13 02 ...`）。
- **可配置项**：IP 类型（静态/DHCP）、模块 IP、网关、子网掩码、设备名称、
  HTTP 端口、波特率、数据位、校验位、停止位、工作方式、本地/远程端口、目标 IP。
- **其他操作**：重启设备、恢复出厂设置、打开设备网页、导出设备列表 CSV。
- **安全策略**：未成功读取参数前禁止保存，写入时对未知字段原样回写，
  避免破坏模块中未公开的参数。

## 目录结构

```
CS-6/
├── src/
│   ├── main.cpp                    # 组合根：初始化应用、主题、主窗口
│   ├── app/
│   │   └── appinfo.h               # 产品名称/组织/版本等唯一来源
│   ├── core/
│   │   ├── protocol/
│   │   │   ├── siggenprotocol.h/.cpp   # 信号源指令与状态解析
│   │   │   └── usrm0protocol.h/.cpp    # USR-M0 帧构造/解析、参数编解码
│   │   ├── siggenclient.h/.cpp     # 链路管理 + 600 ms 指令队列
│   │   └── usrm0client.h/.cpp      # UDP 搜索/读取/写入状态机
│   ├── transport/
│   │   ├── itransport.h            # 字节流抽象
│   │   ├── serialtransport.*       # QSerialPort 实现
│   │   └── tcptransport.*          # QTcpSocket 实现
│   └── ui/
│       ├── mainwindow/             # 主窗口（标题栏、页签、日志）
│       ├── pages/generator/        # 信号源工作区
│       │   ├── generatorpage.*     # 工作区外壳：侧边栏 + 堆叠页
│       │   ├── connectionpage.*    # 设备连接、运行
│       │   ├── homepage.*          # 大号参数显示、基础设置、功能卡片
│       │   ├── referencepage.*     # 参考源设置
│       │   ├── modulationpage.*    # ASK 调制设置
│       │   ├── sweeppage.*         # 频率/功率扫描
│       │   ├── noisepage.*         # 噪声设置（不支持提示）
│       │   ├── systeminfopage.*    # 系统信息
│       │   ├── calibrationpage.*   # 参数校准（保留）
│       │   └── generatorpages.h    # 堆叠页索引定义
│       ├── pages/lanconfig/        # 网口模块配置页
│       └── theme.h/.cpp            # QSS 加载与动态属性刷新
├── resources/
│   ├── resources.qrc               # 样式与图标资源
│   ├── styles/app.qss              # 全部界面渲染规则
│   └── icons/*.svg                 # RF 开关、勾选、下拉箭头
├── build/                          # Debug/Release 输出与 Qt 运行库
├── doc/                            # 协议、使用与设计文档
├── CS-6.pro
├── CS-6.vcxproj
└── CS-6.sln
```

## 架构说明

程序按“界面 → 业务服务 → 链路”分层，协议层保持纯函数：

1. **app**：应用元数据。`main.cpp` 只负责组装对象。
2. **core/protocol**：指令字符串、USR-M0 帧和参数块的编解码，不依赖界面。
3. **core/siggenclient、usrm0client**：业务状态机。`SigGenClient` 自己创建并
   持有传输对象（串口/TCP），界面不再直接操作 `ITransport`；指令队列与
   600 ms 节流都集中在这里。
4. **transport**：`ITransport` 抽象出字节流，`SerialTransport` 与
   `TcpTransport` 各自实现打开、关闭、收发和错误上报。
5. **ui**：页面只把控件动作映射成服务调用，并把服务信号渲染回控件。
   页面不再重复保存 `ui->` 成员指针；`uic` 生成的 `ui_*.h` 是唯一控件入口。
6. **resources**：主题与控件外观集中在 `app.qss`，C++ 中不再拼接样式表。

### 信号源页面拆分

`generatorpage` 只保留侧边栏、`QStackedWidget` 和页面编排逻辑。每个堆叠页
都是独立的自定义控件，拥有自己的 `.ui`、`.h`、`.cpp`：

| 页面 | 控件类 | 职责 |
| --- | --- | --- |
| 设备连接 | `ConnectionPage` | 链路选择、连接/断开、运行入口 |
| 控制主页 | `HomePage` | 频率/功率显示与设置、RF 开关、功能卡片、状态查询 |
| 参考设置 | `ReferencePage` | 参考源选择 |
| 调制模式 | `ModulationPage` | ASK 开关、类型、频率、占空比 |
| 扫描模式 | `SweepPage` | 频率/功率扫描参数 |
| 噪声设置 | `NoisePage` | 不支持提示 |
| 系统信息 | `SystemInfoPage` | 型号、频率/功率范围、接口 |
| 参数校准 | `CalibrationPage` | 保留功能 |

页面通过信号向 `GeneratorPage` 请求操作（如 `frequencyApplyRequested`、
`navigateRequested`），`GeneratorPage` 统一调用 `SigGenClient`，页面之间
不直接依赖。Visual Studio 的筛选器树与 `src/`、`resources/` 的实际目录
一一对应，便于按文件夹浏览工程。

## 主题与控件渲染

- 界面采用石墨黑深色主题：分层炭黑渐变、细描边与信号琥珀色强调色；
  `Theme::apply()` 同时设置深色调色板，保证 Fusion 自绘控件与 QSS 一致。
- `resources/styles/app.qss` 定义全局字体、配色、标题栏、页签、卡片、
  按钮、输入框、下拉框、复选框、表格、滚动条、日志窗口等全部外观。
- RF 输出开关是一个 `QCheckBox#rfToggle`，其 `::indicator` 的四种状态
  （开/关 × 正常/禁用）通过 QSS 引用 `resources/icons/*.svg` 渲染，
  不再使用自绘控件。
- 状态点、连接状态文字等运行时状态通过动态属性表达：
  `connected="true"`，由 `Theme::repolish()` 在属性变化后刷新；
  C++ 不调用 `setStyleSheet()`。
- QSS 以 Qt 资源形式编译进可执行文件，部署时无额外样式文件。

## 构建

### Visual Studio 2022

1. 安装 Qt 6.11.1 MSVC 2022 64-bit 与 Qt VS Tools。
2. 打开 `CS-6.sln`，选择 `Debug|x64` 或 `Release|x64` 生成。
3. 输出为 `build/Debug/CS-6.exe`、`build/Release/CS-6.exe`。
4. 工程已启用 `core;gui;widgets;network;serialport;svg` 模块。

命令行构建示例：

```bat
msbuild CS-6.vcxproj /p:Configuration=Release /p:Platform=x64 ^
  /p:QTDIR=E:\Qt\6.11.1\msvc2022_64 ^
  /p:QtMsBuild=%LOCALAPPDATA%\QtMsBuild
```

### qmake / Qt Creator

`CS-6.pro` 已同步源码、表单、资源与模块；Debug/Release 产物分别写入
`build/debug`、`build/release`。

## 运行

- 直接运行 `build/Release/CS-6.exe`；Qt 运行库已通过 `windeployqt`
  部署到输出目录。
- 本程序不包含演示或模拟模式：未连接真实硬件时，控制项保持禁用；
  网口模块页只有搜索到真实设备后才能读取/保存参数。
- 串口控制需要安装 FT232RL 驱动。本程序使用 QtSerialPort，不依赖 NI-VISA。
- LAN 控制时，先用“网口模块配置”页把模块设为静态 IP（如 `192.168.0.14`）、
  本地端口（如 `1234`），保存并重启；再到“信号源控制”页以 TCP 连接
  `IP:端口`。

## 实现说明与限制

- 信号源指令集完全依据开发手册第 3 章；手册未给出“扫描启动/停止”指令，
  因此界面只负责下发扫描起始、终止、步进和间隔参数。
- USR-M0 参数块布局依据 USR-M0/K6 系列公开的设置协议：
  基础参数 67 字节、端口参数 63 字节，已知字段偏移为
  基础参数 `flags=3`、`staticIP=9`、`gateway=13`、`subnet=17`、`name=21`；
  端口参数 `baud=0..3`、`data=4`、`stop=5`、`parity=6`、`workMode=7`、
  `localPort=12`、`remotePort=14`、目标 IP 字符串 `=16..45`、目标 IP 二进制 `=46..49`。
  未公开字段在保存时原样回写。首次连接真实模块后建议先“读取参数”核对显示值。
- 固件升级和串口配置仅提供操作提示，实际升级/串口配置需使用原厂
  USR-M0 工具。
