## LCD实验 (2018.1.5)
1. `介绍`
```
* vsync : 垂直同步信号
* Hsync : 水平同步信号
* vclk  : 时钟
* VDEN  : 使能信号
* VD0 - VD23 : 数据信号
* LED+ LED- : 背光信号
```
## 2440和lcd的关系
![2440和lcd的关系](https://github.com/GalenDeng/Embedded-Linux/blob/master/16.%20LCD%E5%AE%9E%E9%AA%8C/2440%E4%B8%8Elcd%E7%9A%84%E5%85%B3%E7%B3%BB.JPG)
2. `像素颜色三部分: 红、绿、蓝`
```
显示器上的每个像素的颜色由3部分组成:红、绿、蓝，他们被称为三原色
```
3. `LCD控制器`
```
* LCD控制器支持单色(1BPP)、4级灰色(2BPP)、16级灰色(4BPP)、256色(8BPP)的调色板显示模式
* 16M(24BPP)色的显示模式就是使用24位的数据来表示一个像素的颜色 16M = 2^20 * 2^4,每种原色使用8位
```
4. `NC : not connect`
5. `调色板`
```
调色板就是一块内存，可以对每个索引值设置其颜色，可以使用24BPP和16BPP
```
## 调色板的作用
![调色板的作用](https://github.com/GalenDeng/Embedded-Linux/blob/master/16.%20LCD%E5%AE%9E%E9%AA%8C/%E8%B0%83%E8%89%B2%E6%9D%BF%E7%9A%84%E4%BD%9C%E7%94%A8.JPG)
6. `RGB:RED 、GREEN、BLUE`
* frame buffer : 帧缓冲区
## 帧内存和视口
![帧内存和视口](https://github.com/GalenDeng/Embedded-Linux/blob/master/16.%20LCD%E5%AE%9E%E9%AA%8C/%E5%B8%A7%E5%86%85%E5%AD%98%E5%92%8C%E8%A7%86%E5%8F%A3.JPG)
7. `快捷清屏操作`
* 设置LCD为某一种颜色 ==> TPAL ==> 2440(LCD控制器) ==> LCD 
8.`ChangePalette` 
```
void ChangePalette(UINT32 color)
{
    int i;
    unsigned char red, green, blue;
    UINT32 *palette;

/* 从0xRRGGBB格式的color变量中，提取8位红色值的高5位、8位绿色值的高6位、8位蓝色值的高5位*/
	red   = (color >> 19) & 0x1f;
	green = (color >> 10) & 0x3f;
	blue  = (color >>  3) & 0x1f;
// 组成 5:6:5 格式的16BPP颜色值
	color = (red << 11) | (green << 5) | blue; // 格式5:6:5
```
9. `像素`
* 对于16BPP模式,每个像素占据2字节;对于8BPP模式,每个像素占据1字节.