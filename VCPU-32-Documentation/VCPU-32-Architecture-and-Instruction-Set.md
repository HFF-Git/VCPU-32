
<!--------------------------------------------------------------------------------------------------------->
<!-- VCPU-32 Document                                                                                   --->
<!--------------------------------------------------------------------------------------------------------->

<!--------------------------------------------------------------------------------------------------------->
<!-- use "&nbsp; " to insert a blank in the table column if too narrow...                               --->
<!-- a CSS style to make the tables having the possible width of the page                               --->
<!--------------------------------------------------------------------------------------------------------->

<style>
	div {
		text-align: justify;
	}
    table {
        width: 100%;
        padding: 12px;
    }
    img {
    	display: block;
    	margin-left: auto;
     	margin-right: auto;
		max-width:100%;
    	height: auto;
    }
</style>

<!--------------------------------------------------------------------------------------------------------->

# VCPU-32
# System Architecture and Instruction Set Reference

Helmut Fieres
Version B.00.01
February, 2024

<!--------------------------------------------------------------------------------------------------------->

<div style="page-break-before: always;"></div>

## Introduction

> "A vintage 32-bit register-memory model CPU ? You got to be kidding me. Shouldn't we design a modern superscalar, multi-core RISC based 32-bit machine? Or even better 64-bit?. After all, this is the 21st century and not the eighties."
>
> "Well why not. Think vintage."
>
> "Seriously?“
>
> “OK, seriously. Designers of the eighties CPUs almost all used a microcoded approach with hundreds of instructions. But what if the RISC principles had found their way earlier into these designs? What if a large virtual address space, a fixed instruction length and simple pipeline friendly instructions had found their way into these designs ?
>
> A 32-bit vintage CPU will give us a good set of design challenges to look into and opportunities to include modern RISC features and learn about instruction sets and pipeline design as any other CPU. Although not a modern design, it will still be a useful CPU."
>
> "OK, so what do you have in mind ?"

Welcome to VCPU-32. VCPU-32 is a simple 32-bit CPU with a register-memory model and a segmented virtual memory. The design is heavily influenced by Hewlett Packard's PA_RISC architecture, which was initially a 32 bit RISC-style register-register load-store machine. Many of the key architecture features came from there. However, other processors, the Data General MV8000, the DEC Alpha, and the IBM PowerPc, were also influential.

The original design goal that started this work was to truly understand the design process and implementation tradeoff for a simple pipelined CPU. While it is not a design goal to build a competitive CPU, the CPU should nevertheless be useful and largely follow established common practices. For example, instructions should not be invented because they seem to be useful, but rather designed with the assembler and compilers in mind. Instruction set design is also CPU design. Register memory architectures in the 80s were typically micro-coded complex instruction machine. In contrast, VCPU-32 instructions will be hard coded and should be in general "pipeline friendly" and avoid data and control hazards and stalls where possible.

The instruction set design guidelines center around the following principles. First, in the interest of a simple and efficient instruction decoding step, instructions are of fixed length. A machine word is the instruction word length. As a consequence, address offsets are rather short and addressing modes are required for reaching the entire address range. Instead of a multitude of addressing modes, typically found in the CPUs of the 80s and 90s, VCPU-32 offers very few addressing modes with a simple base address - offset calculation model. No indirection or any addressing mode that would require to read a memory content for address calculation is part of the architecture.

There will be few instructions in total, however, depending on the instruction several options for the instruction execution are encoded to make an instruction more versatile. For example, a boolean instruction will offer options to negate the result thus offering and "AND" and a "NAND" with one instruction. Wherever possible, useful options such as the one outlined before are added to an instruction, such that it covers a range of instructions typically found on the CPUs looked into.

Modern RISC CPUs are load/store CPUs and feature a large number of general registers. Operations takes place between registers. VCPU-32 follows a slightly different route. There is a rather small number of general registers leading to a model where one operand is fetched from memory. There are however no instructions that read and write to memory in one instruction cycle as this would complicate the design considerably.

VCPU-32 offers a large address range, organized into segments. Segment and offset into the segment form a virtual address. In addition, a short form of a virtual address, called a logical address, will select a segment register from the upper logical address bits to form a virtual address. Segments are also associated with access rights and protection identifies. The CPU itself can run in user and privilege mode.

This document describes the architecture, instruction set and runtime for VCPU-32. It is organized into several chapters. The first chapter will give an overview on the architecture. It presents the memory model, registers sets and basic operation modes. The major part of the document then presents the instruction set. Memory reference instructions, branch instructions, computational instructions and system control instructions are described in detail. These chapters are the authoritative reference of the instruction set. The runtime environment chapters present the runtime architecture. Finally, the remainder of the chapters summarize the instructions defined and also offer an instruction and runtime commentary to illustrate key points on the design choices taken.

<!--------------------------------------------------------------------------------------------------------->

<div style="page-break-before: always;"></div>

## Architecture Overview

This chapter presents an overview on the VCPU-32 architecture.

### A Register Memory Architecture.

VCPU-32 implements a **register memory architecture**. For most of the instructions one operand is the register, the other is a flexible operand which could be an immediate, a register content or a memory address where the data is obtained. The result is placed in the register which is also the first operand. These type of machines are also called two address machines. They were typical for machines with few general registers.

```
REG <- REG op OPERAND
```

In contrast to similar historical register-memory designs, there is no operation which does a read and write operation to memory in the same instruction. For example, there is no "increment memory" instruction, since this would require two memory operations and is not pipeline friendly. Memory access is performed on a machine word, half-word or byte basis. Besides the implicit operand fetch in an instruction, there are dedicated memory load / store instructions. Computational operations and memory reference operations use a common addressing model, also called **operand mode**. In addition to the memory register mode, one operand mode supports also a three operand model ( Rs = Ra OP Rb ), specifying two registers and a result register, which allows to perform three address register for computational operations as well. The machine can therefore operate in a memory register model as well as a load / store register model.

### Memory and IO Address Model

VCPU-32 features a physical memory address range of 32-bits. The picture below depicts the physical memory address layout. The physical address range is divided into a memory data portion and an I/O portion. The I/O portion is further divided into 16 channel data areas. Channel zero represents the CPU itself. All others represent a hardware I/O channel. The entire address range is directly accessible with load and store instructions.

```
       0                                        31
      :-------------------------------------------: 0x00000000
      :                                           :
      :                                           :
      :                                           :
      :                                           :
      :                                           :
      :                                           :
      :            Data                           :
      :                                           :
      :                                           :
      :                                           :
      :                                           :          +-- 0xF0000000 -> :-------------------------------------------:
      :                                           :          |                 : Processor dependent code (256Mb )         :
      :                                           :          |                 :                                           :
      :                                           :          |   0xF1000000 -> :-------------------------------------------:
      :                                           :          |                 : I/O Channel 1 (256Mb )                    :
      :                                           :          |                 :                                           :
      :                                           :          |   0xF2000000 -> :-------------------------------------------:
      :-------------------------------------------: ---------+                 :               . . .                       :
      :                                           :                            :                                           : 
      :            IO                             :              0xFF000000 -> :-------------------------------------------:
      :                                           :                            : I/O Channel 15 (256Mb )                   :
      :                                           :                            :                                           :
      :-------------------------------------------: 0xFFFFFFFF --------------->:-------------------------------------------:
```

System implementations can however support a larger physical address range than the 4Gbyte address range. By organizing physical memory into banks of 4 Gbytes, the physical address range can be enlarged. Bank zero is however the only bank with an I/O address range split and the absolute memory load / store instructions can only address bank zero. The operating system can nevertheless map a virtual address to a physical address in another bank during the process of virtual address translation. A CPU needs only implement an address range in bank zero for memory and I/O.

In a multi-processor system, all memory banks 1 to N and the bank zero data address range 0x00000000 to 0xEFFFFFFF are shared among all processors. The IO Space 0xF0000000 to 0xFFFFFFFF is a per processor address range and private to the processor. ( Concept to be further developed... )

### Data Types

VCPU-32 is a big endian 32-bit machine. The fundamental data type is a 32-bit machine word with the most significant bit being left. Bits are numbered from left to right, starting with bit 0 as the MSB bit. Memory access is performed on a word, half-word or byte basis. All address are however expressed as bytes addresses.

```
          MSB                                                      LSB
          0              8              16             24          31
         :-----------------------------------------------------------:
         :  word                                                     :
         :-----------------------------:-----------------------------:
         :  halfword 0                 :  halfword 1                 :
         :--------------:--------------:--------------:--------------:
         :  byte 0      :  byte 1      :  byte 2      :  byte 3      :
         :--------------:--------------:--------------:--------------:
```

### General Register Model

VCPU-32 features a set of registers. They are grouped in general registers, segment registers and control registers. There are eight general registers, labeled GR0 to GR7, and eight segment registers, labeled SR0 to SR7. All general registers can do arithmetic and logical operations. Register GR4 to GR7 are additionally labelled index registers, which are used in the addressing modes. The eight segment registers hold the segment part of the virtual address. The control registers contain system level information such as protection registers and interrupt and trap data registers.

```
               Segment                         General                           Control

           0                     31          0                     31          0                     31
          :------------------------:        :------------------------:        :------------------------:
          :         SR0            :        :          GR0           :        :        CR0             :
          :------------------------:        :------------------------:        :------------------------:
          :                        :        :                        :        :                        :
          :         ...            :        :          ...           :        :                        :
          :                        :        :                        :        :                        :
          :------------------------:        :------------------------:        :                        :
          :         SR7            :        :          GR7           :        :          ...           :
          :------------------------:        :------------------------:        :                        :
                                                                              :                        :
          :------------------------:        :------------------------:        :                        :
          :         IA Seg         :        :          IA Ofs        :        :                        :
          :------------------------:        :------------------------:        :                        :
                                                                              :                        :
                                            :------------------------:        :------------------------:
                                            :         Status         :        :        CR31            :
                                            :------------------------:        :------------------------:
```

### Processor State

VCPU-32 features three registers to hold the processor state. The **instruction address register** holds the address of the current instruction being executed. The instruction address is formed by the instruction address segment, IA-SEG,  and the instruction address offset, IA-OFS. The instruction address is the virtual or absolute memory location of the current instruction. The lower two bits are zero, as instruction words are word aligned in memory. 

```
       0                                          31    0                                     29 30 31 
      :---------------------------------------------:  :----------------------------------------:-----:
      :  IA segment Id                              :  :  IA offset                             : 0   :
      :---------------------------------------------:  :----------------------------------------------:
```

The **status register** holds the processor state information, such as the carry bit or current execution privilege. The status register is labelled ST.

```
       0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31
      :--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:
      : M : C : X : reserved                          :CB: reserved                 :0 :0 :0 :P :D :E :
      :-----------------------------------------------------------------------------------------------:

```

Bits 12 .. 17 are reserved for carry bits. A carry bit, bit 12, is generated for the ADD and SUB instruction. Bits 18 .. 23 represent the bit set that can be modified by the privileged MST instruction.

| Flag | Name | Purpose |
|:---|:---|:---|
| **M** | Machine Check | Machine check. When set, checks are disabled.|
| **C** | Instruction Translation Enable | When set, the instruction translation and access control is enabled. |
| **X** | Execution level | When set, the CPU runs in user mode, otherwise in privileged mode. |
| **CB** | Carry/Borrow | The carry bit for ADD and SUB instructions. |
| **P** | Protection ID Mode | Protection check enable. When set and Code translation is enabled, protection IDs are checked. |
| **D** | Data Translation Enable | When set, the data translation and access control is enabled. |
| **E** | External Interrupt Enable | When set, an external interrupt are enabled. |

### Control Registers

The **control registers** hold information about the processor configuration as well as data needed for the current execution. There are 32 control registers. Examples are the return link address, protection ID values, the current trap handling information, or the interrupt mask.

| CR | Name | Purpose |
|:---|:---|:---|
| 0 | **SWR**  | System Switch Register. On an optional front panel, a set of switches and LEDs represent the switch register. |
| 1 | **RCTR** | Recovery Counter. Can be used to implement a watchdog function. ( tbd )
| 2 | **SHAMT** | Shift Amount register. This is a 5-bit register which holds the value for variable shift, extract and deposit instructions. Used in instructions that allow to use the value coming from this register instead of the instruction encoded value. |
| 8 - 11 | **PID-n**  | Protection ID register 0 to 3. The protection ID register hold the 15-bit protection ID and the write disable bit. When protection ID check is enabled, each virtual page accessed is checked for matching one of the protection ID. The write operation is additionally checked to match the write disable bit. There are four protection ID registers. |
| 12 - 15 | reserved | reserved for future use. |
| 16 | **I-BASE-ADR** | Interrupt and trap vector table base address. The absolute address of the interrupt vector table. When an interrupt or trap is encountered, the next instruction to execute is calculated by adding the the interrupt number shifted by 3 to this address. Each interrupt has eight instruction locations that can be used for this interrupt. The table must be page aligned. |
| 17 | **I-STAT** | When an interrupt or trap is encountered, this control register holds the current status word.
| 18 |  **I-IA-SEG** | When an interrupt or trap is encountered, control register holds the current instruction address segment. |
| 19 |  **I-IA-OFS** | When an interrupt or trap is encountered, control register holds the current instruction address offset. |
| 20 - 22 | **I-PARM-n** | Interrupts and pass along further information through these control registers. |
| 23 | **I-EIM** | External interrupt mask. |
| 24 - 31 | **TEMP-n** | These control registers are scratch pad registers. Temporary registers are typically used in an interrupt handlers as a scratch register space to save general registers so that they can be used in the interrupt routine handler. They also contain some further values for the actual interrupt. These register will neither be saved nor restored upon entry and exit from an interrupt. |

### Segmented Memory Model

The VCPU-32 memory model features a **segmented memory model**. The address space consists of up to 2^24 segments, each of which holds up to 2^24 words in size. Segments are further subdivided into pages with a page size of 4K Words. The concatenation of segment ID and offset form a **virtual address**.


```
       0                                       31     0                          20           31
      :------------------------------------------:   :-------------------------------------------:
      :  segment Id                              :   :   page number            : page offset    :
      :------------------------------------------:   :-------------------------------------------:
                          |                                                |
                          |                                                |
                          v                                                |
                       :-------------------------------------------:       |
                       :                                           :       |
                :-------------------------------------------:      :       |
                :                                           :      :       |
          :-------------------------------------------:     :      :       |
          :                                           :     :      :       |
          :                                           :     :      :       |
          :                                           :     :      :       |
          :                                           :     :      :       |
          :                                           :     :      :       |
          :                                           :     :      :       |
          :-------------------------------------------:     :      : <-----+
          :   A 4Kb page                              :     :      :
          :-------------------------------------------:     :      :
          :                                           :     :      :
          :                                           :     :      :
          :                                           :     :      :
          :                                           :     :      :
          :                                           :     :      :
          :                                           :     :------:
          :                                           :-----:
          :-------------------------------------------:
```

### Address Translation

VCPU-32 defines three types of addresses. At the programmer's level there is the **logical address**. The logical address is a 32-bit word, which contains a 2-bit segment register selector and a 30-bit offset. During data access the segment selector selects from the segment register set SR4 to SR7 and forms together with the 30-bit offset a virtual address. The **virtual address** is the concatenation of a segment and an offset. Together they form a maximum address range of 2^32 segments with 2^32 bits each. Once the virtual address is formed, the translation process is the same for both virtual addressing modes. The following figure shows the translation process for a logical address. A virtual address is translated to a **physical address**. A logical address is an address with the upper two bits indicating which segment register to use and the offset an unsigned index into the segment. The resulting virtual address will have in this case the upper two bits set to zero.

```
                                                      0    1 2                                 31
                                                     :------:------------------------------------:
                         -- selects ---              : sReg : offset                             :    logical
                        |                            :------:------------------------------------:
                        |                            \_sel_/\_  logical page  _/\__ page ofs ___/
                        |                                              |
                        v                                              v
       0                                        31    0    1 2                  20             31
      :-------------------------------------------:  :------:------------------------------------:
      :  segment Id                               :  : sReg : offset                             :    virtual
      :-------------------------------------------:  :------:------------------------------------:
      \______________________________ virtual page ____________________________/\__ page ofs ___/

                                                   |
                                                   v
                                             ( translation )
                                                   |
                                +------------+---------------------------+
                                |            |                           |
                                v            v                           v
                            0       3   0        3    0                           20           31
                           :---------:  :---------:  :-------------------------------------------:
                           : CPU_ID  :  : Bank    :  :  physical address                         :    physical
                           :---------:  :---------:  :-------------------------------------------:
                                  (optional )         \_______ physical page ___/\__ page ofs ___/
```

Address translation can be enabled separately for instruction and data address translation. If virtual address translation for the instruction or data access is disabled, the 32 bit offset portion represents a physical address. A virtual address with a segment portion of zero will directly address physical memory, i.e the virtual address offset is the physical address. In a single CPU implementation, the physical address is the memory address. The maximum memory size is 4 Gbytes, unless memory banks are supported. Within a multi-CPU arrangement, the physical address can be augmented with a CPU ID which allows a set of CPUs share their physical range.

### Access Control

When virtual address translation is enabled, VCPU-32 implements a **protection model** along two dimensions. The first dimension is the access control and privilege level check. Each page is associated with a **page type**. There are read-only, read-write, execute and gateway pages. Each memory access is checked to be compatible with the allowed type of access set in the page descriptor. There are two privilege levels, user and supervisor mode. On each instruction execution, the privilege bit in the instruction address is checked against the access type and privilege information field in the access control field, stored in the TLB. Access type and privilege level form the access control information field, which is checked for each memory access. For read access the privilege level us be least as high as the PL1 field, for write access the privilege level must be as least as high as PL2. For execute access the privilege level must be at least as high as PL1 and not higher than PL2. If the instruction privilege level is not within this bounds, a privilege violation trap is raised. If the page type does not match the instruction access type an access violation trap is raised. In both cases the instruction is aborted and a memory protection trap is raised.

```
       0           2     3
      :-----------:-----:-----:
      : type      : PL1 : PL2 :
      :-----------:-----:-----:

      type: 0 - read-only,   read: PL <= PL1,    write : not allowed,   execute: not allowed
      type: 1 - read-write,  read: PL <= PL1,    write : PL <= PL2,     execute: not allowed
      type: 2 - execute,     read: PL <= PL1,    write : not allowed,   execute: PL2 <= PL <= PL1
      type: 2 - gateway,     read: not allowed,  write : not allowed,   execute: PL2 <= PL <= PL1
```

The second dimension of protection is a **protection ID**. A protection ID is a value assigned to the segment at segment creation time. The processor control register section contains four protection ID registers. For each access to a segment that has a non-zero protection ID associated, one of the protection ID registers must match the segment protection ID. If not, a protection violation trap is raised.

