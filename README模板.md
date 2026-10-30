# 项目名称（STM32F103 呼吸灯 + 串口打印）

基于 STM32F103C8T6 的 PWM 呼吸灯实验，通过 USART1 输出运行状态。

---

## 目录

- [硬件平台](#硬件平台)
- [开发环境](#开发环境)
- [目录结构](#目录结构)
- [编译与烧录](#编译与烧录)
- [引脚分配](#引脚分配)
- [外设说明](#外设说明)
- [调试](#调试)
- [常见问题](#常见问题)
- [Markdown 语法速查](#markdown-语法速查)
- [License](#license)

---

## 硬件平台

| 项目 | 参数 |
|------|------|
| 主控芯片 | STM32F103C8T6 |
| 内核 | ARM Cortex-M3 |
| 主频 | 72 MHz（外部 8 MHz HSE × 9 PLL） |
| Flash | 64 KB |
| RAM | 20 KB |
| 开发板 | 金思特 / 最小系统板 |
| 调试器 | ST-Link V2 / DAP-Link / J-Link |

### 硬件连接

```
DAPLink：
VCC 3.3V
GND
CLK
DIO

PA9  ──→  USART1_TX  ──→  USB-TTL RX
PA10 ←──  USART1_RX  ←──  USB-TTL TX
PC13 ──→  板载LED 正极
GND  ──→  板载LED LED 负极
```

---

## 开发环境

| 工具 | 版本 / 说明 |
|------|-------------|
| 编辑器 | **VSCode 1.141.2**|
| 编译器 | **arm-none-eabi-gcc（14.3 rel1）** |
| 构建系统 | **Makefile** |
| 烧录工具 | **OpenOCD / st-flash / ST-Link Utility** |
| HAL 库 | **STM32CubeF1 Firmware Package** |
| 代码生成 | **STM32CubeMX 6.18.1** **SYS Debug: Serial Wire**
| 开发语言 | **C/C++（基于 HAL 库封装）** |

### VSCode 插件

- **C/C++**（Microsoft）— 语法提示与调试
- **Cortex-Debug** — 在线调试（配合 OpenOCD）
- **Makefile Tools** — 生成make文件
- **Include Autocomplete** — 自动补全提示
- **Markdown All in One** — 本文件预览与目录生成

`.vscode/tasks.json` 中可配置 `make` 任务，实现 `Ctrl+Shift+B` 一键编译。

### 复制MCU专用文件带目录
```
项目根目录/
├── .vscode/
tasks.json   #编译命令
launch.json  #编译和下载配置文件
└──STM32F103.svd
   修改 launch.json，将xxx.elf 全部修改为你项目文件对应的 项目文件.elf
  
  根据自己的下载器选择调试下载类型：
   DAPLink
   STLink

## 目录结构
项目根目录/
├── .vscode/                # VSCode 配置（tasks.json, launch.json, c_cpp_properties.json）
├── Core/                   # 用户代码
│   ├── Inc/                # 头文件
│   │   ├── main.hpp
│   │   ├── bsp.hpp
│   │   └── config.hpp      # 项目宏配置
│   └── Src/                # 源文件
│       ├── main.cpp
│       ├── bsp.cpp
│       └── usart.cpp
├── Drivers/                # HAL/库文件（CubeMX 生成）
│   ├── STM32F1xx_HAL_Driver/
│   └── CMSIS/
├── startup/                # 启动文件
│   └── startup_stm32f103xb.s
├── linker/                 # 链接脚本
│   └── STM32F103C8Tx_FLASH.ld
├── build/                  # 编译输出（gitignore）
├── Makefile                # 构建脚本
└── README.md               # 本文件
```

---

## 编译与烧录

### 1. 编译

```bash
# 直接在项目根目录编译（Makefile 已配置好输出目录）
make -j

# 只编译不链接（检查语法）
make -j

# 清理编译产物
make clean

# 完整重新编译
make clean && make -j
```

常用 Makefile 目标（按实际 Makefile 定义调整）：

| 目标 | 作用 |
|------|------|
| `make` | 编译生成 `.elf` |
| `make hex` | 额外生成 `.hex` |
| `make bin` | 额外生成 `.bin` |
| `make flash` | 编译 + OpenOCD 烧录 |
| `make clean` | 删除 `build/` 产物 |

编译成功后会在 `build/` 下生成 `.elf`、`.hex`、`.bin` 文件。

### 2. 烧录（选一种）

**方式一：Makefile 一键烧录**

```bash
make flash
```

**方式二：OpenOCD 手动烧录**

```bash
openocd -f interface/stlink-v2.cfg -f target/stm32f1x.cfg \
  -c "program build/xxx.elf verify reset exit"
```

**方式三：st-flash**

```bash
st-flash write build/xxx.bin 0x08000000
```

**方式四：VSCode 一键操作**

- 按 `Ctrl + Shift + B` 选择 `Build` 编译。
- 按 `F5` 直接编译 + 烧录 + 启动调试。

---

## 引脚分配

| 引脚 | 功能 | 说明 |
|------|------|------|
| PA9 | USART1_TX | 串口发送，接 USB-TTL RX |
| PA10 | USART1_RX | 串口接收，接 USB-TTL TX |
| PA13 | SWDIO | 调试器数据线（勿挪用） |
| PA14 | SWCLK | 调试器时钟线（勿挪用） |
| PC13 | GPIO / TIM PWM | 呼吸灯输出 |
| 3.3V | 电源 | 外部模块供电 |
| GND | 地 | 共地 |

> 注意：PA13/PA14 默认被 SWD 调试占用，做普通 GPIO 使用前需在代码里关闭 JTAG，否则会断开调试连接。

---

## 外设说明

### USART1
- 波特率：115200
- 数据位：8
- 停止位：1
- 校验：无
- 用途：打印调试信息

### TIM3 PWM
- 通道：CH1（PC13）
- 频率：1 kHz
- 用途：LED 呼吸灯

---

## 调试

1. 连接 ST-Link V2或者DAPLink 到 SWD 引脚（3.3V / GND / SWDIO / SWCLK）。
2. VSCode 按 `F5` 启动调试。
3. 常用快捷键：
   - `F5` — 继续
   - `F10` — 单步跳过
   - `F11` — 单步进入
   - `Shift + F5` — 停止调试

串口调试查看：

```bash
# Linux / macOS
minicom -D /dev/ttyUSB0 -b 115200
```

Windows 用 PuTTY / MobaXterm / 串口助手，波特率 115200，8N1。

---

## 常见问题

**Q: 编译报错 `arm-none-eabi-gcc: command not found`**
A: 确认工具链已安装，并把 `arm-none-eabi-gcc/bin` 目录加到系统 PATH。

**Q: 烧录后芯片没反应**
A: 检查 BOOT0 跳线是否接 GND（从 Flash 启动）；外部晶振是否为 8 MHz；时钟配置是否匹配。

**Q: SWD 连不上**
A: 确认 PA13/PA14 没被代码改成普通 GPIO；可以在复位瞬间连接 OpenOCD 强制擦除。

**Q: C++ 中全局对象构造函数没执行**
A: 检查启动文件是否调用 `__libc_init_array`，以及链接脚本中 `.init_array` 段是否保留。

**Q: printf 没有串口输出**
A: 重定向 `fputc` 到 USART1；确认串口助手波特率与代码一致；TX/RX 是否接反。

**Q: 新增了 .cpp 文件，make 不编译它**
A: 在 Makefile 的 `C_SOURCES` 或 `CXX_SOURCES` 变量中添加该文件路径，然后 `make clean && make`。

**Q: Makefile 报 `No rule to make target`**
A: 检查源文件路径是否写对；确认 `.vscode` 或 Makefile 中 `ARM_NONE_EABI_PATH` 等变量指向正确的工具链位置。

---

## Markdown 语法速查

本文件本身就是一个 Markdown 示例，写文档时可参考：

| 需求 | 语法 |
|------|------|
| 一级标题 | `# 标题` |
| 二级标题 | `## 标题` |
| 无序列表 | `- 文字` |
| 有序列表 | `1. 文字` |
| 粗体 | `**文字**` |
| 斜体 | `*文字*` |
| 删除线 | `~~文字~~` |
| 行内代码 | `` `code` `` |
| 代码块 | 三个反引号包裹 |
| 链接 | `[文字](url)` |
| 图片 | `![描述](图片路径)` |
| 引用 | `> 文字` |
| 表格 | `| 列1 | 列2 |` |
| 分割线 | `---` |
| 任务列表 | `- [ ] 待办` |

VSCode 中按 `Ctrl + Shift + V` 可实时预览本文件。

---

## License

本项目基于 MIT License 开源，详情见 [LICENSE](./LICENSE) 文件。

---

## 作者
- 单位：湖南大学人工智能26级
- 作者：杨科莱
- 联系方式：18597991076
