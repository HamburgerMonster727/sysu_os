# ucore_lab1实验报告

## 练习1：理解通过make生成执行文件的过程

### 操作系统镜像文件ucore.img如何一步一步生成

输入make "V=" 查看makefile执行了哪些命令

编译libs和kern文件夹中所有的.S和.c文件，生成.o文件，并且生成bin/kernel文件

![](https://gitee.com/liangguanxuan/liangguanxuan/raw/master/os/ucore_lab1/a%20(1).png)

编译boot文件夹中的.S和.c文件，生成.o文件，并且生成bin/bootblock.out文件，大小为472bytes，编译tools文件夹中的sign.c文件，生成bin/sign文件，通过bin/sign将bin/bootblock.out文件生成为bin/bootblock文件，大小为512bytes

![](https://gitee.com/liangguanxuan/liangguanxuan/raw/master/os/ucore_lab1/a%20(4).png)

![](https://gitee.com/liangguanxuan/liangguanxuan/raw/master/os/ucore_lab1/a%20(2).png)

通过dd指令将bootblock复制到bin/ucore.img的第一个block，将kernel复制到bin/ucore.img第二个block。

![](https://gitee.com/liangguanxuan/liangguanxuan/raw/master/os/ucore_lab1/a%20(3).png)

### 一个被系统认为是符合规范的硬盘主引导扇区的特征是什么

通过查看源文件的代码：

![](https://gitee.com/liangguanxuan/liangguanxuan/raw/master/os/ucore_lab1/a%20(5).png)

可以看到，一个被系统认为是符合规范的硬盘主引导扇区的特征是：

1.最后两个字节为0x55AA

2.大小为512字节

## 练习2：使用qemu执行并调试lab1中的软件

### 从CPU加电后执行的第一条指令开始，单步跟踪BIOS的执行

因为gdb调试时默认时运行在i386模式下，所以要把他修改为i8086模式，使用“set architecture i8086”进行修改，这样才能正确地使用“x/i $pc”反汇编指令，修改gdbinit文件：

![](https://gitee.com/liangguanxuan/liangguanxuan/raw/master/os/ucore_lab1/b%20(4).png)

执行make debug，在gdb调试窗口中，执行si命令进行单步调试程序，然后执行x /2i $pc查看bios代码：

![](https://gitee.com/liangguanxuan/liangguanxuan/raw/master/os/ucore_lab1/b%20(1).png)

可以看到bios地第一条指令地址为0x0000fff0，这是一条跳转指令，会跳转到0x0000e05b。

### 在初始化位置0x7c00设置实地址断点,测试断点正常

修改gdbinit文件，设置断点：

![](https://gitee.com/liangguanxuan/liangguanxuan/raw/master/os/ucore_lab1/b%20(3).png)

按照上一步进行调试，得到的初始地址为0x00007c00：

![](https://gitee.com/liangguanxuan/liangguanxuan/raw/master/os/ucore_lab1/b%20(2).png)

### 从0x7c00开始跟踪代码运行,将单步跟踪反汇编得到的代码与bootasm.S和 bootblock.asm进行比较

从0x7c00开始单步跟踪反汇编得到的代码：

![](https://gitee.com/liangguanxuan/liangguanxuan/raw/master/os/ucore_lab1/c%20(1).png)

![](https://gitee.com/liangguanxuan/liangguanxuan/raw/master/os/ucore_lab1/c%20(2).png)

![](https://gitee.com/liangguanxuan/liangguanxuan/raw/master/os/ucore_lab1/c%20(3).png)

![](https://gitee.com/liangguanxuan/liangguanxuan/raw/master/os/ucore_lab1/c%20(4).png)

![](https://gitee.com/liangguanxuan/liangguanxuan/raw/master/os/ucore_lab1/c%20(5).png)

将得到代码与bootasm.S和bootblock.asm进行比较，可以发现反汇编的代码与bootbolck.asm相同，但是与bootasm.S有所不同：反汇编得到的代码中的指令没有指示长度的后缀，而bootasm.S中的指令有。例如：反汇编的代码是xor %ax,%ax，而bootasm.S的代码为xorw %ax,%ax。

### 自己找一个bootloader或内核中的代码位置，设置断点并进行测试

修改gdbinit文件，在0x7c16处设置断点：

![](https://gitee.com/liangguanxuan/liangguanxuan/raw/master/os/ucore_lab1/d%20(2).png)

执行make debug，可以看到跳转到了0x00007c16:

![](https://gitee.com/liangguanxuan/liangguanxuan/raw/master/os/ucore_lab1/d%20(1).png)

## 练习3：分析bootloader进入保护模式的过程

### 如何开启A20门

A20门初始值为0，此时地址线控制是被屏蔽的，8086的数据处理位宽位16位，当其访问地址超过1MB时，A20就会从0开始循环计数，此时A20不可访问，但是 A20 地址线控制在保护模式下是要打开的，所以需要将键盘控制器上的A20线置于高电位，此时32条地址线全部可用。打开A20门的步骤：1.等待8042 Input buffer为空。2.发送Write 8042 Output Port （P2） 命令到8042 Input buffer。3.等待8042 Input buffer为空。4.将8042 Output Port（P2） 对应字节的第2位置1，然后写入8042 Input buffer。

在ucore中的boot/bootasm.S中，seta20.1和seat20.2负责实现打开A20门：

![](https://gitee.com/liangguanxuan/liangguanxuan/raw/master/os/ucore_lab1/e%20(3).png)

### 如何初始化GDT表

在ucore中的boot/bootasm.S中的gdt地址处，gdtdesc负责初始化GDT表：

![](https://gitee.com/liangguanxuan/liangguanxuan/raw/master/os/ucore_lab1/e%20(1).png)

### 如何使能和进入保护模式

在ucore中的boot/bootasm.S中，将cr0寄存器的最低位置为1：

![](https://gitee.com/liangguanxuan/liangguanxuan/raw/master/os/ucore_lab1/e%20(2).png)

## 练习4：分析bootloader加载ELF格式的OS的过程

### bootloader如何读取硬盘扇区

读一个扇区的流程大致如下：1.等待磁盘准备好。2.发出读取扇区的命令。3.等待磁盘准备好。4.把磁盘扇区数据读到指定内存。

在boot/bootmain.c的readsect函数中，实现bootloader读取扇区的功能：

![](https://gitee.com/liangguanxuan/liangguanxuan/raw/master/os/ucore_lab1/f%20(1).png)

通过分析函数的源代码，可得知bootloader读取硬盘扇区的过程：

调用waitdisk函数，不断读取0x1F7寄存器的最高两位，当最高位为0，次高位为1时，返回，此时意味着硬盘处于空闲状态。硬盘空闲后，执行读取扇区指令，读取一个扇区，放在0x1F2寄存器中，读取的扇区28位起始编号，分成四个部分依次放在0x1F3,0x1F4,0x1F5,0x1F6寄存器中。再次调用waitdisk函数，等待硬盘空闲。硬盘再次空闲后，从0x1F0寄存器开始读取数据。

### bootloader是如何加载ELF格式的OS

在boot/bootmain.c的bootmain函数中，实现了加载ELF格式的os的功能：

![](https://gitee.com/liangguanxuan/liangguanxuan/raw/master/os/ucore_lab1/f%20(2).png)

![](https://gitee.com/liangguanxuan/liangguanxuan/raw/master/os/ucore_lab1/f%20(3).png)

通过分析函数源代码，可得知bootloader加载ELF格式的os的过程：

先从硬盘中读取了8个扇区的数据到内存0x10000处，并把这里强制转换成elfhdr使用，然后校验e_magic字段，再根据偏移量分别把程序段中的数据读取到内存中。

## 练习5：实现函数调用堆栈跟踪函数

修改kern/debug/kdebug.c::print_stackframe函数，来跟踪函数调用堆栈中记录的返回地址。

![](https://gitee.com/liangguanxuan/liangguanxuan/raw/master/os/ucore_lab1/g%20(1).png)

从注释中可以看到，完成print_stackframe需要用到两个函数reap_eip和read_ebp：

![](https://gitee.com/liangguanxuan/liangguanxuan/raw/master/os/ucore_lab1/g%20(2).png)

![](https://gitee.com/liangguanxuan/liangguanxuan/raw/master/os/ucore_lab1/g%20(3).png)

先调用read_ebp函数获得ebp的值，再调用read_eip函数来获取eip的值；从read_eip函数可以看到，eip获取的时下一条指令的地址，所以我们要把eip减1；更新eip等于ebp[1]，直到ebp为0时，退出循环。

实现代码：

![](https://gitee.com/liangguanxuan/liangguanxuan/raw/master/os/ucore_lab1/g%20(5).png)

执行make qemu，获得实验结果，与实验要求比较，获得类似输出：

![](https://gitee.com/liangguanxuan/liangguanxuan/raw/master/os/ucore_lab1/g%20(4).png)

对最后一行输出进行解释：

ebp:0x00007bf8，这是kern_init函数的堆栈的栈顶地址。

eip:0x00007d68，这是kern_init函数的返回地址。

args:0xc031fcfa 0xc08ed88e 0x64e4d08e 0xfa7502a8，这是四个输入参数的值。

## 练习6：完善中断初始化和处理 

### 中断描述符表中一个表项占多少字节？其中哪几位代表中断处理代码的入口

中断描述符表一个表项占8个字节，其中2-3字节是段选择子，0-1字节和6-7字节拼成偏移量，段选择子去GDT中找到对应的基地址，然后基地址加上偏移量就是中断处理程序的地址。

### 请编程完善kern/trap/trap.c中对中断向量表进行初始化的函数idt_init

在idt_init函数中，依次对所有中断入口进行初始化。使用mmu.h中的SETGATE宏，填充idt数组内容。每个中断的入口由tools/vectors.c生成，使用trap.c中声明的vectors数组即可。

查看idt_init函数：

![](https://gitee.com/liangguanxuan/liangguanxuan/raw/master/os/h%20(1).png)

从注释可以看出，实现idt_init函数需要用到SETGATE：

![](https://gitee.com/liangguanxuan/liangguanxuan/raw/master/os/h%20(2).png)

实现代码：

![](https://gitee.com/liangguanxuan/liangguanxuan/raw/master/os/h%20(4).png)

### 请编程完善trap.c中的中断处理函数trap

在对时钟中断进行处理的部分填写trap函数中处理时钟中断的部分，使操作系统每遇到100次时钟中断后，调用print_ticks子程序，向屏幕上打印一行文字”100 ticks”。

实现代码：

![](https://gitee.com/liangguanxuan/liangguanxuan/raw/master/os/h%20(3).png)

执行make qemu，获得实验结果：

![](https://gitee.com/liangguanxuan/liangguanxuan/raw/master/os/h%20(5).png)