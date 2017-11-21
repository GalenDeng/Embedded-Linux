## GPIO (2017.11.21)
1. `汇编代码`
```
galen@66:/work/hardware/gpio/led_on$ cat led_on.S 
@******************************************************************************
@ File：led_on.S
@ 功能：LED点灯程序，点亮LED1
@******************************************************************************       
            
.text
.global _start
_start:     
            LDR     R0,=0x56000050      @ R0设为GPFCON寄存器。此寄存器   //让R0 = 0x56000050    
                                                                       //LDR : LOAD register 加载寄存器  LDR伪指令
                                                                       //的形式是“LDR Rn,=expr”
                                        @ 用于选择端口B各引脚的功能：
                                        @ 是输出、是输入、还是其他
            MOV     R1,#0x00000100                                     //赋值
            STR     R1,[R0]             @ 设置GPF4为输出口, 位[8:7]=0b01 //将R1的数据传给RO表示的地址中
            LDR     R0,=0x56000054      @ R0设为GPBDAT寄存器。此寄存器
                                        @ 用于读/写端口B各引脚的数据
            MOV     R1,#0x00000000      @ 此值改为0x00000010,
                                        @ 可让LED1熄灭
            STR     R1,[R0]             @ GPF4输出0，LED1点亮
MAIN_LOOP:
            B       MAIN_LOOP               // B : 跳转    这里是个死循环
galen@66:/work/hardware/gpio/led_on$ 
```
```
* STR{条件}  源寄存器，<存储器地址>
STR指令用于从源寄存器中将一个32位的字数据传送到存储器中 [写内存的指令]
* str     r1, [r0]                       ；将r1寄存器的值，传送到地址值为r0的（存储器）内存中
```
2. `Makefile文件`
```
galen@66:/work/hardware/gpio/led_on$ cat Makefile 
led_on.bin : led_on.S           // led_on.S  汇编文件
	arm-linux-gcc -g -c -o led_on.o led_on.S                      //编译
	arm-linux-ld -Ttext 0x0000000 -g led_on.o -o led_on_elf       //链接 把led_on.o链接为led_on_elf格式 -g : 可调
    试 -T : 指定代码段、数据段、bss段的起始地址 这里是指定text(代码段)的起始地址为 0x0000000
	arm-linux-objcopy -O binary -S led_on_elf led_on.bin          //把led_on_elf转化为二进制文件 -S : 不从源文件中复
    制重定位信息和符号信息到目标文件中去  -O bfdname : 使用指定的格式来输出文件 这里是输出二进制文件，输出文件名为
    led_on.bin 格式为： binary
clean:
	rm -f   led_on.bin led_on_elf *.o

```
```
* arm-linux-objcopy : 被用来复制一个目标文件的内容到另一个文件中，
可以使用不同于源文件的格式来输出目的文件
```
* `目标`
* ![目标]()
* `点亮LED的操作原理`
* ![点亮LED的操作原理]()