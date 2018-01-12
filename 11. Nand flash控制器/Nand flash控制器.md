## Nand flash控制器 (2017.12.18)
## 不同的寻址方式
![不同的寻址方式](https://github.com/GalenDeng/Embedded-Linux/blob/master/11.%20Nand%20flash%E6%8E%A7%E5%88%B6%E5%99%A8/Nand%20flash%E5%9B%BE%E7%89%87%E7%AC%94%E8%AE%B0/%E4%B8%8D%E5%90%8C%E7%9A%84%E5%AF%BB%E5%9D%80%E6%96%B9%E5%BC%8F.JPG)
* 凡是芯片的地址线和cpu的引脚直接相连的，这种连接方式是cpu统一编址的，cpu不经其他外设直接连接
* Nand flash 和 s3c2440只有数据线相连，而没有任何地址线相连，所以不属于cpu统一编址系列
## Nand flash结构
![Nand flash结构](https://github.com/GalenDeng/Embedded-Linux/blob/master/11.%20Nand%20flash%E6%8E%A7%E5%88%B6%E5%99%A8/Nand%20flash%E5%9B%BE%E7%89%87%E7%AC%94%E8%AE%B0/Nand%20flash%E7%BB%93%E6%9E%84.JPG)
## s3c2440访问Nand flash
![s3c2440访问Nand flash](https://github.com/GalenDeng/Embedded-Linux/blob/master/11.%20Nand%20flash%E6%8E%A7%E5%88%B6%E5%99%A8/Nand%20flash%E5%9B%BE%E7%89%87%E7%AC%94%E8%AE%B0/s3c2440%E8%AE%BF%E9%97%AENand%20flash.JPG)
## 从nand那里读取led代码进SDRAM中并启动
![从nand那里读取led代码进SDRAM中并启动](https://github.com/GalenDeng/Embedded-Linux/blob/master/11.%20Nand%20flash%E6%8E%A7%E5%88%B6%E5%99%A8/Nand%20flash%E5%9B%BE%E7%89%87%E7%AC%94%E8%AE%B0/%E4%BB%8Enand%E9%82%A3%E9%87%8C%E8%AF%BB%E5%8F%96led%E4%BB%A3%E7%A0%81%E8%BF%9BSDRAM%E4%B8%AD%E5%B9%B6%E5%90%AF%E5%8A%A8.JPG)
* `nand.lds`
```
SECTIONS { 
  firtst  	0x00000000 : { head.o init.o nand.o}
  second 	0x30000000 : AT(4096) { main.o }
} 
```
* `r0、r1、r2 ： 分别对应第一个参数，第二个参数，第三个参数`
```
            ldr     r0,     =0x30000000     @1. 目标地址=0x30000000，这是SDRAM的起始地址
            mov     r1,     #4096           @2.  源地址   = 4096，连接的时候，main.c中的代码都存在NAND Flash地址4096开始处
            mov     r2,     #2048           @3.  复制长度= 2048(bytes)，对于本实验的main.c，这是足够了
            bl      nand_read               @调用C函数nand_read
```
```
/* 读函数 */
void nand_read(unsigned char *buf, unsigned long start_addr, int size)      // nand_read的函数
```
* `即 r0 = buf ; r1 = start_addr ; r2 = size`