```
       0                               14  15
      :----------------------------------:---:
      : Protection ID                    : W :
      :----------------------------------:---:
```

Protection IDs are typically used to form a grouping of access. A good example is a stack data segment, which is accessible at user privilege level in read and write access from every thread that has R/W access with user privilege. However, only the corresponding user thread should be allowed to access the stack data segment. If a protection ID is assigned to the user thread, this can easily be accomplished. In addition, the protection ID also features a write disable bit, for allowing a model where many threads can read the protected segment but only few or one can write to it.


### Adress translation and caching

The previous section depicted the virtual address translation. While the CPU architects the logical, virtual and physical address scheme, it does not specify how exactly the hardware supports the translation process. A very common model is to have for performance reasons TLBs for caching translation results and Caches for data caching. However, split TLBS and unified TLBs, L1 and L2 caches and other schemes are models to consider. This section just outline the common architecture and instructions to manage these CPU components.

#### Translation Lookaside Buffer

For performance reasons, the virtual address translation result is stored in a translation look-aside buffer (TLB). Depending on the hardware implementation, there can be a combined TLB or a separate instruction TLB and data TLB. The TLB is essentially a copy of the page table entry information that represents the virtual page currently loaded at the physical address.

```
TLB fields (example):

       0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31
      :--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:
      :V :U :T :D :B :X : AR        :                 : Protect-ID                                    :
      :-----------------------------------------------------------------------------------------------:
      : VPN - high                                                                                    :
      :-----------------------------------------------------------------------------------------------:
      : 0                                 : VPN - low                                                 :
      :-----------------------------------------------------------------------------------------------:
      : 0         : CPU-ID    : Bank      : PPN                                                       :
      :-----------------------------------------------------------------------------------------------:
```

// ??? **note** perhaps a better field name for "X" ?

| Field | Purpose |
|:---|:---|
| **V** | the entry is valid. |
| **U** | un-cacheable. The page will not be cached, memory is accessed directly. |
| **T** | page reference trap. If the bit is set, a reference to the page results in a trap.|
| **D** | dirty trap. If the bit is set, the first write access to the TLB raises a trap.|
| **B** | data reference trap. If the bit is set, access to the data page raises a trap. |
| **X** | the code page can be modified at the highest privilege level. |
| **Protection ID** | The assigned bit protection ID for the page. |
| **Access Rights** | The access rights for the page. |
| **VPN-H** | the upper 32 bits of a virtual page address, which are also the segment ID. |
| **VPN-L** | the lower 20 bits of a virtual page address, which are the page in the segment. |
| **PPN** | the physical page number in a memory bank. |
| **PPN-BANK** | the memory bank number. ( to be defined ) |
| **CPU-ID** | the CPU ID number. ( to be defined ) |

When address translation is disabled, the respective TLB is is bypassed and the address represents a physical address in bank zero at the local CPU as described before. Also, protection ID checking is disabled. The U,D,T,B only apply to a data TLB. The X bit is only applies to the instruction TLB. When the processor cannot find the address translation in the TLB, a TLB miss trap will invoke a software handler. The handler will walk the page table for an entry that matches the virtual address and update the TLB with the corresponding physical address and access right information, otherwise the virtual page is not in main memory and there will be a page fault to be handled by the operating system. In a sense the TLB is the cache for the address translations found in the page table. The implementation of the TLB is hardware dependent.

### Caches

VCPU-32 is a cache visible architecture, featuring a separate instruction and data cache. While hardware directly controls the cache for cache line insertion and replacement, the software has explicit control on flushing and purging cache line entries. There are instructions to flush and purge cache lines. To speed up the entire memory access process, the cache uses the virtual address to index into the arrays, which can be done in parallel to the TLB lookup. When the physical address tag in the cache and the physical page address in the TLB match, there will be a cache hit. The architecture will not specify the size of cache lines or cache organization. Typically a cache line will hold four to eight machine words, the cache organization is in the simplest case a single set direct mapped cache.

A system may choose direct map or set-associative caches. Furthermore, there are L1 and L2 cache models. For example, separate instruction and data L1 caches connecting to an external L2 cache with larger cache memories. The architecture just specifies instructions to handle cache operations.

### Page Tables

Physical memory is organized in pages with a size of 4K words. The architecture does not specify the page table model. There are again several models of page tables possible. One model is to use an inverted page table model, in which each physical page has an entry in the page table. During a TLB miss, a hash function computes an index into the a hash table which potentially has a link to the first page table entry and then walks the collision chain for a matching entry. A possible implementation of the page table entry is shown below.

```
Hash table Entry (Example):

       0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31
      :--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:
      :                         physical address of first entry in chain                              : --+
      :-----------------------------------------------------------------------------------------------:   |
                                                                                                          |
Page Table Entry (Example):                                                                               |
                                                                                                          |
       0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31
      :--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:   |
      :V :U :T :D :B :X : AR        :                 : Protect-ID                                    : <-+
      :-----------------------------------------------------------------------------------------------:
      : reserved                                                                                      :
      :-----------------------------------------------------------------------------------------------:
      : reserved                                                                                      :
      :-----------------------------------------------------------------------------------------------:
      : reserved                                                                                      :
      :-----------------------------------------------------------------------------------------------:
      : reserved              :  Bank     : PPN                                                       :
      :-----------------------------------------------------------------------------------------------:
      : reserved                          : VPN - low                                                 :
      :-----------------------------------------------------------------------------------------------:
      : VPN - high                                                                                    :
      :-----------------------------------------------------------------------------------------------:
      : physical address of next entry in chain                                                       :
      :-----------------------------------------------------------------------------------------------:
```

Each page table entry contains at least the virtual page address, the physical pages address, the access rights information, a set of status flags and the physical address of the next entry in the page table. On a TLB miss, the page table entry contains all the information that needs to go into the TLB entry. The example shown above contains several reserved fields. For a fast lookup index computation, page table entries should have a power of 2 size, typically 4 or 8 words. The reserved fields could be used for the operating system memory management data.

| Field | Purpose |
|--------|--------|
| **V** | the entry is valid. |
| **U** | un-cacheable. The page will not be cached, memory is accessed directly. |
| **T** | page reference trap. If the bit is set, a reference to the page results in a trap.|
| **D** | dirty trap. If the bit is set, the first write access to the TLB raises a trap.|
| **B** | data reference trap. If the bit is set, access to the data page raises a trap. |
| **X** | the code page can be modified at the highest privilege level. ( **note** better name ? ) |
| **Protection ID** | The assigned bit protection ID for the page. |
| **Access Rights** | The access rights for the page. |
| **VPN-H** | the upper 32 bits of a virtual address, which are also the segment ID. |
| **VPN-L** | the lower 20 bits of a virtual address, which are the page in the segment. |
| **PPN** | the physical page number in a memory bank. |
|**PPN-BANK** | the memory bank number. ( **note** to be defined ) |
|**CPU-ID** | the CPU ID number. ( **note** to be defined ) |
| **next PTE** | the physical address of the next page table entry, zero if there is none. |

Locating a virtual page in the page table requires to first index into the hash table and then follow the chain of page table entires.  The following code fragment is a possible hash function.

```cpp
Hash function ( Example ):

const uint segShift         = 4;      // the number of bits to shift the segment part. This specifies how many
                                      // consecutive pages will result in consecutive hash values.
const uint pageOffsetBits  = 12;      // number of bits for page offset, page size of 4K
const uint hashTableMask   = 0x3FF;   // a hash table of 1024 entries, memory size dependent, must be a power of two.

uint hash_index ( uint32_t segment, uint32_t offset ) {

    return(( segment << segShift ) ^ ( offset >> pageOffsetBits )) & hashTableMask;
}
```

The function builds the hash index, which may also be used for TLB entries, virtually tagged caches and walking the page table itself. In case of a TLB miss, the hash value is directly available to the miss handler via a control register. In the example above, the segment portion is left shifted by a value of 4. The hash function will map 16 consecutive virtual pages to consecutive indices.


### Control Flow

Control flow is implemented through a set of branch instructions. They can be classified into unconditional and conditional instruction branches.

**Unconditional Branches**. Unconditional branches fetch the next instruction address from the computed branch target. The address computation can be relative to the current instruction address ( instruction relative branch ) or relativ to an address register ( base register relative ). For a segment local branch, the instruction address segment part will not change. Unconditional branches are also used to jump to a subroutine and return from there. The branch and link instruction types save the return point to a general register. External calls are quite similar, except that they always branch to an absolute offset in the external segment. Just like the branch instruction, the return point can be saved, but this time the segment is stored in SR0 and the offset in GR0.

**Conditional Branches**. VCPU-32 features two conditional branch instructions. These instructions compares two register values or test a register for a certain condition. If the condition is met a branch to the target address is performed.  The target address is always a local address and the offset is instruction address relative. Conditional branches adopt a static prediction model. Forward branches are assumed not taken, backward branches are assumed taken.

### Interrupts and Exceptions

Interrupts and exceptions are events that occur asynchronously to the instruction execution. Depending on the type, they occur during the execution of an instruction or are checked in between instructions. Example for the former is the  arithmetic overflow tap, example for the latter is an external interrupt. Depending on the interrupt or exception type, the instruction is restarted or execution continues with the next instruction.

```
General flow:

- save ST to the I-STAT control register
- save IA_SEG, IA-OFS to the I-IA-SEG and I-IA-OFS control reg.
- save interrupted instruction to I-TEMP0.
- copy few GRs to shadow GRs( CRs ) to have room to work ( also SRs ? Tbd. )
- set ST  to zero, this will turn address translation off, putting the CPU in privileged mode.
- set IA-SEG to zero.
- set IA-OFS to Interrupt vector table I-BASE-ADR plus the trap index
- start execution of the trap handler
```

The control register I-BASE-ADR holds the absolute address of the interrupt vector table. Upon an interrupt, the current instruction address and the instruction is saved to the respective control registers and control branches then to the interrupt handler in the interrupt vector table. Each entry is just a set of instructions that can be executed.

| TrapId | Name |IA | P0 | P1 | P2 | Comment |
|:----|:---|:---|:---|:---|:---|:---|
| 0 | **reserved** | | ||||
| 1 | ** Machine check** | IA of the current instruction | check type | - | - | |
| 2 | **Power Failure** | IA of the current instruction | -| - | - | |
| 3 | **Recovery Counter Trap** | IA of the current instruction | -| - | - | |
| 4 | **External Interrupt** | IA of the instruction to be executed after servicing the interrupt. | -| - | - | |
| 5 | **Illegal instruction trap** | IA of the current instruction | instr | - | -| |
| 6 | **Privileged operation trap** | IA of the current instruction | instr | - | -| |
| 7 | **Privileged register trap** | IA of the current instruction | instr | - | -| |
| 8 | **Overflow trap** | IA of the current instruction | instr | - | - | |
| 9 | **Instruction TLB miss** | IA of the current instruction | - | - | - | |
| 10 | **Non-access instruction TLB miss** | IA of the current instruction | - | - | - | |
| 11 | **Instruction memory protection trap** | IA of the current instruction | - | - | - | |
| 12 | **Data TLB miss** | IA of the current instruction | instr | data adr segID | data adr Ofs| |
| 13 | **Non-access data TLB miss** | IA of the current instruction | - | - | - | |
| 14 | **Data memory access rights trap** | IA of the current instruction | instr | data adr segID | data adr Ofs| |
| 15 | **Data memory protection trap** | IA of the current instruction | instr | data adr segID | data adr Ofs| |
| 16 | **Page reference trap** | IA of the current instruction | instr | data adr segID | data adr Ofs| |
| 17 | **Break instruction trap** | IA of the current instruction | instr | data adr segID | data adr Ofs| |
| 18 | **Alignment trap** | IA of the current instruction | instr | data adr segID | data adr Ofs| |
| 19 .. 31 | reserved | | | | | | |

<!--------------------------------------------------------------------------------------------------------->

<div style="page-break-before: always;"></div>

## Instruction Set Overview

This chapter gives a brief overview on the instruction set. The instruction set is divided into four general groups of instructions. There are memory reference, branch, computational and system control type instructions.

### General Instruction Encoding

The instruction uses fixed word length instruction format. Instructions are always one machine word. In general there is a 6 bit field to encode the instruction opCode. The instruction opCode field is encoded as follows:

```
       0                 6                                                                          31
      :--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:
      : opCode          : opCode specific portion                                                     :
      :-----------------:-----------------------------------------------------------------------------:
```

In the interest of a simplified hardware design, the opCode and opCode specific instruction fields are regular and on fixed positions whenever possible. As shown above, the opCode is always at bits 0 .. 5. But also registers, modes and offsets are at the same locations. The benefit is that the decoding logic can just extract the fields without completely decoding the instruction upfront. For some instructions not all bits instruction word are used. They are reserved fields and should be filled with a zero for now.

Throughout the remainder of this document numbers are shown in three numeric formats. A decimal number is a number starting with the digits 1 .. 9. An octal number starts with the number 0, and a hex number starts with the "0x" prefix.


### Operand Encoding

At the highest level the processor works with logical addresses. There are several address modes that are used to form an operand address. Most instructions have an operand mode and and operand data field which encodes the address mode and size of data to fetch. There are sixteen address modes for instructions that use the operand encoding format.


```
                         <-res -> <-opt -> <- mode  -> <--------- operand --------------------------->
       0                 6        9        12          16                            26       29    31
      :--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:
      : opCode          : r      :        : 0         : S : val                                       :  immediate
      :-----------------:-----------------------------------------------------------------------------:
      : opCode          : r      :        : 1         : 0                                    : b      :  one register
      :-----------------:-----------------------------------------------------------------------------:
      : opCode          : r      :        : 2         : 0                           : a      : b      :  two registers
      :-----------------:-----------------------------------------------------------------------------:
      : opCode          : r      :        : 3         : S : ofs                     : a      : b      :  extended, word
      :-----------------:-----------------------------------------------------------------------------:
      : opCode          : r      :        : 4 - 7     : S : ofs                                       :  indexed, word
      :-----------------:-----------------------------------------------------------------------------:
      : opCode          : r      :        : 8 - 11    : S : ofs                                       :  indexed, half-word
      :-----------------:-----------------------------------------------------------------------------:
      : opCode          : r      :        : 12 - 15   : S : ofs                                       :  indexed, byte
      :-----------------:-----------------------------------------------------------------------------:
```

The machine addresses memory with a byte address. The **mode** field indicates which addressing mode is used. Mode 0 is the immediate mode, where the operand is a sign-extended value. Mode 1 and 2 are the register modes. Either one or two registers can be specified. Mode 3 features an extended addressing mode, which will allow to access a word in a segment. Modes 4 to 15 are the indexed modes. They fetch a word, a half-word or a byte. The address must be aligned with the size of data to fetch. The address is built from selecting an index register. General register 4 maps to mode 4, 8 and 12, general register 5 to 5, 9 and 13 and so on. The LD instruction features in modes 0 to 2 a register indexed addressing, which takes a registers as base register and adds another register to it to form the final address.


### Memory Reference Instructions

Memory reference instruction load or store a word to memory. The unit of transfer is a word, a half-word or a byte. The instructions use a **W** for word, a **H** for half-word and a **B** for byte operand size. The LDx and STx instruction are load and store a register portion using the operand encoding to specify the logical address. Using logical addresses restricts the segment size to 30-bits address range. To access the full virtual 64-bit address range of a segment, the LDEx and STEx instructions allow to specify a segment and an offset register to access the virtual memory. The LDAx  and STAx instruction implement access to the physical memory using a general register as the physical address to access. For supporting atomic operations two instructions are provided. The LDWR instruction loads a value form memory and remember this access. The STWC instruction will store a value to the same location that the LR instructions used and return a failure if the value was modified since the last LR access. This CPU pipeline friendly pair of instructions allow to build higher level atomic operations on top.

Memory reference instructions can be issued when data translation is on and off. When address translation is turned off, the memory reference instructions will ignore the segment part and replace it with a zero value. It is an architectural requirement that a virtual address with a segment Id of zero maps to an absolute address with the same offset. The absolute address mode instruction also works with translation turned on and off. The extended address mode instructions will raise a trap.


### Control flow Instructions

Control flow instructions can be divided into conditional and unconditional branches. Conditional branches are always instruction address relative. The CBR and TBR instructions are the instructions for conditional branches. They combine a comparison or test with a branch offset. Unconditional branches are subdivided into instruction relative branches and base register branches. The B instruction implements an IA-relative unconditional branch, the BR instruction allows an IA-relative branch with a general register as the offset to use. The BV instruction features a base and offset register to form the base register relative address, the BE instruction implements a base register relative branch to an external segment by specifying the segment register, the base register and an offset to form the final address.

### Computational Instructions

The computation instructions are divided into the numeric instructions ADD and SUB, the logical instructions AND, OR, XOR and the bit field operational instructions EXTR, DEP and DSR. The numeric and logical instructions encode the second operand in the operand field. This allows for immediate values, register values and values accessed via their logical address. The numeric instructions allow for a carry/borrow bit for implementing multi-precision operations as well as the distinction between signed and unsigned operation overflow detection traps. The logical instructions allow to negate the operand as well as the output. The bit field instructions will perform bit extraction and deposit as well as double register shifting. These instruction not only implement bit field manipulation, they are also the base for all shift and rotate operations.

There are three more instructions. The CMP instruction compares two values for a condition and returns a result of zero or one. The SHLA instruction is a combination of shifting an operand and adding a value to it. Simple integer multiplication with small values can elegantly be done with the help of this instruction. The LDI instruction allows to load a 16-bit value into the right or left half of a register. There are options to clear the register upfront, and to deposit the respective half without modifying the other half of the register. This way immediate values with a full 32-bit range can be constructed.

### System Control Instructions

The system control instructions are intended for the runtime designer to control the CPU resources such as TLB, Cache and so on. There are instruction to load a segment or control registers as well as instructions to store a segment or control register. The TLB and the cache modules are controlled by instructions to insert and remove data in these two entities. Finally, the interrupt and exception system needs a place to store the address and instruction that was interrupted as well as a set of shadow registers to start processing an interrupt or exception.

### Instruction Notation

The instructions described in the following chapters contain the instruction word layout, a short description of the instruction and also a high level pseudo code of what the instruction does. The pseudo code uses a register notation describing by their class and index. For example, GR[ 5 ] labels general register 5. SR[ x ] represents the segment registers and CR[ x ] the control register. In addition, some control register also have a dedicated name, such as for example the shift amount control register.

The instruction operation is described in a pseudo C style language using assignment and simple control structures to describe the instruction operation. In addition there are functions that perform operations and compute expressions. The following table lists these functions.

