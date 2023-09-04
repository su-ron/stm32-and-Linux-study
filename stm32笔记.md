# 前言

## 1.单片机从启动到进入main函数的流程？

STM32单片机从启动到进入 `main` 函数的流程如下：

1. **硬件复位**：当STM32单片机上电或经过硬件复位时，处理器会跳转到预定义的复位向量中的地址执行。
2. **复位向量表**：复位向量表是一个存储在特定地址的数据结构，位于存储器的起始位置，其地址通常是0x00000000。复位向量表中的第一个条目就是复位向量，指向了复位处理程序的入口地址。它包含了一系列向量表条目。每个向量表条目对应一个中断或异常的处理函数地址，其中包括复位向量（**在单片机上电或者经过硬件复位时，处理器跳转执行的特定地址，该地址指向了复位处理程序**)。
3. **复位处理程序**：复位处理程序是存储在复位向量表中的指令，用于初始化CPU和外设。此时，处理器会执行复位处理程序，完成基本的硬件初始化、时钟配置、堆栈设置等操作。
4. **初始化堆栈**：在复位处理程序中，会初始化主堆栈指针（MSP，Main Stack Pointer），将其设置为堆栈的起始地址。堆栈用于管理函数调用和中断处理时的局部变量和返回地址。
5. **初始化系统时钟**：在复位处理程序中，会进行系统时钟的初始化。这包括设置时钟源、分频器和PLL（锁相环）等，以确保系统时钟按照期望的频率运行。
6. **初始化全局变量**：复位处理程序可能还会进行一些全局变量的初始化操作，例如清零、赋初值等。
7. **进入 `main` 函数**：完成上述初始化步骤后，处理器会将程序计数器（PC）设置为 `main` 函数的入口地址，程序开始执行 `main` 函数中的代码。



## 2.单片机中断机制，中断向量表，中断函数（如何保护中断现场，PC指针怎么动）

**单片机中断机制:**

单片机中断机制是一种用于处理外部事件或内部异常的重要机制。它允许单片机在执行主程序的同时，能够及时地响应并处理外部事件或内部异常，提高系统的实时性和灵活性。

在单片机中，中断机制通常包括以下几个主要的组成部分：

1. **中断源**：中断源是指能够触发中断的事件或条件。可以是外部输入引脚的电平变化、定时器溢出、串口接收到数据等。每个中断源都有一个对应的中断请求标志位，用于表示是否发生了相应的中断事件。
2. **中断控制器**：中断控制器是用于管理和控制中断的硬件模块。它负责监听中断请求标志位的状态，并根据优先级和屏蔽设置来确定是否响应中断请求。
3. **中断向量表**：中断向量表是存储了中断服务程序（Interrupt Service Routine，ISR）入口地址的数据结构。每个中断源都对应着中断向量表中的一个条目，当相应的中断请求被接受后，处理器会根据中断向量表中的入口地址跳转到对应的中断服务程序。
4. **中断服务程序（ISR）**：中断服务程序是对特定中断事件进行处理的代码段。当中断请求被接受后，处理器会跳转到对应的中断服务程序的入口地址执行，完成相关的中断处理操作。中断服务程序负责响应中断、保存上下文、执行中断处理代码、清除中断标志位等。
5. **中断优先级和屏蔽**：中断优先级用于确定当多个中断请求同时被触发时，哪个中断优先级更高，被优先处理。中断屏蔽用于控制是否允许特定中断请求产生中断。通过设置中断优先级和屏蔽设置，可以在复杂的中断系统中进行灵活的中断处理。

通过使用中断机制，单片机可以在主程序的执行过程中，根据外部事件或内部异常的发生情况及时地进行响应和处理，提高系统的实时性和可靠性。同时，合理的中断优先级和屏蔽设置可以保证中断的按需执行和正确处理，以满足系统的需求。



**中断向量表**

# 0.按键检测

基于正点原子开发板

在main.h中添加定义

```c++
#define KEY0 HAL_GPIO_ReadPin(KEY0_GPIO_Port, KEY0_Pin) /* 读取 KEY0 引脚 */
#define KEY1 HAL_GPIO_ReadPin(KEY1_GPIO_Port, KEY1_Pin) /* 读取 KEY1 引脚 */
#define WK_UP HAL_GPIO_ReadPin(WK_UP_GPIO_Port, WK_UP_Pin) /* 读取 WKUP 引脚 */
#define KEY0_PRES 1 /* KEY0 按下 */
#define KEY1_PRES 2 /* KEY1 按下 */
#define WKUP_PRES 3 /* KEY_UP 按下(即 WK_UP) */
```

在gpio.h中声明函数

```c++
uint8_t key_scan(uint8_t mode);
```

在gpio.c中

```c++
uint8_t key_scan(uint8_t mode)
{
 static uint8_t key_up = 1; /* 按键按松开标志 */
uint8_t keyval = 0;
 if (mode) key_up = 1; /* 支持连按 */
 if (key_up && (KEY0 == 0 || KEY1 == 0 || WK_UP == 1))
 { /* 按键松开标志为 1, 且有任意一个按键按下了 */
 HAL_Delay(10); /* 去抖动 */
 key_up = 0;
 if (KEY0 == 0) keyval = KEY0_PRES;
 if (KEY1 == 0) keyval = KEY1_PRES;
 if (WK_UP == 1) keyval = WKUP_PRES;
 }
 else if (KEY0 == 1 && KEY1 == 1 && WK_UP == 0)
 { /* 没有任何按键按下, 标记按键松开 */
 key_up = 1;
 }
 return keyval; /* 返回键值 */
}
```

在main.c中

```c++
uint8_t key;
while (1)
  {
    /* USER CODE END WHILE */
   //ch=getchar();
   // HAL_UART_Transmit(&huart1,&ch,1,0);
    /* USER CODE BEGIN 3 */
      key = key_scan(0); /* 得到键值 */
      printf("key=%d\n",key);
 if (key)
 {
     printf("大只1测试\n");
 switch (key)
 {
 case WKUP_PRES: 
  ToggLED0(); /* BEEP 状态取反 */
 break;
 case KEY1_PRES: /* 控制 LED1(GREEN)翻转 */
 ToggLED1(); /* LED1 状态取反 */
 break;
 case KEY0_PRES: /* 同时控制 LED0, LED1 翻转 */
 ToggLED0(); /* LED0 状态取反 */
 ToggLED1(); /* LED1 状态取反 */
 break;
 }
 }
 else
 {
 HAL_Delay(10);
 }
 }
  
  /* USER CODE END 3 */
}
```



# 1.外部中断

