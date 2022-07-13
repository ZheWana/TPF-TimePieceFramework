# TPF-TimePieceFramework
## 简介

**时间片轮询**是裸机开发的过程中十分常用的一种程序架构，其主要的思想是将单个定时器的时间时分复用，可以同时处理多种不同时间间隔的定时任务。

笔者在很久之前的电赛总结的文章中就曾经说过，自己实现了一个时间片轮询架构并且用在了电赛中，后续又为其添加了一些常用的功能。但是前段时间笔者自行审视的时候发现曾经架构中其实存在一些比较严重的问题，于是将该架构进行了简单的重构，代码量--的同时，安全性++。

该架构面向的使用场景是对于实时性要求**并不十分严格**的多任务开发场景。虽然任务对于处理器的长时间占用会导致框架内的所有任务实时性均受到影响，但是得益于非阻塞式的结构，框架外的代码功能（尤其是处于中断内的内容）绝大部分并不会受到影响。

## 使用说明

要使用TPF时间片轮询框架，你需要在你的工程中做出如下处理：

* 将TPF.c以及TPF.h加入到你的工程中

* 为你的**每个任务**定义一个`tptask_t`句柄

* 为**每个任务**编写任务处理函数，函数原型为`void xx(void)`

* 为**每个任务**调用`TP_TaskInit`函数进行任务初始化

  注：调用初始化函数的顺序将会决定你的任务轮询优先级；越早初始化的任务，轮询优先级越高。初始化后的任务默认处于就绪态。

* 以一个固定的时间间隔调用`TP_TickUpdate`函数，参数为**调用周期**（ms）

* 将`TP_TaskHandler`函数放进您的主循环`while(1)`内部

* 观察运行效果

***

## 补充说明与示例

### 用户函数

框架内面向用户的函数列表总结如下：

|      函数名      |     功能     |             参数             |  返回值  |
| :--------------: | :----------: | :--------------------------: | :------: |
|  TP_TickUpdate   | 更新框架时钟 |           更新周期           |   void   |
|  TP_TaskHandler  | 任务处理函数 |             void             |   void   |
|   TP_TaskInit    |  任务初始化  | 任务句柄,时间片,处理函数指针 | tpStatus |
|   TP_TaskDelay   |   任务延时   |   任务句柄,延时时间（ms）    | tpStatus |
|  TP_GlobalDelay  |   框架延时   |        延时时间（ms）        | tpStatus |
| TP_TaskTerminate |   任务终止   |           任务句柄           | tpStatus |
|  TP_TaskRework   |   任务恢复   |           任务句柄           | tpStatus |

### 主循环及任务内容

由于任务处理函数是放在主循环中轮询的，故**不建议在主循环中任务处理函数外进行耗时处理**，否则将影响到整个框架的运行。您也可以选择将任务处理函数放在定时器中断中运行**（十分不推荐）**，此种情况下要保证实时性请至少遵循以下规则：

* 任务处理函数的执行频率**至少应大于等于**框架时钟更新的频率方可保证系统实时性
* 全部任务执行总时间应远小于中断周期

### 数据结构

每个任务采用如下句柄实现不同任务之间数据分离，彼此独立：

```C
typedef struct tptask_t {
    struct tptask_t *last;
    struct tptask_t *next;

    uint8_t ID;
    uint32_t timeoutTick;

    taskStatus state;

    uint32_t timePiece;

    void (*pHandler)(void);
} tptask_t;
```

任务列表采用双向链表存储，但是目前的功能中仅仅使用了其中单向的部分，采用双向链表是为后续添加功能做准备。

### 示例

实例中创建两个任务：Blink控制LED闪烁；Print用于打印数据。

仅提供Arm内核架构STM32HAL库环境下参考，其余同理：

```C
//------------  systick.c/blablabla.c  -------------//
//定时器函数，以周期为1ms的SysTick为例
void SysTick_Handler(void){
    TP_TickUpdate(1);
}


//--------------------  main.c  ---------------------//

tptask_t blinkTask, printTask;

void BlinkHandler() {
    HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
    printf("Toggle GPIOC Pin13\n");
}

void PrintHandler() {
    static int a = 1;
    printf("print:%d\n", a++);
    if (a == 100) {
        TP_TaskTerminate(&blinkTask);
    } else if (a == 500) {
        TP_TaskRework(&blinkTask);
    }
}

int main(void){
    TP_TaskInit(&blinkTask, 500, BlinkHandler);
    TP_TaskInit(&printTask, 50, PrintHandler);
    
    while(1){
        TP_TaskHandler();
    }
}
```