| Function | Description |
|:---|:----|
| **cat( x, y )** | concatenates the value of x and y. |
| **catImm( x, y )** | Assembles the immediate fields of an instruction. There are several formats for encoding the immediate. The individual fields are concatenated with "x" being the left most number of bits, followed by "y". |
| **signExtend( x, len )** | performs a sign extension of the value "x". The "len" parameter specifies the number of bits in the immediate. The sign bit is the leftmost bit. |
| **zeroExtend( x, len )** | performs a zero bit extension of the value "x". The "len" parameter specifies the number of bits in the immediate.|
| **segSelect( x )** | returns the segment register number based on the leftmost three bits of the argument "x". |
| **operandAdrSeg( instr )** | computes the segment Id from the instruction and mode information. ( See the operand encoding diagram for modes ) |
| **operandAdrOfs( instr )** | computes the offset portion from the instruction and mode information. ( See the operand encoding diagram for modes ) |
| **operandBitLen( instr )** | computes the operand bit length from the instruction and mode information. ( See the operand encoding diagram for modes ) |
| **ofsSelect( x )** | returns the 30-bit offset portion of the argument "x". |
| **add30( x, y )** | performs a 30-bit addition of x and y. The result does not overflow into the leftmost 2 bits of the result word. Typically, the "x" operand is an unsigned value while "y" is a signed value. The result of the operation will wrap around is case of overflow.|
| **add32( x, y )** | performs a 32-bit addition of x and y. Typically, the "x" operand is an unsigned value while "y" is a signed value. The result of the operation will wrap around is case of overflow.|
| **rshift( x, amt )** | logical right shift of the bits in x by "amt" bits. |
| **memLoad( seg, ofs, bitLen )** | loads data from virtual or physical memory. The "seg" parameter is the segment and "ofs" the offset into the segment. The bitLen is teh number of bits to load. If the bitLen is less than 32, the data is zero sign extended. If virtual to physical translation is disabled, the "seg" is zero and "ofs" is the offset from where to load a word from physical memory.  |
| **memStore( seg, ofs, val, bitLen )** | store data to virtual or physical memory. The "seg" parameter is the segment and "ofs" the offset into the segment. If virtual to physical translation is disabled, the "seg" is zero and "ofs" is the offset from where to store the word "val" from physical memory. |
| **loadPhysAdr( seg, ofs )** | returns the physical address for a virtual address. The "seg" parameter is the segment and "ofs" the offset into the segment. If virtual to physical translation is disabled, the "ofs" is the physical address in memory. |
| **searchInstructionTlbEntry( seg, ofs, entry )** | lookup the TLB entry and if found return a pointer to the entry. |
| **allocateInstructionTlbEntry( seg, ofs, entry )** | allocate an entry in the TLB and return a pointer to the entry. |
| **purgeIinstructionTlbEntry( entry )** | purge the TLB entry by simply invalidating the entry. |
| **searchDataTlbEntry( seg, ofs, entry )** | lookup the TLB entry and if found return a pointer to the entry.|
| **readAccessAllowed( tlbEntry, privLevel )** | checks the TLB entry for the requested access mode and privilege level. |
| **writeAccessAllowed( tlbEntry, privLevel )** | checks the TLB entry for the requested access mode and privilege level. |
| **allocateDataTlbEntry( seg, ofs, entry )** | allocate an entry in the TLB and return a pointer to the entry. |
| **purgeDataTlbEntry( entry )** | purge the TLB entry by simply invalidating it. |
| **flushDataCache( seg, ofs )** | write the cache line that contains the virtual address back to memory and purge the entry. |
| **purgeDataCache( seg, ofs )** | remove the cache line that contains the virtual address by simply invalidating it. |
| **purgeInstructionCache( seg, ofs )** | remove the cache line that contains the virtual address by simply invalidating it. |


<!--------------------------------------------------------------------------------------------------------->

<div style="page-break-before: always;"></div>

## Memory Reference Instructions

The memory reference instructions load data from memory into a general register and store data from a general registers to memory. The unit of load and store is either a machine word, a half-word or a byte. The **LDx** and **STx** instructions use the operand encoding to define the operand address, which is expressed as a logical address. Address mode 3 uses a virtual address to access the entire virtual address range of the CPU. Address mode 4 to 7 use an immediate offset encoded in the instruction and general registers 4 to 7 as segment relative base register.

In addition to the memory reference instructions using the operand mode, the **LDEx** and **STEx** instructions offer access to memory using a full virtual address, encoded in the segment and offset register in the instruction. The offset is formed from a general register plus a signed offset also encoded in the instruction. A segment that is addressed beyond what a logical address with its rich addressing modes can reach, can be accessed using these instruction.

The **LDAx** and **STAx** instructions offer access to the physical memory, with a general register holding an absolute address. The final address is formed from a general register plus a signed offset also encoded in the instruction. LDA and STA are privileged instructions.

To support instruction stream synchronization, there are two instructions defined. The **LDWR** instruction loads a value from a memory location and sets an internal marker for the load operation that was done. The counterpart instruction **STWC**, will store a value to that location if the value at that location has not been modified since the last LR instruction to that location occurred.

The remainder of this chapter contains the detailed description for the memory reference instructions. An instruction description starts with the opCode mnemonic and the instruction word structure. Next a short description of the instruction and its purpose are given. Any options and bit specific bit fields as well as exceptions that can be raised by the instruction are given next. Finally, any further notes are presented.


<!--------------------------------------------------------------------------------------------------------->

<div style="page-break-before: always;"></div>

### LDW, LDH, LDB

<hr>

Loads a memory value into a general register.

#### Format

```
      LDx <GR r>, <GR a>( GR<b> )            ; opMode 0 .. 2
      LDx <GR r>, <ofs> ( <SR a>, <GR b> )   ; opMode 3
      LDx <GR r>, <ofs> ( GR y )             ; opMode 4 .. 15, "y" -> GR4 .. GR7
```

```
       0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31
      :--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:
      : LDx     ( 0xx ) : r      : 0      : opMode    :  opArg                                        :
      :-----------------:-----------------------------------------------------------------------------:
```

#### Description

The load instruction will load the operand into the general register "r". There are four major modes for this instruction. Mode 4 - 7, 8 - 11 and 12 - 16 compute the address by adding the signed offset to an index registers. Mode 4, 8 and 12 refer to index register GR4, mode 5, 9, 13 to index register GR5 and so on. The upper two bits of the selected index register are used to select from the segment registers SR 4 to 7. Mode 4 - 7 will fetch a word from the computed address, mode 8 - 11 fetch a half-word and mode 12 - 15 a byte. Mode 3 is the extended address mode. The "a" field contains the segment register, and "b" the unsigned offset. Operand mode 0 - 2 form the operand address by using "a" as the base register and add the signed offset in "b" to it. Mode 0 will fetch a word, mode 1 a half-word and mode 2 a byte. The fetched half-word or byte operand is zero sign extended and stored in "r".

#### Operation

```
   seg <- operandAdrSeg( instr );
   ofs <- operandAdrOfs( instr );
   len <- operandBitLen( instr );

   GR[ r ] <- zeroExtend( memLoad( seg, ofs, len ), 32 - len );
```

#### Exceptions

- Illegal instruction trap
- DTLB miss/data page fault
- Data memory access rights trap
- Data memory protection Id trap
- Page reference trap
- Alignment trap

#### Notes

None.

<!--------------------------------------------------------------------------------------------------------->

<div style="page-break-before: always;"></div>

### STW, STH, STB

<hr>

Stores a general register value into memory

#### Format

```
      STx <ofs> ( <SR a>, <GR b> ), <GR r>   ; opMode 3
      STx <ofs> ( GR y ), <GR r>             ; opMode 4 .. 15, "y" -> GR4 .. GR7
```

```
       0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31
      :--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:
      : STx     ( 0xx ) : r      : 0      : opMode    :  opArg                                        :
      :-----------------:-----------------------------------------------------------------------------:
```

#### Description

The load instruction will store the data in general register "r" to memory. There are four major modes for this instruction. Mode 4 - 7, 8 - 11 and 12 - 16 compute the address by adding the signed offset to an index registers. Mode 4, 8 and 12 refer to index register GR4, mode 5, 9, 13 to index register GR5 and so on. The upper two bits of the selected index register are used to select from the segment registers SR 4 to 7. Mode 4 - 7 will store the general register to the computed address, mode 8 - 11 store bits 16 - 31 to memory and mode 12 - 15 store bits 24 .. 31 to memory. Mode 3 is the extended address mode. The "a" field contains the segment register, and "b" the unsigned offset.The operand size for mode 3 is a word. Operand mode 0 - 2 is undefined for this instruction.

#### Operation

```
      if ( opMode < 3  ) illegalInstructionTrap( );

      seg <- operandAdrSeg( instr );
      ofs <- operandAdrOfs( instr );
      len <- operandBitLen( instr );

      memStore( seg, ofs, GR< r >, len );
```

#### Exceptions

- Illegal instruction trap
- DTLB miss/data page fault
- Data memory access rights trap
- Data memory protection Id trap
- Page reference trap
- Alignment trap

#### Notes

None.


<!--------------------------------------------------------------------------------------------------------->

<div style="page-break-before: always;"></div>

### LDWE, LDHE, LDBE

<hr>

Loads the memory content into a general register using a virtual address.

#### Format

```
      LDxE <GR r>, <ofs> ( <SR a>, <GR b> )
      
```

```
       0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31
      :--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:
      : LDWE   ( 0xx )  : r      : ofs2               : ofs1                        : a      : b      :
      ------------------:-----------------------------------------------------------------------------:
      : LDHE   ( 0xx )  : r      : ofs2               : ofs1                        : a      : b      :
      :-----------------:-----------------------------------------------------------------------------:
      : LDBE   ( 0xx )  : r      : ofs2               : ofs1                        : a      : b      :
      :-----------------:-----------------------------------------------------------------------------:
```

#### Description

The load extended instructions will load the operand into the general register "r" using a virtual address. The virtual address is formed by the segment register "a" and the general register "b" plus the signed offset built from "ofs1" and "ofs2". There are three instructions for loading a word a half-word or byte. Half-word and byte data is zero sign extended.

#### Operation

```
      LDWE: GR[r] <- zeroExtend( memLoad( SR[a], add32( GR[b], signExtend( catImm( ofs1, ofs2 ), 17 )), 32 ));
      LDHE: GR[r] <- zeroExtend( memLoad( SR[a], add32( GR[b], signExtend( catImm( ofs1, ofs2 ), 17 )), 16 ));
      LDBE: GR[r] <- zeroExtend( memLoad( SR[a], add32( GR[b], signExtend( catImm( ofs1, ofs2 ), 17 )), 8  ));
```

#### Exceptions

- DTLB miss/data page fault
- Data memory access rights trap
- Data memory protection Id trap
- Page reference trap
- Alignment trap

#### Notes

None.


<!--------------------------------------------------------------------------------------------------------->

<div style="page-break-before: always;"></div>

### STWE, STWH, STWB

<hr>

Stores a general register value into memory using a virtual address.

#### Format

```
      STxE <ofs> ( <SR a>, <GR b> ), <GR r>
```

```
       0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31
      :--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:
      : STWE   ( 0xx )  : r      : ofs2               : ofs1                        : a      : b      :
      ------------------:-----------------------------------------------------------------------------:
      : STHE   ( 0xx )  : r      : ofs2               : ofs1                        : a      : b      :
      :-----------------:-----------------------------------------------------------------------------:
      : STBE   ( 0xx )  : r      : ofs2               : ofs1                        : a      : b      :
      :-----------------:-----------------------------------------------------------------------------:
```

#### Description

The store extended instructions will store data from general register "r" using a virtual address. The virtual address is formed by the segment register "a" and the general register "b" plus the signed offset built from "ofs1" and "ofs2". There are three instructions for storing a word a half-word or byte. A half-word store takes bits 16 .. 31 from the general register "r", a byte store bits 24 .. 31.

#### Operation

```
      STWE: memStore( SR[a], add32( GR[b], signExtend( catImm( ofs1, ofs2 ), 17 )), GR[r], 32 );
      STHE: memStore( SR[a], add32( GR[b], signExtend( catImm( ofs1, ofs2 ), 17 )), GR[r], 16 );
      STBE: memStore( SR[a], add32( GR[b], signExtend( catImm( ofs1, ofs2 ), 17 )), GR[r], 8  );
```

#### Exceptions

- DTLB miss/data page fault
- Data memory access rights trap
- Data memory protection Id trap
- Page reference trap
- Alignment trap

#### Notes

None.


<!--------------------------------------------------------------------------------------------------------->

<div style="page-break-before: always;"></div>

### LDWA, LDHA, LDBA

<hr>

Loads the memory content into a general register using an absolute address.

#### Format

```
      LDxA <GR r>, <ofs> ( <GR b> )
```

```
       0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31
      :--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:
      : LDWA   ( 0xx )  : r      : ofs2               : ofs1                                 : b      :
      :-----------------:-----------------------------------------------------------------------------:
      : LDHA   ( 0xx )  : r      : ofs2               : ofs1                                 : b      :
      :-----------------:-----------------------------------------------------------------------------:
      : LDBA   ( 0xx )  : r      : ofs2               : ofs1                                 : b      :
      :-----------------:-----------------------------------------------------------------------------:
```

#### Description

The load absolute instruction will load the content of the physical memory address into the general register "r". The absolute 32-bit address is computed from adding the signed offset formed by concatenating the "ofs1" and "ofs2" fields to general register "b". There are three instructions for loading a word a half-word or byte. Half-word and byte data is zero sign extended. The LDxA instructions are privileged instructions.

#### Operation

```
      if ( ! ST.[ PRIV ] ) privilegedOperationTrap( );

      LDWA: GR[r] <- zeroExtend( memLoad( 0, add32( GR[b], signExtend( catImm( ofs1, ofs2 ), 20 )), 32 ));
      LDHA: GR[r] <- zeroExtend( memLoad( 0, add32( GR[b], signExtend( catImm( ofs1, ofs2 ), 20 )), 16 ));
      LDBA: GR[r] <- zeroExtend( memLoad( 0, add32( GR[b], signExtend( catImm( ofs1, ofs2 ), 20 )), 8  ));
```

#### Exceptions

- Privileged operation trap
- Alignment trap

#### Notes

None.


<!--------------------------------------------------------------------------------------------------------->

<div style="page-break-before: always;"></div>

### STWA, STHA, STBA

<hr>

Stores a general register value into memory using an absolute physical address.

#### Format

```
      STxA <ofs> ( <SR a>, <GR b> ), <GR r>
```

```
       0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31
      :--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:
      : STWA   ( 0xx )  : r      : ofs2               : ofs1                                 : b      :
      :-----------------:-----------------------------------------------------------------------------:
      : STHA   ( 0xx )  : r      : ofs2               : ofs1                                 : b      :
      :-----------------:-----------------------------------------------------------------------------:
      : STBA   ( 0xx )  : r      : ofs2               : ofs1                                 : b      :
      :-----------------:-----------------------------------------------------------------------------:
```

#### Description

The store absolute instruction will store the target register into memory using a physical address. The absolute 32-bit address is computed from adding the signed offset formed by concatenating the "ofs1" and "ofs2" fields. There are three instructions for storing a word a half-word or byte. A half-word store takes bits 16 .. 31 from the general register "r", a byte store bits 24 .. 31. The STxA instructions are a privileged instruction.

#### Operation

```
      if ( ! ST.[ PRIV ] ) privilegedOperationTrap( );

      STWA: memStore( 0, add32( GR[b], signExtend( catImm( ofs1, ofs2 ), 20 )), GR[r], 32 );
      STHA: memStore( 0, add32( GR[b], signExtend( catImm( ofs1, ofs2 ), 20 )), GR[r], 16 );
      STBA: memStore( 0, add32( GR[b], signExtend( catImm( ofs1, ofs2 ), 20 )), GR[r], 8  );
```

#### Exceptions

- Privileged operation trap
- Alignment trap

#### Notes

None.


<!--------------------------------------------------------------------------------------------------------->

<div style="page-break-before: always;"></div>

### LDWR

<hr>

Loads the operand into the target register from the address and marks that address.

#### Format

```
      LDWR <GR r>, <ofs> ( <SR a>, <GR b> )   ; opMode 3
      LDWR <GR r>, <ofs> ( GR y )             ; opMode 4 .. 7, "y" -> GR4 .. GR7
```

```
       0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31
      :--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:
      : LDWR   ( 0xx )  : r      : 0      : opMode    : opArg                                         :
      :-----------------------:-----------------------------------------------------------------------:
```

#### Description

The LDWR instruction is used for implementing semaphore type operations. Operand mode 0, 1, 2 and 8 .. 15 are undefined for this instruction. Operand mode 3 creates a virtual address from a segment register "a" and the base address in register "b" plus an offset embedded in the instruction. Operand mode 4 to 7 concatenate the immediate with the "ofs2" field to form the signed offset. The first part of the instruction behaves exactly like the LDW instruction. A logical memory address is computed. Next, the memory content is loaded into general register "r". The second part remembers the address and will detect any modifying reference to it.

#### Operation

```
      seg <- operandAdrSeg( instr );
      ofs <- operandAdrOfs( instr );
      len <- operandBitLen( instr );

      GR[ r ] <- zeroExtend( memLoad( seg, ofs, len ), 32 - len );
      
      lrValid = true;
      lrArg   = GR[r];
```

#### Exceptions

- iIllegal instruction trap
- DTLB miss/data page fault
- Data memory access rights trap
- Data memory protection Id trap
- Page reference trap
- Alignment trap

#### Notes

The "remember the access part" is highly implementation dependent. One option is to implement this feature in the L1 data cache. Under construction...


<!--------------------------------------------------------------------------------------------------------->

<div style="page-break-before: always;"></div>

### STWC

<hr>

Conditionally store a value to memory.

#### Format

```
      STWC <ofs> ( <SR a>, <GR b> ), <GR r>   ; opMode 3
      STWC <ofs> ( GR y ), <GR r>             ; opMode 4 .. 7, "y" -> GR4 .. GR7
```

```
       0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31
      :--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:
      : LDWR   ( 0xx )  : r      : 0      : opMode    : opArg                                         :
      :-----------------:-----------------------------------------------------------------------------:
```

#### Description

The STWR conditional instruction will store a value in "r" to the memory location specified by the operand address. The store is however only performed when the data location has not been written to since the last load reference instruction execution for that address. If the operation is successful a value of zero is returned otherwise a value of one. Operand mode 3 creates a virtual address from a segment register "a" and the base address in register "b" plus an offset embedded in the instruction.Operand mode 4 to 7 concatenate the immediate with the "ofs2" field to form the signed offset. Operand mode 0, 1 and 2 are undefined for this instruction.

