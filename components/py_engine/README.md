@page py_engine py_engine

[更正文档](https://gitee.com/alios-things/py_engine/edit/master/README.md) &emsp;&emsp;&emsp;&emsp; [贡献说明](https://help.aliyun.com/document_detail/302301.html)

# 概述
py_engine (Python轻应用引擎) 以MicroPython为基础打造而成.可以快速实现IoT场景连云、控端、AI等最常见功能。快速上手文档和编程接口请参考[Python轻应用编程参考](https://g.alicdn.com/HaaSAI/PythonDoc/quickstart/index.html)
组件支持以下功能：
- 连云：支持基于linkSDK连接阿里云物联网平台
- 控端：支持PWD、UART、SPI、I2C、ADC、DAC、GPIO等基本硬件接口控制
- AI：支持端上AI和云端AI能力，覆盖人脸、人体、视频、文字等150+场景

## 版权信息
> Apache license v2.0

## 目录结构
```tree
py_engine
├── adaptor                 # 平台适配层
  |- haas
  |- haas506
  |- haas510
├── engine                  # 引擎核心层
├── external                # 外部适配库
├── framework               # Python 架构层，用来存放应用层适配文件
├── tests                   # 测试框架及测试集
├── package.yaml            # 编译配置文件
├── create_python_home.py   # 系统编译python文件，用来编译不同的文件到系统中
└── README.md               # 帮助文档
    
```

## 依赖组件
* rhino
* cli
* haas100
* osal_aos
* ulog
* kv
* mbedtls
* cjson
* fatfs
* haas1000
* vfs

# 常用配置
系统中相关配置已有默认值，如需修改配置，统一在yaml中**def_config**节点修改，具体如下：
> AOS_COMP_VFS: 默认1, 如需修改，在yaml中修改AOS_COMP_VFS配置

```yaml
def_config:
  AOS_COMP_VFS: 1
```
  > fatfs，默认1，可按照实际需要修改yaml配置如：
```yaml
  def_config:
    AOS_COMP_FATFS: 2
```

# API说明
@ref mpy_run
@ref mpy_init
@ref mpy_deinit

# 使用示例

组件使用示例相关的代码下载、编译和固件烧录均依赖AliOS Things配套的开发工具，所以首先需要参考[《AliOS Things集成开发环境使用说明之搭建开发环境》](https://help.aliyun.com/document_detail/302378.html)，下载安装。
待开发环境搭建完成后，可以按照以下步骤进行示例的测试。

## 步骤1 创建或打开工程

**打开已有工程**

如果用于测试的案例工程已存在，可参考[《AliOS Things集成开发环境使用说明之打开工程》](https://help.aliyun.com/document_detail/302381.html)打开已有工程。

**创建新的工程**

组件的示例代码可以通过编译链接到AliOS Things的任意案例（solution）来运行，这里选择helloworld_demo案例。helloworld_demo案例相关的源代码下载可参考[《AliOS Things集成开发环境使用说明之创建工程》](https://help.aliyun.com/document_detail/302379.html)。

## 步骤2 添加组件

案例下载完成后，需要在helloworld_demo组件的package.yaml中添加对组件的依赖：

```yaml
depends:
  - py_engine: master # helloworld_demo中引入py_engine组件
```

## 步骤3 下载组件

在已安装了  的开发环境工具栏中，选择Terminal -> New Terminal启动终端，并且默认工作路径为当前工程的workspace，此时在终端命令行中输入：

```shell

aos install py_engine

```

上述命令执行成功后，组件源码则被下载到了./components/py_engine路径中。

## 步骤4 添加示例

> 在py_engine组件的CMakeLists.txt 文件 file(GLOB SOURCEFILE "") 语句后面添加example配置:
```yaml
include(${EXAMPLEDIR}/aos.mk)
```

## 步骤5 编译固件

在示例代码已经添加至组件的配置文件，并且helloworld_demo已添加了对该组件的依赖后，就可以编译helloworld_demo案例来生成固件了，具体编译方法可参考[《AliOS Things集成开发环境使用说明之编译固件》](https://help.aliyun.com/document_detail/302384.html)。

## 步骤6 烧录固件

helloworld_demo案例的固件生成后，可参考[《AliOS Things集成开发环境使用说明之烧录固件》](https://help.aliyun.com/document_detail/302383.html)来烧录固件。

## 步骤7 打开串口

固件烧录完成后，可以通过串口查看示例的运行结果，打开串口的具体方法可参考[《AliOS Things集成开发环境使用说明之查看日志》](https://help.aliyun.com/document_detail/302382.html)。

当串口终端打开成功后，可在串口中输入help来查看已添加的测试命令。

## 步骤8 测试示例

> CLI命令行输入：
```sh
python
```

**关键日志**

> CLI日志：
```sh
start micropython!
```

# 注意事项
如果需要使用py_engine，就需要对接两个接口：**mpy_init** and **mpy_run**。(declare in `mpy_main.h`)
```

# FAQ
Q1： python 使用示例？
> from driver import ADC
> adc = ADC()
> adc.open("ADC0")
> value = adc.read()
> adc.close()