参考资料：[(60条消息) 【STM32】HAL库 STM32CubeMX教程三----外部中断(HAL库GPIO讲解)_hal_gpio_exti_callback_Z小旋的博客-CSDN博客](https://blog.csdn.net/as480133937/article/details/98983268?spm=1001.2014.3001.5502)

基于正点原子stm32F103 精英版



![image-20230711162848422](https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/stm32_picture/image-20230711162848422.png)



<img src="https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/stm32_picture/image-20230711163110640.png" alt="image-20230711163110640" style="zoom:50%;" />



LED

<img src="https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/stm32_picture/image-20230711163207077.png" alt="image-20230711163207077" style="zoom: 50%;" />

GPIO函数库讲解

首先打开stm32f1xx_hal_gpio.h 发现一共定义有8个函数

![image-20230711165157274](https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/stm32_picture/image-20230711165157274.png)



在gpio.c中添加这部分代码

```c++
/* USER CODE BEGIN 2 */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if(GPIO_Pin==KEY0_Pin)
  {
  
    if(HAL_GPIO_ReadPin(KEY0_GPIO_Port,KEY0_Pin)==0)
    {
      LED0(0);
      LED1(0);
  
    }
    __HAL_GPIO_EXTI_CLEAR_IT(KEY0_Pin);
  }
  else if(GPIO_Pin==KEY1_Pin)
  {
 
    if(HAL_GPIO_ReadPin(KEY1_GPIO_Port,KEY1_Pin)==0)
    {
      LED0(1);
      LED1(1);
    
    }
    __HAL_GPIO_EXTI_CLEAR_IT(KEY1_Pin);


}
  }
```



# 2.定时器中断

主要参考：[(60条消息) 【STM32】HAL库 STM32CubeMX教程六----定时器中断_hal_tim_irqhandler_Z小旋的博客-CSDN博客](https://blog.csdn.net/as480133937/article/details/99201209?spm=1001.2014.3001.5502)

定时器溢出时间：

![image-20230711160408900](https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/stm32_picture/image-20230711160408900.png)

关注函数：

**定时器溢出中断回调函数**

-  **void TIM3_IRQHandler(void)**   首先进入中断函数
-  **HAL_TIM_IRQHandler(&htim2)**;之后进入定时器中断处理函数
- 判断产生的是哪一类定时器中断(溢出中断/PWM中断.....) 和定时器通道
-  **void HAL_TIM_PeriodElapsedCallback(&htim2)**;    进入相对应中断回调函数
  在中断回调函数中添加用户代码
  **在HAL库中，中断运行结束后不会立刻退出，而是会先进入相对应的中断回调函数，处理该函数中的代码之后，才会退出中断，**所以在HAL库中我们一般将中断需要处理代码放在中断回调函数中

在main.c函数中初始化定时器2

关键语句：

```c++
  /* USER CODE BEGIN 2 */
    /*使能定时器1中断*/
    HAL_TIM_Base_Start_IT(&htim2);
  /* USER CODE END 2 */
```



在timer.c中添加溢出中断回调函数

```c++
//定时器溢出中断回调函数
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim == (&htim2))
    {
        ToggLED1();
    }
}
```



其中在main.h中添加LED灯宏定义

```c++
/* USER CODE BEGIN Private defines */
//写GPIO
#define LED0(n) HAL_GPIO_WritePin(LED0_GPIO_Port, LED0_Pin, n?GPIO_PIN_SET:GPIO_PIN_RESET )
#define LED1(n) HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, n?GPIO_PIN_SET:GPIO_PIN_RESET )

//GPIO状态取反-即反转
#define ToggLED0() HAL_GPIO_TogglePin(LED0_GPIO_Port, LED0_Pin)
#define ToggLED1() HAL_GPIO_TogglePin(LED1_GPIO_Port, LED1_Pin)
```



**这里有一个问题是，我尝试在中断函数去添加LED闪烁，但没出现效果**

![image-20230711160921863](https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/stm32_picture/image-20230711160921863.png)



# 3.串口（RS-232)

RS232的电平

<img src="https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/stm32_picture/image-20230711204347580.png" alt="image-20230711204347580" style="zoom:33%;" />

RS232的数据帧

<img src="C:/Users/su/Desktop/%E5%B5%8C%E5%85%A5%E5%BC%8F/%E9%80%9A%E4%BF%A1%E5%AE%9E%E9%AA%8C/%E9%80%9A%E4%BF%A1%E7%AC%94%E8%AE%B0/%E5%9B%BE%E7%89%87/image-20230711204546403.png" alt="image-20230711204546403" style="zoom: 67%;" />

启动位：必须占一个位长，保持0电平

有效数据位：可选5、6、7、8、9个位长，LSB(最低有效位)在前，MSB（最高有效位）在后

RX232通信示意图

![image-20230711204633604](https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/stm32_picture/image-20230711204633604.png)



# 4.UART串口通信

usart(universal synchronous asynchronous receiver transmitter)  通用同步异步收发器

uart(universal  asynchronous receiver transmitter) 通用异步收发器

都能与外部设备进行全双工异步通信

参考资料：[(61条消息) 【STM32】HAL库 STM32CubeMX教程四---UART串口通信详解_hal_uart_transmit_Z小旋的博客-CSDN博客](https://blog.csdn.net/as480133937/article/details/99073783?spm=1001.2014.3001.5502)



理解串口中断资料参考：[STM32-HAL库串口DMA空闲中断的正确使用方式+解析SBUS信号_stm32 空闲中断_何为其然的博客-CSDN博客](https://blog.csdn.net/qq_30267617/article/details/131209656?ops_request_misc=%7B%22request%5Fid%22%3A%22169141237816800182190858%22%2C%22scm%22%3A%2220140713.130102334.pc%5Fall.%22%7D&request_id=169141237816800182190858&biz_id=0&utm_medium=distribute.pc_search_result.none-task-blog-2~all~first_rank_ecpm_v1~rank_v31_ecpm-6-131209656-null-null.142^v92^controlT0_2&utm_term=航模串口接收 SBUS&spm=1018.2226.3001.4187)

**通过本篇博客您将学到：**

STM32CubeMX创建串口例程

HAL库UATR函数库

重定义printf函数

HAL库，[UART](https://so.csdn.net/so/search?q=UART&spm=1001.2101.3001.7020)中断接收

HAL库UATR接收与发送例程

精英版串口-串口1

<img src="https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/stm32_picture/image-20230713095216322.png" alt="image-20230713095216322" style="zoom:50%;" />

注意：使用串口通信时，**编译需要勾选上Use MicroLIB**

<img src="https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/stm32_picture/image-20230712092131571.png" alt="image-20230712092131571" style="zoom:67%;" />

**1、串口发送/接收函数**

```c++
HAL_UART_Transmit();串口发送数据，使用超时管理机制 
HAL_UART_Receive();串口接收数据，使用超时管理机制
HAL_UART_Transmit_IT();串口中断模式发送  
HAL_UART_Receive_IT();串口中断模式接收
HAL_UART_Transmit_DMA();串口DMA模式发送
HAL_UART_Transmit_DMA();串口DMA模式接收

```

**2.串口中断函数**

```c++
HAL_UART_IRQHandler(UART_HandleTypeDef *huart);  //串口中断处理函数
HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart);  //串口发送中断回调函数
HAL_UART_TxHalfCpltCallback(UART_HandleTypeDef *huart);  //串口发送一半中断回调函数（用的较少）
HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart);  //串口接收中断回调函数
HAL_UART_RxHalfCpltCallback(UART_HandleTypeDef *huart);//串口接收一半回调函数（用的较少）
HAL_UART_ErrorCallback();串口接收错误函数

```

对串口打印进行调试

在usart.c中重写fputc和fgetc函数

**在usart.h需要包含#include<stdio.h>   **

```c++
int fputc(int ch, FILE *f)
{
  HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1, 0xffff);
  return ch;
}
int fgetc(FILE * f)
{
  uint8_t ch = 0;
  HAL_UART_Receive(&huart1,&ch, 1, 0xffff);
  return ch;
}

```

在main.c中

```c++
   printf("大只测试\n");
    sprintf(str,"今天是个好日子,20%02d-%02d-%02d",15,10,04);  
  /* 调用格式化输出函数打印输出数据 */
  printf("%s\n",str);
  /* USER CODE END 2 */
  
  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */
    ch=getchar();
    HAL_UART_Transmit(&huart1,&ch,1,0);
    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
```

![image-20230807213402855](https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/freertos/picture/image-20230807213402855.png)

**UART接收中断**
因为中断接收函数只能触发一次接收中断，所以我们需要在中断回调函数中再调用一次中断接收函数

**具体流程：**
1、初始化串口

2、在main中第一次调用接收中断函数

3、进入接收中断，接收完数据  进入中断回调函数，需要在IRQHandler函数中清除中断标志位（**USART_ClearITPendingBit(UART5,USART_IT_RXNE); //清除中断标志**）

4、修改HAL_UART_RxCpltCallback中断回调函数，处理接收的数据，

5  回调函数中要调用一次HAL_UART_Receive_IT函数，使得程序可以重新触发接收中断

<img src="https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/stm32_picture/image-20230712094805765.png" alt="image-20230712094805765" style="zoom:50%;" />

**函数流程图：**

HAL_UART_Receive_IT(启动接收中断接收函数)    ->  USART2_IRQHandler(void)(中断服务函数)    ->    HAL_UART_IRQHandler(UART_HandleTypeDef *huart)(中断处理函数)    ->    UART_Receive_IT(UART_HandleTypeDef *huart) (接收函数)   ->    HAL_UART_RxCpltCallback(huart);(中断回调函数)

HAL_UART_RxCpltCallback函数就是用户要重写在main.c里的回调函数。



**具体而言：当我们在XCOM中发送一个字符时，就会触发USART2_IRQHandler(void)(中断服务函数)**,进而调用有我们编写的中断回调函数

![image-20230712100357610](https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/stm32_picture/image-20230712100357610.png)

main主程序

```c++
int main(void)
{

  uint8_t ch;
  HAL_Init();

  SystemClock_Config();
  MX_GPIO_Init();
  MX_USART1_UART_Init();
  MX_USART2_UART_Init();
  HAL_UART_Receive_IT(&huart1, (uint8_t *)&c, 1);
  printf("请输入一个英文字符：\r\n\r\n");
  while (1)
  {

    if(flag==1){
			//发送信息
			HAL_UART_Transmit(&huart1, (uint8_t *)&message, strlen(message),0xFFFF); 
			
			//延时
			HAL_Delay(1000);

  }

}
        
    /* 串口数据接收完成回调函数 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  //当输入的指令为0时,发送提示并改变flag
	if(c=='0'){
		flag=0;
		HAL_UART_Transmit(&huart1, (uint8_t *)&tips2, strlen(tips2),0xFFFF); 
	}
	
	//当输入的指令为1时,发送提示并改变flag
	else if(c=='1'){
		flag=1;
		HAL_UART_Transmit(&huart1, (uint8_t *)&tips1, strlen(tips1),0xFFFF); 
	}


	//重新设置中断
		HAL_UART_Receive_IT(&huart1, (uint8_t *)&c, 1); 

}
```



串口中断参考资料：[STM32CubeMX串口USART中断发送接收数据_cubemx串口中断接收_糖果罐子♡的博客-CSDN博客](https://blog.csdn.net/wuyiyu_/article/details/129238627?ops_request_misc=%7B%22request%5Fid%22%3A%22169140734716800211515217%22%2C%22scm%22%3A%2220140713.130102334.pc%5Fall.%22%7D&request_id=169140734716800211515217&biz_id=0&utm_medium=distribute.pc_search_result.none-task-blog-2~all~first_rank_ecpm_v1~rank_v31_ecpm-2-129238627-null-null.142^v92^controlT0_2&utm_term=stm32cubemx 串口中断标志位、&spm=1018.2226.3001.4187)

2.[STM32-HAL库串口DMA空闲中断的正确使用方式+解析SBUS信号_stm32 空闲中断_何为其然的博客-CSDN博客](https://blog.csdn.net/qq_30267617/article/details/131209656?ops_request_misc=%7B%22request%5Fid%22%3A%22169141237816800182190858%22%2C%22scm%22%3A%2220140713.130102334.pc%5Fall.%22%7D&request_id=169141237816800182190858&biz_id=0&utm_medium=distribute.pc_search_result.none-task-blog-2~all~first_rank_ecpm_v1~rank_v31_ecpm-6-131209656-null-null.142^v92^controlT0_2&utm_term=航模串口接收 SBUS&spm=1018.2226.3001.4187)

# 5.看门狗（独立看门狗，窗口看门狗）

参考文档：[(61条消息) 【STM32】HAL库 STM32CubeMX教程五----看门狗(独立看门狗,窗口看门狗)_stm32看门狗_Z小旋的博客-CSDN博客](https://blog.csdn.net/as480133937/article/details/99121645?spm=1001.2014.3001.5502)

**前言：看门狗可以有效解决程序的跑飞，在使用过程中比较常见，是防止芯片故障的有效外设**

简单说：**看门狗的本质就是定时计数器**，计数器使能之后一直在累加 而喂狗就是重新写入计数器的值，定时计数器重新累加，

如果在一定时间内没有接收到喂狗信号（表示MCU已经挂了），便实现处理器的自动复位重启（发送复位信号）.**两个看门狗设备（独立看门狗、窗口看门狗）**可以用来检测和解决由软件错误引起的故障。**当计数器达到给定的超时值时，触发一个中断**（仅适用窗口看门狗）或者产生**系统复位。**

- **独立看门狗**（IWDG)由专用的低速时钟（LSI）驱动（40kHz），即使主时钟发生故障它仍有效。独立看门狗适合应用于需要看门狗作为一个在主程序之外 能够完全独立工作，并且对时间精度要求低的场合。
- **窗口看门狗**由从APB1时钟（36MHz）分频后得到时钟驱动。通过可配置的时间窗口来检测应用程序非正常的过迟或过早操作。  窗口看门狗最适合那些要求看门狗在精确计时窗口起作用的程序。

## 5.1 独立看门狗

 IWDG时钟预分频系数 4分频  

 计数器重装载值 4095  **RLR**

 LSI时钟频率：40KHZ

   **超出（溢出）时间计算：**

​                                                                                  **Tout=((4×2^PRER) ×RLR)/LSI时钟频率**

<img src="https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/stm32_picture/image-20230712103835745.png" alt="image-20230712103835745" style="zoom:33%;" />



**HAL库独立窗口狗函数库讲解：**

**看门狗初始化：**

```c++
HAL_IWDG_Init(IWDG_HandleTypeDef *hiwdg)
```

**喂狗函数：**

```c++
HAL_IWDG_Refresh(IWDG_HandleTypeDef *hiwdg)
```

现象：系统不断重启，LED闪烁

main函数

![image-20230712120558496](https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/stm32_picture/image-20230712120558496.png)



iwdg.c

```c++
/* IWDG init function */
void MX_IWDG_Init(void)
{

  hiwdg.Instance = IWDG;
  hiwdg.Init.Prescaler = IWDG_PRESCALER_32;
  hiwdg.Init.Reload = 4095;
  if (HAL_IWDG_Init(&hiwdg) != HAL_OK)
  {
    Error_Handler();
  }
}

void iwdg_feed(void)
{
    HAL_IWDG_Refresh(&hiwdg);  /* 重装载计数器 */
}
/* USER CODE END 1 */
```



## 5.2 窗口看门狗

**窗口看门狗的定义**
窗口看门狗跟独立看门狗一样，也是一个递减计数器不断的往下递减计数，当减到一个固定值 0x3F 时还不喂狗的话，产生复位，这个值叫窗口的下限，是固定的值，不能改变。

窗口看门狗之所以称为窗口，就是因为**其喂狗时间是在一个有上下限的范围内（计数器减到某个值~计数器减到0x3F），在这个范围内才可以喂狗，可以通过设定相关寄存器，设定其上限时间（但是下限是固定的0x3F）**




















**看门狗初始化：**

```C++
HAL_WWDG_Init(WWDG_HandleTypeDef *hwwdg)
```

**喂狗：**

```c++
HAL_WWDG_Refresh(WWDG_HandleTypeDef *hwwdg)
```

**看门狗中断处理函数：**

```c++
HAL_WWDG_IRQHandler(WWDG_HandleTypeDef *hwwdg)
```

功能： 判断中断是否正常，并进入中断回调函数

**看门狗中断回调函数**：

```c++
 __weak HAL_WWDG_EarlyWakeupCallback(hwwdg);
```

**在HAL库中，每进行完一个中断，并不会立刻退出，而是会进入到中断回调函数中，**我们主要在这部分函数中添加功能来查看效果





# 6.GPIO八种模式及工作原理详解

参考文档：[(61条消息) 【STM32】STM32F4 GPIO八种模式及工作原理详解_stm32 ft引脚_Z小旋的博客-CSDN博客](https://blog.csdn.net/as480133937/article/details/98063549?spm=1001.2014.3001.5502)

## 6.1 **GPIO的复用**

STM32F4 有很多的内置外设，这些外设的外部引脚都是与 GPIO 共用的。也就是说，一个引脚可以有很多作用，但是默认为IO口，如果想使用一个 GPIO内置外设的功能引脚，就需要GPIO的复用，**那么当这个 GPIO 作为内置外设使用的时候，就叫做复用。    比如说串口  就是GPIO复用为串口**

## 6.2 GPIO的工作模式

1、4种输入模式

**（1）GPIO_Mode_IN_FLOATING 浮空输入**
**（2）GPIO_Mode_IPU 上拉输入**
**（3）GPIO_Mode_IPD 下拉输入**
**（4）GPIO_Mode_AIN 模拟输入**

2、4种输出模式 

**（5）GPIO_Mode_Out_OD 开漏输出（带上拉或者下拉）**
**（6）GPIO_Mode_AF_OD 复用开漏输出（带上拉或者下拉）**
**（7）GPIO_Mode_Out_PP 推挽输出（带上拉或者下拉）**
**（8）GPIO_Mode_AF_PP 复用推挽输出（带上拉或者下拉）**
**3、4种最大输出速度**
**（1）2MHZ  (低速)**
**（2）25MHZ  (中速)**
**（3）50MHZ  (快速)**
**（4）100MHZ  (高速)**



# 7.PWM 脉冲宽度调制

参考资料：[(61条消息) 【STM32】HAL库 STM32CubeMX教程七---PWM输出(呼吸灯)_stm32 hal pwm输出_Z小旋的博客-CSDN博客](https://blog.csdn.net/as480133937/article/details/99231677?spm=1001.2014.3001.5502)



pwm控制电机，就是占空比愈大，那么电机的速度就越快

## PWM工作原理

**SMT32F1系列**共有8个定时器：

**高级定时器（TIM1、TIM8）；通用定时器（TIM2、TIM3、TIM4、TIM5）；基本定时器（TIM6、TIM7）**

**STM32的每个通用定时器都有独立的4个通道可以用来作为：输入捕获、输出比较、PWM输出、单脉冲模式输出等。**

**STM32的定时器除了TIM6和TIM7（基本定时器）之外，其他的定时器都可以产生PWM输出。其中，高级定时器TIM1、TIM8可以同时产生7路PWM输出**

**原理讲解：**

**下图为向上计数模式：**

![image-20230712152333168](https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/stm32_picture/image-20230712152333168.png)

- 在PWM输出模式下，除了CNT（计数器当前值）、ARR（自动重装载值）之外，还多了一个值CCRx（捕获/比较寄存器值）。
- **当CNT小于CCRx时，TIMx_CHx通道输出低电平**；
- **当CNT等于或大于CCRx时，TIMx_CHx通道输出高电平**。

**PWM的一个周期**

- **定时器从0开始向上计数**
- **当0-t1段,定时器计数器TIMx_CNT值小于CCRx值,输出低电平**
- **t1-t2段,定时器计数器TIMx_CNT值大于CCRx值,输出高电平**
- **当TIMx_CNT值达到ARR时,定时器溢出,重新向上计数...循环此过程**
- **至此一个PWM周期完成**



**总结：**

**每个定时器有四个通道,每一个通道都有一个捕获比较寄存器,** 

**将寄存器值和计数器值比较,通过CCRx比较结果输出高低电平,便可以实现脉冲宽度调制模式（PWM信号）**

**TIMx_ARR寄存器确定PWM频率，**

**TIMx_CCRx寄存器确定占空比**



## 计算

在 Parameter Settings 页配置预分频系数为 71，计数周期(自动加载值)为 499，定时器溢出频率，即PWM的周期，就是 
$$
72MHz/(71+1)/(499+1) = 2kHz
$$
PWM频率：

$$
Fpwm =Tclk / ((arr+1)*(psc+1))(单位：Hz)
$$
**arr 是计数器值**
**psc 是预分频值**
占空比：
$$
duty circle = TIM3->CCR1 / arr(单位：%)
$$
**TIM3->CCR1  用户设定值**
比如  定时器频率Tclk = 72Mhz  arr=499   psc=71     那么PWM频率就是720000/500/72=  2000Hz，

即2KHz，arr=499,TIM3->CCR1=250     则pwm的占空比为50%  

**改CCR1可以修改占空比，修改arr可以修改频率**



**重要库函数：**

使能TIM3的PWM Channel1 输出。

```c++
  HAL_TIM_PWM_Start(&htim3,TIM_CHANNEL_1);
```

修改pwm占空比

```c++
__HAL_TIM_SetCompare(&htim3, TIM_CHANNEL_1, pwmVal);    //修改比较值，修改占空比
//TIM3->CCR1 = pwmVal;    与上方相同
```

在main函数实现

```c++
int main(void)
{
   //呼吸灯例程，需要把正点原子PB5与PA6连在一起
  
    uint16_t pwmVal=0;   //PWM占空比  
    uint8_t dir=1;
  HAL_Init();

  SystemClock_Config();

  MX_GPIO_Init();
  MX_TIM3_Init();
  /* USER CODE BEGIN 2 */
  HAL_TIM_PWM_Start(&htim3,TIM_CHANNEL_1);
  while (1)
  {
    /* USER CODE END WHILE */
    while (pwmVal< 500)
	  {
		  pwmVal++;
		  __HAL_TIM_SetCompare(&htim3, TIM_CHANNEL_1, pwmVal);    //修改比较值，修改占空比
//		  TIM3->CCR1 = pwmVal;    与上方相同
		  HAL_Delay(1);
	  }
	  while (pwmVal)
	  {
		  pwmVal--;
		  __HAL_TIM_SetCompare(&htim3, TIM_CHANNEL_1, pwmVal);    //修改比较值，修改占空比
//		  TIM3->CCR1 = pwmVal;     与上方相同
		  HAL_Delay(1);
	  }
	  HAL_Delay(200);

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}
```

# 8.定时器输入捕获

参考资料：[(61条消息) 【STM32】HAL库 STM32CubeMX教程八---定时器输入捕获_hal_tim_readcapturedvalue_Z小旋的博客-CSDN博客](https://blog.csdn.net/as480133937/article/details/99407485?spm=1001.2014.3001.5502)

**输入捕获概念**

输入捕获模式可以用来测量脉冲宽度或者测量频率。STM32的定时器，除了TIM6、TIM7，其他的定时器都有输入捕获的功能。







# 9.ADC

参考资料：[(61条消息) 【STM32】HAL库 STM32CubeMX教程九---ADC_stm32cubemx adc_Z小旋的博客-CSDN博客](https://blog.csdn.net/as480133937/article/details/99627062?spm=1001.2014.3001.5502)



## 9.1**什么是ADC**

Analog-to-Digital Converter的缩写。指模/数转换器或者模拟/数字转换器。是**指将连续变量的模拟信号转换为离散的数字信号的器件**。

**典型的模拟数字转换器将模拟信号转换为表示一定比例电压值的数字信号。**

简单地说就是将模拟电压值，转换成对应的肉眼可读数值


**12位ADC是一种逐次逼近型模拟数字转换器。**它有，**3个ADC控制器，多达18个通道**，可测量**16个外部和2个内部信号源**。各通道的A/D转换可以单次、连续、扫描或间断模式执行。ADC的结果可以左对齐或右对齐方式存储在16位数据寄存器中。

**12位模拟数字转换器**
就是ADC的数字存储是12位的 也就是说转换器通过采集转换所得到的最大值是4095 “111111111111”=4095 二进制的12位可表示0-4095个数， 对应着所测电压的实际值，转换的电压范围是0v-3.3v的话，转换器就会把0v-3.3v平均分成4096份。设转换器所得到的值为x，所求电压值为y。
$$
y=(x/4096)*3.3V
$$
同理，可以理解8位精度和10位精度,具体的转压范围下面我们会讲



**3个ADC控制器**

ADC1,ADC2,ADC3

**18个通道**

**16个外部通道和2个内部信号源** 具体是哪一个IO 口可以从手册查询到

<img src="https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/stm32_picture/image-20230712162145099.png" alt="image-20230712162145099" style="zoom:50%;" />



**16个外部通道**：芯片上有16个引脚是可以接到模拟电压上进行电压值检测的

**2个内部信号源** ： 一个是内部温度传感器，一个是内部参考电压

一共支持**23个引脚**支持ADC，包括21个外部和2个内部信号源



## ADC的转换模式

可看参考资料：



## 左对齐或右对齐

因为ADC得到的数据是12位精度的，但是数据存储在 16 位数据寄存器中，所以ADC的存储结果可以分为**左对齐或右对齐**方式（12位）

<img src="https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/stm32_picture/image-20230712164107549.png" alt="image-20230712164107549" style="zoom:50%;" />



## ADC工作内容

**1.电压输入范围**

![image-20230712164520823](https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/stm32_picture/image-20230712164520823.png)

**ADC一般用于采集小电压，其输入值不能超过VDDA**，即ADC输入范围：VREF- ≤ VIN ≤ VREF+。具体的定义见上图。
**一般把VSSA和VREF- 接地， VREF+ 和 VDDA接3V3，那么ADC的输入范围是0~3.3V。**

**2.ADC输入通道**
从**ADCx_INT0-ADCx_INT15** 对应三个ADC的16个外部通道，进行模拟信号转换 此外，还有**两个内部通道：温度检测或者内部电压检测**
选择对应通道之后，便会选择对应GPIO引脚，相关的引脚定义和描述可在开发板的数据手册里找

**3.注入通道，规则通道**
我们看到，在选择了ADC的相关通道引脚之后，在模拟至数字转换器中有两个通道，注入通道，规则通道，**规则通道至多16个，注入通道至多4个**

**规则通道：**

规则通道相当于你正常运行的程序，看它的名字就可以知道，很规矩，就是正常执行程序
**注入通道：**
注入通道可以打断规则通道，听它的名字就知道不安分，如果在规则通道转换过程中，有注入通道进行转换，那么就要先转换完注入通道，等注入通道转换完成后，再回到规则通道的转换流程
![ ](https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/stm32_picture/image-20230712164727440.png)   

可以简单地把注入通道理解为中断形式，可以更好理解



**4.ADC时钟**
图中的**ADC预分频器的ADCCLK是ADC模块的时钟来源**。通常，由时钟控制器提供的**ADCCLK时钟和PCLK2（APB2时钟）同步**。RCC控制器为ADC时钟提供一个专用的可编程预分频器。 分频因子由RCC_CFGR的ADCPRE[1:0]配置，可配置2/4/6/8分频

**STM32的ADC最大的转换速率为1MHz,也就是说最快转换时间为1us，为了保证ADC转换结果的准确性，ADC的时钟最好不超过14M**。
$$
T = 采样时间 + 12.5个周期，其中1周期为1/ADCCLK
$$
例如，当 ADCCLK=14Mhz 的时候，并设置 1.5 个周期的采样时间，则得到： Tcovn=1.5+12.5=14 个周期=1us。



**5.外部触发转换**
**ADC 转换可以由ADC 控制寄存器2: ADC_CR2 的ADON 这个位来控制，写1 的时候开始转换，写0 的时候停止转换**

当然，除了ADC_CR2寄存器的ADON位控制转换的开始与停止，还可以支持**外部事件触发转换（比如定时器捕捉、EXTI线）**

包括**内部定时器触发**和**外部IO触发**。具体的触发源由**ADC_CR2的EXTSEL[2:0]位（规则通道触发源 ）和 JEXTSEL[2:0]位（注入通道触发源）**控制。

同时ADC3的触发源与ADC1/2的触发源有所不同，上图已经给出，

具体查看第五部分框图即可理解


### 6中断

中断触发条件有三个，**规则通道转换结束**，**注入通道转换结束**，或者**模拟看门狗状态位被设置**时都能产生中断，

![image-20230712165745161](https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/stm32_picture/image-20230712165745161.png)

转换结束中断就是正常的ADC完成一次转换，进入中断，这个很好理解

**模拟看门狗中断**
当被ADC转换的模拟电压值低于低阈值或高于高阈值时，便会产生中断。阈值的高低值由ADC_LTR和ADC_HTR配置
模拟看门狗，听他的名字就知道，在ADC的应用中是为了防止读取到的电压值超量程或者低于量程



**DMA**
同时ADC还支持DMA触发，规则和注入通道转换结束后会产生DMA请求，用于将转换好的数据传输到内存。

注意，只有ADC1和ADC3可以产生DMA请求

因为涉及到DMA传输，所以这里我们不再详细介绍，之后几节会更新DMA,一般我们在使用ADC 的时候都会开启DMA 传输。



## ADC主要特征

![image-20230712165919835](https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/stm32_picture/image-20230712165919835.png)

## ADC Hal库函数

开启ADC 3种模式 ( 轮询模式 中断模式 DMA模式 ）

**• HAL_ADC_Start(&hadcx);       //轮询模式开启ADC**
**• HAL_ADC_Start_IT(&hadcx);      //中断轮询模式开启ADC**
**• HAL_ADC_Start_DMA(&hadcx)；       //DMA模式开启ADC**

关闭ADC 3种模式 ( 轮询模式 中断模式 DMA模式 ）

**• HAL_ADC_Stop()**
**• HAL_ADC_Stop_IT()**
**• HAL_ADC_Stop_DMA()**

ADC校准函数 ：

• **HAL_ADCEx_Calibration_Start(&hadcx);**   

读取ADC转换值

**• HAL_ADC_GetValue()**

等待转换结束函数

**• HAL_ADC_PollForConversion(&hadc1, 50);**

第一个参数为那个ADC,第二个参数为最大等待时间

ADC中断回调函数
**• HAL_ADC_ConvCpltCallback()**

转换完成后回调，DMA模式下DMA传输完成后调用

规则通道及看门狗配置

**• HAL_ADC_ConfigChannel() 配置规则组通道**
**• HAL_ADC_AnalogWDGConfig(）**

例程：

在main.c中加上

```c++
  /* USER CODE BEGIN 0 */
	uint16_t ADC_Value;
  /* USER CODE END 0 */
```

在ADC初始化后加入AD校准函数

```c++
  MX_ADC1_Init();
  HAL_ADCEx_Calibration_Start(&hadc1);    //AD校准
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */
```

while中加上：

```c++
 HAL_ADC_Start(&hadc1);     //启动ADC转换
 HAL_ADC_PollForConversion(&hadc1, 50);   //等待转换完成，50为最大等待时间，单位为ms
 
 
 if(HAL_IS_BIT_SET(HAL_ADC_GetState(&hadc1), HAL_ADC_STATE_REG_EOC))
 {
  ADC_Value = HAL_ADC_GetValue(&hadc1);   //获取AD值

  printf("ADC1 Reading : %d \r\n",ADC_Value);
  printf("PA3 True Voltage value : %.4f \r\n",ADC_Value*3.3f/4096);
  printf("大只测试\r\n");
}
HAL_Delay(1000);
```



**中断读取**

如果使能了ADC转换结束中断，并且使能了定时器中断，可以这样写：

```c++
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)    //定时器中断回调
{
    HAL_ADC_Start_IT(&hadc1); //定时器中断里面开启ADC中断转换，1ms开启一次采集    
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)    //ADC转换完成回调
{
    HAL_ADC_Stop_IT(&hadc1);    　　　　//关闭ADC
    HAL_TIM_Base_Stop_IT(&htim3);　　  //关闭定时器
    AD_Value=HAL_ADC_GetValue(&hadc1);　　//获取ADC转换的值

    printf("ADC1 Reading : %d \r\n",AD_Value);
    printf("%.4f V\r\n",(AD_Value*3.3/4096));　　　  //串口打印电压信息
    HAL_TIM_Base_Start_IT(&htim3); 　　　　  //开启定时器
}

```



# DAC

参考资料：[(62条消息) 【STM32】HAL库 STM32CubeMX教程十---DAC_Z小旋的博客-CSDN博客](https://blog.csdn.net/as480133937/article/details/102309242?spm=1001.2014.3001.5502)

Digital-to-Analog Converter的缩写。**数模转换器**。又称D/A转换器，简称DAC，是**指将离散的数字信号转换为连续变量的模拟信号的器件**。

典型的数字模拟转换器**将表示一定比例电压值的数字信号转换为模拟信号**。

STM32F1中有两个DAC,可以同时使用STM32的DAC模块是12位数字输入，电压输出型的DAC。

DAC 有两个用途：**输出波形**和**输出固定电压**



# 10.DMA（直接内存访问）

参考资料：[(91条消息) 【STM32】HAL库 STM32CubeMX教程十一---DMA (串口DMA发送接收)_cubemx spi dma_Z小旋的博客-CSDN博客](https://blog.csdn.net/as480133937/article/details/104827639)

## 10.1 DMA基本介绍

### 1.DMA定义

**DMA用来提供在外设和存储器之间或者存储器和存储器之间的高速数据传输。无须CPU的干预，通过DMA数据可以快速地移动。这就节省了CPU的资源来做其他操作。**

![image-20230725191519127](https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/freertos/picture/image-20230725191519127.png)

### 2.DMA传输方式

- 外设到内存
- 内存到外设
- 内存到内存
- 外设到外设



### 3.DMA传输参数

DMA所需要的4个核心参数
**1 数据的源地址 2 数据传输位置的目标地址 ，3 传递数据多少的数据传输量 ，4 进行多少次传输的传输模式** 



当用户将参数设置好，主要涉及**源地址、目标地址、传输数据量**这三个，DMA控制器就会启动数据传输，当剩余传输数据量为0时 达到传输终点，结束DMA传输 ，当然，**DMA 还有循环传输模式** 当到达传输终点时会重新启动DMA传输。



![image-20230726142824146](https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/freertos/picture/image-20230726142824146.png)

### 4.DMA的主要特征

每个通道都直接连接专用的硬件DMA请求，每个通道都同样支持软件触发。这些功能通过软件来配置；

- 在同一个DMA模块上，多个请求间的优先权可以通过软件编程设置（共有四级：很高、高、中等和低），优先权设置相等时由硬件决定（请求0优先于请求1，依此类推）；
- 独立数据源和目标数据区的传输宽度（字节、半字、全字），模拟打包和拆包的过程。源和目标地址必须按数据传输宽度对齐；
- 支持循环的缓冲器管理；
- 每个通道都有3个事件标志（DMA半传输、DMA传输完成和DMA传输出错），**这3个事件标志逻辑或成为一个单独的中断请求；**
- 存储器和存储器间的传输、外设和存储器、存储器和外设之间的传输；
- 闪存、SRAM、外设的SRAM、APB1、APB2和AHB外设均可作为访问的源和目标；
- 可编程的数据传输数目：最大为65535。



对于大容量的STM32芯片有**2个DMA控制器** 两个DMA控制器，DMA1有7个通道，DMA2有5个通道。每个通道都可以配置一些外设的地址。



### 5.DMA工作过程

首先通过对比有与没有DMA的情况下，ADC采集的数据如何放到SRAM

**没有DMA**

如果没有DMA，CPU传输数据还要以内核作为中转站具体过程如下：

**内核通过DCode经过总线矩阵协调，从获取AHB存储的外设ADC采集的数据，然后内核再通过DCode经过总线矩阵协调把数据存放到内存SRAM中。**

![image-20230726145014946](https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/freertos/picture/image-20230726145014946.png)



**有DMA**

- **DMA传输时外设对DMA控制器发出请求。**
- **DMA控制器收到请求，触发DMA工作。**
- **DMA控制器从AHB外设获取ADC采集的数据，存储到DMA通道中**
- **DMA控制器的DMA总线与总线矩阵协调，使用AHB把外设ADC采集的数据经由DMA通道存放到SRAM中，这个数据的传输过程中，完全不需要内核的参与，也就是不需要CPU的参与**

<img src="C:/Users/su/AppData/Roaming/Typora/typora-user-images/image-20230726150823919.png" alt="image-20230726150823919" style="zoom:50%;" />

在发生一个事件后，外设向DMA控制器发送一个**请求信号**。DMA控制器根据通道的优先权处理请求。当DMA控制器开始访问发出请求的外设时，DMA控制器立即发送给它一个应答信号。当从DMA控制器得到**应答信号**时，外设立即释放它的请求。一旦外设释放了这个请求，DMA控制器同时撤销应答信号。DMA传输结束，如果有更多的请求时，外设可以启动下一个周期。


### 6.DMA的传输方式

方法1：**DMA_Mode_Normal**，**正常模式，**

当一次DMA数据传输完后，停止DMA传送 ，也就是只传输一次
　　
方法2：**DMA_Mode_Circular** ，**循环传输模式**

当传输结束时，硬件自动会将传输数据量寄存器进行重装，进行下一轮的数据传输。 也就是多次传输模式



### 7.存储器到存储器模式

**DMA通道的操作可以在没有外设请求的情况下进行，这种操作就是存储器到存储器模式。**

**存储器到存储器模式不能与循环模式同时使用**



### 8.DMA中断

每个DMA通道都可以在DMA传输过半、传输完成和传输错误时产生中断。为应用的灵活性考虑，通过设置寄存器的不同位来打开这些中断。

![image-20230726154020057](https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/freertos/picture/image-20230726154020057.png)

使没开启，我们也可以通过查询这些位来获得当前 DMA 传输的状态。这里我们常用的是 TCIFx位，即数据流 x 的 DMA 传输完成与否标志。



**DMA库函数配置过程：**

**1、使能DMA时钟：RCC_AHBPeriphClockCmd();**

**2、初始化DMA通道：DMA_Init();**

**//设置通道；传输地址；传输方向；传输数据的数目；传输数据宽度；传输模式；优先级；是否开启存储器到存储器。**

**3、使能外设DMA；**

**4、使能DMA通道传输；**

**5、查询DMA传输状态。**



CubeMx 如何创建DMA

具体流程如下：

<img src="https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/freertos/picture/image-20230726154620569.png" alt="image-20230726154620569" style="zoom:50%;" />





## 10.2 DMA的常用API

### 1.串口发送/接收函数

- **HAL_UART_Transmit()**;串口发送数据，使用超时管理机制
- **HAL_UART_Receive()**;串口接收数据，使用超时管理机制
- **HAL_UART_Transmit_IT()**;串口中断模式发送
- **HAL_UART_Receive_IT()**;串口中断模式接收
- **HAL_UART_Transmit_DMA()**;串口DMA模式发送
- **HAL_UART_Transmit_DMA()**;串口DMA模式接收
- **HAL_UART_DMAPause()** 暂停串口DMA
- **HAL_UART_DMAResume()**; 恢复串口DMA
- **HAL_UART_DMAStop()**; 结束串口DMA



**串口DMA发送数据**：

```c++
 HAL_UART_Transmit_DMA(UART_HandleTypeDef *huart, uint8_t *pData, uint16_t Size)
```

功能：串口通过DMA发送指定长度的数据。

参数：

- UART_HandleTypeDef *huart UATR的别名 如 : UART_HandleTypeDef huart1; 别名就是huart1
- *pData 需要存放接收数据的数组
- Size 接受的字节数

举例：

```c++
HAL_UART_Transmit_DMA(&huart1, (uint8_t *)Recbuff, sizeof(Recbuff));  //串口发送Senbuff数组
```



**串口DMA恢复函数**

```c++
HAL_UART_DMAResume(&huart1)
```

**作用**： 恢复DMA的传输

**返回值**： 0 正在恢复 1 完成DMA恢复



## 10.3 串口IDLE接收空闲中断+DMA

参考资料：[(94条消息) STM32 HAL CubeMX 串口IDLE接收空闲中断+DMA___hal_dma_get_counter_Z小旋的博客-CSDN博客](https://blog.csdn.net/as480133937/article/details/105013368)

### **1.STM32 IDLE 接收空闲中断**

在使用串口接受字符串时，可以使用空闲中断**（IDLEIE置1，即可使能空闲中断）**，这样在接收完一个字符串，进入空闲状态时（IDLE置1）便会激发一个空闲中断。在中断处理函数，我们可以解析这个字符串。

**接受完一帧数据，触发中断**



**STM32的IDLE的中断产生条件**：

在串口无数据接收的情况下，不会产生，当清除IDLE标志位后，必须有接收到第一个数据后，才开始触发，一但接收的数据断流，没有接收到数据，即产生IDLE中断.



### 2.**STM32 RXNE接收数据中断**

**功能：**
当串口接收到一个bit的数据时，(读取到一个停止位) 便会触发 RXNE接收数据中断

**接受到一个字节的数据，触发中断**

**比如给上位机给单片机一次性发送了8个字节，就会产生8次RXNE中断，1次IDLE中断。**



### 3.相关寄存器

**串口CR1寄存器**

![image-20230727094310036](https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/freertos/picture/image-20230727094310036.png)

**对bit4写1开启IDLE接受空闲中断,对bit5写1开启RXNE接收数据中断。**



**串口ISR寄存器**

![image-20230727094404701](https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/freertos/picture/image-20230727094404701.png)

**此寄存器为串口状态查询寄存器**

当串口接收到数据时，bit5 RXNE就会自动变成1，当接收完一帧数据后，bit4就会变成1.



### 重点：

**1.清除RXNE中断标志位**的方法为：

**只要把接收到的一个字节读出来，就会清除这个中断**



2.在STM32F1 /STM32F4 系列中 **清除IDLE中断标志位**的方法为：

1. **先读SR寄存器，**
2. **再读DR寄存器。**



### 4.相关API

**memset()函数**

```c++
extern void *memset(void *buffer, int c, int count)        
```

该函数用于将指定内存区域的内容设置为特定的值

- buffer：为指针或是数组
- c：是赋给buffer的值
- count：是buffer的长度.

**USART采用DMA接收时，如何读取当前接收字节数？**

```c++
   #define __HAL_DMA_GET_COUNTER(__HANDLE__) ((__HANDLE__)->Instance->CNDTR);
```

**DMA接收时该宏将返回当前接收空间剩余字节**

**实际接受的字节= 预先定义的接收总字节 - __HAL_DMA_GET_COUNTER()**



其本质就是读取NTDR寄存器，DMA通道结构体中定义了NDTR寄存器，读取该寄存器即可得到未传输的数据数呢



### 5.实现方法

两种利用串口IDLE空闲中断的方式接收一帧数据，方法如下:

**方法1：实现思路：**

**1.采用STM32F103的串口1，并配置成空闲中断IDLE模式且使能DMA接收，并同时设置接收缓冲区和初始化DMA。**

2.那么初始化完成之后，当外部给单片机发送数据的时候，假设这次接受的数据长度是200个字节，那么在单片机接收到一个字节的时候并不会产生串口中断，而是DMA在后台把数据默默地搬运到你指定的缓冲区数组里面。**当整帧数据发送完毕之后串口才会产生一次中断，此时可以利用__HAL_DMA_GET_COUNTER(&hdma_usart1_rx);函数计算出当前DMA接收存储空间剩余字节**

**3.本次的数据接受长度=预先定义的接收总字节-接收存储空间剩余字节**

**方法2：实现思路：**

**直接利用stm32的RXNE和IDLE中断进行接收不定字节数据。** 每次接收到一个字节的数据，触发RXNE中断 将该字节数据存放到数组里，传输完成之后，触发一次IDLE中断，对已经获取到的数据进行处理





### 6.例程

**例程1**

使用DMA+串口接受空闲中断 **实现将接收的数据完整发送到上位机的功能**

<img src="https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/freertos/picture/image-20230727104745952.png" alt="image-20230727104745952" style="zoom:50%;" />

**接收数据的流程：**

首先在初始化的时候**打开DMA接收**，**使能IDLE中断**

void MX_USART1_UART_Init(void)

```c++
__HAL_UART_ENABLE_IT(&huart1, UART_IT_IDLE); //使能IDLE中断
```

当MCU通过USART接收外部发来的数据时，在进行第①②③步的时候，DMA直接将接收到的数据写入缓存rx_buffer[100] //接收数据缓存数组，程序此时也不会进入接收中断，在软件上无需做任何事情，要在初始化配置的时候设置好配置就可以了。



**数据接收完成的流程：**

**当数据接收完成之后产生接收空闲中断**④,（就是说只有在发送的数据完成后，才会产生空闲中断）在中断服务函数中做这几件事：

- **判断是否为IDLE接受空闲中断**
- **在中断服务函数中将接收完成标志位置1**
- **关闭DMA防止在处理数据时候接收数据，产生干扰。**
- **计算出接收缓存中的数据长度，清除中断位，**

```c++
void USART1_IRQHandler(void)
{
  /* USER CODE BEGIN USART1_IRQn 0 */
  	uint32_t tmp_flag = 0;
	uint32_t temp;
	tmp_flag =__HAL_UART_GET_FLAG(&huart1,UART_FLAG_IDLE); //获取IDLE标志位
	if((tmp_flag != RESET))//idle标志被置位,判断是否为IDLE接受空闲中断
	{ 
		__HAL_UART_CLEAR_IDLEFLAG(&huart1);//清除标志位
		//temp = huart1.Instance->SR;  //清除状态寄存器SR,读取SR寄存器可以实现清除SR寄存器的功能
		//temp = huart1.Instance->DR; //读取数据寄存器中的数据
		//这两句和上面那句等效
		HAL_UART_DMAStop(&huart1); //关闭DMA防止在处理数据时候接收数据，产生干扰。
		temp  =  __HAL_DMA_GET_COUNTER(&hdma_usart1_rx);// 获取DMA中未传输的数据个数   
		//temp  = hdma_usart1_rx.Instance->NDTR;//读取NDTR寄存器 获取DMA中未传输的数据个数，
		//这句和上面那句等效
		rx_len =  BUFFER_SIZE - temp; //总计数减去未传输的数据个数，得到已经接收的数据个数
		recv_end_flag = 1;	// 接受完成标志位置1	
	 }

  /* USER CODE END USART1_IRQn 0 */
  HAL_UART_IRQHandler(&huart1);
  /* USER CODE BEGIN USART1_IRQn 1 */

  /* USER CODE END USART1_IRQn 1 */
}
```

1. 先读SR寄存器,再读DR寄存器.能够实现清除IDLE中断标志位

   

**while循环 主程序流程：**

- **主程序中检测到接收完成标志被置1**

- **进入数据处理程序，现将接收完成标志位置0，**

- **将接收到的数据重新发送到上位机**

- **重新设置DMA下次要接收的数据字节数，使能DMA进入接收数据状态。**

- ```c++
   while (1)
    {
      /* USER CODE END WHILE */
  if(recv_end_flag == 1)  //接收完成标志
  		{
  			
  			
  			DMA_Usart_Send(rx_buffer, rx_len);//把接收到的数据通过DMA写入内存
  			rx_len = 0;//清除计数
  			recv_end_flag = 0;//清除接收结束标志位
  //			for(uint8_t i=0;i<rx_len;i++)
  //				{
  //					rx_buffer[i]=0;//清接收缓存
  //				}
  				memset(rx_buffer,0,rx_len);//清空rx_buffer
    }
  		HAL_UART_Receive_DMA(&huart1,rx_buffer,BUFFER_SIZE);//重新打开DMA接收
  
      /* USER CODE BEGIN 3 */
    }
    /* USER CODE END 3 */
  
  ```

DMA方式对于我们来说其实挺重要的，现在可能还看不出来，因为我们现在的程序都只是完成一个功能，没有其他并行的程序，三种方式看不出有什么直观的区别。但是当我们要处理的程序多了以后，就会考虑CPU的处理能力的问题，CPU如果同时需要处理的程序很多负担就会很大，程序就很容易跑飞。

例程2：

参考：[(94条消息) STM32 HAL CubeMX 串口IDLE接收空闲中断+DMA___hal_dma_get_counter_Z小旋的博客-CSDN博客](https://blog.csdn.net/as480133937/article/details/105013368)





## 10.4 DMA基本实验

参考实验：4.3 DMA_test

参考资料：[STM32 HAL库 STM32CubeMX -- DMA（直接存储区访问）_Dir_xr的博客-CSDN博客](https://blog.csdn.net/Dir_x/article/details/128961012?ops_request_misc=&request_id=&biz_id=102&utm_term=stm32cubemx DMA&utm_medium=distribute.pc_search_result.none-task-blog-2~all~sobaiduweb~default-1-128961012.142^v92^controlT0_2&spm=1018.2226.3001.4187)

**函数介绍**

**串口DMA发送函数**

```c++
 HAL_UART_Transmit_DMA(UART_HandleTypeDef *huart, uint8_t *pData, uint16_t Size)
```

**功能：** 串口通过DMA发送指定长度的数据。

**参数：**

UART_HandleTypeDef *huart UATR的别名 如 : UART_HandleTypeDef huart1; 别名就是huart1
***pData：** 需要发送的数据
**Size ：** 发送的字节数

举例：

```c++
示例：HAL_UART_Transmit_DMA(&huart1, tx_buf, sizeof(tx_buf));  //串口发送tx_buf数组
```



**串口DMA接收数据**

```c++
HAL_UART_Receive_DMA(UART_HandleTypeDef *huart, uint8_t *pData, uint16_t Size)
```

**功能：** 串口通过DMA接受指定长度的数据。

举例

```c++
示例：HAL_UART_Transmit_DMA(&huart1, rx_buf, sizeof(rx_buf));  //串口发送rx_buf数组
```



参考代码：

```c++
	/* Private define ------------------------------------------------------------*/
	/* USER CODE BEGIN PD */
		uint8_t tx_buf[] = "hello lu shi jun \r\n ";
		uint8_t rx_buf[4];
	/* USER CODE END PD */

	  /* USER CODE BEGIN 2 */
		HAL_UART_Receive_DMA(&huart1,rx_buf,sizeof(rx_buf));	//打开DMA接收
	  /* USER CODE END 2 */


/* USER CODE BEGIN 4 */


/* 串口DMA收到数据回调函数 */
/* UART一旦开启DMA之后，DMA收发中断强制开启，DMA收发函数直接可以编写回调函数 */
/* 当串口接收到数据以后，发送一段特定的数据 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
	{
    if(huart->Instance == USART1)		//判断串口号
    {
      HAL_UART_Transmit_DMA(&huart1,tx_buf,sizeof(tx_buf));		//发送数据  
    //HAL_UART_Receive_DMA(&huart1,rx_buf,sizeof(rx_buf));	//打开DMA接收
    }

	}
/* USER CODE END 4 */

```



# 11. IIC通信（EEPROM)

参考资料：[(62条消息) 【STM32】HAL库 STM32CubeMX教程十二---IIC(读取AT24C02 )_cubemx iic_Z小旋的博客-CSDN博客](https://blog.csdn.net/as480133937/article/details/105259075?spm=1001.2014.3001.5502)

**IIC(Inter－Integrated Circuit)总线**是一种由NXP（原PHILIPS）公司开发的两线式串行总线，用于连接微控制器及其外围设备。多用于主控制器和从器件间的主从通信，在小数据量场合使用，传输距离短，任意时刻只能有一个主机等特性。

在 CPU 与被控 IC 之间、IC 与 IC 之间进行双向传送，高速 IIC 总线一般可达 400kbps 以上。

PS： **这里要注意IIC是为了与低速设备通信而发明的，所以IIC的传输速率比不上SPI**



## 11.1IIC的物理层

**IIC一共有只有两个总线： 一条是双向的数据线ＳＤＡ，一条是串行时钟线ＳＣＬ**

所有接到I2C总线设备上的串行数据SDA都接到总线的SDA上，各设备的时钟线SCL接到总线的SCL上。I2C总线上的每一个设备都对应一个唯一的地址。

![image-20230712191539121](https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/stm32_picture/image-20230712191539121.png)

**IIC起始信号和终止信号：**

- **起始信号**：SCL保持高电平，SDA由高电平变为低电平后，延时(>4.7us)，SCL变为低电平。

- **停止信号**：SCL保持高电平。SDA由低电平变为高电平。

- <img src="https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/stm32_picture/image-20230712192258171.png" alt="image-20230712192258171" style="zoom:50%;" />

  **数据有效性**
  **IIC信号在数据传输过程中，当SCL=1高电平时，数据线SDA必须保持稳定状态，不允许有电平跳变，只有在时钟线上的信号为低电平期间，数据线上的高电平或低电平状态才允许变化。**

  **SCL=1时 数据线SDA的任何电平变换会看做是总线的起始信号或者停止信号。**

  也就是在IIC传输数据的过程中，SCL时钟线会频繁的转换电平，以保证数据的传输

  <img src="https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/stm32_picture/image-20230712192703019.png" alt="image-20230712192703019" style="zoom:50%;" />

**应答信号**
每当主机向从机发送完一个字节的数据，主机总是需要等待从机给出一个应答信号，以确认从机是否成功接收到了数据，

**应答信号：主机SCL拉高，读取从机SDA的电平，为低电平表示产生应答**

**应答信号为低电平时，规定为有效应答位（ACK，简称应答位），表示接收器已经成功地接收了该字节；**
**应答信号为高电平时，规定为非应答位（NACK），一般表示接收器接收该字节没有成功。**
<img src="https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/stm32_picture/image-20230712192846815.png" alt="image-20230712192846815" style="zoom:50%;" />

**每发送一个字节**（8个bit）在一个字节传输的8个时钟后的第九个时钟期间，接收器接收数据后必须回一个ACK应答信号给发送器，这样才能进行数据传输。

应答出现在每一次主机完成8个数据位传输后紧跟着的时钟周期，低电平0表示应答，1表示非应答，

<img src="https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/stm32_picture/image-20230713091247362.png" alt="image-20230713091247362" style="zoom:50%;" />



精英版

<img src="https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/stm32_picture/image-20230713095701789.png" alt="image-20230713095701789" style="zoom:50%;" />

芯片的寻址：
AT24C设备地址为如下，前四位固定为1010，A2~A0为由管脚电平。AT24CXX EEPROM Board模块中默认为接地。所以A2~A0默认为000，最后一位表示读写操作。所以AT24Cxx的读地址为0xA1,写地址为0xA0。

也就是说如果是
**写24C02的时候，从器件地址为10100000（0xA0）；**
**读24C02的时候，从器件地址为10100001（0xA1）。**

## 11.2 IIC的HAL库函数

在i2c.c文件中可以看到IIC初始化函数。在stm32f1xx_hal_i2c.h头文件中可以看到I2C的操作函数。分别对应**轮询，中断和DMA三种控制方式**

<img src="https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/stm32_picture/image-20230713101804188.png" alt="image-20230713101804188"  />

上**面的函数看起来多，但是只是发送和接收的方式改变了，函数的参数和本质功能并没有改变**比方说IIC发送函数 还是发送函数，只不过有普通发送，DMA传输，中断 的几种发送模式

这里我们仅介绍下普通发送，其他的只是改下函数名即可



**IIC写函数**

```c++
 HAL_I2C_Master_Transmit(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint8_t *pData, uint16_t Size, uint32_t Timeout);
```

功能：**IIC写数据**
参数：

- ***hi2c** 设置使用的是那个IIC 例：&hi2c2
- **DevAddress** 写入的地址 设置写入数据的地址 例 0xA0
- ***pData** 需要写入的数据
- **Size** 要发送的字节数
- **Timeout** **最大传输时间**，超过传输时间将自动退出传输函数



**IIC读函数**

```c++
HAL_I2C_Master_Receive(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint8_t *pData, uint16_t Size, uint32_t Timeout);
```

举例：

```c++
HAL_I2C_Master_Transmit(&hi2c1,0xA1,(uint8_t*)TxData,2,1000) ；
```



**发送两个字节数据**

**IIC写数据函数**

```c++
HAL_I2C_Mem_Write(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint16_t MemAddress, uint16_t MemAddSize, uint8_t *pData, uint16_t Size, uint32_t Timeout);
```

功能： IIC写多个数据 该函数适用于IIC外设里面还有子地址寄存器的设备，比方说E2PROM,除了设备地址，每个存储字节都有其对应的地址

参数：

- *hi2c： I2C设备号指针，设置使用的是那个IIC 例：&hi2c2

- DevAddress： 从设备地址 从设备的IIC地址 例E2PROM的设备地址 0xA0

- MemAddress： 从机寄存器地址 ，每写入一个字节数据，地址就会自动+1

- MemAddSize： 从机寄存器地址字节长度 8位或16位

- 写入数据的字节类型 8位还是16位
  I2C_MEMADD_SIZE_8BIT
  I2C_MEMADD_SIZE_16BIT

- ***pData：** **需要写入的的数据的起始地址**
- **Size：** 传输数据的大小 多少个字节
- **Timeout：** **最大读取时间**，超过时间将自动退出函数



使用**HAL_I2C_Mem_Write**等于先使用**HAL_I2C_Master_Transmit**传输第一个寄存器地址，再用**HAL_I2C_Master_Transmit**传输写入第一个寄存器的数据。可以传输多个数据

```c++
void Single_WriteI2C(uint8_t REG_Address,uint8_t REG_data)
{
    uint8_t TxData[2] = {REG_Address,REG_data};
    while(HAL_I2C_Master_Transmit(&hi2c1,I2C1_WRITE_ADDRESS,(uint8_t*)TxData,2,1000) != HAL_OK)
    {
        if (HAL_I2C_GetError(&hi2c1) != HAL_I2C_ERROR_AF)
                {
                  Error_Handler();
                }
    }
}
```

**在传输过程，寄存器地址和源数据地址是会自加的。**



至于读函数也是如此，因此用HAL_I2C_Mem_Write和HAL_I2C_Mem_Read，来写读指定设备的指定寄存器数据是十分方便的，让设计过程省了好多步骤。

**举例：**

**8位：**

```c++
HAL_I2C_Mem_Write(&hi2c2, ADDR, i, I2C_MEMADD_SIZE_8BIT,&(I2C_Buffer_Write[i]),8, 1000);

HAL_I2C_Mem_Read(&hi2c2, ADDR, i, I2C_MEMADD_SIZE_8BIT,&(I2C_Buffer_Write[i]),8, 1000);
```

16位

```c++
HAL_I2C_Mem_Write(&hi2c2, ADDR, i, I2C_MEMADD_SIZE_16BIT,&(I2C_Buffer_Write[i]),8, 1000);

HAL_I2C_Mem_Read(&hi2c2, ADDR, i, I2C_MEMADD_SIZE_16BIT,&(I2C_Buffer_Write[i]),8, 1000);
```

**如果只往某个外设中写数据，则用Master_Transmit。　**

**如果是外设里面还有子地址，例如我们的E2PROM，有设备地址，还有每个数据的寄存器存储地址。则用Mem_Write。Mem_Write是2个地址，Master_Transmit只有从机地址**.





## **实验：**

在实现串口通信后（实现步骤这里就不赘述了）

main.c

main函数之前声明

```c++
/* USER CODE BEGIN PV */
#include <string.h>

#define ADDR_24LCxx_Write 0xA0
#define ADDR_24LCxx_Read 0xA1
#define BufferSize 256
uint8_t WriteBuffer[BufferSize],ReadBuffer[BufferSize];
uint16_t i;
/* USER CODE END PV */
```



```c++
  /* USER CODE BEGIN 2 */
	for(i=0; i<256; i++)
    WriteBuffer[i]=i;    /* WriteBuffer init */


		printf("\r\n***************I2C Example Z小旋测试*******************************\r\n");
			for (int j=0; j<32; j++)
        {
                if(HAL_I2C_Mem_Write(&hi2c1, ADDR_24LCxx_Write, 8*j, I2C_MEMADD_SIZE_8BIT,WriteBuffer+8*j,8, 1000) == HAL_OK)
                {
                                printf("\r\n EEPROM 24C02 Write Test OK \r\n");
                        HAL_Delay(20);
                }
                else
                {
                         HAL_Delay(20);
                                printf("\r\n EEPROM 24C02 Write Test False \r\n");
                }
		}
		/*
		// wrinte date to EEPROM   如果要一次写一个字节，写256次，用这里的代码
		for(i=0;i<BufferSize;i++)
		{
		    HAL_I2C_Mem_Write(&hi2c1, ADDR_24LCxx_Write, i, I2C_MEMADD_SIZE_8BIT,&WriteBuffer[i],1，0xff);//使用I2C块读，出错。因此采用此种方式，逐个单字节写入
		  HAL_Delay(5);//此处延时必加，与AT24C02写时序有关
		}
		printf("\r\n EEPROM 24C02 Write Test OK \r\n");
		*/

		HAL_I2C_Mem_Read(&hi2c1, ADDR_24LCxx_Read, 0, I2C_MEMADD_SIZE_8BIT,ReadBuffer,BufferSize, 0xff);

		for(i=0; i<256; i++)
			printf("0x%02X  ",ReadBuffer[i]);
			
  /* USER CODE END 2 */
```

**注意事项：**

- **AT24C02的IIC每次写之后要延时一段时间才能继续写 每次写之后要delay 5ms左右** 不管硬件IIC采用何种形式（DMA，IT），都要确保两次写入的间隔大于5ms;
- 读写函数最后一个超时调整为1000以上 因为我们一次写8个字节，延时要久一点
- AT24C02页写入只支持8个byte，所以需要分32次写入。这不是HAL库的bug，而是AT24C02的限制，其他的EEPROM可以支持更多byte的写入。
- 当然，你也可以每次写一个字节，分成256次写入，也是可以的 那就用注释了的代码即可

- **注意读取AT24C02数据的时候延时也要久一点，否则会造成读的数据不完整**



# 12.SPI通信(FLASH)

参考资料：[(68条消息) 【STM32】HAL库 STM32CubeMX教程十四---SPI_cubemx spi_Z小旋的博客-CSDN博客](https://blog.csdn.net/as480133937/article/details/105849607?spm=1001.2014.3001.5502)

## **什么是SPI**

SPI 是英语**Serial Peripheral interface**的缩写，顾名思义就是**串行外围设备接口**。是Motorola(摩托罗拉)首先在其MC68HCXX系列处理器上定义的。

SPI，是一种高速的，全双工，同步的通信总线，并且在芯片的管脚上只占用四根线，节约了芯片的管脚，同时为PCB的布局上节省空间，提供方便，主要应用在 EEPROM，FLASH，实时时钟，AD转换器，还有数字信号处理器和数字信号解码器之间。



**SPI主从模式**
SPI分为主、从两种模式，一个SPI通讯系统需要包含一个（且只能是一个）主设备，一个或多个从设备。提供时钟的为主设备（Master），接收时钟的设备为从设备（Slave），SPI接口的读写操作，都是由主设备发起。当存在多个从设备时，通过各自的片选信号进行管理。

**SPI是全双工且SPI没有定义速度限制，一般的实现通常能达到甚至超过10 Mbps**



**SPI信号线**
SPI接口一般使用四条信号线通信：
SDI（数据输入），SDO（数据输出），SCK（时钟），CS（片选）

**MISO： 主设备输入/从设备输出引脚。该引脚在从模式下发送数据，在主模式下接收数据。**
**MOSI： 主设备输出/从设备输入引脚。该引脚在主模式下发送数据，在从模式下接收数据。**
**SCLK：串行时钟信号，由主设备产生。**
**CS/SS：从设备片选信号，由主设备控制。它的功能是用来作为“片选引脚”，也就是选择指定的从设备，让主设备可以单独地与特定从设备通讯，避免数据线上的冲突。**

**SPI一对一**

<img src="https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/stm32_picture/image-20230713114617872.png" alt="image-20230713114617872" style="zoom:50%;" />



**SPI一对多**

<img src="C:\Users\su\AppData\Roaming\Typora\typora-user-images\image-20230713114643967.png" alt="image-20230713114643967" style="zoom:50%;" />

**SPI数据发送接收**
SPI主机和从机都有一个串行移位寄存器，主机通过向它的SPI串行寄存器写入一个字节来发起一次传输。

1. **首先拉低对应SS信号线，表示与该设备进行通信**
2. **主机通过发送SCLK时钟信号，来告诉从机写数据或者读数据，这里要注意，SCLK时钟信号可能是低电平有效，也可能是高电平有效，因为SPI有四种模式，这个我们在下面会介绍**
3. **主机(Master)将要发送的数据写到发送数据缓存区(Menory)，缓存区经过移位寄存器(0~7)，串行移位寄存器通过MOSI信号线将字节一位一位的移出去传送给从机，，同时MISO接口接收到的数据经过移位寄存器一位一位的移到接收缓存区。**
4. **从机(Slave)也将自己的串行移位寄存器(0~7)中的内容通过MISO信号线返回给主机。同时通过MOSI信号线接收主机发送的数据，这样，两个移位寄存器中的内容就被交换。**

![image-20230713120800936](https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/stm32_picture/image-20230713120800936.png)

**SPI只有主模式和从模式之分，没有读和写的说法，外设的写操作和读操作是同步完成的。如果只进行写操作，主机只需忽略接收到的字节；反之，若主机要读取从机的一个字节，就必须发送一个空字节来引发从机的传输。也就是说，你发一个数据必然会收到一个数据；你要收一个数据必须也要先发一个数据。**



## SPI工作模式

根据时钟极性（CPOL）及相位（CPHA）不同，SPI有四种工作模式。
时钟极性(CPOL)定义了时钟空闲状态电平：

- CPOL=0为时钟空闲时为低电平
- CPOL=1为时钟空闲时为高电平

时钟相位(CPHA)定义数据的采集时间。

- CPHA=0:在时钟的第一个跳变沿（上升沿或下降沿）进行数据采样。
- CPHA=1:在时钟的第二个跳变沿（上升沿或下降沿）进行数据采样。

![image-20230713121443951](https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/stm32_picture/image-20230713121443951.png)



## W25Q128 FLASH芯片介绍

**W25Q128是一款SPI通信的FLASH芯片**，可以通过标准/两线/四线SPI控制，其FLASH的大小为16M，分为 256 个块（Block），每个块大小为 64K 字节，每个块又分为 16个扇区（Sector），每个扇区 4K 个字节。通过SPI通信协议即可实现MCU(STM32)和 W25Q128 之间的通信。实现W25Q128的控制需要通过SPI协议发送相应的控制指令，并满足一定的时序。

![image-20230713144738413](https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/stm32_picture/image-20230713144738413.png)

**写使能(Write Enable) (06h)**

向FLASH发送0x06 写使能命令即可开启写使能，首先CS片选拉低，控制写入字节函数写入命令，CS片选拉高。

**扇区擦除指令(Sector Erase) (0x20h)**

扇区擦除指令，数据写入前必须擦除对应的存储单元，该指令先拉低/CS引脚电平,接着传输“20H”指令和要24位要擦除扇区的地址。

**读命令(Read Data) (03h)**

读数据指令可从存储器依次一个或多个数据字节，该指令通过主器件拉低/CS电平使能设备开始传输，然后传输“03H”指令，接着通过DI管脚传输24位地址，从器件接到地址后，寻址存储器中的数据通过DO引脚输出。每传输一个字节地址自动递增，所以只要时钟继续传输，可以不断读取存储器中的数据。

**状态读取命令(Read Status Register)**

读状态寄存器1(05H)，状态寄存器2（35H）,状态寄存器3（15H），写入命令0x05，即可读取状态寄存器的值。

**写入命令(Page Program) (02h)**

在对W25Q128 FLASH的写入数据的操作中一定要先擦出扇区，在进行写入，否则将会发生数据错误。
W25Q128 FLASH一次性最大写入只有256个字节。
在进行写操作之前，一定要开启写使能(Write Enable)。
当只接收数据时不但能只检测RXNE状态 ，必须同时向发送缓冲区发送数据才能驱动SCK时钟跳变。



STM32(精英版）有硬件NSS(片选信号)，可以选择使能，也可以使用其他IO口接到芯片的NSS上进行代替

**其中SIP2的片选NSS ： SPI2_NSS（PB12）**

如果片选引脚没有连接SPI2_NSS（PB12），则需要选择软件片选

**NSS管脚及我们熟知的片选信号**，**作为主设备NSS管脚为高电平，从设备NSS管脚为低电平**。当NSS管脚为低电平时，该spi设备被选中，可以和主设备进行通信。





## SPI函数详解

在stm32f1xx_hal_spi.h头文件中可以看到spi的操作函数。分别对应**轮询，中断和DMA**三种控制方式。

<img src="https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/stm32_picture/image-20230713155806198.png" alt="image-20230713155806198" style="zoom: 67%;" />

- **轮询：** 最基本的发送接收函数，就是正常的发送数据和接收数据
- **中断：** 在SPI发送或者接收完成的时候，会进入SPI回调函数，用户可以编写回调函数，实现设定功能
- **DMA：** DMA传输SPI数据

利用SPI接口发送和接收数据主要调用以下两个函数：

```c++
HAL_StatusTypeDef  HAL_SPI_Transmit(SPI_HandleTypeDef *hspi, uint8_t *pData, uint16_t Size, uint32_t Timeout);//发送数据
HAL_StatusTypeDef  HAL_SPI_Receive(SPI_HandleTypeDef *hspi, uint8_t *pData, uint16_t Size, uint32_t Timeout);//接收数据
```

参数:

- *hspi: 选择SPI1/2，比如&hspi1，&hspi2
- *pData ： 需要发送的数据，可以为数组
- Size： 发送数据的字节数，1 就是发送一个字节数据
- Timeout： 超时时间，就是执行发送函数最长的时间，超过该时间自动退出发送函数



**SPI接收回调函数：**

```c++
　HAL_SPI_TransmitReceive_IT(&hspi1, TXbuf,RXbuf,CommSize);
```

当SPI上接收出现了 CommSize个字节的数据后，中断函数会调用SPI回调函数：

```c++
　HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi)
```









# 13.CAN通信

文档

[(59条消息) CAN通信知识梳理及在Stm32上的应用（HAL库）_冬瓜~的博客-CSDN博客](https://blog.csdn.net/weixin_44793491/article/details/107298426)

[(70条消息) cubemx配置can通信教程（stm32）（带项目工程文件）_stm32cubemx can_一个爱茶的工科男的博客-CSDN博客](https://blog.csdn.net/weixin_38800901/article/details/126662080?ops_request_misc=%7B%22request%5Fid%22%3A%22168924944316800180615244%22%2C%22scm%22%3A%2220140713.130102334..%22%7D&request_id=168924944316800180615244&biz_id=0&utm_medium=distribute.pc_search_result.none-task-blog-2~all~top_click~default-1-126662080-null-null.142^v88^insert_down1,239^v2^insert_chatgpt&utm_term=stm32cubemx can通信&spm=1018.2226.3001.4187)



## 13.1CAN简介

### 显性电平和隐性电平

<img src="https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/stm32_picture/image-20230713195948980.png" alt="image-20230713195948980" style="zoom:50%;" />



- **显性电平：逻辑0** CAN_H和CAN_L电压差大
- **隐性电平：逻辑1** CAN_H和CAN_L电压差小
- 在总线上显性电平具有优先权，只要有一个单元输出显性电平，总线上即为显性电平
- 出现连续的11位隐性电平，那么总线就处于空闲状态

<img src="https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/stm32_picture/image-20230713200132986.png" alt="image-20230713200132986" style="zoom:50%;" />

### 报文（帧）的类型

- **数据帧**
  发送单元向接收单元传送数据的帧
- **遥控帧**
  接收单元向具有相同 ID 的发送单元请求数据的帧
- **错误帧**
  当检测出错误时向其它单元通知错误的帧
- **过载帧**
  接收单元通知其尚未做好接收准备的帧
- **间隔帧**
  将数据帧及遥控帧与前面的帧分离开来的帧

### 数据帧

<img src="https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/stm32_picture/image-20230713200345631.png" alt="image-20230713200345631" style="zoom: 67%;" />

### 位时序

由发送单元在非同步的情况下发送的每秒钟的位数称为位速率，一个位可分为 4 段，每个段又由若干个 Time Quantum（以下称为 Tq） 构成，这称为位时序

<img src="https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/stm32_picture/image-20230713200842044.png" alt="image-20230713200842044" style="zoom:50%;" />

![image-20230713200924199](https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/stm32_picture/image-20230713200924199.png)

- 采样点

<img src="https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/stm32_picture/image-20230713201002569.png" alt="image-20230713201002569" style="zoom: 67%;" />

### 工作模式

- **静默模式**

![image-20230713201939839](https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/stm32_picture/image-20230713201939839.png)

节点的输出端的逻辑0数据会直接传输到自己的输入端，逻辑1可以被发送到总线，所以**它不能向总线发送显性位(0)， 只能发送隐性位(1)**。输入端可以从总线接收内容。

- **回环模式**

  ![image-20230713202134183](https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/stm32_picture/image-20230713202134183.png)

节点输出端的所有内容都直接传输到自己的输入端，输出端的内容同时也会被传输到总线上，**输入端只接收自己发送端的内容，不接收来自总线上的内容。**
使用回环模式可以进行自检。

- **环回与静默组合模式**

![image-20230713202313285](https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/stm32_picture/image-20230713202313285.png)

节点的输出端的所有内容都直接传输到自己的输入端，输入端不接收来自总线上的内容，不会向总线发送显性位影响总线

## 13.2  CAN通信模型

图片讲解

<img src="https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/stm32_picture/image-20230710111004432.png" alt="image-20230710111004432" style="zoom:50%;" />



1.首先进行can的初始化can_init()

2.配置筛选器

<img src="https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/stm32_picture/image-20230710111409537.png" alt="image-20230710111409537" style="zoom:50%;" />



**发送邮箱**

一共有3个发送邮箱，即最多可以缓存3个待发送的报文

**接收FIFO**

一共有2个接收FIFO，每个FIFO中有3个邮箱，即最多可以缓存6个接收到的报文。当接收到报文时，FIFO的报文计数器会自增，而STM32内部读取FIFO数据之后，报文计数器会自减，通过状态寄存器可获知报文计数器的值，而通过前面主控制寄存器的RFLM位，可设置锁定模式，锁定模式下FIFO溢出时会丢弃新报文，非锁定模式下FIFO溢出时新报文会覆盖旧报文。


## 13.3 实验

参考资料：[(70条消息) 最新CubeMX配置CAN通讯教程，避免踩坑，附全套工程文件_cubemx can_皮皮爹地的博客-CSDN博客](https://blog.csdn.net/prolop87/article/details/122671441?spm=1001.2101.3001.6661.1&utm_medium=distribute.pc_relevant_t0.none-task-blog-2~default~CTRLIST~Rate-1-122671441-blog-126662080.235^v38^pc_relevant_sort_base3&depth_1-utm_source=distribute.pc_relevant_t0.none-task-blog-2~default~CTRLIST~Rate-1-122671441-blog-126662080.235^v38^pc_relevant_sort_base3&utm_relevant_index=1)

[(70条消息) CAN总线学习笔记 | STM32CubeMX配置CAN环回测试_can 回环测试_安迪西嵌入式的博客-CSDN博客](https://blog.csdn.net/Chuangke_Andy/article/details/127770680?ops_request_misc=&request_id=&biz_id=102&utm_term=stm32cubemx can通信&utm_medium=distribute.pc_search_result.none-task-blog-2~all~sobaiduweb~default-5-127770680.142^v88^insert_down1,239^v2^insert_chatgpt&spm=1018.2226.3001.4187)





# 14.CanOpen通信

参考资料：https://zhuanlan.zhihu.com/p/490465815

[(80条消息) CANOPEN详解_行思坐忆，志凌云的博客-CSDN博客](https://blog.csdn.net/qq_38025219/article/details/105912586?ops_request_misc=%7B%22request%5Fid%22%3A%22168964707416800215060117%22%2C%22scm%22%3A%2220140713.130102334..%22%7D&request_id=168964707416800215060117&biz_id=0&utm_medium=distribute.pc_search_result.none-task-blog-2~all~sobaiduend~default-1-105912586-null-null.142^v88^insert_down1,239^v2^insert_chatgpt&utm_term=canopen详解&spm=1018.2226.3001.4187)

[(80条消息) CAN Open基础知识_canopen_搬砖的MATTI的博客-CSDN博客](https://blog.csdn.net/SHYHOOD/article/details/117445606?ops_request_misc=%7B%22request%5Fid%22%3A%22168964241216800188526541%22%2C%22scm%22%3A%2220140713.130102334..%22%7D&request_id=168964241216800188526541&biz_id=0&utm_medium=distribute.pc_search_result.none-task-blog-2~all~top_positive~default-1-117445606-null-null.142^v88^insert_down1,239^v2^insert_chatgpt&utm_term=canopen&spm=1018.2226.3001.4187)



## 14.1 什么是CANopen?

​         CANopen是一种基于CAN的[通信协议](https://so.csdn.net/so/search?q=通信协议&spm=1001.2101.3001.7020)。这项协议非常有用，因为它可以让设备、节点（如工业机械）之间具有现成的互操作性，以及它提供了安装前和安装后配置设备的标准方法。CANopen最初是为面向运动的机器控制系统设计的。    

相较于CAN总线和J1939协议，CANopen协议新增了**6个核心概念**：

- **通信模式**。设备/节点的通信有3种模式： 主/从站、客户端/服务器和生产者/消费者。
- **通信协议**。用于通信的协议，如配置节点（SDO）或传输实时数据（PDO）等。
- **设备状态**。一个设备支持不同的状态，一个 "主站 "节点可以改变一个 "从站 "节点，包括重置等操作。
- **对象字典**。每个设备都有一个OD，其中有指定设备配置等的条目，它可以通过SDO访问。
- **电子数据表**。EDS是OD条目的标准文件格式，它允许使用服务工具来更新设备。
- **设备设置文件**。描述了I/O模块（CiA 401）和运动控制（CiA 402）等供应商独立性



**三种模式：**

**第一， 主/从站**。 一个节点（例如控制接口）作为应用主站或主控制器。它向从站设备（例如伺服电机）请求数据。这个过程被用于诊断或状态管理。在标准应用中，可以有0到127个从站。但需要注意，请注意：在单个CANopen网络中，可以有不同的主机控制器共享同一个数据链路层。

**第二， 客户端/服务器**。 客户端向服务器发送数据请求，服务器回复请求的数据。例如，当应用程序主站需要从站的OD中获取数据时使用这一模式。从服务器上读取是一种 “上传”，而“写入”是一种 “下载”（该术语采用服务器端的角度）。

**第三， 消费者/生产者**。 该模式中生产者节点向网络广播数据，由消费者节点消费。生产者根据请求（拉模型）或没有特定请求（推模型）发送此数据。（PDO）



## 14.2 CANopen功能描述 

**（1）报文分类**

![image-20230718171918274](https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/freertos/picture/image-20230718171918274.png)

**（2）对象字典**

  对象字典（OD：Object Dictionary）是CANopen的**核心概念**，网络中的每一个 CANopen设备都有一个对象字典。对象字典是一组有序的数据对象的集合，**这些对象描述了该设备的所有通讯和设备参数**，并且通过**16位的索引（index）和8位的子索引（subindex）**来确定其在对象字典中的位置。



## 14.3 SDO(服务数据对象）报文说明

**SDO 主要用于CANopen主站对从节点的参数配置**。服务确认是SDO的最大的特点，为每个消息都生成一个应答，确保数据传输的准确性。通常CANopen从节点作为SDO服务器，CANopen主节点作为客户端（称为 CS 通讯）。SDO 客户端通过索引和子索引，能够访问SDO服务器上的对象字典.

![image-20230718192639042](https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/freertos/picture/image-20230718192639042.png)

![image-20230718192655832](https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/freertos/picture/image-20230718192655832.png)

![image-20230718192750484](https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/freertos/picture/image-20230718192750484.png)

​                                                                                                 **Master(主)  slave(从)**

![image-20230718192810719](https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/freertos/picture/image-20230718192810719.png)

## 14.4 NMT（网络管理）报文说明

**什么是NMT？**

​          CANopen网络为了稳定可靠可控，都需要置一个**网络管理主机NMT-Master（Network Management-Master）**。所以每个 CANopen 从节点的 CANopen 协议栈中，必须具备 NMT 管理的相应代码，这是节点具备 CANopen 协议的最基本的要素。

​         **NMT主机**一般是CANopen网络中具备监控的PLC或者PC（**当然也可以是一般的功能节点**），所以也成为CANopen主站。相对应的其他CANopen节点就是NMT从机(NMT-slaves)。

NMT主机和NMT从机之间**通讯的报文就称为NMT网络管理报文**。管理报文负责层管理、网络管理和 ID 分配服务。例如，初始化、配置和网络管理（其中包括节点保护）。网络管理中，**同一个网络中只允许有一个主节点、一个或多个从节点，并遵循主从模式**。


**（1）NMT节点状态**

![image-20230718193340160](https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/freertos/picture/image-20230718193340160.png)

**（2）NMT从站节点上线报文**

​         任何一个NMT从站节点上线后，为了提示主站它已加入网络，这个从站必须发送节点上线报文，如下表：

![image-20230718193722243](https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/freertos/picture/image-20230718193722243.png)

**（3）NMT从站节点状态与心跳报文**

​        为了监控CANopen节点是否在线与目前的节点状态。CANopen 应用中通常都**要求在线上电的从站定时发送心跳报文**，以便于主站确认从站是否故障、是否脱离网络。

​       心跳报文格式如下表：

​                                   ![image-20230718194636143](https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/freertos/picture/image-20230718194636143.png)

CANopen从站按其对象字典中1017h中填写的心跳生产时间（ms）进行心跳报文的发送，而CANopen主站（NMT 主站）则会按其1016h中填写的心跳消费时间进行检查，**假设超过若干次心跳消费时间没有收到从站的心跳报文，则认为从站已经离线或者损坏。**



**（4）NMT节点状态切换命令**

​         NMT网络管理中，最核心的就是NMT节点状态切换命令，这是NMT主站所进行网络管理的

“命令”报文。 使用者必须牢记这些命令。

​        COB-ID均为 0x000，具备最高的CAN优先级，**数据为2个字节**：

![image-20230718200702735](https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/freertos/picture/image-20230718200702735.png)

如果要对整个网络所有节点进行控制，**则数据2的节点地址为0x00即可**

**（5）NMT心跳报文配置例程**

![image-20230718200942134](https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/freertos/picture/image-20230718200942134.png)

**1000的16进制是03E8h,对象字典中1017h,2B是M->S进行设置**

在 CAN（Controller Area Network）通信协议中，数据发送的顺序**是从低位开始发送**



## 14.5 PDO（过程数据对象）报文说明

PDO属于过程数据，用来传输实时数据，即单向传输，无需接收节点回应CAN报文来确认，从通讯术语上来说是属于“生产消费”模型。

在PDO预定义中，人为规定了TPDO和RPDO，规定了Node-ID在PDO中的位置，规定了 PDO 的编号。PDO分为TPDO（发送PDO）和(接收RPDO)，发送和接收是以CANopen节点自身为参考（如果CAN主站或者其他从站就相反）。 **TPDO和RPDO分别有4个数据对象，每种数据对象就是1条CAN报文封装，这些都是数据收发的容器。**

**（1）PDO的COB-ID定义**

T（transmit)   R(receive)

![image-20230718201835098](https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/freertos/picture/image-20230718201835098.png)

**(2)PDO的传输形式**

![image-20230718202033832](https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/freertos/picture/image-20230718202033832.png)

**（3）PDO通信参数**

PDO通信参数，定义了该设备所使用的COB-ID、传输类型、定时周期等。RPDO通讯参数位于对象字典索引的0x1400至0x15FF，TPDO通讯参数位于对象字典索引的0x1800至0x19FF。每条索引代表一个PDO的通信参数集，其中的子索引分别指向具体的各种参数。如下表所示：

![image-20230718202923250](https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/freertos/picture/image-20230718202923250.png)

**子索引个数**：即本索引下子索引个数；

**COB-ID**:即这个PDO发送或接收时对应的CAN帧ID；

**发送类型**：本产品目前仅支持两种PDO触发方式；

**生产禁止约束时间**：约束 PDO发送的最小间隔，避免导致总线负载剧烈增加；

**定时器触发时间**：此参数定义PDO传输形式定时器触发方式的时间。



**（4）PDO的映射参数**

​                                             ![image-20230718203206758](https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/freertos/picture/image-20230718203206758.png)

PDO映射个数：当前索引下映射的对象个数，是子索引1/2/3/4下已映射的总和；

PDO1/2/3/4：填入待映射的对象字典的信息，如索引，子索引，数据类型；



**(5) PDO配置例程**

配置母线电压（0x2048）改变时上传

![image-20230718203531565](https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/freertos/picture/image-20230718203531565.png)

**（6）保存PDO映射**

![image-20230718203613866](https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/freertos/picture/image-20230718203613866.png)



## 14.6 CiA402说明

​        CiA402协议中定义了运动控制设备的标准状态机，同时还定义了各种运行模式，以及它们在对象字典中的定义。

​        标准状态机（State machine）描述了设备的状态和驱动可能的控制序列。每一步状态表示了一个特定的内部或者外部行为，设备的状态也决定了哪些命令可以被接收。

![image-20230718204723113](https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/freertos/picture/image-20230718204723113.png)

- 状态机各状态对应说明如下表：

![image-20230718204751500](https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/freertos/picture/image-20230718204751500.png)

![image-20230718204805300](https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/freertos/picture/image-20230718204805300.png)

驱动器状态机通过控制字（对象 6040h）的bit0~bit3、 bit7位来控制，具体描述如下表：

![image-20230718205030876](https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/freertos/picture/image-20230718205030876.png)

状态机中各个状态可以通过状态字（对象6041h）的 bit0~bit3、bit5、bit6 显示，具体描述如下表：

![image-20230718205100779](https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/freertos/picture/image-20230718205100779.png)

（2）控制字与状态字

​        **驱动器的启停控制指令和状态描述主要通过控制字6040h与状态字6041h 实现，因此对控制字和状态字的熟练使用十分必要，下表简要描述了控制字和状态字各位的定义。**

![image-20230718205336074](https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/freertos/picture/image-20230718205336074.png)

实例： 上电后对驱动器初始化操作，初始化后进入正常工作状态，该操作一般在上电后进行。

![image-20230718210146411](https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/freertos/picture/image-20230718210146411.png)

CANopen通过**对象 6060h（Mode of Operation）**对驱动器工作模式进行设置，并通过对象**6061h（Mode of operation display）**反映驱动器当前的工作模式状态。 ZLAC8015系列驱动器目前支持 3 种工作模式：**位置模式**（Profile Position Mode），**速度模式**（Profile Velocity Mode），**转矩模式**（Profile Torque Mode）。

![image-20230718210310572](https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/freertos/picture/image-20230718210310572.png)

## 14.7 位置模式

**（1）位置模式说明**

位置模式采用**S形加减速曲线**实现，用户可以通过**总线设置起始速度（地址200800h）**、**最大速度（地址 608100h）**、**加速时间（地址 608300h）**、**减速时间（地址 608400h）** 和**总脉冲数（地址 607A00h）**几个参数来实现精确的位置控制。S形加减速曲线如下图所示。

![image-20230718210747277](https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/freertos/picture/image-20230718210747277.png)

**当用户设定的总脉冲数个数较少时，电机可能在加速到最大速度之前就需要进行减速（即电机实际运行过程中未加速到用户设定的最大速度）**，速度曲线如下图所示。图中实线所示为电机实际运行曲线，虚线为要加速到设定最大速度需要运行的曲线。理论总脉冲数为按照用户设定参数（起始速度、最大速度、加速时间、减速时间）计算得到的最小总脉冲数。当用户设定的总脉冲数小于理论总脉冲数时，电机就会按下图中实线运行。

![image-20230718210902748](https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/freertos/picture/image-20230718210902748.png)

- **相关对象字典内容**

![image-20230718211046005](https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/freertos/picture/image-20230718211046005.png)

控制字和状态字

位置模式下的控制字通过bit4~bit6、 bit8进行控制：****

![image-20230718211119761](https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/freertos/picture/image-20230718211119761.png)

- **状态字的Bit10、Bit12、Bit15显示驱动器状态**

​                                                     ![image-20230718211228071](https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/freertos/picture/image-20230718211228071.png)





**（2）位置模式配置例程**

![image-20230718211403925](https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/freertos/picture/image-20230718211403925.png)



## 14.8 对程序的解读

**（1）以这段代码为例：**

![image-20230719091321703](https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/freertos/picture/image-20230719091321703.png)

**这里发送的是SDO报文，0x23是命令字，说明是M->S发送4个字节的数据，data[0],data[1]是索引，6083h是指设置加速时间（地址 608300h），data[3]0x00h是子索引，命令字设置了数据是4字节，所以data[4]是数据，96h是指加速时间设置为150ms.**



![image-20230719091304843](https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/freertos/picture/image-20230719091304843.png)



**（2）解读这段代码**

![image-20230719101427052](https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/freertos/picture/image-20230719101427052.png)

这里可能会产生疑问？为什么控制板给驱动写入控制字映射和目标位置是使用RPDO呢？

**我的理解：因为PDO属于过程数据，用来传输实时数据，即单向传输，无需接收节点回应CAN报文来确认，从通讯术语上来说是属于“生产消费”模型**

**那么此时驱动属于生产者，而控制板属于消费者吧，所以需要使用接收过程数据对象RPDO**

RPDO用于从其他节点接收数据，而TPDO用于向其他节点发送数据。这两种消息类型都包含一个数据对象以及一些控制和标识信息。

RPDO（Receive Process Data Object）：

- RPDO是用于接收（接收进程数据）的消息类型。
- **RPDO的发送者是数据提供者（服务器），接收者是数据使用者（客户端）**。
- 服务器可以配置 RPDO 的传输频率，使其以特定的时间间隔或在触发条件下发送数据。
- **客户端通过配置和映射来指定希望接收的 RPDO。**

TPDO（Transmit Process Data Object）：

- TPDO是用于发送（传输进程数据）的消息类型。
- TPDO的发送者是数据使用者（客户端），接收者是数据提供者（服务器）。
- 客户端可以配置和映射 TPDO，以指定要发送的数据和传输频率。
- 当客户端的数据更新时，TPDO将定期或在触发条件下发送数据给服务器。



# 15.FreeRTOS工程

参考资料：[(83条消息) STM32CubeMX学习笔记（28）——FreeRTOS实时操作系统使用（任务管理）_freertos oskernelstart_Leung_ManWah的博客-CSDN博客](https://leung-manwah.blog.csdn.net/article/details/122037092?spm=1001.2014.3001.5502)

[(83条消息) CubeMX使用FreeRTOS编程指南_cube freertos_Top嵌入式的博客-CSDN博客](https://blog.csdn.net/qq_45396672/article/details/120877303?ops_request_misc=%7B%22request%5Fid%22%3A%22168975472116782425197389%22%2C%22scm%22%3A%2220140713.130102334..%22%7D&request_id=168975472116782425197389&biz_id=0&utm_medium=distribute.pc_search_result.none-task-blog-2~all~top_positive~default-1-120877303-null-null.142^v90^control_2,239^v2^insert_chatgpt&utm_term=cubemx freertos&spm=1018.2226.3001.4187)

注意：在设置freertos时，软件定时器需要使能

![image-20230719203402876](https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/freertos/picture/image-20230719203402876.png)

## 15.1 任务管理

### 1.主要API

 **1.osThreadId**

任务ID。例如，对`osThreadCreate`的调用返回。可用作参数到`osThreadTerminate`以删除任务。

```c++
typedef TaskHandle_t osThreadId;
```

（任务句柄）

```c++
osThreadId LED0Handle;
```



**2.osThreadCreate(创建任务)**

**使用动态/静态内存的方法创建一个任务。**

![image-20230719211240257](https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/freertos/picture/image-20230719211240257.png)



**3.osThreadTerminate（删除任务）**

**删除任务。任务被删除后就不复存在，也不会再进入运行态。**

![](https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/freertos/picture/image-20230719211334120.png)



**4.osKernelStart（开启调度器）**

​        在创建完任务的时候，我们需要开启调度器，因为创建仅仅是把任务添加到系统中，还没真正调度，并且空闲任务也没实现，定时器任务也没实现，这些都是在开启调度函数 osKernelStart() 中实现的。为什么要空闲任务？因为 FreeRTOS 一旦启动，就必须要保证系统中每时每刻都有一个任务处于运行态（Runing），并且空闲任务不可以被挂起与删除，空闲任务的优先级是最低的，以便系统中其他任务能随时抢占空闲任务的 CPU 使用权。 
![image-20230719211545208](https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/freertos/picture/image-20230719211545208.png)



### 2.任务的优先级

**Tick**:FreeRTOS 中也有心跳，它使用定时器产生固定间隔的中断。这叫 Tick、滴答，

![image-20230719212151272](https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/freertos/picture/image-20230719212151272.png)

**优先级实验**

```c++
 /* definition and creation of LED0 */
  osThreadDef(LED0, LED0_Task, osPriorityIdle, 0, 128);
  LED0Handle = osThreadCreate(osThread(LED0), NULL);

  /* definition and creation of LED1 */
  osThreadDef(LED1, LED1_Task, osPriorityLow, 0, 128);
  LED1Handle = osThreadCreate(osThread(LED1), NULL);
```

```c++
void LED0_Task(void const * argument)
{
  /* USER CODE BEGIN LED0_Task */
  /* Infinite loop */
       while (1)
  {
      ToggLED0();
      printf("大只0测试\n");
     
  }
  /* USER CODE END LED0_Task */
}
void LED1_Task(void const * argument)
{
  /* USER CODE BEGIN LED1_Task */
  /* Infinite loop */
    while (1)
  {
     ToggLED1();
     printf("大只1测试\n");
    // osDelay(1000);
  }
  /* USER CODE END LED1_Task */
}
```

实验结果：可以发现优先级高的LED1_Task一直在执行

<img src="https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/freertos/picture/image-20230719212651532.png" alt="image-20230719212651532" style="zoom:50%;" />



### 3.任务状态

**非运行状态：**

-  **阻塞状态(Blocked)**
-  **暂停状态(Suspended)**
-  **就绪状态(Ready)**

​	

状态转换图：

<img src="https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/freertos/picture/image-20230719213345102.png" alt="image-20230719213345102" style="zoom:50%;" />

**相关API：**

​     **1. osThreadSuspend**（任务挂起）

**挂起指定任务。被挂起的任务绝不会得到 CPU 的使用权，不管该任务具有什么优先级。**

![image-20230719213555944](https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/freertos/picture/image-20230719213555944.png)

**2.osThreadSuspendAll**

**将所有的任务都挂起。**

![image-20230719213848162](https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/freertos/picture/image-20230719213848162.png)

3.**osThreadResume（任务就绪）**

**让挂起的任务重新进入就绪状态，恢复的任务会保留挂起前的状态信息，在恢复的时候根据挂起时的状态继续运行。如果被恢复任务在所有就绪态任务中，处于最高优先级列表的第一位，那么系统将进行任务上下文的切换。可用在中断服务程序中。**

![image-20230719213733030](https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/freertos/picture/image-20230719213733030.png)

**4.osThreadResumeAll**

**将所有的任务都恢复。**

![image-20230719213929220](https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/freertos/picture/image-20230719213929220.png)



### 4.空闲任务及其钩子函数

**空闲任务的功能：**

**1.释放被删除的任务的内存。**（如果使用 vTaskDelete()来删除任务，那么你就要确保空闲任务有机会执行，否则就无法释放被删除任务的内存。）

**2.一个良好的程序，它的任务都是事件驱动的：平时大部分时间处于阻塞状态。有可能我们自己创建的所有任务都无法执行，但是调度器必须能找到一个可以运行的任务：所以，我们要提供空闲任务。**

**特点：**

- **空闲任务优先级为 0：它不能阻碍用户任务运行.**
- **空闲任务要么处于就绪态，要么处于运行态，永远不会阻塞.**



**钩子函数：**

**作用：**

- **执行一些低优先级的、后台的、需要连续执行的函数**
- **测量系统的空闲时间：空闲任务能被执行就意味着所有的高优先级任务都停止了，所以测量空闲任务占据的时间，就可以算出处理器占用率。**
- **让系统进入省电模式：空闲任务能被执行就意味着没有重要的事情要做，当然可以进入省电模式了。**

空闲任务钩子函数主要目的就是调用 WFI 指令使 STM32F103 进入睡眠模式，在进入和退出低功耗模式的时候也可以做一些其他处理，比如关闭外设时钟等等，用法和Tickless 模式类似。



**使用钩子函数的前提：**

在 FreeRTOS\Source\tasks.c 中，可以看到如下代码，所以前提就是：

- **把这个宏定义为 1：configUSE_IDLE_HOOK**
- **实现 vApplicationIdleHook 函数**

![image-20230720104431767](https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/freertos/picture/image-20230720104431767.png)

实验：在freertos.c添加钩子函数

![image-20230720110945704](https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/freertos/picture/image-20230720110945704.png)

结果：led灯点亮



<img src="C:/Users/su/AppData/Roaming/Typora/typora-user-images/image-20230720111009021.png" alt="image-20230720111009021" style="zoom:50%;" />



### 5.调度算法

**1.配置调度算法**

通过配置文件FreeRTOSConfifig.h的两个配置项来配置调度算法：confifigUSE_PREEMPTION、confifigUSE_TIME_SLICING。

列表如下：

![image-20230720111625683](https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/freertos/picture/image-20230720111625683.png)

- **A：可抢占+时间片轮转+空闲任务让步**
- **B：可抢占+时间片轮转+空闲任务不让步**
- **C：可抢占+非时间片轮转+空闲任务让步**
- **D：可抢占+非时间片轮转+空闲任务不让步**
- **E：合作调度**





## 15.2 同步互斥与通信

涉及概念：任务通知(task notifification)、队列(queue)、事件组(event group)、信号量(semaphoe)、互斥量(mutex)等。

同一时间只能有一个人使用的资源，被称为**临界资源**。

我们使用了“休眠-唤醒”的同步机制实现了“临界资源”的“互斥访问”。

### 进程与进程之间的通信方式

### 1.各类方法的对比

<img src="https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/freertos/picture/image-20230720112252738.png" alt="image-20230720112252738" style="zoom: 80%;" />



## 15.3 队列

参考资料：[(83条消息) STM32CubeMX学习笔记（29）——FreeRTOS实时操作系统使用（消息队列）_cubemx freertos 消息队列_Leung_ManWah的博客-CSDN博客](https://leung-manwah.blog.csdn.net/article/details/122187066)

### **1.队列的特性**

- 队列可以包含若干个数据：队列中有若干项，这被称为"长度"(length)
- 每个数据大小固定
- 创建队列时就要指定长度、数据大小
- 数据的操作采用先进先出的方法(FIFO，First In First Out)：写数据时放到尾部，读数据时从头部读
- 也可以强制写队列头部：覆盖头部数据

![image-20230720113730051](https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/freertos/picture/image-20230720113730051.png)

详细操作：

![image-20230720124114103](https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/freertos/picture/image-20230720124114103.png)

### 2.传输数据的两种方法

使用队列传输数据时有两种方法：

- 拷贝：把数据、把变量的值复制进队列里
- 引用：把数据、把变量的地址复制进队列里

freertos使用拷贝值的方法：

- 无需分配buffffer来保存数据，队列中有buffer
- 局部变量可以马上再次使用
- 发送任务、接收任务解耦：接收任务不需要知道这数据是谁的、也不需要发送任务来释放数据
- 如果数据实在太大，你还是可以使用队列传输它的地址
- 队列的空间有FreeRTOS内核分配，无需任务操心对于有内存保护功能的系统，如果队列使用引用方法，也就是使用地址，必须确保双方任务对这个

​       地址都有访问权限。使用拷贝方法时，则无此限制：内核有足够的权限，把数据复制进队列、再把数据复制出队列。



### 3.队列的阻塞访问

任务读写队列时，如果读写不成功就进入阻塞状态；能读写了，就马上进入就绪态；

同时也能设定阻塞时间，。如果队列有数据了，则该阻塞的任务会变为就绪态。如果一直都没有数据，则时间到之后它也会进入就绪态。

**有多个任务在等待同一个队列的数据。当队列中有数据时，哪个任务会进入就绪态？**

- **优先级最高**的任务
- 如果大家的**优先级相同**，那**等待时间最久**的任务会进入就绪态



### 4.队列函数(主要API)

#### **1.osThreadDef**（定义线程的属性和配置）

`osThreadDef`是ARM Cortex-M系列微控制器上使用的CMSIS-RTOS RTX中的宏定义，用于定义线程的属性和配置。

```C++
osThreadDef(thread_name, thread_func, priority, instances, stack_size);
```

- `thread_name`：线程的名称，作为唯一标识符。
- `thread_func`：线程的入口函数，即线程创建后首先执行的函数。
- `priority`：线程的优先级，决定了线程在调度时的执行顺序。
- `instances`：线程实例的数量，指定可以同时创建多少个相同的线程。
- `stack_size`：线程堆栈的大小，用于保存线程执行过程中的局部变量和中间结果。

举例：

```c++
osThreadDef(Receive, ReceiveTask, osPriorityIdle, 0, 128);
```



#### **2.osMessageQDef（定义消息队列的大小、数据类型和名称）**

在CMSIS-RTOS RTX中，osMessageQDef是一个宏，用于定义消息队列的属性。它提供了一种简化的方式来定义消息队列的大小、数据类型和名称。

```c++
osMessageQDef(queue_name, queue_size, data_type);
```

- `queue_name` 是您为消息队列指定的名称。
- `queue_size` 是消息队列的容量，即最大可以容纳的消息数量。
- `data_type` 是消息队列中消息的数据类型。

举例：

```
osMessageQDef(TestQueue, 16, uint32_t);
```



#### **3.osMessageQId**

**队列ID**。例如，对`osMessageCreate`的调用返回。可用作参数到`osMessageDelete`以删除队列。

```c++
typedef QueueHandle_t osMessageQId;
```

举例：

```
osMessageQId TestQueueHandle;
```

#### **4. osMessageCreate**

**使用动态内存的方式创建一个新的队列**

![image-20230721163653886](https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/freertos/picture/image-20230721163653886.png)

举例：

```c++
  osMessageQDef(TestQueue, 16, uint32_t);
  TestQueueHandle = osMessageCreate(osMessageQ(TestQueue), NULL);
```



#### **5.osMessageDelete**

**队列删除函数是根据消息队列ID直接删除的，删除之后这个消息队列的所有信息都会被系统回收清空，而且不能再次使用这个消息队列了。**

![image-20230721165554542](https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/freertos/picture/image-20230721165554542.png)

举例：

```
osMessageDelete(TestQueueHandle);
```



**消息发送与接收**

#### **6.osMessagePut(发送消息）**

**用于向队列尾部发送一个队列消息。消息以拷贝的形式入队，而不是以引用的形式。可用在中断服务程序中。**

![image-20230721165848692](https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/freertos/picture/image-20230721165848692.png)

举例：

```c++
void SendTask(void const * argument)
{
  /* USER CODE BEGIN SendTask */
    osEvent xReturn;
    uint32_t send_data1 = 1;
    uint8_t key;
  /* Infinite loop */
  for(;;)
  {
      key = key_scan(0); /* 得到键值 */
      printf("key=%d\n",key);
     if(key==2) 
    { 
        /* KEY1 被按下 */ 
        printf("send_data1!\n"); 
        xReturn.status = osMessagePut(TestQueueHandle, /* 消息队列的句柄 */ 
                                      send_data1,      /* 发送的消息内容 */ 
                                      0);              /* 等待时间 0 */ 
        if(osOK != xReturn.status)  
        {
            printf("send fail!\n\n"); 
        }
    } 
    osDelay(100);

  }
  /* USER CODE END SendTask */
}
```



#### **6.osMessageGet（接收消息）**

**用于从一个队列中接收消息并把消息从队列中删除。接收的消息是以拷贝的形式进行的，所以我们必须提供一个足够大空间的缓冲区。具体能够拷贝多少数据到缓冲区，这个在队列创建的时候已经设定。可用在中断服务程序中。**

![image-20230721171639850](https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/freertos/picture/image-20230721171639850.png)

```c++
void ReceiveTask(void const * argument)
{
  /* USER CODE BEGIN ReceiveTask */
    osEvent event;
  /* Infinite loop */
  for(;;)
  {
    event = osMessageGet(TestQueueHandle, /* 消息队列的句柄 */ 
                          osWaitForever); /* 等待时间 一直等 */ 
    if(osEventMessage == event.status) 
    {
        printf("receive data:%d\n\n", event.value.v); 
    }
    else 
    {
        printf("error: 0x%d\n", event.status); 
    }

  }
  /* USER CODE END ReceiveTask */
}
```



#### **7.osMessagePeek**

**osMessagePeek() 也是从从队列中接收数据单元，不同的是并不从队列中删出接收到的单元。osMessagePeek() 从队列首接收到数据后，不会修改队列中的数据，也不会改变数据在队列中的存储序顺。可用在中断服务程序中。**

![image-20230721172229314](https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/freertos/picture/image-20230721172229314.png)



#### **8.osMessageWaiting**

**用于查询队列中当前有效数据单元个数。**

![image-20230722110813195](https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/freertos/picture/image-20230722110813195.png)

```c++
void StartDefaultTask(void const * argument)
{
    uint32_t nums;
  /* USER CODE BEGIN StartDefaultTask */
  /* Infinite loop */
  for(;;)
  {
     nums=osMessageWaiting(TestQueueHandle);
      printf("data numbers:%d\n\n", nums); 
    osDelay(1);
  }
  /* USER CODE END StartDefaultTask */
}
```



### 5.实验

在创建工程时，记住要勾上Use MicroLIB

![image-20230721151212394](https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/freertos/picture/image-20230721151212394.png)

按下按键key1（不灵敏，因为按键没有使用到中断），可以看到发送消息任务有发送任务，接收消息任务有接收消息

<img src="https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/freertos/picture/image-20230721173148904.png" alt="image-20230721173148904" style="zoom:50%;" />





## 15.4 信号量

**参考资料：**[myBlogResource/freertos/FreeRTOS完全开发手册之上册_快速入门.pdf at main · su-ron/myBlogResource (github.com)](https://github.com/su-ron/myBlogResource/blob/main/freertos/FreeRTOS完全开发手册之上册_快速入门.pdf)

传递状态，并不需要传递具体的信息，此时需要用到信号量，它更节省内存



### 1.信号量的特性

计数型信号量的典型场景是：

- **计数：事件产生时"give"信号量，让计数值加1；处理事件时要先"take"信号量，就是获得信号量，让计数值减1。**
- **资源管理：要想访问资源需要先"take"信号量，让计数值减1；用完资源后"give"信号量，让计数值加1。**



二进制信号量跟计数型的唯一差别，就是计数值的最大值被限定为1。

![image-20230722112411575](https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/freertos/picture/image-20230722112411575.png)



**信号量与队列的对比：**

![image-20230722112607206](https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/freertos/picture/image-20230722112607206.png)

**两种信号量的对比：**

信号量的计数值都有限制：限定了最大值。如果最大值被限定为1，那么它就是二进制信号量；如果最大值不是1，它就是计数型信号量。

![image-20230722113023524](https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/freertos/picture/image-20230722113023524.png)





### 2.信号量函数(相关API)

#### 0.osSemaphoreId

osSemaphoreId是一个数据类型，用于表示信号量的标识符或句柄。

举例：

```c++
osSemaphoreId CountSemHandle;
osSemaphoreId BinarySemHandle;
```

**osThreadDef**

osThreadDef是一个宏，用于定义线程的属性。它提供了一种简化的方式来定义线程的名称、入口函数、优先级、堆栈大小和其他属性。

```c++
osThreadDef(thread_name, thread_function, priority, instances, stack_size);
```

- `thread_name` 是您为线程指定的名称。
- `thread_function` 是线程的入口函数，即线程启动时要执行的函数。
- `priority` 是线程的优先级，指定线程相对于其他线程的执行顺序。
- `instances` 是线程实例的数量。对于CMSIS-RTOS RTX，通常将其设置为0。
- `stack_size` 是堆栈的大小，即线程所需的内存空间。

举例：

```
osThreadDef(defaultTask, StartDefaultTask, osPriorityNormal, 0, 128);
```



#### 1.osSemaphoreCreate

**用于创建一个二值信号量，并返回一个ID。**

![image-20230722145621702](https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/freertos/picture/image-20230722145621702.png)

举例：

```c++
osThreadId defaultTaskHandle;
defaultTaskHandle = osThreadCreate(osThread(defaultTask), NULL);
```



#### 2.osSemaphoreDelete

**用于删除一个信号量，包括二值信号量，计数信号量，互斥量和递归互斥量。如果有任务阻塞在该信号量上，那么不要删除该信号量。**

![image-20230722145703238](https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/freertos/picture/image-20230722145703238.png)

#### 3.osSemaphoreRelease

**用于释放信号量的宏。释放的信号量对象必须是已经被创建的，可以用于二值信号量、计数信号量、互斥量的释放，但不能释放由函数 xSemaphoreCreateRecursiveMutex() 创建的递归互斥量。可用在中断服务程序中。**

![image-20230722145747071](https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/freertos/picture/image-20230722145747071.png)



举例：

```c++
void SendTask(void const * argument)
{
  /* USER CODE BEGIN SendTask */
   osStatus xReturn;
    uint8_t key;
  /* Infinite loop */
  for(;;)
  {
       key = key_scan(0); /* 得到键值 */
    //  printf("key=%d\n",key);
    if(key==2) 
    { 
        xReturn = osSemaphoreRelease(BinarySemHandle);//给出二值信号量 
        if(osOK == xReturn)
        {
            printf("release!\r\n"); 
        }
        else 
        {
            printf("BinarySem release fail!\r\n"); 
        }
    } 
    osDelay(100);
  }
  /* USER CODE END SendTask */
}
```



#### **4.osSemaphoreWait**

**用于获取信号量，不带中断保护。获取的信号量对象可以是二值信号量、计数信号量和互斥量，但是递归互斥量并不能使用这个 API 函数获取。可用在中断服务程序中。**

![image-20230722145830804](https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/freertos/picture/image-20230722145830804.png)

举例：

```c++
void ReceiveTask(void const * argument)
{
  /* USER CODE BEGIN ReceiveTask */
      osStatus xReturn = osErrorValue;
  /* Infinite loop */
  for(;;)
  {
    xReturn = osSemaphoreWait(BinarySemHandle, /* 二值信号量句柄 */ 
                               osWaitForever); /* 等待时间 */ 
    if(osOK == xReturn) 
    {
       LED0(0);
       HAL_Delay(1000);
       printf("BinarySem get!\n\n");
       LED0(1);
    }
  }
  /* USER CODE END ReceiveTask */
}
```





#### 3.二值信号量

运作机制

![image-20230722152728771](C:/Users/su/AppData/Roaming/Typora/typora-user-images/image-20230722152728771.png)

创建信号量时，系统会为创建的信号量对象分配内存，并把可用信号量初始化为用户自定义的个数， 二值信号量的最大可用信号量个数为 1。



#### 4.计数信号量

运作机制

![image-20230722153130637](https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/freertos/picture/image-20230722153130637.png)



## 15.5 互斥量

参考资料：[(96条消息) STM32CubeMX学习笔记（31）——FreeRTOS实时操作系统使用(互斥量)_osmutexcreate_Leung_ManWah的博客-CSDN博客](https://leung-manwah.blog.csdn.net/article/details/122230132)

- 量：值为0、1
- 互斥：用来实现互斥访问
- 它的核心在于：谁上锁，就只能由谁开锁



### 1.使用场合

- 访问外设：例如多个任务进程访问串口
- 读、修改、写操作导致的问题
- 对于同一个变量，比如 int a ，如果有两个任务同时写它就有可能导致问题。
- 对于变量的修改，C代码只有一条语句，比如： a=a+8; ，它的内部实现分为3步：读出原值、修改、写入。

![image-20230727151749485](https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/freertos/picture/image-20230727151749485.png)

我们想让任务A、B都执行add_a函数，a的最终结果是 1+8+8=17 。假设任务A运行完代码①，在执行代码②之前被任务B抢占了：现在任务A的R0等于1。

任务B执行完add_a函数，a等于9。任务A继续运行，在代码②处R0仍然是被抢占前的数值1，执行完②③的代码，a等于9，这跟预期的17不符合。



**临界资源**

上述问题的解决方法是：任务A访问这些全局变量、函数代码时，独占它，就是上个锁。这些**全局变量、函数代码必须被独占地使用**，它们被称为**临界资源**。



互斥量也被称为互斥锁，使用过程如下：

- **互斥量初始值为1**
- **任务A想访问临界资源，先获得并占有互斥量，然后开始访问**
- **任务B也想访问临界资源，也要先获得互斥量：被别人占有了，于是阻塞**
- **任务A使用完毕，释放互斥量；任务B被唤醒、得到并占有互斥量，然后开始访问临界资源**
- **任务B使用完毕，释放互斥量**



正常来说：在任务A占有互斥量的过程中，任务B、任务C等等，都无法释放互斥量。

但是FreeRTOS未实现这点：任务A占有互斥量的情况下，任务B也可释放互斥量。



**二值信号量与互斥量**

​        如果想要用于实现同步（任务之间或者任务与中断之间），二值信号量或许是更好的选择，虽然互斥量也可以用于任务与任务、任务与中断的同步，但是**互斥量更多的是用于保护资源的互锁。**

​      但是**二值信号量会导致的另一个潜在问题，那就是任务优先级翻转。**而 FreeRTOS 提供的互斥量可以通过**优先级继承算法**，可以降低优先级翻转问题产生的影响，所以，**用于临界资源的保护一般建议使用互斥量。**

​      



### 2.运作机制

<img src="https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/freertos/picture/image-20230727154033227.png" alt="image-20230727154033227" style="zoom: 67%;" />

用互斥量处理不同任务对临界资源的同步访问时，任务想要获得互斥量才能进行资源访问，如果一旦有任务成功获得了互斥量，则互斥量立即变为闭锁状态，此时其他任务会因为获取不到互斥量而不能访问这个资源，任务会根据用户自定义的等待时间进行等待，直到互斥量被持有的任务释放后，其他任务才能获取互斥量从而得以访问该临界资源，此时互斥量再次上锁，如此一来就可以确保每个时刻只有一个任务正在访问这个临界资源，保证了临界资源操作的安全性。


**互斥量与递归互斥量**

- 互斥量更适合于可能会引起优先级翻转的情况。
- 递归互斥量更适用于任务可能会多次获取互斥量的情况下。这样可以避免同一任务多次递归持有而造成死锁的问题。



### 3.相关API

**1.osMutexId MuxSemHandle;**//`MuxSemHandle`可以理解为一个变量名，用于表示一个互斥锁的句柄（handle）或引用

**2.osMutexDef(MuxSem);**定义了一个名为`MuxSem`的互斥锁对象



**3.osMutexCreate**

**用于创建一个互斥量，并返回一个互斥量ID。**

![image-20230727192608297](https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/freertos/picture/image-20230727192608297.png)

举例：

```c++
  osMutexDef(MuxSem);//定义了一个名为MuxSem的互斥锁对象
  MuxSemHandle = osMutexCreate(osMutex(MuxSem));
```



**4.osRecursiveMutexCreate**

用于创建一个递归互斥量，不是递归的互斥量由函数 osMutexCreate() 创建，且只能被同一个任务获取一次，如果同一个任务想再次获取则会失败。递归信号量则相反，它可以被同一个任务获取很多次，获取多少次就需要释放多少次。递归信号量与互斥量一样，都实现了优先级继承机制，可以减少优先级反转的反生。
![image-20230727192832165](https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/freertos/picture/image-20230727192832165.png)

```
 osRecursiveMutexId MuxSemHandle;  // 声明可重入互斥锁变量
 osMutexDef(MuxSem);//定义了一个名为MuxSem的互斥锁对象
  MuxSemHandle = osMutexCreate(osMutex(MuxSem));
```



**5.osMutexDelete**

**用于删除一个互斥量。**

![image-20230727193457783](https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/freertos/picture/image-20230727193457783.png)



**6.osMutexWait**

**用于获取互斥量，但是递归互斥量并不能使用这个 API 函数获取**

![image-20230727193559177](https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/freertos/picture/image-20230727193559177.png)

```c++
osStatus xReturn;
xReturn = osMutexWait(MuxSemHandle,     /* 互斥量句柄 */ 
                      osWaitForever);   /* 等待时间 */
```



**7.osRecursiveMutexWait**

用于获取递归互斥量的宏，与互斥量的获取函数一样，osMutexWait()也是一个宏定义，它最终使用现有的队列机制，实际执行的函数是 xQueueTakeMutexRecursive() 。 获取递归互斥量之前必须由 osRecursiveMutexCreate() 这个函数创建。要注意的是该函数不能用于获取由函数 osMutexCreate() 创建的互斥量。
![image-20230727194038630](https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/freertos/picture/image-20230727194038630.png)



**8.osMutexRelease**

**用于释放互斥量，但不能释放由函数 osRecursiveMutexCreate() 创建的递归互斥量。**

![image-20230727194305961](https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/freertos/picture/image-20230727194305961.png)

举例：

```c++
osStatus xReturn;
xReturn = osMutexRelease(MuxSemHandle);//给出互斥量 
```



**9.osRecursiveMutexRelease**

用于释放一个递归互斥量。已经获取递归互斥量的任务可以重复获取该递归互斥量。使用 osRecursiveMutexWait() 函数成功获取几次递归互斥量，就要使用 osRecursiveMutexRelease() 函数返还几次，在此之前递归互斥量都处于无效状态，别的任务就无法获取该递归互斥量。使用该函数接口时，只有已持有互斥量所有权的任务才能释放它，每释放一该递归互斥量，它的计数值就减 1。当该互斥量的计数值为 0 时（即持有任务已经释放所有的持有操作），互斥量则变为开锁状态，等待在该互斥量上的任务将被唤醒。如果任务的优先级被互斥量的优先级翻转机制临时提升，那么当互斥量被释放后，任务的优先级将恢复为原本设定的优先级。
![image-20230727194522793](https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/freertos/picture/image-20230727194522793.png)



### 4.实验

实验在 15.4freertos_mutexes中

关键代码

```c++
/* USER CODE END Header_LowPriorityTask */
void LowPriorityTask(void const * argument)
{
  /* USER CODE BEGIN LowPriorityTask */
     static uint32_t i; 
     osStatus xReturn;
  /* Infinite loop */
  for(;;)
  {
    printf("LowPriority_Task Get Mutex\n");
    //获取互斥量 MuxSem,没获取到则一直等待 
    xReturn = osMutexWait(MuxSemHandle,     /* 互斥量句柄 */ 
                          osWaitForever);   /* 等待时间 */
    if(osOK == xReturn) 
    {
        printf("LowPriority_Task Runing\n\n"); 
    }
    
    for(i = 0; i < 2000000; i++) 
    { //模拟低优先级任务占用互斥量 
        taskYIELD();//发起任务调度 
    } 
    
    printf("LowPriority_Task Release Mutex\r\n"); 
    xReturn = osMutexRelease(MuxSemHandle);//给出互斥量 
    
    osDelay(1000);
  }
  /* USER CODE END LowPriorityTask */
}

/* USER CODE BEGIN Header_MidPriorityTask */
/**
* @brief Function implementing the MidPriority thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_MidPriorityTask */
void MidPriorityTask(void const * argument)
{
  /* USER CODE BEGIN MidPriorityTask */
  /* Infinite loop */
  for(;;)
  {
      printf("MidPriority_Task Runing\n"); 
      osDelay(1000);
  }
  /* USER CODE END MidPriorityTask */
}

/* USER CODE BEGIN Header_HighPriorityTask */
/**
* @brief Function implementing the HighPriority thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_HighPriorityTask */
void HighPriorityTask(void const * argument)
{
  /* USER CODE BEGIN HighPriorityTask */
    osStatus xReturn;
  /* Infinite loop */
  for(;;)
  {
     printf("HighPriority_Task Get Mutex\n"); 
    //获取互斥量 MuxSem,没获取到则一直等待 
    xReturn = osMutexWait(MuxSemHandle,     /* 互斥量句柄 */ 
                          osWaitForever);   /* 等待时间 */
    if(osOK == xReturn) 
    {
        printf("HighPriority_Task Runing\n"); 
    }
  
    printf("HighPriority_Task Release Mutex\r\n"); 
    xReturn = osMutexRelease(MuxSemHandle);//给出互斥量

    osDelay(1000);

  }
  /* USER CODE END HighPriorityTask */
}
```

结果：

![image-20230727194826723](https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/freertos/picture/image-20230727194826723.png)



**5.谁上锁就由谁解锁**

CMsis_v1实现了互斥锁的功能

本节代码：15.freertos_mutex_who_give



实现结果

![image-20230727203212410](https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/freertos/picture/image-20230727203212410.png)



**6.优先级反转**

假设任务A、B都想使用串口，A优先级比较低：

- 任务A获得了串口的互斥量

- 任务B也想使用串口，它将会阻塞、等待A释放互斥量

- 高优先级的任务，被低优先级的任务延迟，这被称为"优先级反转"(priority inversion）

  

  就是如果有三个任务在调度，分别是低优先级，中优先级和高优先级，如果低优先级和高优先级共用一个互斥锁，那么在低优先级调度时上锁了，，然后高优先级由于获取不到互斥锁一直阻塞，而中优先级一直运行，导致低优先级的任务释放不了锁，从而导致高优先级任务执行不了。

  

  

<img src="https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/freertos/picture/image-20230728110446481.png" alt="image-20230728110446481" style="zoom:50%;" />



**7.优先级继承**

优先级继承：

- **假设持有互斥锁的是任务A，如果更高优先级的任务B也尝试获得这个锁**
- **任务B说：你既然持有宝剑，又不给我，那就继承我的愿望吧**
- **于是任务A就继承了任务B的优先级**
- **这就叫：优先级继承**
- **等任务A释放互斥锁时，它就恢复为原来的优先级**
- **互斥锁内部就实现了优先级的提升、恢复**



### 5.递归锁



**1.死锁的概念**

假设有2个互斥量M1、M2，2个任务A、B：

- **A获得了互斥量M1**
- **B获得了互斥量M2**
- **A还要获得互斥量M2才能运行，结果A阻塞**
- **B还要获得互斥量M1才能运行，结果B阻塞**
- **A、B都阻塞，再无法释放它们持有的互斥量**
- **死锁发生！**



**2.自我死锁**

假设这样的场景：

- **任务A获得了互斥锁M**
- **它调用一个库函数**
- **库函数要去获取同一个互斥锁M，于是它阻塞：任务A休眠，等待任务A来释放互斥锁！**
- **死锁发生！**



**3.那么怎么去解决这类死锁问题？**

使用递归锁，他的特性：

- **任务A获得递归锁M后，它还可以多次去获得这个锁**
- **"take"了N次，要"give"N次，这个锁才会被释放**



## 15.6 事件组

参考资料：[(97条消息) STM32CubeMX学习笔记（32）——FreeRTOS实时操作系统使用（事件）_Leung_ManWah的博客-CSDN博客](https://leung-manwah.blog.csdn.net/article/details/122249672)



### 1. **事件组概念与操作**

**1.1概念**

事件组可以简单地认为就是一个整数：

- **每一位表示一个事件**
- **每一位事件的含义由程序员决定，比如：Bit0表示用来串口是否就绪，Bit1表示按键是否被按下**
- **这些位，值为1表示事件发生了，值为0表示事件没发生**
- **一个或多个任务、ISR都可以去写这些位；一个或多个任务、ISR都可以去读这些位**
- **可以等待某一位、某些位中的任意一个，也可以等待多位**



​         事件是一种实现任务间通信的机制，主要用于实现多任务间的同步，**但事件通信只能是事件类型的通信，无数据传输**。 与信号量不同的是，它可以实现一对多，多对多的同步。即**一个任务可以等待多个事件的发生**：可以是任意一个事件发生时唤醒任务进行事件处理；也可以是**几个事件都发生后才唤醒任务进行事件处理**。同样，也可以是多个任务同步多个事件。

​       **每一位代表一个事件，任务通过“逻辑与”或“逻辑或”与一个或多个事件建立关联，形成一个事件组。** 事件的“逻辑或”也被称作是独立型同步，指的是任务感兴趣的所有事件任一件发生即可被唤醒；事件“逻辑与”则被称为是关联型同步，指的是任务感兴趣的若干事件都发生时才被唤醒，并且事件发生的时间可以不同步。

![image-20230728145545069](https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/freertos/picture/image-20230728145545069.png)

事件组用一个整数来表示，其中的高8位留给内核使用，只能用其他的位来表示事件。那么这个整数是多少位的？

- 如果**confifigUSE_16_BIT_TICKS是1**，那么这个整数就是16位的，低8位用来表示事件
- 如果**confifigUSE_16_BIT_TICKS是0**，那么这个整数就是32位的，低24位用来表示事件



**1.2 事件组的操作**

- **先创建事件组**
- **任务C、D等待事件：**

​           等待什么事件？可以等待某一位、某些位中的任意一个，也可以等待多位。简单地说就是"或"、"与"的关系。

​           得到事件时，要不要清除？可选择清除、不清除。

- **任务A、B产生事件：设置事件组里的某一位、某些位**



在 FreeRTOS 事件中，每个事件获取的时候，用户可以选择感兴趣的事件，并且选择读取事件信息标记，它有三个属性，分别是**逻辑与，逻辑或以及是否清除标记。**当任务等待事件同步时，**可以通过任务感兴趣的事件位和事件信息标记来判断当前接收的事件是否满足要**求，如果满足则说明任务等待到对应的事件，系统将唤醒等待的任务；否则，任务会根据用户指定的阻塞超时时间继续等待下去。


**1.3 运作机制**

![image-20230728151840165](https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/freertos/picture/image-20230728151840165.png)

<img src="https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/freertos/picture/image-20230728152542336.png" alt="image-20230728152542336" style="zoom:50%;" />

事件唤醒机制，当任务因为等待某个或者多个事件发生而进入阻塞态，当事件发生的时候会被唤醒。2



### **2.  相关API**

#### 1.osEventFlagsNew

**用于创建一个事件组，并返回对应的ID。**

![image-20230728163859895](https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/freertos/picture/image-20230728163859895.png)

举例：

```c++
osEventFlagsId_t EventHandle;
const osEventFlagsAttr_t Event_attributes = {
  .name = "Event"
};
EventHandle = osEventFlagsNew(&Event_attributes);
```



#### **2.osEventFlagsDelete**

**当系统不再使用事件对象时，可以通过删除事件对象控制块来释放系统资源。**

![image-20230728164235466](https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/freertos/picture/image-20230728164235466.png)



#### 3.osEventFlagsSet

用于置位事件组中指定的位，当位被置位之后，阻塞在该位上的任务将会被解锁。使用该函数接口时，通过参数指定的事件标志来设定事件的标志位，然后遍历等待在事件对象上的事件等待列表，判断是否有任务的事件激活要求与当前事件对象标志值匹配，如果有，则唤醒该任务。简单来说，就是设置我们自己定义的事件标志位为 1，并且看看有没有任务在等待这个事件，有的话就唤醒它。该函数可以在中断中使用。 
![image-20230728164339703](https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/freertos/picture/image-20230728164339703.png)

 举例：

```c++
#define KEY0_EVENT (0x01 << 0)//设置事件掩码的位 0 
#define KEY1_EVENT (0x01 << 1)//设置事件掩码的位 1
osEventFlagsSet(EventHandle, KEY0_EVENT); 
osEventFlagsSet(EventHandle, KEY1_EVENT); 
```



#### 4.osEventFlagsWait

**用于获取事件组中的一个或多个事件发生标志，当要读取的事件标志位没有被置位时任务将进入阻塞等待状态。**

![image-20230728164623764](https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/freertos/picture/image-20230728164623764.png)

举例：

```c++
void ReceiveTask(void *argument)
{
  /* USER CODE BEGIN ReceiveTask */
    uint32_t r_event; /* 定义一个事件接收变量 */
  /* Infinite loop */
  for(;;)
  {
     r_event = osEventFlagsWait(EventHandle,             /* 事件对象句柄 */ 
                              KEY1_EVENT|KEY0_EVENT,    /* 接收任务感兴趣的事件 */ 
                              osFlagsWaitAll,           /* 退出时清除事件位，同时满足感兴趣的所有事件 */ 
                              osWaitForever);           /* 指定超时事件,一直等 */ 
  
    if((r_event & (KEY1_EVENT|KEY0_EVENT)) == (KEY1_EVENT|KEY0_EVENT)) 
    { 
        /* 如果接收完成并且正确 */ 
        printf ("KEY1 and KEY2 down\n"); 
    } 
    else 
    {
        printf ("Event Error！\n"); 
    }

  }
  /* USER CODE END ReceiveTask */
}
```



#### 5.osEventFlagsClear

**用于清除事件组指定的位，如果在获取事件的时候没有将对应的标志位清除，那么就需要用这个函数来进行显式清除。该函数可以在中断中使用。**

![image-20230728165152545](https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/freertos/picture/image-20230728165152545.png)



### 3.实验

实验的具体代码在16.freertos_event文件夹中

按按键1显示KEY1 down

按按键2显示KEY2 down

同时按下则是显示KEY1 and KEY2 down(等待多个事件)

<img src="https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/freertos/picture/image-20230728165435565.png" alt="image-20230728165435565" style="zoom: 67%;" />







# 16.Linux驱动与内核

**参考文件：**

[呕心沥血整理的的嵌入式学习路线_一口Linux的博客-CSDN博客](https://blog.csdn.net/daocaokafei/article/details/108513929)

[从0学习Linux驱动 内核、目录 3_哔哩哔哩_bilibili](https://www.bilibili.com/video/BV19v411h7g9/?spm_id_from=333.788&vd_source=e6a100138906f3892c6413488ca8e688)

<img src="https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/freertos/picture/image-20230828204822196.png" alt="image-20230828204822196" style="zoom: 67%;" />