#### Operation

```
      if (( lrValid ) && ( lrVal == GR[r])) {

         if (( opMode < 3 ) || ( opMode > 7 )) illegalInstructionTrap( );
         memStore( operandAdrSeg( instr ), operandAdrOfs( instr ), GR[r], 32 );
         GR[r] <- 0;

      } else GR[r] <- 1;
```

#### Exceptions

- Illegal instruction tap
- Data TLB miss/data page fault
- Data memory access rights trap
- Data memory protection Id trap
- Page reference trap
- Alignment trap

#### Notes

The "check the access part" is highly implementation dependent. One option is to implement this feature in the L1 data cache. Under construction...

<!--------------------------------------------------------------------------------------------------------->

<div style="page-break-before: always;"></div>

## Branch Instructions

The control flow instructions are divided into unconditional and conditional branch type instructions. The unconditional branch type instructions allow in addition to altering the control flow, the save of the return point to a general register. A subroutine call would for example compute the target branch address and store this address + 4 in that register. Upon subroutine return, there is a branch instruction to return via a general register content.

The **B** and **BL** instruction are the unconditional branch type instruction within a program segment. They add a signed offset to the current instruction address. The BL instruction additionally saves a return link in a general register. The **BR** and **BLR** instruction behave similar, except that a general register contains the instruction address relative offset. The **BV** and **BVR** are instruction segment absolute instructions. The branch address for the BV instruction is the segment relative value specified in a general register. The BVR instruction features two register, one being the segment relative base and the other a signed offset to be added.

The **BE** and **BLE** instruction are the inter-segment branches. The BE instruction branches to a segment absolute address encoded in the segment and general offset register. In addition, a signed offset encoded in the instruction can is added to form the target segment offset. The BLE instruction will in addition return the return address in the segment register SR0 and the offset on the general register GR0.

Segment relative and external branches potentially branch to pages with a different privilege level. The **GATE** instruction will promote the privilege level to the page where the GATE instruction resides. 

The conditional branch instructions combine an operation such as comparison or test with a local instruction address relative branch if the comparison or test condition is met. The **CBR** instruction will compare two general registers for a condition encoded in the instruction. If the condition is met, an instruction address relative branch is performed. The target address is formed by adding an offset encoded in the instruction to the instruction address. In a similar way, the **TBR** instruction will test a general register for a condition and perform an instruction relative branch if the condition is met.

The remainder of this chapter contains the detailed description for the branching instructions. An instruction description starts with the opCode mnemonic and the instruction word structure. Next a short description of the instruction and its purpose are given. Any options and bit specific bit fields as well as exceptions that can be raised by the instruction are given next. Finally, any further notes are presented.


<!--------------------------------------------------------------------------------------------------------->

<div style="page-break-before: always;"></div>

### B

<hr>

Perform an unconditional IA-relative branch with a static offset.

#### Format

```
      B <ofs>
```

```
       0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31
      :--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:
      : B     ( 0xx )   : 0      : ofs2               : ofs1                                          :
      :-----------------:-----------------------------------------------------------------------------:
```

#### Description

The branch instruction performs a branch to an instruction address relative location. The target address is formed by concatenating adding the "ofs1" and "ofs2" field to form a sign extended immediate value, shift by 2 to the left and add it to the current instruction offset. The virtual target address is built from the instruction address segment and offset. If code translation is disabled, the target address is the absolute physical address.

#### Operation

```
      IA-OFS <- add32( IA-OFS, signExt( catImm( ofs1, ofs2 ) << 2, 23 ));
```

#### Exceptions

- Taken branch trap.

#### Notes

None.

<!--------------------------------------------------------------------------------------------------------->

<div style="page-break-before: always;"></div>

### BL

<hr>

Perform an unconditional IA-relative branch with a static offset and store the return address an a general register.

#### Format

```
      BL <GR r>, <ofs>
```

```
       0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31
      :--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:
      : BL    ( 0xx )   : r      : ofs2               : ofs1                                          :
      :-----------------:-----------------------------------------------------------------------------:
```

#### Description

The branch instruction performs a branch to an instruction address relative location. The target address is formed by concatenating adding the "ofs1" and "ofs2" field to form a sign extended immediate value and add it to the current instruction address offset. The current instruction address offset + 4 is returned in general register "r". If code translation is disabled, the return value is the absolute physical address.

#### Operation


```
      GR[r] <- add32( IA-OFS, 4 );
      IA-OFS <- add32( IA-OFS, signExt( catImm( ofs1, ofs2 ) << 2, 23 ));
```

#### Exceptions

- Taken branch trap.

#### Notes

None.

<!--------------------------------------------------------------------------------------------------------->

<div style="page-break-before: always;"></div>

### BR

<hr>

Perform an unconditional IA-relative branch with a dynamic offset from a general register.

#### Format

```
      BR <GR b>
```

```
       0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31
      :--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:
      : BR    ( 0xx )   : 0                                                                  : b      :
      :-----------------:-----------------------------------------------------------------------------:
```

#### Description

The branch register instruction performs an unconditional IA-relative branch. The target address is formed by adding the content of register "b" shifted by two bits to the left to the current instruction address. If code translation is disabled, the target address is the absolute physical address.

#### Operation

```
      IA-OFS <- add32( IA-OFS, ( GR[b] << 2 ));
```

#### Exceptions

- Taken branch trap.

#### Notes

None.

<!--------------------------------------------------------------------------------------------------------->

<div style="page-break-before: always;"></div>

### BLR

<hr>

Perform an unconditional IA-relative branch with a dynamic offset stored in a general register and store the return address in a general register.

#### Format

```
      BL <GR r>, <GR b>
```

```
       0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31
      :--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:
      : BLR   ( 0xx )   : r      : 0                                                         : b      :
      :-----------------:-----------------------------------------------------------------------------:
```

#### Description

The branch register instruction performs an unconditional IA-relative branch. The target address is formed adding the content of register "b" to the current instruction address. If code translation is disabled, the target address is the absolute physical address. The current instruction address offset + 4 is returned in general register "r".

#### Operation

```
      GR[r]  <- add32( IA-OFS, 4 );
      IA-OFS <- add32( IA-OFS, GR[b] );
```

#### Exceptions

- Taken branch trap

#### Notes

None.

<!--------------------------------------------------------------------------------------------------------->

<div style="page-break-before: always;"></div>

### BV

<hr>

Perform an unconditional branch using a general register containing the base relative target branch address.

#### Format

```
      BV <GR b>
```

```
       0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31
      :--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:
      : BV    ( 0xx )   : 0                                                                  : b      :
      :-----------------:-----------------------------------------------------------------------------:
```

#### Description

The branch vectored instruction performs an unconditional branch to the target address in register in "b". Register "b" is interpreted as an instruction address in the current code segment. This unconditional jump allows to reach the entire code address range. If code translation is disabled, the resulting offset is the absolute physical address.

Since the BV instruction is a segment base relative branch, a branch to page with a different privilege level is possible. A branch from a lower level to a higher level result in an instruction protection trap. A branch from a higher privilege to a lower privilege level result in the privilege level adjusted in the status register. Otherwise, the privilege level remains unchained.

#### Operation

```
      IA-OFS <- GR[b];
```

#### Exceptions

- Taken branch trap
- Instruction memory protection trap

#### Notes

The BV instruction is typically used in a procedure return. The BL instruction left a segment relative address which can directly be used by this instruction to return to the location after the BL instruction.

<!--------------------------------------------------------------------------------------------------------->

<div style="page-break-before: always;"></div>

### BVR

<hr>

Perform an unconditional branch using a base and offset general register for forming the target branch address.

#### Format

```
      BVR <GR b>, <GR a>
```

```
       0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31
      :--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:
      : BVR   ( 0xx )   : 0                                                         : a      : b      :
      :-----------------:-----------------------------------------------------------------------------:
```

#### Description

The branch vectored instruction performs an unconditional branch by adding the offset register in "a" to the base address in register in "b". The result is interpreted as an instruction address in the current code segment. This unconditional jump allows to reach the entire code address range. If code translation is disabled, the resulting offset is the absolute physical address.

Since the BVR instruction is a segment base relative branch, a branch to page with a different privilege level is possible. A branch from a lower level to a higher level result in an instruction protection trap. A branch from a higher privilege to a lower privilege level result in the privilege level adjusted in the status register. Otherwise, the privilege level remains unchained.

#### Operation

```
      GR[r] <- add32( IA-OFS, 4 );
      IA-OFS <- add32( GR[b], GR[a] );
```

#### Exceptions

- Taken branch trap
- Instruction memory protection trap

#### Notes

None.


<!--------------------------------------------------------------------------------------------------------->

<div style="page-break-before: always;"></div>

### BE

<hr>

Perform an unconditional external branch.

#### Format

```
      BE <ofs> ( <SR a>, <GR b> )
```

```
       0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31
      :--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:
      : BE    ( 0xx )   : ofs2                        : ofs1                        : a      : b      :
      :-----------------:-----------------------------------------------------------------------------:
```

#### Description

The branch external instruction branches to a segment relative location in another code segment. The target address is built from the segment register in field "a" and the base register "b" to which the sign extended offset built from "ofs1" and ofs2" is added. If code translation is disabled, the offset is the absolute physical address and the "a" field being ignored.

Since the BE instruction is a segment base relative branch, a branch to page with a different privilege level is possible. A branch from a lower level to a higher level result in an instruction protection trap. A branch from a higher privilege to a lower privilege level result in the privilege level adjusted in the status register. Otherwise, the privilege level remains unchained.

#### Operation

```
      IA-SEG <- GR[a];
      IA-OFS <- add22( GR[b], signExt( catImm( ofs1, ofs2 )), 20 );
```

#### Exceptions

- Taken branch trap
- Instruction memory protection trap

#### Notes

None.


<!--------------------------------------------------------------------------------------------------------->

<div style="page-break-before: always;"></div>

### BLE

<hr>

Perform an unconditional external branch and save the return address.

#### Format

```
      BLE <ofs> ( <SR a>, <GR b> )
```

```
       0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31
      :--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:
      : BLE   ( 0xx )   : ofs2                        : ofs1                        : a      : b      :
      :-----------------:-----------------------------------------------------------------------------:
```

#### Description

The branch and link external instruction branches to an absolute location in another code segment. The target address is built from the segment register in field "a" and the base register "b" to which the sign extended offset built from "ofs1" and ofs2" is added. The return address is saved in SR0 and GR0. If code translation is disabled, the offset is the absolute physical address and the "a" field being ignored.

Since the BLE instruction is a segment base relative branch, a branch to page with a different privilege level is possible. A branch from a lower level to a higher level result in an instruction protection trap. A branch from a higher privilege to a lower privilege level result in the privilege level adjusted in the status register. Otherwise, the privilege level remains unchained.

#### Operation

```
      SR[0] <- IA-SEG;
      GR[0] <- add32( IA-OFS, 4 );

      IA-SEG <- GR[a];
      IA-OFS <- add32( GR[b], signExt( catImm( ofs1, ofs2 )),20 );
```

#### Exceptions

- Taken branch trap
- Instruction memory protection trap

#### Notes

None.


<!--------------------------------------------------------------------------------------------------------->

<div style="page-break-before: always;"></div>

### GATE

<hr>

Perform an instruction address relative branch and change the privilege level.

#### Format

```
      GATE <GR r>, <ofs>
```

```
       0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31
      :--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:
      : GATE  ( 0xx )         : r         : ofs2                  : ofs1                              :
      :-----------------------:-----------------------------------------------------------------------:
```

#### Description

The GATE instruction computes the target address concatenating the fields "ofs1" and "ofs2", adding it to the current instruction address offset. If code translation is enabled, the privilege level is changed to the privilege field in the TLB entry for the page from which the GATE instruction is fetched. The change is only performed when it results in a higher privilege level, otherwise the current privilege level remains. If code translation is disabled, the privilege level is set to zero. The resulting privilege level is deposited in the P bit of the GR "r". Execution continues at the target address with the new privilege level.

#### Operation

```
      GR[r] <- cat( IA-OFS.[P], ofsSelect( GR[r] );

      if ( ST.[C] ) {

         searchInstructionTlbEntry( seg, ofs, &entry );
	     if ( entry.[PageType] == 3 ) priv <- entry.[ PL1 ];
	     else priv <- IA-OFS.[P];
      }
      else priv <- 0;

      IA-OFS <- add22( ofsSelect( IA-OFS ), signExt( catImm( ofs1, ofs2 ), 15 ));
      IA-OFS.[P] <- priv;
```

#### Exceptions

None.

#### Notes

None.


<!--------------------------------------------------------------------------------------------------------->

<div style="page-break-before: always;"></div>

### CBR

Compare two registers and branch on condition.

#### Format

```
      CBR [ .<cond> ] <GR a>, <GR b>, <ofs>
```

```
       0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31
      :--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:
      : CBR ( 0xx )     : cond   : ofs2               :  ofs1                       : a      : b      :
      :-----------------:-----------------------------------------------------------------------------:
```

#### Description

The CBR instruction compares general registers "a" and "b" for the condition specified in the "cond" field. The instruction address relative branch offset is formed by concatenating the fields "ofs1" and ofs2", sign extended. If the condition is met, the target branch address is the offset shifted by two to the left and added to the current instruction address, otherwise execution continues with the next instruction. The conditional branch is predicted taken for a backward branch and predicted not taken for a forward branch.

#### Branch condition

The condition field is encoded as follows:

 ```
      0 : EQ  ( a == b )                              4 : NE  ( a != b )
      1 : LT ( a < b, Signed )                        5 : LE ( a <= b, Signed )
      2 : GT ( a > b, Signed )                        6 : GE ( a >= b, Signed )
      3 : LS ( a <= b, Unsigned )                     7 : HI ( a > b, Unsigned )
```

#### Operation

```
      switch( cond ) {

         case 0: res <- ( GR[a] ==  GR[b]);  break;
         case 1: res <- ( GR[a] <   GR[b]);  break;
         case 2: res <- ( GR[a] >   GR[b]);  break;
         case 3: res <- ( GR[a] <<= GR[b]);  break;
         case 4: res <- ( GR[a] !=  GR[b]);  break;
         case 5: res <- ( GR[a] <=  GR[b]);  break;
         case 6: res <- ( GR[a] >=  GR[b]);  break;
         case 7: res <- ( GR[a] >>  GR[b]);  break;
      }

    if ( res ) IA-OFS <- add32( IA-OFS, ( signExt( catImm( ofs1, ofs2 ), 17 ) << 2 ));
	else add22( IA_OFS, 4 );
```

#### Exceptions

None.

#### Notes

Often a comparison is followed by a branch in an instruction stream. VCPU-32 therefore features conditional branch instructions that both offer the combination of a register evaluation and branch depending on the condition specified.


<!--------------------------------------------------------------------------------------------------------->

<div style="page-break-before: always;"></div>

### TBR

Test a register and branch on condition.

#### Format

```
      TBR [ .<cond> ] <GR b>, <ofs>
```

```
       0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31
      :--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:
      : TBR ( 0xx )     : cond   : ofs2               :  ofs1                       : 0      : b      :
      :-----------------:-----------------------------------------------------------------------------:
```

#### Description

The TBR instruction tests general register "b" for the condition specified in the "cond" field. The instruction relative branch offset is formed by concatenating the fields "ofs1" and ofs2", sign extended. If the condition is met, the target address is the offset shifted by two to the left and added to the current instruction address. Otherwise, execution continues with the next instruction. The conditional branch is predicted taken for a backward branch and predicted not taken for a forward branch.

#### Branch condition

```
      0 : EQ ( b == 0 )                               4 : NE  ( b != 0 )
      1 : LT ( b < 0, Signed )                        5 : LE ( b <= 0, Signed )
      2 : GT ( b > 0, Signed )                        6 : GE ( b >= 0, Signed )
      3 : EV ( even( b ))                             7 : OD ( odd( b ))
```

#### Operation

```
      switch( cond ) {

         case 0: res <- ( GR[b] == 0 );   break;
         case 1: res <- ( GR[b] >  0 );   break;
         case 2: res <- ( GR[b] <  0 );   break;
         case 3: res <- ( ~ GR[b].[31] ); break;
         case 4: res <- ( GR[b] != 0 );   break;
         case 5: res <- ( GR[b] <= 0 );   break;
         case 6: res <- ( GR[b] >= 0 );   break;
         case 7: res <- ( GR[b].[31] );   break;
      }

      if ( res ) IA-OFS <- add32( IA-OFS, ( signExt( catImm( ofs1, ofs2 ), 17 ) << 2 ));
      else add32( IA_OFS, 4 );
```

#### Exceptions

None.

#### Notes

Often a test is followed by a branch in an instruction stream. VCPU-32 therefore features conditional branch instructions that both offer the combination of a register evaluation and branch depending on the condition specified.


<!--------------------------------------------------------------------------------------------------------->

<div style="page-break-before: always;"></div>

## Computational Instructions

The arithmetic, logical and bit field operations instruction represent the computation type instructions. Most of the computational instructions use the operand instruction format, where the operand fields, opMode and opArg, define one of the instruction operands. In addition, these instructions have a three bit field which further specifies instruction specific options.

The computation **ADD** and **SUB** instructions perform the numeric arithmetic. **AND**, **OR** and **XOR** represent logical operations. There are instruction specific options to negate the input operand and the output. Negating an **OR** results in an **NOR**, and so on. Negating the operand input allows for an effective way to create complementary bit masks using the **AND** instruction.

The bit shift, deposit and extract instructions **EXTR**, **DEP** and **DSR**, are available for bit field manipulations in a data word. Their instruction format specifies a bit field with a bit position and bit length. With these three instructions bit operations such as setting or clearing a bit are easy and performant to do. Furthermore, the bit shift right and left and rotate operations can easily be done with these three instructions.

The **LDI** instruction is used to load a constant value int a register. There are options to select the lower or upper half of the register such that a two instruction sequence covers create the entire numeric 24-bit range. The **CMP** instruction compares two values and stores a one or zero depending on the comparison result. The **CMR** instruction will test a register for a condition and store another register or immediate value into the target register.

The shift and add instruction **SHLA** combines a shift operation with an add operation. This is allows to perform multiplication with small integer constants with very few instructions. For example, multiplying a register value by 5 could be done by simply shifting the general register "r" by two bits and an add the same "r" value to it.

The **LEA** instruction will return the address offset portion of the operand. The **LDSID** instruction will return the segment id of the operand address. Both instructions are purely computational instruction with no memory reference. Finally. there is also a **NOP** instruction, which has the opcode of zero and performs, you guessed it, a no-operation.

