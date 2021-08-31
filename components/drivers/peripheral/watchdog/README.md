@page watchdog watchdog

**[更正文档](https://gitee.com/alios-things/watchdog/edit/master/README.md)** &emsp;&emsp;&emsp;&emsp; **[贡献说明](https://help.aliyun.com/document_detail/302301.html)**

# 概述
> WATCHDOG VFS驱动子系统，该驱动的目的是给应用或组件提供通过VFS形式（open/close/ioctl）访问WATCHDOG控制器驱动对上层提供复位看门狗的接口。
> 该组件初始化过程中，会根据hardware/chip/<chip_name>/package.yaml中定义的CONFIG_WDG_NUM来依此创建如下名称的设备节点：

* /dev/wdg0
* /dev/wdg1
* ...
* /dev/wdg[CONFIG_WDG_NUM - 1]

> WATCHDOG VFS驱动子系统内部会将open/close/ioctl请求转换成对应硬件的HAL层接口调用，通过HAL层接口发起复位看门狗的请求。

## 版权信息
> Apache license v2.0

## 目录结构
```tree
│── src
│   └── wdg_dev.c      # WATCHDOG VFS驱动程序源代码
├── example            # WATCHDOG VFS驱动程序使用案例
│   └── wdg_example.c  # 通过WATCHDOG VFS接口进行WATCHDOG通信的案例文件
├── include            # WATCHDOG驱动子系统对外提供头文件目录
│   ├── aos
│   │   └── hal
│   │       └── wdg.h  # AOS HAL API头文件声明，本文件中的API是给厂商对接HAL API的原型声明
│   └── vfsdev
│       └── wdg_dev.h  # WATCHDOG VFS驱动子系统对应用/组件提供的接口声明文件
└── package.yaml       # WATCHDOG VFS驱动子系统的组件配置文件

```

## 依赖组件
* base     # 最底层核心驱动模型、驱动自动加载机制以及Device VFS core
* vfs      # VFS API抽象库

# 常用配置
> 本组件可以通过CONFIG_WDG_NUM配置对上层提供设备节点的数量，CONFIG_WDG_NUM代表WATCHDOG设备节点的数量。
> CONFIG_WDG_NUM默认是在hardware/chip/<chip_name>/package.yaml中的**define**节点进行配置。
> 如果没有定义CONFIG_WDG_NUM，则代码中会定义默认对外输出的设备节点数量。
> 设备节点数量: 默认1个, 如需修改，在hardware/chip/<chip_name>/package.yaml中修改CONFIG_WDG_NUM配置，一般来说一个产品只需要一个watchdog功能
```sh
define:
  CONFIG_WDG_NUM: 1
```

# API说明
## 打开设备节点
```c
int open(const char *pathname, int flags);
```

|args                                    |description|
|:-----                                  |:----|
|pathname                                |看门狗外设VFS路径|
|flags                                   |目前固定为0值|

兼容POSIX标准的open接口。其中参数*pathname*为*/dev/wdg*加看门狗外设id，例如*/dev/wdg0*。

## 关闭设备节点
```c
int close(int fd);
```
兼容POSIX标准的close接口。

## 复位看门狗
在应用层需要复位看门狗操作时呼叫此API
```c
int ioctl(int fd, IOC_WDG_RELOAD, unsigned long arg);
// arg固定传入0即可
ioctl (fd, IOC_WDG_RELOAD, 0);
```

# 使用示例

组件使用示例相关的代码下载、编译和固件烧录均依赖AliOS Things配套的开发工具，所以首先需要参考[《AliOS Things集成开发环境使用说明之搭建开发环境》](https://help.aliyun.com/document_detail/302378.html)，下载安装。
待开发环境搭建完成后，可以按照以下步骤进行示例的测试。

## 步骤1 创建或打开工程

**打开已有工程**

如果用于测试的案例工程已存在，可参考[《AliOS Things集成开发环境使用说明之打开工程》](https://help.aliyun.com/document_detail/302381.html)打开已有工程。

**创建新的工程**

组件的示例代码可以通过编译链接到AliOS Things的任意案例（solution）来运行，这里选择helloworld_demo案例。helloworld_demo案例相关的源代码下载可参考[《AliOS Things集成开发环境使用说明之创建工程》](https://help.aliyun.com/document_detail/302379.html)。

## 步骤2 添加组件
> 如果芯片内部含有WATCHDOG控制器，则芯片厂在操作系统对接的时候已经将WATCHDOG组件添加在了芯片级别配置文件“hardware/chip/<chip_name>/package.yaml”中，此配置主要包含“设置组件依赖关系”及“设置芯片内部所含WATCHDOG控制器数量”两个配置。
```yaml
# 设置组件依赖关系
depends:
  - watchdog: master

# 设置芯片内部所含WATCHDOG控制器数量
define:
  CONFIG_WDG_NUM: 1
```

## 步骤3 下载组件

在已安装了  的开发环境工具栏中，选择Terminal -> New Terminal启动终端，并且默认工作路径为当前工程的workspace，此时在终端命令行中输入：

```shell

aos install watchdog

```

上述命令执行成功后，组件源码则被下载到了./components/drivers/peripheral/watchdog路径中。

## 步骤4 添加示例

在i2c组件的package.yaml中添加[example示例代码](https://gitee.com/alios-things/watchdog/tree/master/example)：

```yaml
source_file:
#WATCHDOG device driver
  - src/wdg_dev.c ? <CONFIG_U_WATCHDOG_DEV>
#WATCHDOG VFS driver example
- example/wdg_example.c ? <CONFIG_U_WATCHDOG_DEV>
```

## 步骤5 编译固件

在示例代码已经添加至组件的配置文件，并且helloworld_demo已添加了对该组件的依赖后，就可以编译helloworld_demo案例来生成固件了，具体编译方法可参考[《AliOS Things集成开发环境使用说明之编译固件》](https://help.aliyun.com/document_detail/302384.html)。

## 步骤6 烧录固件

helloworld_demo案例的固件生成后，可参考[《AliOS Things集成开发环境使用说明之烧录固件》](https://help.aliyun.com/document_detail/302383.html)来烧录固件。

## 步骤7 打开串口

固件烧录完成后，可以通过串口查看示例的运行结果，打开串口的具体方法可参考[《AliOS Things集成开发环境使用说明之查看日志》](https://help.aliyun.com/document_detail/302382.html)。

当串口终端打开成功后，可在串口中输入help来查看已添加的测试命令。

## 步骤8 测试示例

**CLI命令行输入：**
```shell
wdg_reload <count> <period, in unit of ms>
# 向WATCHDOG通道0（watchdog_example.c中设定）以200ms为周期连续输出15个重置看门狗的信号
wdg_reload 15 200
```

> 关键日志
```shell
wdg comp output test success!
```

# 注意事项
> WATCHDOG测试是输出硬件信号复位看门狗，可以通过示波器来量测watchdog复位引脚的波形图

# FAQ
> 无

