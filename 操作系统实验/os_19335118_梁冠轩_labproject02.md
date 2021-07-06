# ucore_lab2实验报告

## 练习0：填写已有实验

使用meld的Directory comparison功能，比较lab1与lab2

![](https://gitee.com/liangguanxuan/liangguanxuan/raw/master/os/ucore_lab2/a%20(2).jpg)

把lab1的kdebug.c和trap.c文件的代码复制到lab2

![](https://gitee.com/liangguanxuan/liangguanxuan/raw/master/os/ucore_lab2/a%20(3).jpg)

## 练习1：实现 first-fit 连续物理内存分配算法

根据提示，可知 first fit 的分配算法都是在 default_pmm.c 中实现的，因此练习 1 的工作就是修改完善 default_pmm.c 中的相关函数。 首先要了解物理页的数据结构 Page，该数据结构定义在 memlayout.h 中，如图所示。

![](https://gitee.com/liangguanxuan/liangguanxuan/raw/master/os/ucore_lab2/a%20(4).jpg)

然后要了解管理所有连续的空闲内存空间块的数据结构 free_area_t，它的定义也在 memlayout.h 中。

![](https://gitee.com/liangguanxuan/liangguanxuan/raw/master/os/ucore_lab2/a%20(5).jpg)

free_list 是一个 list_entry 结构的双向链表指针，nr_free 记录当前空闲页的个数。

### default_init()

根据提示可以直接使用该函数，暂时不必修改。并且由注释可知该函数的功能是对 free_area_t 的双向链表和空闲块的数目进行初始化。

![](https://gitee.com/liangguanxuan/liangguanxuan/raw/master/os/ucore_lab2/a%20(6).jpg)

### default_init_memmap()

首先查看注释（如图），可知该函数的功能是初始化空闲页链表，初始化每一个空闲页，然后计算空闲页的总数。

![](https://gitee.com/liangguanxuan/liangguanxuan/raw/master/os/ucore_lab2/a%20(7).jpg)

首先传入物理页基地址和物理页的个数（大于 0），然后对每块物理页进行设置：先判断是 否为保留页，如果不是，则进行下一步。将标志位清零，连续空页个数清零，然后将标志位 设置为 1，将引用此物理页的虚拟页的个数清零。然后再加入空闲链表。最后计算空闲页的个数，修改物理基地址页的 property 的个数为 n。得到的代码如下图所示。

![](https://gitee.com/liangguanxuan/liangguanxuan/raw/master/os/ucore_lab2/a%20(8).jpg)

### default_alloc_pages()

根据提示可知该函数的功能主要是从空闲页块的链表中遍历，找到第一块大小大于 n 的块，然后分配出来，从空闲页链表中移除，如果有多余的页，就将剩余部分再加入到空闲页链表中。

![](https://gitee.com/liangguanxuan/liangguanxuan/raw/master/os/ucore_lab2/a%20(9).jpg)

根据注释修改代码。首先判断空闲页的大小是否大于所需的页块大小。如果需要分配的数量大于空闲页的数量，则直接 return NULL 表示分配失败。然后遍历整个空闲链表，如果找到合适的空闲页，即 p->property >= n（表示从该页开始的连续空闲页数量大于 n），因此可分配，重新设置标志位。具体操作是调用 SetPageReserved(pp)和 ClearPageProperty(pp)，设置当前页面预留以及清空该页面的连续空闲页面数量值。然后从空闲页链表 free_area_t 中，删除此项。如果当前空闲页大小大于所需大小，则分割页块。最后计算剩余空闲页个数并返回分配的页块地址。代码如下图所示。

![](https://gitee.com/liangguanxuan/liangguanxuan/raw/master/os/ucore_lab2/a%20(10).jpg)

### default_free_pages()

根据提示可知该函数的功能主要是完成对于页的释放操作。

![](https://gitee.com/liangguanxuan/liangguanxuan/raw/master/os/ucore_lab2/a%20(11).jpg)

首先 assert 语句断言这个基地址所在的页是否为预留，如果不是预留页，则说明它是 free状态，无法再次 free，因为只有处在占用的页才能有 free 操作。然后声明一个页 p，p 遍历整个物理空间，直到遍历到 base 所在位置停止，开始释放操作。找到整个基地址后就可以将空闲页重新加进来，然后与初始化空闲页一样设置标记位。如果插入基地址附近的地址可以合并，那么就进行合并，更新相应的连续空闲页数量。代码如下图所示。

![](https://gitee.com/liangguanxuan/liangguanxuan/raw/master/os/ucore_lab2/a%20(12).jpg)

![](https://gitee.com/liangguanxuan/liangguanxuan/raw/master/os/ucore_lab2/a%20(13).jpg)

### 你的first fit算法是否有进一步的改进空间

first fit 算法有改进空间，目前算法的分配和释放的复杂度都是 O(n)，可以采用 DFS 算法将效率提高为 O(logn)。

## 练习2：实现寻找虚拟地址对应的页表项

根据提示可以用已有的物理内存管理器 default_pmm_manager 来获得所需的空闲物理页。

![](https://gitee.com/liangguanxuan/liangguanxuan/raw/master/os/ucore_lab2/a%20(14).jpg)

![](https://gitee.com/liangguanxuan/liangguanxuan/raw/master/os/ucore_lab2/a%20(15).jpg)

根据提示可知通过 default_pmm_manager 的 default_alloc_pages 函数获得一个空闲物理页，这个页的起始物理地址就是页目录表的起始地址，同理也通过这种方式获得各个页表所需的空间。页表的空间大小取决于页表要管理的物理页数，一个页表项可管理一个物理页，页表需要占 n/1024 个物理页空间。一开始并没有存在所有的二级页表，而是等到需要的时候再添加对应的二级页表。当建立从一级页表到二级页表的映射是，需要注意设置控制位，这里应该同时设置 PTE_U、PTE_W 和 PTE_P（定义于 mmu.h）。如果原来就有二级页表，或者新建了页表，则只需返回对应项的地址即可。如果 create 参数为 0，则 get_pte 返回 NULL；不为 0，则 pte_pte 需要申请一个新的物理页。代码如下图所示。

![](https://gitee.com/liangguanxuan/liangguanxuan/raw/master/os/ucore_lab2/a%20(16).jpg)

### 请描述页目录项和页表项中每个组成部分的含义以及对ucore而言的潜在用处

页目录项是指向存储页表的页面的，所以本质上与页表项相同，结构也应该相同，每个页表项的高 20 位，就是该页表项指向的物理页面的首地址的高 20 位（低 12 位全为 0），每个页表项的低 12 位是一些功能位，可以再 mmu.h 的一组宏定义中看到。对于实现页替换算法来说，页目录项作为一个双向链表存储了目前所有页的物理地址和逻辑地址的对应，即在实内存中的所有页，替换算法中被换出的页从 pgdir 中选出。页表（pte）则存储了替换算法中被换入的页的信息，替换后会将其映射到一物理地址。

### 如果ucore执行过程中访问内存，出现了页访问异常，请问硬件要做哪些事情

产生页访问异常后，CPU 把引起页访问异常的线性地址装到寄存器 CR2 中，并给出了出错码 errorCode，说明了页访问异常的类型。ucore os 会把这个值保存在 struct trapframe中 tf_err 成员变量中。而中断服务例程会调用页访问异常处理函数 do_pgfault 进行具体处理。

## 练习3：释放某虚地址所在的页并取消对应二级页表项的映射

根据提示可知要先判断该页被引用的次数，如果只被引用了一次，那么直接释放该页，否则删除二级页表的该表项，即该页的入口。

![](https://gitee.com/liangguanxuan/liangguanxuan/raw/master/os/ucore_lab2/a%20(17).jpg)

根据提示可知先判断页表中该表项是否存在，然后判断是否只被引用了一次，如果只被引用了一次，那么可以释放此页，如果被多次引用，则不能释放此页，只用释放二级页表的表项， 最后更新页表。编写代码如下所示。

![](https://gitee.com/liangguanxuan/liangguanxuan/raw/master/os/ucore_lab2/a%20(18).jpg)

执行 make qemu，可得如下图所示结果，因此可知程序运行正确。

![](https://gitee.com/liangguanxuan/liangguanxuan/raw/master/os/ucore_lab2/a%20(19).jpg)

### 数据结构Page的全局变量的每一项与页表中的页目录项和页表项有无对应关系？如果有，其对应关系是啥

存在对应关系：由于页表项中存放着对应的物理页的物理地址，因此可以通过这个物理地址来获取到对应到的Page数组的对应项。

### 如果希望虚拟地址与物理地址相等，则需要如何修改lab2，完成此事

阅读实验参考书“系统执行中地址映射的四个阶段”，物理地址和虚拟地址之间存在offset：phy addr + KERNBASE = virtual addr。所以，KERNBASE = 0时，phy addr = virtual addr。所以把memlayout.h中的KERNBASE改为0即可。