Note that the assembler will use the instruction format *OP GR1,GR2* for the *regR = regR op regB* operation. This should be translated to an operand mode two type instruction of the form *GR1 <- Gr1 OP GR2*.  The assembler therefore maps the instruction formats *GR1 -> GR1 OP GR2* and *GR1 <- GR2 OP GR3* to operand mode two.

Operand mode one is of the form *GR1 <- 0 OP GR2*, i.e. one argument is zero. This is very useful for building pseudo instructions such as NEG, which for example is a subtract of GR2 from 0. Operand mode one is only used for pseudo instructions that explicitly map to operand mode 1, such as the NEG example shown.

The remainder of this chapter contains the detailed description for the computational instructions. An instruction description starts with the opCode mnemonic and the instruction word structure. Next a short description of the instruction and its purpose are given. Any options and bit specific bit fields as well as exceptions that can be raised by the instruction are given next. Finally, any further notes are presented.


<!--------------------------------------------------------------------------------------------------------->

<div style="page-break-before: always;"></div>

### ADD

<hr>

Adds the operand to the target register.

#### Format

```
      ADD [ .<opt> ] <GR r>, <val>                      ; opMode 0
      ADD [ .<opt> ] <GR r>, <GR b>                     ; opMode 2
      ADD [ .<opt> ] <GR r>, <GR a>, <GR b>             ; opMode 2
      ADD [ .<opt> ] <GR r>, <ofs> ( <SR a>, <GR b> )   ; opMode 3
      ADD [ .<opt> ] <GR r>, <ofs> ( GR y )             ; opMode 4 .. 15, "y" -> GR4 .. GR7
```

```
       0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31
      :--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:
      : ADD   ( 0xx )   : r      :C :L :O : opMode    : opArg                                         :
      :-----------------:-----------------------------------------------------------------------------:
```

#### Description

The instruction fetches the operand and adds it to the general register "r". If the C bit is set, the status register carry bit is added too. The L bit specifies that his is an unsigned add. If the O bit is set, a numeric overflow will cause an overflow trap. 

There are four major mode groups for this instruction. Operand mode 0 adds a sign extended constant to "r". Operand mode 1 adds a zero value to the b" content and stores in the result reg "r". The "a" field should be set to zero. Operand mode 2 allows to specify two general register to be the input operands, independent of the target general register in "r". This way, a three register address add operation of the form "r" = "a" + "b" as well as a "r" = "r" op "b" can be done. The operations will set the carry/borrow bits in the processor status word. Operand mode 3 to 7 fetch the operand according to the addressing mode and perform a R = R + operand operation.

Mode 4 - 7, 8 - 11 and 12 - 16 compute the address by adding the signed offset to an index registers. Mode 4, 8 and 12 refer to index register GR4, mode 5, 9, 13 to index register GR5 and so on. The upper two bits of the selected index register are used to select from the segment registers SR 4 to 7. Mode 4 - 7 will fetch a word from the computed address, mode 8 - 11 fetch a half-word and mode 12 - 15 a byte. Mode 3 is the extended address mode. The "a" field contains the segment register, and "b" the unsigned offset.

#### Operation

```
      switch( opMode ) {

         case 0:  tmpA <- GR[r];
	              tmpB <- signExt( opArg, 16 );
		          break;

         case 1:  tmpA <- 0;
		          tmpB <- GR[b];
			      break;

         case 2:  tmpA <- GR[a];
		          tmpB <- GR[b];
			      break;

         default: seg <- operandAdrSeg( instr );
                  ofs <- operandAdrOfs( instr );
                  len <- operandBitLen( instr );

                  tmpA <- GR[r];
                  tmpB <- zeroExtend( memLoad( seg, ofs, len ), 32 - len );
      }

      if ( instr.[C] ) res <- tmpA + tmpB + instr.[C];
      else res <- tmpA + tmpB;

      if ( instr.[O] && overflow ) overflowTrap( );
      else {

         GR[r] <- res;
         if ( ! instr.[L] ) PSW[C/B] <- carry/borrow Bits;
      }
```

#### Exceptions

- Overflow trap
- DTLB miss/data page fault
- Data memory access rights trap
- Data memory protection Id trap
- Page reference trap
- Alignment trap

#### Notes

None.


<!--------------------------------------------------------------------------------------------------------->

<div style="page-break-before: always;"></div>

### SUB

<hr>

Subtracts the operand from the target register.

#### Format

```
      SUB [ .<opt> ] <GR r>, <val>                      ; opMode 0
      SUB [ .<opt> ] <GR r>, <GR b>                     ; opMode 2
      SUB [ .<opt> ] <GR r>, <GR a>, <GR b>             ; opMode 2
      SUB [ .<opt> ] <GR r>, <ofs> ( <SR a>, <GR b> )   ; opMode 3
      SUB [ .<opt> ] <GR r>, <ofs> ( GR y )             ; opMode 4 .. 15, "y" -> GR4 .. GR7
```

```
       00  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31
      :--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:
      : SUB   ( 0xx )   : r      :C :L :O : opMode    : opArg                                         :
      :-----------------:-----------------------------------------------------------------------------:
```

#### Description

The instruction fetches the operand and subtracts it from the general register "r". If the C bit is set, the status register carry bit is added too. The L bit specifies that his is an unsigned add. If the O bit is set, a numeric overflow will cause an overflow trap.

There are four major mode groups for this instruction. Operand mode 0 adds a sign extended constant to "r". Operand mode 1 subtracts a zero value to the b" content and stores in the result reg "r". The "a" field should be set to zero. Operand mode 2 allows to specify two general register to be the input operands, independent of the target general register in "r". This way, a three register address operation of the form "r" = "a" + "b" as well as a "r" = "r" op "b" can be done. The operations will set the carry/borrow bits in the processor status word. Operand mode 3 to 7 fetch the operand according to the addressing mode and perform a R = R + operand operation.

Mode 4 - 7, 8 - 11 and 12 - 16 compute the address by adding the signed offset to an index registers. Mode 4, 8 and 12 refer to index register GR4, mode 5, 9, 13 to index register GR5 and so on. The upper two bits of the selected index register are used to select from the segment registers SR 4 to 7. Mode 4 - 7 will fetch a word from the computed address, mode 8 - 11 fetch a half-word and mode 12 - 15 a byte. Mode 3 is the extended address mode. The "a" field contains the segment register, and "b" the unsigned offset. 

#### Operation

```
      switch( opMode ) {

         case 0:  tmpA <- GR[r];
	              tmpB <- signExt( opArg, 16 );
		          break;

         case 1:  tmpA <- 0;
		          tmpB <- GR[b];
			      break;

         case 2:  tmpA <- GR[a];
		          tmpB <- GR[b];
			      break;

         default: seg <- operandAdrSeg( instr );
                  ofs <- operandAdrOfs( instr );
                  len <- operandBitLen( instr );

                  tmpA <- GR[r];
                  tmpB <- zeroExtend( memLoad( seg, ofs, len ), 32 - len );
      }

      if ( instr.[L] )  res <- tmpA + ( ~ tmpB ) + 1;
      else  res <- tmpA + ( ~ tmpB ) + instr.[C];

      if ( instr.[O] && overflow ) overflowTrap( );
      else {

         GR[r] <- res;
         if ( ! instr[L] ) PSW[C/B] <- carry/borrow Bits;
      }
```

#### Exceptions

- Overflow trap
- DTLB miss/data page fault
- Data memory access rights trap
- Data memory protection Id trap
- Page reference trap
- Alignment trap

#### Notes

None.


<!--------------------------------------------------------------------------------------------------------->

<div style="page-break-before: always;"></div>

### AND

<hr>

Performs a bitwise AND of the operand and the target register and stores the result into the target register.

#### Format

```
      AND [ .<opt> ] <GR r>, <val>                      ; opMode 0
      AND [ .<opt> ] <GR r>, <GR b>                     ; opMode 2
      AND [ .<opt> ] <GR r>, <GR a>, <GR b>             ; opMode 2
      AND [ .<opt> ] <GR r>, <ofs> ( <SR a>, <GR b> )   ; opMode 3
      AND [ .<opt> ] <GR r>, <ofs> ( GR y )             ; opMode 4 .. 15, "y" -> GR4 .. GR7
```

```
       00  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31
      :--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:
      : AND   ( 0xx )   : r      :N :C :0 : opMode    : opArg                                         :
      :-----------------:-----------------------------------------------------------------------------:
```

#### Description

