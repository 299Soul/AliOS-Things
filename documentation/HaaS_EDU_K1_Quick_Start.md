@page HaaS_EDU_K1_Quick_Start HaaS EDU K1快速开始

具体操作分为三步：

- 搭建开发环境
- 创建工程
- 开发及调试

# 1. 搭建开发环境
## Visual Studio Code
Visual Studio Code(下称VS Code) 是一款广泛使用的IDE工具。AliOS-Things 3.3的开发环境是主要基于VS Code搭建的。

安装VS Code:
下载链接: [https://code.visualstudio.com/docs/?dv=osx](https://code.visualstudio.com/docs/?dv=osx)


## AliOS-Studio
AliOS-Studio是开发AliOS Things所需的核心插件, 安装插件流程如下:

1) 点击左侧活动栏的插件图标

<div align=left>
    <img src="https://img.alicdn.com/imgextra/i3/O1CN01onbOks1juy8VN151B_!!6000000004609-2-tps-1089-526.png" style="zoom:50%;" />
</div>

2) 在搜索框中输入插件名称

<div align=left>
    <img src="https://img.alicdn.com/imgextra/i2/O1CN01siKd6J1SE3YbGSCA3_!!6000000002214-2-tps-1079-491.png" style="zoom:50%;" />
</div>

3) 搜索到AliOS-Studio插件后点击安装(install)

<div align=left>
    <img src="https://img.alicdn.com/imgextra/i2/O1CN01SKgZ1n1xpNvdpsOim_!!6000000006492-2-tps-1798-468.png" style="zoom:50%;" />
</div>


## aos-tools
aos-tools主要于编译AliOS-Things代码和镜像烧录。alios-studio安装成功后，VS Code会自动在窗口的右下角提示安装aos-tools, 请点击“是”确认安装：

<div align=left>
    <img src="https://img.alicdn.com/imgextra/i2/O1CN01fisFa91USpZAVS5Bw_!!6000000002517-2-tps-785-321.png" style="zoom:50%;" />
</div>

等待几分钟后，aos-tools安装完毕。窗口右下角会显示“成功安装aos-tools”字样。
**如果没有弹出安装aos-tools的提示请重启VS Code。**
已安装(INSTALLED)插件列表中会显示"alios-studio"和"C/C++"两个插件：

<div align=left>
    <img src="https://img.alicdn.com/imgextra/i3/O1CN01dGWaNq2A0d2c9vRxt_!!6000000008141-2-tps-1707-1015.png" style="zoom:50%;" />
</div>



# 2. 创建工程
创建工程会自动拉取AliOS-Things的核心组件代码，并配置好相关编译选项。
## 2.1 开始创建
点击左下角的"+"图标开始创建工程
<div align=left>
    <img src="https://img.alicdn.com/imgextra/i4/O1CN01R4EtQc22seq5Re3AK_!!6000000007176-0-tps-1584-548.jpg" style="zoom:50%;" />
</div>


## 2.2 选择解决方案和开发板

随后VS Code会依次提示“请选择解决方案”和“请选择开发板”:
- 选择解决方案，即选择什么功能的示例程序，如eduk1_demo, hal demo等
- 选择开发板，即选择在哪块开发板上运行这套示例程序，如HaaS EDU等

选择意向的解决方案（本文以 eduk1_demo为例）

<div align=left>
    <img src="https://img.alicdn.com/imgextra/i2/O1CN01QIJaDh1Qj59taTq8f_!!6000000002011-2-tps-2774-2044.png" style="zoom:50%;" />
</div>

选择解决方案，之后用鼠标选择开发板（本文以 Haaseduk1为例）

<div align=left>
    <img src="https://img.alicdn.com/imgextra/i1/O1CN01sdxB6p1x2NY6rgrsd_!!6000000006385-0-tps-3216-1158.jpg" style="zoom:50%;" />
</div>


## 2.3 输入项目名称和路径

输入项目名称

