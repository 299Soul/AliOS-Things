# GPIO - 通用输入输出

* 类功能：
GPIO通用输入输出。

## GPIO - 创建GPIO对象
* 函数原型：
> GPIO.GPIO()

* 参数说明：  
无。

* 返回值：  
GPIO对象创建成功，返回GPIO对象；GPIO对象创建失败，抛出`ENOMEN`异常。

* 示例代码：

```python
from driver import GPIO
gpio = GPIO()
```

## open - 打开GPIO设备

* 函数功能  
打开GPIO设备节点，并根据节点的配置信息初始化GPIO。

* 函数原型：
> GPIO.open(nodeName)

* 参数说明

| 属性 | 类型 | 必填 | 描述 |
| --- | --- | --- | --- |
| node | String | 是 | 待操作的GPIO设备节点，定义在board.json文件中。 |

board.json中的GPIO类型设备属性配置项说明如下：

|属性字段|数据类型|属性值|必选项？|字段说明|
|-----|:---:|----|:---:|----|
|port|int|1、2、3……|是|配置端口值，端口值跟硬件接口有一一对应关系|
|dir|string|output、input、irq、analog|否|配置引脚方向。默认为输出模式。|
|pull|string|pullup、pulldown、pushpull、opendrain|否|配置输入输出驱动模式或上下拉电阻。输入模式下可用属性值为pullup、pulldown，默认（pull字段空缺时）为pullup模式。输出模式下可用属性值为pushpull、opendrain，默认（pull字段空缺时）为pushpull模式。|
|intMode|string|rising、falling、both|否|配置引脚中断触发模式。默认为双沿触发。|

* 返回值：  
成功：0；失败：故障码。

* 示例： 

```python
# 假设25号端口外接一个led，在board.json中将此端口命名为"led"，则board.json中的内容如下：
'''
{
    "name": "board-name",
    "version": "1.0.0",
    "io": {
        "led": {
        "type": "GPIO",
        "port": 25,
        "dir": output,
        "pull": "pullup"
        }
    }
}
'''

from driver import GPIO

led = GPIO()
ret = led.open("led")

print(ret)
```

## close - 关闭GPIO设备

* 函数原型：
> GPIO.close()

* 参数说明：  
无。

* 返回值：  
成功：0；失败：故障码。

* 示例代码：
```python
from driver import GPIO
led = GPIO()
ret = led.open("led")
ret = led.close()
```
## read - 获取GPIO设备输入电平

* 函数原型：
> GPIO.read()

* 参数说明：  
无。

* 返回值：  
   - 0：低电平。
   - 1：高电平。
   - 负整数：故障码。

* 示例代码：

```python
from driver import GPIO
led = GPIO()
ret = led.open("led")
ret = led.read()
```
## write - 设置GPIO设备输出电平

* 函数原型：  
> GPIO.write(value)

* 参数说明

| 属性 | 类型 | 必填 | 描述 |
| --- | --- | --- | --- |
| value | int | 是 | GPIO输出电平，0为低电平，1为高电平。 |

* 返回值：  
成功：0；失败：故障码。

* 示例代码：

```python
from driver import GPIO
led = GPIO()
ret = led.open("led")
ret = led.write(1)
```

## on - 设置GPIO设备中断回调函数

* 函数原型：
> GPIO.on(func)

* 参数说明

| 属性 | 类型 | 必填 | 描述 |
| --- | --- | --- | --- |
| func | function | 是 | GPIO中断回调函数，中断触发方式定义在board.json文件中。 |

* 返回值：  
成功：0；失败：故障码。

* 示例代码：

```python
'''
{
    "name": "board-name",
    "version": "1.0.0",
    "io": {
        "gpio_irq": {
            "type": "GPIO",
            "port": 32,
            "dir": "irq",
            "pull": "pullup",
            "intMode": "falling"
        }
    }
}
'''

from driver import GPIO

irq_times = 0

def irq_handler(data):
    global irq_times
    irq_times += 1

gpio_irq = GPIO()
ret = gpio_irq.open("gpio_irq")
ret = gpio_irq.on(irq_handler)

```

* 使用实例：

```python
# 通过33号端口控制32号端口产生10次中断
'''
{
    "name": "board-name",
    "version": "1.0.0",
    "io": {
        "gpio_irq": {
        "type": "GPIO",
        "port": 32,
        "dir": "irq",
        "pull": "pullup",
        "intMode": "falling"
        },
        "gpio_irq_ctl": {
        "type": "GPIO",
        "port": 33,
        "dir": "output"
        }
    }
}
'''

from driver import GPIO

irq_times = 0

def irq_handler(data):
    global irq_times
    irq_times += 1

# main entry
if __name__ == "__main__":
    times = 10
    gpio_irq = GPIO()
    gpio_irq.open("gpio_irq")
    gpio_irq.on(irq_handler)
    gpio_irq_ctl = GPIO()
    gpio_irq_ctl.open("gpio_irq_ctl")
    i = times
    while i > 0 :
        i -= 1
        utime.sleep_ms(10)
        gpio_irq_ctl.write(1)
        utime.sleep_ms(10)
        gpio_irq_ctl.write(0)
    utime.sleep_ms(50)
    global irq_times
    gpio_irq_ctl.close()
    gpio_irq.close()
    print("irq_times ", irq_times)
```

* 输出：

```python
  irq_times 10
```