The instruction fetches the data specified by the operand and performs a bitwise AND of the general register "r" and the operand fetched. The result is stored in general register "r". The N bit allows to negate the result making the AND a NAND operation. The C bit allows to complement ( 1's complement ) the operand input, which is the regB register.

There are four major mode groups for this instruction. Operand mode 0 ANDs a sign extended constant into "r". Operand mode 1 ANDs a zero value to the "b" content and stores in the result reg "r". The "a" field should be set to zero. Operand mode 2 allows to specify two general register to be the input operands, independent of the target general register in "r". This way, a three register address operation of the form "r" = "a" + "b" as well as a "r" = "r" op "b" can be done. The operations will set the carry/borrow bits in the processor status word.

Mode 4 - 7, 8 - 11 and 12 - 16 compute the address by adding the signed offset to an index registers. Mode 4, 8 and 12 refer to index register GR4, mode 5, 9, 13 to index register GR5 and so on. The upper two bits of the selected index register are used to select from the segment registers SR 4 to 7. Mode 4 - 7 will fetch a word from the computed address, mode 8 - 11 fetch a half-word and mode 12 - 15 a byte. Mode 3 is the extended address mode. The "a" field contains the segment register, and "b" the unsigned offset.

#### Operation

```
      switch( opMode ) {

         case 0:  tmpA <- GR[r];
	              tmpB <- signExt( opArg, 16 );
		          break;

         case 1:  tmpA <- 0;
		          tmpB <- GR[b];
			      break;

         case 2:  tmpA <- GR[a];
		          tmpB <- GR[b];
			      break;

         default: seg <- operandAdrSeg( instr );
                  ofs <- operandAdrOfs( instr );
                  len <- operandBitLen( instr );

                  tmpA <- GR[r];
                  tmpB <- zeroExtend( memLoad( seg, ofs, len ), 32 - len );
      }

      if ( instr.[C]) tmpB <- ~ tmpB;
      res <- tmpA & tmpB;

      if ( instr.[N]) res <- ~res;
      GR[r] <- res;
```

#### Exceptions

- DTLB miss/data page fault
- Data memory access rights trap
- Data memory protection Id trap
- Page reference trap
- Alignment trap

#### Notes

Complementing the operand input allows to perform a bit clear in a register word by complementing the bit mask stored in the operand before performing the AND. Typically this is done in a program in two steps, which are first to complement the mask and then AND to the target variable. The C option allows to do this more elegantly in one step. In operand mode one, regB is AND-ed to a zero value and stored in the result reg. This is equivalent to a clear word instruction. A pseudo instruction for this could be a "CLR reg". Also, negating the result could be an efficient way to set all bits in a word to one. Finally, the
AND instrcution can also serve as a NOP instruction. AND.C R0, #0 will "AND" R0 with R0 and an immediate with all bits set. 

<!--------------------------------------------------------------------------------------------------------->

<div style="page-break-before: always;"></div>

### OR

<hr>

Performs a bitwise OR of the operand and the target register and stores the result into the target register.

#### Format

```
      OR [ .<opt> ] <GR r>, <val>                      ; opMode 0
      OR [ .<opt> ] <GR r>, <GR b>                     ; opMode 2
      OR [ .<opt> ] <GR r>, <GR a>, <GR b>             ; opMode 2
      OR [ .<opt> ] <GR r>, <ofs> ( <SR a>, <GR b> )   ; opMode 3
      OR [ .<opt> ] <GR r>, <ofs> ( GR y )             ; opMode 4 .. 15, "y" -> GR4 .. GR7
```

```
       0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31
      :--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:
      : OR    ( 004 )   : r      :N :C :0 : opMode    : opArg                                         :
      :-----------------:-----------------------------------------------------------------------------:
```

#### Description

The instruction fetches the data specified by the operand and performs a bitwise OR of the general register "r" and the operand fetched. The result is stored in general register "r". The N bit allows to negate the result making the OR a NOR operation. 

There are four major mode groups for this instruction. Operand mode 0 ORs a sign extended constant into "r". Operand mode 1 ORs a zero value to the "b" content and stores in the result reg "r". The "a" field should be set to zero. Operand mode 2 allows to specify two general register to be the input operands, independent of the target general register in "r". This way, a three register address operation of the form "r" = "a" op "b" as well as a "r" = "r" op "b" can be done. 

Mode 4 - 7, 8 - 11 and 12 - 16 compute the address by adding the signed offset to an index registers. Mode 4, 8 and 12 refer to index register GR4, mode 5, 9, 13 to index register GR5 and so on. The upper two bits of the selected index register are used to select from the segment registers SR 4 to 7. Mode 4 - 7 will fetch a word from the computed address, mode 8 - 11 fetch a half-word and mode 12 - 15 a byte. Mode 3 is the extended address mode. The "a" field contains the segment register, and "b" the unsigned offset. 

#### Operation

```
      switch( opMode ) {

         case 0:  tmpA <- GR[r];
	              tmpB <- signExt( opArg, 16 );
		          break;

         case 1:  tmpA <- 0;
		          tmpB <- GR[b];
			      break;

         case 2:  tmpA <- GR[a];
		          tmpB <- GR[b];
			      break;

         default: seg <- operandAdrSeg( instr );
                  ofs <- operandAdrOfs( instr );
                  len <- operandBitLen( instr );

                  tmpA <- GR[r];
                  tmpB <- zeroExtend( memLoad( seg, ofs, len ), 32 - len );
      }

      if ( instr.[C]) tmpB <- ~ tmpB;
      res <- tmpA | tmpB;

      if ( instr.[N]) res <- ~res;
      GR[r] <- res;
```

#### Exceptions

- DTLB miss/data page fault
- Data memory access rights trap
- Data memory protection Id trap
- Page reference trap
- Alignment trap

#### Notes

Using operand mode one will result in OR-ing a zero with general register "b", which is a copy of general register "b" to the general register "r". A pseudo opCode instruction could be is "CPY regR, regB".


<!--------------------------------------------------------------------------------------------------------->

<div style="page-break-before: always;"></div>

### XOR

<hr>

Performs a bitwise XORing the operand and the target register and stores the result into the target register.

#### Format

```
      XOR [ .<opt> ] <GR r>, <val>                      ; opMode 0
      XOR [ .<opt> ] <GR r>, <GR b>                     ; opMode 2
      XOR [ .<opt> ] <GR r>, <GR a>, <GR b>             ; opMode 2
      XOR [ .<opt> ] <GR r>, <ofs> ( <SR a>, <GR b> )   ; opMode 3
      XOR [ .<opt> ] <GR r>, <ofs> ( GR y )             ; opMode 4 .. 15, "y" -> GR4 .. GR7
```

```
       0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31
      :--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:
      : XOR   ( 0xx )   : r      :N :C :0 : opMode    : opArg                                         :
      :-----------------:-----------------------------------------------------------------------------:
```

#### Description

The instruction fetches the data specified by the operand and performs a bitwise XOR of general register "r" and the operand fetched. The result is stored in general register "r". The N bit allows to negate the result making the XOR a XNOR operation The C bit allows to complement ( 1's complement ) the operand input, which is for operand mode one and two general register "b". 

There are four major mode groups for this instruction. Operand mode 0 XORs a sign extended constant into "r". Operand mode 1 XORs a zero value to the "b" content and stores in the result reg "r". The "a" field should be set to zero. Operand mode 2 allows to specify two general register to be the input operands, independent of the target general register in "r". This way, a three register address operation of the form "r" = "a" op "b" as well as a "r" = "r" op "b" can be done. 

Mode 4 - 7, 8 - 11 and 12 - 16 compute the address by adding the signed offset to an index registers. Mode 4, 8 and 12 refer to index register GR4, mode 5, 9, 13 to index register GR5 and so on. The upper two bits of the selected index register are used to select from the segment registers SR 4 to 7. Mode 4 - 7 will fetch a word from the computed address, mode 8 - 11 fetch a half-word and mode 12 - 15 a byte. Mode 3 is the extended address mode. The "a" field contains the segment register, and "b" the unsigned offset. 

#### Operation

```
      switch( opMode ) {

         case 0:  tmpA <- GR[r];
	              tmpB <- signExt( opArg, 16 );
		          break;

         case 1:  tmpA <- 0;
		          tmpB <- GR[b];
			      break;

         case 2:  tmpA <- GR[a];
		          tmpB <- GR[b];
			      break;

         default: seg <- operandAdrSeg( instr );
                  ofs <- operandAdrOfs( instr );
                  len <- operandBitLen( instr );

                  tmpA <- GR[r];
                  tmpB <- zeroExtend( memLoad( seg, ofs, len ), 32 - len );
      }

      if ( instr.[C]) tmpB <- ~ tmpB;
      res <- tmpA ^ tmpB;
      if ( instr.[N]) res <- ~res;
      GR[r] <- res;
```

#### Exceptions

- DTLB miss/data page fault
- Data memory access rights trap
- Data memory protection Id trap
- Page reference trap
- Alignemtn trap

#### Notes

The OR instrcution can also serve as a NOP instruction. OR R0, #0 will "OR" R0 with R0 and an immediate with all bits cleared. 

None.

<!--------------------------------------------------------------------------------------------------------->

<div style="page-break-before: always;"></div>

### CMP

<hr>

Compares a register and an operand and stores the comparison result in the target register.

#### Format


```
      CMP [ .<opt> ] <GR r>, <val>                      ; opMode 0
      CMP [ .<opt> ] <GR r>, <GR b>                     ; opMode 2
      CMP [ .<opt> ] <GR r>, <GR a>, <GR b>             ; opMode 2
      CMP [ .<opt> ] <GR r>, <ofs> ( <SR a>, <GR b> )   ; opMode 3
      CMO [ .<opt> ] <GR r>, <ofs> ( GR y )             ; opMode 4 .. 15, "y" -> GR4 .. GR7
```

```
       0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31
      :--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:
      : CMP   ( 0xx )   : r      : cond   : opMode    : opArg                                         :
      :-----------------:-----------------------------------------------------------------------------:
```

#### Description

The compare instruction will compare two operands for the condition specified in the "cond" field. A value of one is stored in "r" when the condition is met, otherwise a zero value is stored. A typical code pattern would be to issue the compare instruction with the comparison result in general register "r", followed by a conditional branch instruction. 

There are four major mode groups for this instruction. Operand mode 0 compares a sign extended constant into "r". Operand mode 1 compares a zero value to the "b" content and stores in the result reg "r". The "a" field should be set to zero. Operand mode 2 allows to compare two general register to be the input operands, independent of the target general register in "r". This way, a three register address operation of the form "r" = "a" op "b" as well as a "r" = "r" op "b" can be done. 

Mode 4 - 7, 8 - 11 and 12 - 16 compute the address of the "b" operand by adding the signed offset to an index registers. Mode 4, 8 and 12 refer to index register GR4, mode 5, 9, 13 to index register GR5 and so on. The upper two bits of the selected index register are used to select from the segment registers SR 4 to 7. Mode 4 - 7 will fetch a word from the computed address, mode 8 - 11 fetch a half-word and mode 12 - 15 a byte. Mode 3 is the extended address mode. The "a" field contains the segment register, and "b" the unsigned offset. 

#### Comparison condition

The compare condition are encoded as follows.

```
      0 : EQ ( r == operand )                               4 : NE ( r != operand )
      1 : LT ( r <  operand, Signed )                       5 : LE ( r <= operand, Signed )
      2 : GT ( r >  operand, Signed )                       6 : GE ( r >= operand, Signed )
      3 : LS ( r <= operand, Unsigned )                     7 : HI ( r >  operand, Unsigned )
```

#### Operation

```
      switch( opMode ) {

         case 0:  tmpA <- GR[r];
	              tmpB <- signExt( opArg, 16 );
		          break;

         case 1:  tmpA <- 0;
		          tmpB <- GR[b];
			      break;

         case 2:  tmpA <- GR[a];
		          tmpB <- GR[b];
			      break;

         default: seg <- operandAdrSeg( instr );
                  ofs <- operandAdrOfs( instr );
                  len <- operandBitLen( instr );

                  tmpA <- GR[r];
                  tmpB <- zeroExtend( memLoad( seg, ofs, len ), 32 - len );
      }

      switch ( cond ) {

         case 0: res <- tmpA ==  tmpB; break;
         case 1: res <- tmpA <   tmpB; break;
         case 2: res <- tmpA >   tmpB; break;
         case 3: res <- tmpA <<= tmpB; break;
         case 4: res <- tmpA !=  tmpB; break;
         case 5: res <- tmpA <=  tmpB; break;
         case 6: res <- tmpA >=  tmpB; break;
         case 7: res <- tmpA >>  tmpB; break;
      }

      GR[r] <- res;
```

#### Exceptions

- DTLB miss/data page fault
- Data memory access rights trap
- Data memory protection Id trap
- Page reference trap
- Alignment trap

#### Notes

None.


<!--------------------------------------------------------------------------------------------------------->

<div style="page-break-before: always;"></div>

### CMR

<hr>

Test a general register for a condition and conditionally move a register value or immediate value to the target register.

#### Format

```
      CMR [ .<cond> ] <GR r>, <GR b>, <GR a>
      CMR [ .<cond> ] <GR r>, <GR b, <val>>
```

```
       0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31
      :--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:
      : CMR   ( 0xx )   : r      : cond   :I : val                                  : a      : b      :
      :-----------------:-----------------------------------------------------------------------------:
```

#### Description

The conditional move instruction will test register "b" for the condition. If the condition is met, general register "a" is moved to "r". The "I" bit indicates to use an immediate value constructed from the "val" field concatenated with the "a" field of the instruction instead of the contents of register "a".


#### Comparison condition codes

```
      0 : EQ ( b == 0 )                               4 : NE  ( b != 0 )
      1 : LT ( b < 0, Signed )                        5 : LE ( b <= 0, Signed )
      2 : GT ( b > 0, Signed )                        6 : GE ( b >= 0, Signed )
      3 : EV ( even( b ))                             7 : OD ( odd( b ))
```

#### Operation

```
      if ( instr[I] ) tmp <- signExt( catImm( val1, a ), 16 );
      else tmp <- GR[a];

      switch( cond ) {

         case 0: res <- ( GR[b] == 0 );   break;
         case 1: res <- ( GR[b] > 0 );    break;
         case 2: res <- ( GR[b] < 0 );    break;
         case 3: res <- ( ~ GR[b].[31] ); break;
         case 4: res <- ( GR[b] != 0 );   break;
         case 5: res <- ( GR[b] <= 0 );   break;
         case 6: res <- ( GR[b] >= 0 );   break;
         case 7: res <- ( GR[b].[31] );   break;
      }

      if ( res ) GR[r] <- GR[a];
```

#### Exceptions

None.

#### Notes

In combination with the CMP instruction, simple instruction sequences such as "if ( a < b ) c = d" could be realized without pipeline unfriendly branch instructions.

```
Example:

   C-code:     if ( a < b ) c = d

   Assumption: ( R2 -> a, R6 -> b, R1 -> c, R4 -> d )

   CMP.LT R1,R2,R6  ; compare R2 < R6 and store a zero or one in R1
   CMR    R1,R4,R1  ; test R1 and store R4 when condition is met
```

The CMR instruction was a rather specialized instruction. It highly depends on a good peephole optimizer to detect such a situation.

<!--------------------------------------------------------------------------------------------------------->

<div style="page-break-before: always;"></div>

### EXTR

<hr>

Performs a bit field extract from a general register and stores the result in the targetReg.

#### Format

```
      EXTR [ .<opt> ] <GR r>, <GR b>, <pos, <len>
```

```
       0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31
      :--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:
      : EXTR  ( 0xx )   : r      :S :A : 0            : len          : pos          : 0      : b      :
      :-----------------:-----------------------------------------------------------------------------:
```

#### Description

The instruction performs a bit field extract specified by the position and length instruction data from general register "b". The "pos" field specifies the rightmost bit of the bitfield to extract. The "len" field specifies the bit size if the field to extract. The extracted bit field is stored right justified in the general register "r". If set, the "S" bit allows to sign extend the extracted bit field. If the "A" bit is set, the shift amount control register is used for obtaining the position value.

#### Operation

```
      if ( instr.[A] ) tmpPos <- SHAMT.[27..31];
      else tmpPos <- pos;

      GR[r] <= extract( GR[b], pos, len );

      if ( instr.[S] ) signExtend( GR[r], len );
      else zeroExtend( GR[r], len );
```

#### Exceptions

- illegal instruction trap.

#### Notes

The VCPU-32 instruction set does not have dedicated instructions for left and right shift operations. They can easily be realized with the EXTR and DEP instructions. Refer to the chapter for pseudo instructions.


<!--------------------------------------------------------------------------------------------------------->

<div style="page-break-before: always;"></div>

### DEP

<hr>

Performs a bit field deposit of the value extracted from a bit field in reg "B" and stores the result in the targetReg.

#### Format

```
      DEP [ .<opt> ] <GR r>, <GR b>, <pos, <len>
      DEP [ .<opt> ] <GR r>, <val>, <pos>, <len>
```

```
       0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31
      :--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:
      : DEP  ( 0xx )    : r      :Z :A :I : 0         : len          : pos          : 0      : b      :
      :-----------------:-----------------------------------------------------------------------------:
```

#### Description

The instruction extracts the right justified bit field of length "len" in general register "b" and deposits this field in the general register "r" at the specified position. The "pos" field specifies the rightmost bit for the bit field to deposit. The "len" field specifies the bit size if the field to extract. The extracted bit field is stored right justified in the general register "r". The Z bit clears the target register "r" before storing the bit field, the I bit specifies that the instruction bits 26..31 contain an immediate value instead of a register. If the "A" bit is set, the shift amount control register is used for obtaining the position value.

#### Operation

```
      if ( instr.[A] ) tmpPos <- SHAMT.[27..31];
      else tmpPos <- pos;

      if ( instr.[Z] ) GR[r] <- 0;

      if ( instr.[I] ) tmpB = instr.[26..31];
      else tmpB <- GR[b];

      deposit( GR[r], tmpB, pos, len );
```

#### Exceptions

Specifying an invalid value in the "len" or "pos" field results in an illegal instruction trap.

#### Notes

The VCPU-32 instruction set does not have dedicated instructions for left and right shift operations. They can easily be realized with the EXTR and DEP instructions. Refer to the chapter for pseudo instructions.


<!--------------------------------------------------------------------------------------------------------->

<div style="page-break-before: always;"></div>

### DSR

<hr>

Performs a right shift of two concatenated registers for shift amount bits and stores the result in the target register.

#### Format

```
      DSR [ .<opt> ] <GR r>, <GR b>, <GR a>, <shAmt>
```

```
       0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31
      :--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:
      : DSR   ( 0xx )   : r      :  :A : 0            : shamt        : 0            : a      : b      :
      :-----------------:-----------------------------------------------------------------------------:
```

#### Description

The double shift right instruction concatenates the general registers specified by "b" and "a" and performs a right shift operation of "shamt" bits. register "b" is the left part, register "a" the right part. The lower 32 bits of the result are stored in the general register "r". The "shamt" field range is from 0 to 31 for the shift amount. If the "A" bit is set, the shift amount is taken from the shift amount control register.

#### Operation

```
      if ( instr.[A] ) tmpShAmt <- SHAMT.[27..31];
      else tmpShAmt <- shamt;

      GR[r] <- rshift( cat( GR[b], GR[a] ), shamt );
```

#### Exceptions

- illegal instruction trap.

#### Notes

The VCPU-32 instruction set does not have dedicated instructions for shift and rotate operations. They can easily be realized with the DSR instructions. Refer to the chapter for pseudo instructions.


<!--------------------------------------------------------------------------------------------------------->

<div style="page-break-before: always;"></div>

### SHLA

<hr>

Performs a combined shift left and add operation and stores the result into the target register.

#### Format

```
      SHLA [ .<opt> ] <GR r>, <GR a>, <GR b>, <smAmt>
      SHLA [ .<opt> ] <GR r>, <GR a>, <val>, <smAmt>
```

```
       0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31
      :--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:
      : SHLA  ( 0xx )   : r      :Z :L :O : 0         : sa  : 0                     : a      : b      :
      :-----------------:-----------------------------------------------------------------------------:
```

#### Description

The shift left and add instruction will shift general register "a" left by the bits specified in the "sa" field, add the content of general register "b" to it and store the result in general register "r". This combined operation allows for an efficient multiplication operation for multiplying a value with a small integer value. The "Z" bit, when set, uses a value of zero instead of the "b" register content. The "L" bit indicates that this is an operation on unsigned quantities. The "O" bit is set to raise a trap if the instruction either shifts beyond the number range of general register "r" or when the addition of the shifted general register "a" plus the value in general register "b" results in an overflow.

#### Operation

```

   if( instr.[I] ) tmpB <- instr.[21..23];
   else tmpB <- GR[b];

   GR[r] <- ( GR[a] << sa ) + GR[b];

   if ( instr.[O] ) && ( ovl ) overflowTrap( );
```

#### Exceptions

- Overflow trap

#### Notes

None.


<!--------------------------------------------------------------------------------------------------------->

<div style="page-break-before: always;"></div>

### LDI

<hr>

Loads an immediate value into the target register.

#### Format

```
      LDI [ .<opt> ] <GR r>, <val>
```

```
       0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31
      :--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:
      : LDI   ( 0xx )   : r      :Z :L :0             : val                                           :
      :-----------------:-----------------------------------------------------------------------------:
```

#### Description

The load immediate instruction loads a 16-bit immediate value embedded in the instruction into the general register "r". The "Z" bit specifies to clear the target register first. The "L" bit specifies that the value is to be stored into the upper half of the target register without affecting the lower half of the target register. Using the "L" option, two LDI instructions can thus set any immediate value.

#### Operation

```
      if ( instr.[Z] GR[r] = 0;
      tmpA = catImm( val1, val2 );

      if ( instr.[L] ) tmpB =  GR[r] << 16;
      else             tmpB = GR[r] & 0xFFFF;

      GR[r] = tmpA | tmpB;
```

#### Exceptions

None.

#### Notes

None.


<!--------------------------------------------------------------------------------------------------------->

<div style="page-break-before: always;"></div>

### LEA

<hr>

Loads the effective address of the operand.

#### Format

```
      LDE <GR r>, <GR a>( GR<b> )            ; opMode 0 .. 2
      LEA <GR r>, <ofs> ( <SR a>, <GR b> )   ; opMode 3
      LEA <GR r>, <ofs> ( GR y )             ; opMode 4 .. 15, "y" -> GR4 .. GR7
```

```
       0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31
      :--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:
      : LEA    ( 0xx )  : r      : 0      : opMode    :     opArg                                     :
      :-----------------:-----------------------------------------------------------------------------:
```

#### Description

The LEA instruction loads the computed address offset into general register "r". It just performs the address computation part comparable to the LDx instruction, however with no memory access.


#### Operation

```
      GR[ r ] <- operandAdrOfs( instr );
```

#### Exceptions

- Illegal Instruction Trap

#### Notes

None.


<!--------------------------------------------------------------------------------------------------------->

<div style="page-break-before: always;"></div>

### LSID

<hr>

Load the segment identifier for a logical address.

#### Format

```
      LSID <GR r>, <GR b>
```

```
       0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31
      :--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:
      : LSID ( 0xx )    : r      : 0                                                         : b      :
      :-----------------:-----------------------------------------------------------------------------:
```

#### Description

The LSID instruction returns the segment identifier of the segment encoded in the logical address in general register "b".

#### Operation

```
      GR[r] <- SR[ segSelect( GR[b] ) ];
```

#### Exceptions

None.

#### Notes

None.


<!--------------------------------------------------------------------------------------------------------->

<div style="page-break-before: always;"></div>

## System Control Instructions

The system control instruction group contains instruction used to manage CPU hardware components such as the TLB als well as instructions to manage the virtual memory subsystem. Most if the instructions found in this group require that the processor runs in privileged mode.

The **MR** instructions is used to transfer data to and from a segment register or a control register. The processor status instruction **MST** allows for setting or clearing an individual an accessible bit of the status word.

The **LDPA** and **PRB** instructions are used to obtain information about the virtual memory system. The LDPA instruction returns the physical address of the virtual page, if present in memory. The PRB instruction tests whether the desired access mode to an address is allowed.

The TLB and Caches are managed by the **ITLB**, **PTLB** and **PCA** instruction. The ITLB and PTLB instructions insert into the TLB and removes translations from the TLB. The PCA instruction manages the instruction and data cache and features to flush and / or just purge a cache entry. There is no instruction that inserts data into a cache as this is completely controlled by hardware.

The **RFI** instruction is used to restore a processor state from the control registers holding the interrupted instruction address, the instruction itself and the processor status word. Optionally, general registers that have been stored into the reserved control registers will be restored. The **BRK** instruction is transferring control to the breakpoint trap handler.

The **DIAG** instruction is a control instructions to issue hardware specific implementation commands. Example is the memory barrier command, which ensures that all memory access operations are completed after the execution of this DIAG instruction.

The remainder of this chapter contains the detailed description for the system control instructions. An instruction description starts with the opCode mnemonic and the instruction word structure. Next a short description of the instruction and its purpose are given. Any options and bit specific bit fields as well as exceptions that can be raised by the instruction are given next. Finally, any further notes are presented.


<!--------------------------------------------------------------------------------------------------------->

<div style="page-break-before: always;"></div>

### MR

<hr>

Copy data from or to a segment or control register.

#### Format

```
      MR <targetReg>, <sourceReg>
```

```
       0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31
      :--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:
      : MR    ( 0xx )   : r      :Z :D :M : 0                                       : s               :
      :-----------------:-----------------------------------------------------------------------------:
```

#### Description

The move register instruction MR copies data from a segment or control register "s" to a general register "r" and vice versa. If the "D" bit is set, the copy direction is from "r" to a segment or control register, otherwise from these registers to "r". The "M" bit indicates whether a segment register or a control register is moved. The "Z" option uses a zero value for the source. Setting a value for a privileged segment or control register is a privileged operation.

#### Operation

```
      if ( instr.[D] ) {

         if ( instr.[Z] ) {

            if ( instr.[M] ) CR[ instr.[27..31]] <- 0;
            else             SR[ instr.[29..31]] <- 0;
         
         } else { 

            if ( instr.[M] ) GR[ instr.[27..31]] <- GR[r];
            else             SR[ instr.[29..31]] <- GR[r];
         }
      }
      else {

         if ( instr.[Z] ) {

            if ( instr.[M] ) GR[r] <- 0;
            else             GR[r] <- 0;

         } else {

            if ( instr.[M] ) GR[r] <- CR[ instr.[27..31]];
            else             GR[r] <- SR[ instr.[29..31]];
         }
      }
```

#### Exceptions

- privileged operation trap.

#### Notes

None.


<!--------------------------------------------------------------------------------------------------------->

<div style="page-break-before: always;"></div>

### MST

<hr>

Set and clears bits in the processor status word.

#### Format

```
      MST <GR b>
      MST.S <val>
      MST.C >val>
```

```
       0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31
      :--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:
      : MST   ( 0xx )   : r      :mode : 0                                          : val / b         :
      :-----------------:-----------------------------------------------------------------------------:
```

#### Description

The MST instruction sets the status bits 26.. 31 of the processor status word. The previous bit settings are returned in "r". There are three modes. Mode 0 will replace the user status bits 26..31 with the bits in the general register "b". Mode 1 and 2 will interpret the 6-bit field bits 26..31 as the bits to set or clear. Modifying the status register is a privileged instruction. The "mode" field encodes the options as shown below. The options "S" and "C" are mapped to mode one and two, mode zero is the register usage case. MST is a privileged operation.

#### Mode

```
      0 - copy status bits using general register "b" ( bits 26..31 )
      1 - set status bits using the bits in "val" ( bits 26..31 )
      2 - clears status bits using the bits in "val" ( bits 26..31 )
      3 - undefined.
```

#### Operation

```
      if ( ! ST.[ PRIV ] ) privilegedOperationTrap( );

      GR[r] <- cat( 0.[0..25], ST.[26..31] );

      switch( mode ) {

         case 0: {

	      ST.[26..31] <- GR[b].[26..31];

         } break;

         case 1: {

            if ( instr.[26] ) ST.[26] <- 1;
            if ( instr.[27] ) ST.[27] <- 1;
            if ( instr.[28] ) ST.[28] <- 1;
            if ( instr.[29] ) ST.[29] <- 1;
            if ( instr.[30] ) ST.[30] <- 1;
            if ( instr.[31] ) ST.[31] <- 1;

         } break;

         case 2: {

            if ( instr.[26] ) ST.[26] <- 0;
            if ( instr.[27] ) ST.[27] <- 0;
            if ( instr.[28] ) ST.[28] <- 0;
            if ( instr.[29] ) ST.[29] <- 0;
            if ( instr.[30] ) ST.[30] <- 0;
            if ( instr.[31] ) ST.[31] <- 0;

	    } break;

         default: illegalInstructionTrap( );
      }
```

#### Exceptions

- privileged operation trap

#### Notes

None.


<!--------------------------------------------------------------------------------------------------------->

<div style="page-break-before: always;"></div>

### LDPA

<hr>

Load the physical address for a virtual address.

#### Format

```
      LDPA <GR r>, ( <GR b> )
      LDPA <GR r>, ( <SR a>, <GR b> )
```

```
       0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31
      :--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:
      : LDPA  ( 0xx )   : r      : L : 0                                            : a      : b      :
      :-----------------:-----------------------------------------------------------------------------:
```

#### Description

The LDPA instruction returns the physical address for a logical or virtual address encoded in "a" and "b" into register "r". The "L" bit will specify whether the address is a full virtual address with the space in "a" and the offset in "b". If the "L" bit is set, the address is a logical address in "b" and the segment is selected by the 2 upper bits of the logical address forming the full virtual address. If the virtual address is not in main memory, a zero result is returned. The result is ambiguous for a virtual address that translates to a zero address. LDPA is a privileged operation.

#### Operation

```
      if ( ! ST.[ PRIV ] ) privilegedOperationTrap( );

      if ( intr.[L] ) {

         seg <- SR[ segSelect( GR[b] )];
         ofs <- ofsSelect( GR[b] );
      
      } else {

         seg <- SR[a];
         ofs <- GR[b];
      }

      gr[r] <- loadPhysAdr( seg, ofs );
```

#### Exceptions

- privileged operation trap
- non-access data TLB miss / page fault

#### Notes

None.


<!--------------------------------------------------------------------------------------------------------->

<div style="page-break-before: always;"></div>

### PRB

<hr>

Probe data access to a virtual address.

#### Format

```
      PRB <opt> <GR r>, ( <GR b> )
      PRB <opt> <GR r>, ( <SR a>, <GR b> )
```

```
       0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31
      :--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:
      : PRB   ( 0xx )   : r      :L :M : 0                                          : a      : b      :
      :-----------------:-----------------------------------------------------------------------------:
```

#### Description

The PRB instruction determines whether data access at the requested privilege level stored in the general register "r" is allowed. If the probe operation succeeds, a value of one is returned in GR "r", otherwise a zero is returned. The "M" bit specifies whether a read or write access is requested. A value of zero is a read access, a value of one a read/write access. The "L" bit will specify whether the address is a full virtual address with the space in "a" and the offset in "b". If the "L" bit is set, the address is a logical address in "b" and the segment is selected by the 3 upper bits of the logical address forming the full virtual address. If the protection ID is enabled in the processor status word, the protection ID is checked as well. The instruction performs the necessary virtual to physical data translation regardless of the processor status bit for data translation.

#### Operation

```
      if ( intr.[L] ) {

         seg <- SR[ segSelect( GR[b] )];
         ofs <- ofsSelect( GR[b] );
      
      } else {

         seg <- SR[a];
         ofs <- GR[b];
      }

      if ( ! searchDataTlbEntry( seg, ofs, &entry )) {

         if      ((   instr.[M] ) && ( writeAccessAllowed( entry, instr.[P] )))    GR[r] <- 1;
         else if (( ! instr.[M] ) && ( readAccessAllowed( entry, instr.[P] )))     GR[r] <- 1;
         else                                                                      GR[r] <- 0;
      
      } else nonAccessDataTlbMiss( );
```

#### Exceptions

- non-access data TLB miss trap

#### Notes

None.


<!--------------------------------------------------------------------------------------------------------->

<div style="page-break-before: always;"></div>

### ITLB

<hr>

Inserts a translation into the instruction or data TLB.

#### Format

```
      ITLB <opt> <GR r>, ( <GR b> )
      ITLB <opt> <GR r>, ( <SR a>, <GR b> )
```

```
       0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31
      :--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:
      : ITLB  ( 0xx )   : r      :L : T :M : 0                                      : a      : b      :
      :-----------------:-----------------------------------------------------------------------------:
```

#### Description

The ITLB instruction inserts a translation into the instruction or data TLB. The data to be inserted is grouped into two steps. The first step will enter the virtual address and physical address is associated with. The second step will enter the access rights and protection information and marks the entry valid. The TLB entry is set to invalid and the address fields are filled. The "T" bit specifies whether the instruction or the data TLB is addressed. A value of zero references the instruction TLB, a value of one refers to the data TLB. The "L" bit will specify whether the address is a full virtual address with the space in "a" and the offset in "b". If the "L" bit is set, the address is a logical address in "b" and the segment is selected by the 2 upper bits of the logical address forming the full virtual address.

The "r" register contains either the physical address or the access rights and protection information. The "M" bit indicates which part of the TLB insert operation is being requested. A value of zero refers to part one, which will enter the address information, a value of one will enter the access rights and protection information. The sequence should be to first issue the ITLB.A instruction for the address part, which still leaves the TLB entry marked invalid. Issuing the second part, the ITLB.P instruction, which sets the access rights and protection information, will mark the entry valid after the operation.

#### Argument Word Layout

Depending on the "T" bit, the protection information part or the address part is passed to the TLB subsystem. The individual bits are described in the architecture chapter TLB section.

```
       0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31
      :--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:
      : 0         : CPU-ID    : Bank      : PPN                                                       :      T = 0
      :-----------------------------------------------------------------------------------------------:

	 0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31
      :--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:
      :U :T :D :B : AR        : 0                     : Protect-Id                                    :      T = 1
      :-----------------------------------------------------------------------------------------------:
```


#### Operation

```
      if ( ! ST.[ PRIV ] ) privilegedOperationTrap( );

      if ( intr.[L] ) {

         seg <- SR[ segSelect( GR[b] )];
         ofs <- ofsSelect( GR[b] );
      
      } else {

         seg <- SR[a];
         ofs <- GR[b];
      }

      if ( instr.[T] )
         if ( ! searchDataTlbEntry( seg, ofs, &entry )) allocateDataTlbEntry( seg, ofs, &entry );
      else
         if ( ! searchInstructionTlbEntry( seg, ofs, &entry )) allocateInstructionTlbEntry( seg, ofs, &entry );

      if ( instr.[M] ) {

         entry.[addressPart] <- GR[r];
         entry.[V] <- false;
	  
      } else {

         entry.[protectPart] <- GR[r];
         entry.[V] <- true;
      }

```

#### Exceptions

- Privileged operation trap

#### Notes

None.


<!--------------------------------------------------------------------------------------------------------->

<div style="page-break-before: always;"></div>

### PTLB

<hr>

Removes a translation entry from the TLB.

#### Format

```
      PTLB <opt> ( <GR b> )
      PTLB <opt> ( <SR a>, <GR b> )
```

```
       0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31
      :--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:
      : PTLB  ( 0xx )   : 0      :L :T : 0                                          : a      : b      :
      :-----------------:-----------------------------------------------------------------------------:
```

#### Description

The DTLB instruction removes a translation from the instruction or data TLB by marking the entry invalid. The "T" bit indicates whether the instruction or the data TLB is addressed. A value of zero references the instruction TLB. The "L" bit will specify whether the address is a full virtual address with the space in "a" and the offset in "b". If the "L" bit is set, the address is a logical address in "b" and the segment is selected by the 2 upper bits of the logical address forming the virtual address. Otherwise it is a virtual address with segment "a" and "offset "b".

#### Operation

```
     if ( ! ST.[ PRIV ] ) privilegedOperationTrap( );

      if ( intr.[L] ) {

         seg <- SR[ segSelect( GR[b] )];
         ofs <- ofsSelect( GR[b] );
      
      } else {

         seg <- SR[a];
         ofs <- GR[b];
      }

      if ( instr.[T] )
         if ( searchDataTlbEntry( seg, ofs, &entry )) purgeDataTlbEntry( seg, ofs, &entry );
      else
         if ( searchInstructionTlbEntry( seg, ofs, &entry )) purgeInstructionTlbEntry( seg, ofs, &entry );
```

#### Exceptions

- privileged operation trap

#### Notes

None.


<!--------------------------------------------------------------------------------------------------------->

<div style="page-break-before: always;"></div>

### PCA

<hr>

Flush and / or remove cache lines from the cache.

#### Format

```
      PCA <opt> ( <GR b>, )
      PCA <opt> ( <SR a>, <GR b> )
```

```
       0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31
      :--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:
      : PCA   ( 0xx )   : 0      :L :T :F : 0                                       : a      : b      :
      :-----------------:-----------------------------------------------------------------------------:
```

#### Description

The PCA instruction flushes or purges a cache line from the instruction or data cache. An instruction cache line can only be purged, a data cache line can also be written back to memory and then optionally purged. A flush operation is only performed when the cache line is dirty. The "T" bit indicates whether the instruction or the data cache is addressed. A value of zero references the instruction cache. The "L" bit will specify whether the address is a full virtual address with the space in "a" and the offset in "b". If the bit is set, the address is a logical address in "b" and the segment is selected by the 2 upper bits of the logical address forming the virtual address with segment "a" and "offset "b". The "F" bit will indicate whether the data cache is to be purged without flushing it first to memory. If "F" is zero, the entry is first flushed and then purged, else just purged. The "F" bit has no meaning for an instruction cache.

#### Operation

```
      if ( instr.[L] ) {

         seg <- SR[ segSelect( GR[b] )];
         ofs <- ofsSelect( GR[b] );
      
      } else {

         seg <- SR[a];
         ofs <- GR[b]|;
      }

      if ( instr.[T] ) {

         if ( ! instr.[F] ) flushDataCache( seg, ofs );
         purgeDataCache( seg, ofs );
      }
      else purgeInstructionCache( seg, ofs );
```

#### Exceptions

- privileged operation trap (?)
- Non-access ITLB trap
- Non-access DTLB trap

#### Notes

None.


<!--------------------------------------------------------------------------------------------------------->

<div style="page-break-before: always;"></div>

### RFI

<hr>

Restore the processor state and restart execution.

#### Format

```
      RFI
```

```
       0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31
      :--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:
      : RFI   ( 0xx )   : 0                                                                           :
      :-----------------:-----------------------------------------------------------------------------:
```

#### Description

The RFI instruction restores the instruction address segment, instruction address offset and the processor status register from the control registers I-IA-SEG, I-IA-OFS and I-STAT.

// ??? **note** perhaps an option to also restore some shadow regs ? ( GR and SRs ? )
// ??? **note** what bits to restore in the status word from the I-STAT ? Bits 26 .. 31 ?

#### Operation

```
      if ( ! ST.[ PRIV ] ) privilegedOperationTrap( );

      IA_SEG <- I-IA-SEG;
      IA-OFS <- I-IA-OFS;
      ST     <- I-STAT;
```

#### Exceptions

- privileged operation trap

#### Notes

The RFI instruction is also used to perform a context switch. Changing from one thread to another is accomplished by loading the control registers **I-IA-SEG**, **I-IA-OFS** and **I-STAT** and then issue the RFI instruction. Setting bits other than the system mask is generally accomplished by constructing the respective status word and then issuing an RFI instruction to set them.


<!--------------------------------------------------------------------------------------------------------->

<div style="page-break-before: always;"></div>

### DIAG

<hr>

Issues commands to hardware specific components and implementation features.

#### Format

```
      DIAG
```

```
       0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31
      :--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:
      : DIAG   ( 076 )  : 0                                                                           :
      :-----------------:-----------------------------------------------------------------------------:
```

#### Description

The DIAG instruction sends a command to an implementation hardware specific components.

// ??? **note** under construction...

#### Operation

```
      if ( ! ST.[ PRIV ] ) privilegedOperationTrap( );

     ...
```

#### Exceptions

- privileged operation trap

#### Notes

None.


<!--------------------------------------------------------------------------------------------------------->

<div style="page-break-before: always;"></div>

### BRK

<hr>

Trap to the debugger subsystem.

#### Format

```
      BRK
```

```
       0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31
      :--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:
      : BRK   ( 0xx )   :  0                          : info                                          :
      :-----------------:-----------------------------------------------------------------------------:
```

#### Description

The BRK instruction raises a debug breakpoint trap and enters the debug trap handler. The "info" field is a 16-bit value to pass to the debug subystem.

// ??? **note** explain what you can actually debug ...

#### Operation

```
      debugBreakpointTrap( );
```

#### Exceptions

- debug breakpoint trap

#### Notes

The instruction opCode for BRK is the opCode value of zero. A zero instruction word result is a BRK #0 instrcution which raises a trap.

None.


<!--------------------------------------------------------------------------------------------------------->

<div style="page-break-before: always;"></div>

## Pseudo Instructions

The instruction set allows for a rich set of options on the individual instruction functions. Setting a defined option bit in the instruction adds useful capabilities to an instruction with little additional overhead to the overall data path. For better readability, pseudo operations could be defined that allows for an easier usage. Furthermore, some instructions have no assembler format counterpart. The only way to execute them is through the pseudo operation. This applies for example to the operand mode one case in computational instructions. For pseudo instructions, options and traps are those specified by the actually used instructions.

The pseudo instructions presented in this chapter are listed with their assembler mnemonic, a short description of their operation and the actual VCPU-32 instruction used for implementation. Within this chapter, the pseudo instructions are just called instructions too.

<!--------------------------------------------------------------------------------------------------------->

<div style="page-break-before: always;"></div>

### NOP

<hr>

A no operation.

#### Format

```
      NOP
```

#### Description

The NOP pseudo instruction is just a no operation. The assembler uses either the AND or the OR instruction with opMode zero. The result is always "r".

```
      NOP    ->    AND.C GR0, #0
      NOP    ->    OR    GR0, #0
```

#### Notes

The idea is to have an instruction which does not affect the program state. The AND and OR instrcution can be used for this purpose. From a debugger and dissanbler perspective it would be beneficial to settle on one combination and display it as a NOP for better reading.


<!--------------------------------------------------------------------------------------------------------->

<div style="page-break-before: always;"></div>

### CLR

<hr>

Clear a general register.

#### Format

```
      CLR <GR x>
```

#### Description

The CLR pseudo instruction simply clears a general register. The assembler uses the AND instruction with opMode one, which performs an AND operation of a zero value with "r". The result is always "r".

```
      CLR <GR x>    ->    AND GRx, GRx, OpMode 1
```

#### Notes

None.

<!--------------------------------------------------------------------------------------------------------->

<div style="page-break-before: always;"></div>

### MR

<hr>

Copy a general register value to a general register.

#### Format

```
      MR <GR x >, <GR y >
```

#### Description

THE MR pseudo instruction copies a general register to another general register. It overlaps with the MR instruction. The assembler will detect the GR to GR case and instead of the MR instruction generate an OR instruction with opMode one, which OR-s a zero value with "r".

#### Mapping

```
      MR <GR x>, <GR y>    ->    OR GRx, GRy, OpMode 1
```

#### Notes

None.


<!--------------------------------------------------------------------------------------------------------->

<div style="page-break-before: always;"></div>

### ASR, LSR, LSL

<hr>

Perform arithmetic and logical shift operations.

#### Format

```
      ASR <GR x>, sa
      LSR <GR x>, sa
      LSL <GR x>, sa
```

#### Description

All shift operations can be realized with the EXTR and DEP instructions. For the right shift operations, the shift amount is transformed into the bot position of the bit field and the length of the field to shift right. For arithmetic shifts, the result is sign extended. The shift left instruction is implemented using the DEP instruction.

```
      ASR <GR x>, sa      ->    EXTR.S Rx, Rx, 31 -s a, 32 - sa
      LSR <GR x>, sa      ->    EXTR   Rx, Rx, 31 - sa, 32 - sa
      LSL <GR x>, sa      ->    DEP.Z  Rx, Rx, 31 - sa, 32 - sa
```

#### Notes

// ??? **note** Perhaps it s a better idea to use the DSR instruction, which allows for a variable shift amount. ASR and ASL would map to "DSR Rx, Rx, shamt". One issue is that for a shift left, the "a" position needs to be a zero value. We would need a Z bit in the DSR instruction for this. As an alternative, we could allow to allow the deposit to specify a field that would go beyond the left side, just ignoring what does not fit.


<!--------------------------------------------------------------------------------------------------------->

<div style="page-break-before: always;"></div>

### ROL, ROR

<hr>

Rotate a general register value.

#### Format

```
      ROL <GR x>, cnt
      ROR <GR x>, cnt
```

#### Description

The ROL and ROR pseudo instruction can be realized using the DSR instruction.

```
      ROL <GR x>, <GR y>, cnt       ->    DSR Rx, Rx, cnt
      ROR <GR x>, <GR y>, cnt       ->    DSR Rx, Rx, 24 - cnt
```

#### Notes

None.


<!--------------------------------------------------------------------------------------------------------->

<div style="page-break-before: always;"></div>

### INC, DEC

<hr>

Adjust a general register value by an immediate value.

#### Format

```
      INC [ .<opt> ] <GR x>, <n>
      DEC [ .<opt> ] <GR x>, <n>
```

#### Description

The INC and DEC instruction will adjust the value in a general register be the signed immediate value "n". The options of the underlying instructions, i.e. whether the operation is checked for overflow, or an unsigned integer operation are observed by simply setting the option bits. The numeric range of the immediate value is limited to the operand mode zero field.

```
      INC [ .<opt> ] <GR x>, <n>      ->    ADD [ .<opt> ] GR x, n
      DEC [ .<opt> ] <GR x>, <n>      ->    SUB [ .<opt> ] GR x, n
```

#### Notes

None.


<!--------------------------------------------------------------------------------------------------------->

<div style="page-break-before: always;"></div>

### NEG, COM

<hr>

Negate a signed integer, complement an unsigned integer.

#### Format

```
      NEG <GR x>
      COM <GR x>
```

#### Description

The NEG and COM operation negate the value of a general register. The assembler uses the SUB and OR instruction with an operand mode of 1. The NEG instruction will subtract the Rx value from zero, the COM instruction will OR a zero value with the operand and invert the output.

```
      NEG [ .<opt> ] <GR x>      ->    SUB [ .<opt> ] GRx, GRx, opMode 1
      COM <GR x>                 ->    OR.N  GRx, GRx, opMode 1
```

#### Notes

None.


<!--------------------------------------------------------------------------------------------------------->

<div style="page-break-before: always;"></div>

## Instruction Set Summary

This appendix lists all instructions by instruction group.

### Computational Instructions

```
       0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31
      :--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:
      : ADD     ( 0x00 ): r      :C :L :O : opMode    : opArg                                         :
      :-----------------:-----------------------------------------------------------------------------:
      : SUB     ( 0x00 ): r      :C :L :O : opMode    : opArg                                         :
      :-----------------:-----------------------------------------------------------------------------:
      : AND     ( 0x00 ): r      :N :C :0 : opMode    : opArg                                         :
      :-----------------:-----------------------------------------------------------------------------:
      : OR      ( 0x00 ): r      :N :C :0 : opMode    : opArg                                         :
      :-----------------:-----------------------------------------------------------------------------:
      : XOR     ( 0x00 ): r      :N :C :0 : opMode    : opArg                                         :
      :-----------------:-----------------------------------------------------------------------------:
      : CMP     ( 0x00 ): r      : cond   : opMode    : opArg                                         :
      :-----------------:-----------------------------------------------------------------------------:
      : LEA     ( 0x00 ): r      : 0      : opMode    : opArg                                         :
      :-----------------:-----------------------------------------------------------------------------:
      : LSID    ( 0x00 ): r      :        : opMode    : opArg                                         :
      :-----------------:-----------------------------------------------------------------------------:
      : LDI     ( 0x00 ): r      :Z :L :0             : val1                                          :
      :-----------------:-----------------------------------------------------------------------------:
      : EXTR    ( 0x00 ): r      :S :N :0             : len          : pos          : 0      : b      :
      :-----------------:-----------------------------------------------------------------------------:
      : DEP     ( 0x00 ): r      :Z :I :0             : len          : pos          : 0      : b      :
      :-----------------:-----------------------------------------------------------------------------:
      : DSR     ( 0x00 ): r      : 0                  : shamt        : 0            : a      : b      :
      :-----------------:-----------------------------------------------------------------------------:
      : SHLA    ( 0x00 ): r      :I :L :O             : sa  : 0                              : b      :
      :-----------------:-----------------------------------------------------------------------------:
      : CMR     ( 0x00 ): r      : cond   :I : 0      : val                         : a      : b      :
      :-----------------:-----------------------------------------------------------------------------:
```

### Memory Reference Instruction

```
       0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31
      :--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:
      : LDW/H/B ( 0x00 ): r      : 0      : opMode    : opArg                                         :
      :-----------------:-----------------------------------------------------------------------------:
      : STW/H/B ( 0x00 ): r      : 0      : opMode    : opArg                                         :
      :-----------------:-----------------------------------------------------------------------------:
      : LDWR    ( 0x00 ): r      : 0      : opMode    : opArg                                         :
      :-----------------:-----------------------------------------------------------------------------:
      : STWC    ( 0x00 ): r      : 0      : opMode    : opArg                                         :
      :-----------------:-----------------------------------------------------------------------------:
```

### Extended Memory Reference Instructions

```
      :-----------------:-----------------------------------------------------------------------------:
      : LDWE    ( 0x00 ): r      : ofs2               : ofs1                        : a      : b      :
      :-----------------:-----------------------------------------------------------------------------:
      : LDHE    ( 0x00 ): r      : ofs2               : ofs1                        : a      : b      :
      :-----------------:-----------------------------------------------------------------------------:
      : LDBE    ( 0x00 ): r      : ofs2               : ofs1                        : a      : b      :
      :-----------------:-----------------------------------------------------------------------------:
      : TDWE    ( 0x00 ): r      : ofs2               : ofs1                        : a      : b      :
      :-----------------:-----------------------------------------------------------------------------:
      : STHE    ( 0x00 ): r      : ofs2               : ofs1                        : a      : b      :
      :-----------------:-----------------------------------------------------------------------------:
      : STBE    ( 0x00 ): r      : ofs2               : ofs1                        : a      : b      :
      :-----------------:-----------------------------------------------------------------------------:
```

### Absolute Memory Reference Instructions

```
      :-----------------:-----------------------------------------------------------------------------:
      : LDWA    ( 0x00 ): r      : ofs2               : ofs1                                 : b      : 
      :-----------------:-----------------------------------------------------------------------------:
      : LDHA    ( 0x00 ): r      : ofs2               : ofs1                                 : b      :
      :-----------------:-----------------------------------------------------------------------------:
      : LDBA    ( 0x00 ): r      : ofs2               : ofs1                                 : b      :
      :-----------------:-----------------------------------------------------------------------------:
      : STWA    ( 0x00 ): r      : ofs2               : ofs1                                 : b      :
      :-----------------:-----------------------------------------------------------------------------:
      : STHA    ( 0x00 ): r      : ofs2               : ofs1                                 : b      :
      :-----------------:-----------------------------------------------------------------------------:
      : STBA    ( 0x00 ): r      : ofs2               : ofs1                                 : b      :
      :-----------------:-----------------------------------------------------------------------------:
```

### Control Flow Instructions

```
       0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31
      :--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:
      : B       ( 0x00 ): 0      : ofs2               : ofs1                                          :
      :-----------------:-----------------------------------------------------------------------------:
      : BL      ( 0x00 ): r      : ofs2               : ofs1                                          :
      :-----------------:-----------------------------------------------------------------------------:
      : GATE    ( 0x00 ): r      : ofs2               : ofs1                                          :
      :-----------------:-----------------------------------------------------------------------------:
      : BR      ( 0x00 ): 0                                                                  : b      :
      :-----------------:-----------------------------------------------------------------------------:
      : BLR     ( 0x00 ): r      : 0                                                         : b      :
      :-----------------:-----------------------------------------------------------------------------:
      : BV      ( 0x00 ): 0                                                                  : b      :
      :-----------------:-----------------------------------------------------------------------------:
      : BVR     ( 0x00 ): 0                                                         : a      : b      :
      :-----------------:-----------------------------------------------------------------------------:
      : BE      ( 0x00 ): ofs2                        : ofs1                        : a      : b      :
      :-----------------:-----------------------------------------------------------------------------:
      : BLE     ( 0x00 ): ofs2                        : ofs1                        : a      : b      :
      :-----------------:-----------------------------------------------------------------------------:
      : CBR     ( 0x00 ): cond   : ofs2               : ofs1                        : a      : b      :
      :-----------------:-----------------------------------------------------------------------------:
      : TBR     ( 0x00 ): cond   : ofs2               : ofs1                        : 0      : b      :
      :-----------------:-----------------------------------------------------------------------------:
```

### System Control Instructions

```
       0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31
      :--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:
      : MR      ( 0x00 ): r      :T : 0                                             : g      : b      :
      :-----------------:-----------------------------------------------------------------------------:
      : MST     ( 0x00 ): r      :mode : 0                                          : s               :
      :-----------------:-----------------------------------------------------------------------------:
      : LDPA    ( 0x00 ): r      :L : 0                                             : a      : b      :
      :-----------------:-----------------------------------------------------------------------------:
      : PRBx    ( 0x00 ): r      :L :R : 0                                          : a      : b      :
      :-----------------:-----------------------------------------------------------------------------:
      : ITLB    ( 0x00 ): r      :L :T :M : 0                                       : a      : b      :
      :-----------------:-----------------------------------------------------------------------------:
      : PTLB    ( 0x00 ): 0      :L :T : 0                                          : a      : b      :
      :-----------------:-----------------------------------------------------------------------------:
      : PCA     ( 0x00 ): 0      :L :T :F : 0                                       : a      : b      :
      :-----------------:-----------------------------------------------------------------------------:
      : RFI     ( 0x00 ): 0                                                                           :
      :-----------------:-----------------------------------------------------------------------------:
      : DIAG    ( 0x00 ): 0                                                                           :
      :-----------------:-----------------------------------------------------------------------------:
      : BRK     ( 0x00 ): 0                           : info                                          :
      :-----------------:-----------------------------------------------------------------------------:
```

<!--------------------------------------------------------------------------------------------------------->

<div style="page-break-before: always;"></div>

## TLB and Cache Models

// ??? **note** a new chapter on the subject ?

<!--------------------------------------------------------------------------------------------------------->

<div style="page-break-before: always;"></div>

## VCPU-32 Runtime Environment

// ??? **note** a new chapter on the subject ?

<!--------------------------------------------------------------------------------------------------------->

<div style="page-break-before: always;"></div>

## Instruction and Architecture Commentary

Programming language design is Compiler Design. In a similar sense, instruction set and architecture design is CPU design. This chapter will present background information on the CPU design and discusses how the hardware design options influenced the instruction set design.

### A pipelined CPU

The first VCPU-32 implementation uses a three stage pipeline model. There stages are the **instruction fetch and decode stage**, the **memory access** stage and the **execute** stage. This section gives a brief overview on the pipelining considerations using the three-stage model. The architecture does not demand that particular model. It is just the first implementation of VCPU-32. The typical challenges such as structural hazards and data hazards will be identified and discussed.

- **Instruction fetch and decode**. The first stage will fetch the instruction word from memory and decode it. There are two parts. The first part of the stage will use the instruction address and attempt to fetch the instruction word from the instruction cache. At the same time the translation look-aside buffer will be checked whether the virtual to physical translation is available and if so whether the access rights match. The second part of the stage will decode the instruction and also read the general registers from the register file.

- **Memory access**. The memory access stage will take the instruction decoded in the previous stage and compute the address for the memory data access. This also the stage where any segment or control register are accessed. In addition, unconditional branches are handled at this stage. Memory data item are read or stored depending on the instruction. Due to the nature of a register/memory architecture, the memory access has to be performed before the execute stage. This also implies that there needs to be an address arithmetic unit at this state. The classical 5-stage RISC pipeline with memory access past the execute stage uses the ALU for this purpose.

- **Execute**. The Execute Stage will primarily do the computational work using the values passed from the MA stage. The computational result will be written back to the registers on the next clock cycle.

Note that this is perhaps one of many ways to implement a pipeline. The three major stages could also be further divided internally. For example, the fetch and decode stage could consist of two sub stages. Likewise, the memory access stages could be divided into an address calculation sub-stage and the actual data access. Dividing into three stages however simplifies the bypass logic as there are only two spots to insert any register overriding. This is especially important for the memory access stage, which uses the register content to build addresses. Two separate stages, i.e. address computation and memory access, would require options to redo the address arithmetic when detecting a register interlock at the memory access stage. 

### Register - Memory Model

Register - memory machines typically offer computational instructions that are of the form ***REGx <- REGx op Operand*** where the operand is either another register or a memory location. Instruction sets are of variable length, depending on the addressing modes offered. VCPU-32 does offer several addressing modes for the "operand" combined with a fixed instruction word length. As a consequence, the number of registers and the offset width are limited. Instead of 32 register found in modern RISC architectures, VCPU-32 has eight general registers with four of them capable to be used as index registers in the indexed addressing mode. Having only eight registers also implies that register zero is a valid register and not a source of zero or target of a null store. This in turn means that there must be another way to encode a zero when needed. Most of the instructions that could make use of a "zero" register have a bit "Z" to indicate using a zero value instead of a register value.

### Memory Reference Instructions

Memory reference instruction operate on memory and a general register. In that sense the architecture is a typical load/store architecture. However, in contrast to a load/store architecture VCPU-32 load / store instructions are not the only instructions that access memory. Being a register/memory architecture, all instructions with an operand field encoding, will access memory as well. There are no instructions that will access data memory access for reading and writing back an operand. Due to the operand instruction format and the requirement to offer a fixed length instruction word, the offset for an address operation is rather limited but still covers a large address range.

### Absolute, Virtual and Logical Address

VCPU-32 features a rather large **virtual address** range of 64 bits. The range is divided into a 32-bit segment identifier and a 32-bit offset. Implmentations may choose to only offer 16 bits or 24 bits as the segment identifier range. The memory address range is 32-bit which allows for a 4 Gbyte memory. 

To accommodate an evan larger physical memory, a memory bank identifier was added. Up to 16 banks are allowed. However, the LDA/STA instructions can only access the first bank, the bank with a zero identifier. All other banks can only be reached with a virtual address. The TLB contains a bank and the physical offset for as translation result. 

In addition, it is envisioned that several CPUs, up to 16 CPUs, can be connected to form a cluster. The CPU-ID the also becomes part of the physical address, referring to an address located on another CPU bank and offset. Right now, this feature is not supported yet.

For performance reasons, one would not like to carry around the full virtual address for parameter passing, data access and so on. A **logical address** concept complements the virtual and physical address. A logical address is a 32-bit word, with the upper two bits selecting one of the segment registers SR4 to SR7. From a logical to virtual translation perspective, the logical address needs to be loaded into any of the four index registers GR4 to GR7. For example, loading a logical address into GR7, an index register, would allow to access a virtual address computed from the logical address, by using the index addressing mode 7, 11 or 15, which specifies to use GR7 as the index register. The upper two bits then select the segment register.

### Operand modes

An operand field in the respective instruction encodes several operand modes. It needs to support the idea of an immediate value, a combination of a zero and a general register, two general registers, a segment and general register combination and an index register with a a signed offset. Operand modes also need to distinguish word, half-word and byte size content access. 

There are no auto pre/post offset adjustment or indirect modes. Indirect addressing modes are not pipeline friendly, they require an additional memory access. Pre or post offset adjustments have not been implemented so far, the bits needed for encoding are rather used to widen the operand mode offset itself. Nevertheless, auto pre/post adjustment would be an option for the load/store type instructions.

Operand mode 2 featuring two general registers is an addition to a typical register memory architecture. It allows to perform computation with two arbitrary general registers, storing the result into a general register. Operand mode 1 features a  combination of a zero and a general register as computation operands, which allows to use a zero value in an operation. For example, the SUB instruction using this operand mode, will perform a zero minus REG operation, i.e. a negate operation. The CPU pipeline implements all of the immediate and register modes in selecting the correct inputs to the ALU. From a hardware perspective, all computational operations are of the form Reg-R <- Reg-A op Reg-B. The general register reads take place in the instruction fetch / decode stage. The general register store in the execution stage.

Operand mode 3 is the extended addressing mode. It allows to use any data location in the entire virtual address range of the architecture. 

Finally, operand modes 4 to 15, are the indexed addressing modes. There three groups 4..7, 8..11 and 12..15, which implement the indexing address modes distinguishing between word, half-word and byte operands. Within each group operand modes refer to the index registers GR4..GR7 and use the upper two bits of the selected register to select a segment register SR4 to SR7. Note that any index register can hold a value to select any of the four segment registers. It is the upper two bits that select the segment register. 

### Branch Instructions

There are two types of branches, conditional and unconditional. Both come with a possible penalty for the instruction pipeline. That is, unconditional branches will always have a pipeline penalty. The target address computation takes place in the memory access stage, resulting in a pipeline penalty of one instruction.

Branch and link type instructions store the return address in a general register. While the new target address is computed in the memory access stage, the store of the return address is performed in the execute stage. An instruction at the branch target that references the general register where the returned address is stored will stall the pipeline for one cycle.

Conditional branches are implemented with a static branch prediction scheme. Forward branches are predicted not taken, backward branches are predicted taken. The decision is predicted during the fetch and decode stage and if predicted correctly will result in no pipeline penalty.

### Privilege changes

Modern processors support the distinction between several privilege levels. Instructions can only access data or execute instructions when the privilege level matches the privilege level required. Two or four levels are the most common implementation. Changing between levels is often done with a kind of trap operation to a higher privilege level software, for example the operating system kernel. However, traps are expensive, as they cause a CPU pipeline to be flush when going through a trap handler.

VCPU-32 follows the PA-RISC route of gateways. A special branch instruction will raise the privilege level when the instruction is executed on a gateway page. The branch target instruction continues execution with the privilege level specified by the gateway page. Return from a higher privilege level to a code page with lower privilege level, will automatically set the lower privilege level. 

In contrast to PA-RISC, VCPU-32 implement only two privilege levels, **user** and **priv** mode. The privilege status is not encoded in the instruction address, but rather in the process status word. A transition from user to privilege mode is accomplished by setting the "X" bit in the status register when the GATE instruction is executed on a gateway page with appropriate privilege mode settings. Each instruction fetch compares the privilege level of the page where the instruction is fetched from with the status register "P" bit. A higher privilege level on an execution page results in a privilege violation trap. A lower level of the execute page and a higher level in the status register will in an automatic demotion of the privilege level. If the architecture chooses one day to implement a four level privilege level protection architecture, the access rights fields and where to store the current execution privilege level needs to be revisited. Perhaps it should then just follow the PA-RISC architecture in this matter.

### Computational Instructions

Computational instructions feature the basic set of arithmetic, logical and bit operations. Option bits in the respective instructions allow for a great flexibility of how an instruction is executed. For example, negating the output of a logical operation reverses the logic and an ADD would become a NAND operation.

### Bit Operations Instructions

The instruction set does not explicitly offer any shift and rotate instructions. Instead bit field extract and deposit operations allow to implement these functions. Additionally, as bit field extract and deposit operations are very common, a sequence of shift and masking operations are elegantly replaced by single extract and deposit instructions.

### Atomic Instructions

The **LDWR** and **SWDC** instruction implement the foundation upon which several synchronization schemes can be implemented.

// ??? **note** ( how is it implemented ? linked to the cache line ? )


### Instruction and Data Breakpoints

A fundamental requirement is the ability to set instruction and data breakpoints. An instruction breakpoint is encountering the BRK instruction in an instruction stream. The break instruction will cause an instruction break trap and pass control to the trap handler. Just like the other traps, the pipeline will be emptied by completing the instructions in flight and flushing all instructions after the break instruction. Since break instructions are normally not in the instruction stream, they need to be set dynamically in place of the instruction normally found at this instruction address. The key mechanism for a debugger is then to exchange the instruction where to set a break point, hit the breakpoint and replace again the break point instruction with the original instruction when the breakpoint is encountered. Upon continuation and a still valid breakpoint for that address, the break point needs to be set after the execution of the original instruction.

Since a code is non-writeable, there needs to be a mechanism to allow the temporary modification of the instruction. One approach is to change the page type temporarily to a data page and do the modification and change it back again. Care has to be taken to ensure the caches are properly flushed, since a modification ends up in the data cache. Another approach is to allocate a debug bit for the page table. The continuous instruction privilege check would also check the debug bit and in this case allow to write to a code page. All these modifications only take place at the highest privilege level. When resuming the debugger, the original instruction executes and after execution, the break point instruction needs to be set again, if desired. This would be the second  trap, but this time only the break point is set again.

Data breakpoints are traps that are taken when a data reference to the page is done. They do not modify the instruction stream. Depending on the data access, the trap could happen before or after the instruction that accesses the data. A write operation is trapped before the data is written, a read instruction is trapped after the instruction took place.

<!--------------------------------------------------------------------------------------------------------->

<div style="page-break-before: always;"></div>

## Table of Contents

[TOC]

<!---------------------------------------------------------------------------->
<!- End of file -------------------------------------------------------------->
<!---------------------------------------------------------------------------->