<div align=left>
    <img src="https://img.alicdn.com/imgextra/i4/O1CN01x5uEE01uz3RWI7i0U_!!6000000006107-0-tps-1586-384.jpg" style="zoom:50%;" />
</div>

工作区路径即为源码路径(此目录名称必须为连续的英文字符)

<div align=left>
    <img src="https://img.alicdn.com/imgextra/i2/O1CN014WS79U29uDUrzFDJx_!!6000000008127-2-tps-2702-914.png" style="zoom:50%;" />
</div>

经过上述几步，VS Code会自动下载AliOS-Things代码，稍等几分钟即可看到整份源码：

<div align=left>
    <img src="https://img.alicdn.com/imgextra/i1/O1CN01XuYZqy1bN3tDyCgkL_!!6000000003452-2-tps-689-422.png" style="zoom:50%;" />
</div>



# 3. 开发及调试
## 3.1 编译工程
点击状态栏的编译图标。

<div align=left>
    <img src="https://img.alicdn.com/imgextra/i2/O1CN01E8SQ7j1tNfUwboVIO_!!6000000005890-0-tps-3278-1694.jpg" style="zoom:50%;" />
</div>

稍等片刻（由于需要安装编译器，第一次编译耗时稍久）就能看到编译成功的提示。
<div align=left>
    <img src="https://img.alicdn.com/imgextra/i4/O1CN01MMVCEz1q1HHv8ULME_!!6000000005435-0-tps-2280-1158.jpg" style="zoom:50%;" />
</div>

## 3.2 烧录镜像
- 如下图，用数据线把HaaS开发板和电脑连起来。

<div align=left>
    <img src="https://img.alicdn.com/imgextra/i1/O1CN01hFIvG81xtxtF7B5kb_!!6000000006502-0-tps-3648-2736.jpg" style="zoom:50%;" />
</div>

- 启动烧录
   - 点击VS Code下方状态栏中的烧录图标，随后VS Code会显示电脑已识别到的串口的列表(Avaliable)
   - 根据HaaS开发板的端口号，输入相对应的串口序号, 依据提示重启HaaS Edu K1开始烧录

<div align=left>
    <img src="https://img.alicdn.com/imgextra/i2/O1CN01LlUD0P1BwcdDfd3mt_!!6000000000010-2-tps-2402-1448.png" style="zoom:50%;" />
</div>

- 稍等两分钟就可以看到VS Code提示“成功烧录”

<div align=left>
    <img src="https://img.alicdn.com/imgextra/i1/O1CN01nCB0L01npFekkzmjN_!!6000000005138-2-tps-2506-1234.png" style="zoom:50%;" />
</div>


## 3.3 查看日志
- 点击VS Code的Serial Monitor按钮, 选择对应串口号。

<div align=left>
    <img src="https://img.alicdn.com/imgextra/i3/O1CN01aG5qNC1GvJZsy5419_!!6000000000684-2-tps-2376-1948.png" style="zoom:50%;" />
</div>

- 设置波特率为1500000

<div align=left>
    <img src="https://img.alicdn.com/imgextra/i4/O1CN017FqUBk1OA9iyd6bn7_!!6000000001664-2-tps-2466-652.png" style="zoom:50%;" />
</div>

- 串口日志输出

<div align=left>
    <img src="https://img.alicdn.com/imgextra/i4/O1CN01QGiV5M1pSTBde0A23_!!6000000005359-2-tps-2728-1038.png" style="zoom:50%;" />
</div>

- 打开HaaS Edu K1, 参考 [https://blog.csdn.net/HaaSTech/article/details/113989581](https://blog.csdn.net/HaaSTech/article/details/113989581)开始体验吧

<div align=left>
    <img src="https://img.alicdn.com/imgextra/i3/O1CN01UQZIYy1r4lvN7nEDB_!!6000000005578-0-tps-5120-3840.jpg" style="zoom:50%;" />
</div>




