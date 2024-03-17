
<!--------------------------------------------------------------------------------------------------------->
<!-- VCPU-32 Document                                                                                   --->
<!--------------------------------------------------------------------------------------------------------->

<!--------------------------------------------------------------------------------------------------------->
<!-- use "&nbsp; " to insert a blank in the table column if too narrow...                               --->
<!-- a CSS style to make the tables having the possible width of the page                               --->
<!-- using orphans and widow parameters for keep a block together...                                    --->
<!--------------------------------------------------------------------------------------------------------->

<style>
   div {
	   text-align: justify;
      orphans:2;
      widow:2;
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
   blockquote {
      orphans:3;
      widow:2;
    }
   code {
	   font-size: 12px;
	   line-height: 17px;
      orphans:3;
      widow:2;
   }
</style>


<!--------------------------------------------------------------------------------------------------------->

# VCPU-32 System Architecture and Instruction Set Reference

Helmut Fieres
Version B.00.03
March, 2024

<!--------------------------------------------------------------------------------------------------------->

<div style="page-break-before: always;"></div>

## Introduction

> "A vintage 32-bit register-memory model CPU ? You got to be kidding me. Shouldn't we design a modern superscalar, multi-core RISC based 32-bit machine? Or even better 64-bit?. After all, this is the 21st century and not the eighties. A lot happened since the nineties."
>
> "Well why not. Think vintage."
>
> "Seriously?“
>
> “OK, seriously. Designers of the eighties CPUs almost all used a micro-coded approach with hundreds of instructions. The nineties shifted to RSIC based designs with large sets of registers, fixed word instruction length, and instructions that are in general pipeline friendly. What if these principles had found their way earlier into these designs? What if a large virtual address space, a fixed instruction length and simple pipeline friendly instructions had found their way into these designs ?
>
> A 32-bit vintage CPU will give us a good set of design challenges to look into and opportunities to include modern RISC features and learn about instruction sets and pipeline design as any other CPU. Although not a modern design, it will still be a useful CPU. Let's see where this leads us."
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

VCPU-32 implements a **register memory architecture** offering few addressing modes for the "operand" combined with a fixed instruction word length. For most of the instructions one operand is the register, the other is a flexible operand which could be an immediate, a register content or a memory address where the data is obtained. The result is placed in the register which is also the first operand. These type of machines are also called two address machines.

```
   REG <- REG op OPERAND
```

In contrast to similar historical register-memory designs, there is no operation which does a read and write operation to memory in the same instruction. For example, there is no "increment memory" instruction, since this would require two memory operations and is not pipeline friendly. Memory access is performed on a machine word, half-word or byte basis. Besides the implicit operand fetch in an instruction, there are dedicated memory load / store instructions. Computational operations and memory reference operations use a common addressing model, also called **operand mode**. In addition to the memory register mode, one operand mode supports also a three operand model ( Rs = Ra OP Rb ), specifying two registers and a result register, which allows to perform three address register for computational operations as well. The machine can therefore operate in a memory register model as well as a load / store register model.

### Memory and IO Address Model

VCPU-32 features a physical memory address range of 32-bits. The picture below depicts the physical memory address layout. The physical address range is divided into a memory data portion and an I/O portion. The I/O portion is further divided into 16 channel data areas. Channel zero represents the CPU itself. All others represent a hardware I/O channel. The entire address range is directly accessible with load and store instructions.

```
    0                                     31
   :-------------------------------------: 0x00000000
   :                                     :
   :                                     :
   :                                     :
   :                                     :
   :                                     :
   :                                     :
   :            Data                     :
   :                                     :
   :                                     :
   :                                     :
   :                                     :          +-- 0xF0000000 -> :-------------------------------------:
   :                                     :          |                 : Processor dependent code (16Mb )    :
   :                                     :          |                 :                                     :
   :                                     :          |   0xF1000000 -> :-------------------------------------:
   :                                     :          |                 : I/O Channel 1 (16Mb )               :
   :                                     :          |                 :                                     :
   :                                     :          |   0xF2000000 -> :-------------------------------------:
   :-------------------------------------: ---------+                 :            . . .                    :
   :                                     :                            :                                     : 
   :            IO                       :              0xFF000000 -> :-------------------------------------:
   :                                     :                            : I/O Channel 15 (16Mb )              :
   :                                     :                            :                                     :
   :-------------------------------------: 0xFFFFFFFF --------------->:-------------------------------------:
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

VCPU-32 features a set of registers. They are grouped in general registers, segment registers and control registers. There are eight general registers, labeled GR0 to GR15, and eight segment registers, labeled SR0 to SR7. All general registers can do arithmetic and logical operations. Register GR10 to GR15 are additionally labelled index registers, which are used in the addressing modes. The eight segment registers hold the segment part of the virtual address. The control registers contain system level information such as protection registers and interrupt and trap data registers.

```
                  Segment                         General                           Control
       0                     31          0                     31          0                     31
      :------------------------:        :------------------------:        :------------------------:
      :         SR0            :        :          GR0           :        :        CR0             :
      :------------------------:        :------------------------:        :------------------------:
      :                        :        :                        :        :                        :
      :         ...            :        :                        :        :                        :
      :                        :        :                        :        :                        :
      :------------------------:        :                        :        :                        :
      :         SR7            :        :                        :        :          ...           :
      :------------------------:        :                        :        :                        :
                                        :         ...            :        :                        :
                                        :                        :        :                        :
                                        :                        :        :                        :
                                        :                        :        :                        :
                                        :                        :        :                        :
                                        :                        :        :                        :
                                        :                        :        :                        :
                                        :------------------------:        :                        :
                                        :          GR15          :        :                        :
                                        :------------------------:        :                        :
                                                                          :                        :
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
| 16 | **I-BASE-ADR** | Interrupt and trap vector table base address. The absolute address of the interrupt vector table. When an interrupt or trap is encountered, the next instruction to execute is calculated by adding the interrupt number shifted by 32 to this address. Each interrupt has eight instruction locations that can be used for this interrupt. The table must be page aligned. |
| 17 | **I-STAT** | When an interrupt or trap is encountered, this control register holds the current status word.
| 18 |  **I-IA-SEG** | When an interrupt or trap is encountered, control register holds the current instruction address segment. |
| 19 |  **I-IA-OFS** | When an interrupt or trap is encountered, control register holds the current instruction address offset. |
| 20 - 22 | **I-PARM-n** | Interrupts and pass along further information through these control registers. |
| 23 | **I-EIM** | External interrupt mask. |
| 24 - 31 | **TEMP-n** | These control registers are scratch pad registers. Temporary registers are typically used in an interrupt handlers as a scratch register space to save general registers so that they can be used in the interrupt routine handler. They also contain some further values for the actual interrupt. These register will neither be saved nor restored upon entry and exit from an interrupt. |

### Segmented Memory Model

The VCPU-32 memory model features a **segmented memory model**. The address space consists of up to 2^32 segments, each of which holds up to 2^32 words in size. Segments are further subdivided into pages with a page size of 4K Words. The concatenation of segment ID and offset form a **virtual address**.


```
    0                                       31     0                          20            31
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

VCPU-32 defines three types of addresses. At the programmer's level there is the **logical address**. The logical address is a 32-bit word, which contains a 2-bit segment register selector and a 30-bit offset. During data access the segment selector selects from the segment register set SR4 to SR7 and forms together with the 30-bit offset a virtual address. From a logical to virtual translation perspective, the logical address needs to be loaded into an index registers GR8 to GR15. 

The **virtual address** is the concatenation of a segment and an offset. Together they form a maximum address range of 2^32 segments with 2^32 bits each. Once the virtual address is formed, the translation process is the same for both virtual addressing modes. The following figure shows the translation process for a logical address. A virtual address is translated to a **physical address**. A logical address is an address with the upper two bits indicating which segment register to use and the offset an unsigned index into the segment. The resulting virtual address will have in this case the upper two bits set to zero.

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

When address translation is disabled, the respective TLB is is bypassed and the address represents a physical address in bank zero at the local CPU as described before. Also, protection ID checking is disabled. The U, D, T, B only apply to a data TLB. The X bit is only applies to the instruction TLB. When the processor cannot find the address translation in the TLB, a TLB miss trap will invoke a software handler. The handler will walk the page table for an entry that matches the virtual address and update the TLB with the corresponding physical address and access right information, otherwise the virtual page is not in main memory and there will be a page fault to be handled by the operating system. In a sense the TLB is the cache for the address translations found in the page table. The implementation of the TLB is hardware dependent.

To accommodate an even larger physical memory, a memory bank identifier was added. Up to 16 banks are allowed. However, the LDA/STA instructions can only access the first bank, the bank with a zero identifier. All other banks can only be reached with a virtual address. The maximum physical memory is thus 64 Gbytes. The TLB contains a bank and the physical offset for as translation result. In addition, it is envisioned that several CPUs, up to 16 CPUs, can be connected to form a cluster. The CPU-ID the also becomes part of the physical address, referring to an address located on another CPU bank and offset. Right now, this feature is not supported yet.

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
Page Table Entry (Example):                                                                            |
                                                                                                       |
    0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31    |
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

### Privilege changes

Modern processors support the distinction between several privilege levels. Instructions can only access data or execute instructions when the privilege level matches the privilege level required. Two or four levels are the most common implementation. Changing between levels is often done with a kind of trap operation to a higher privilege level software, for example the operating system kernel. However, traps are expensive, as they cause a CPU pipeline to be flush when going through a trap handler.

VCPU-32 follows the PA-RISC route of gateways. A special branch instruction will raise the privilege level when the instruction is executed on a gateway page. The branch target instruction continues execution with the privilege level specified by the gateway page. Return from a higher privilege level to a code page with lower privilege level, will automatically set the lower privilege level.

In contrast to PA-RISC, VCPU-32 implement only two privilege levels, **user** and **priv** mode. The privilege status is not encoded in the instruction address, but rather in the process status word. A transition from user to privilege mode is accomplished by setting the "X" bit in the status register when the GATE instruction is executed on a gateway page with appropriate privilege mode settings. Each instruction fetch compares the privilege level of the page where the instruction is fetched from with the status register "P" bit. A higher privilege level on an execution page results in a privilege violation trap. A lower level of the execute page and a higher level in the status register will in an automatic demotion of the privilege level. If the architecture chooses one day to implement a four level privilege level protection architecture, the access rights fields and where to store the current execution privilege level needs to be revisited. Perhaps it should then just follow the PA-RISC architecture in this matter.

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
| 17 | **Alignment trap** | IA of the current instruction | instr | data adr segID | data adr Ofs| |
| 18 | **Break instruction trap** | IA of the current instruction | instr | data adr segID | data adr Ofs| |
| 19 .. 31 | reserved | | | | | | |

## Instruction and Data Breakpoints

A fundamental requirement is the ability to set instruction and data breakpoints. An instruction breakpoint is encountering the BRK instruction in an instruction stream. The break instruction will cause an instruction break trap and pass control to the trap handler. Just like the other traps, the pipeline will be emptied by completing the instructions in flight and flushing all instructions after the break instruction. Since break instructions are normally not in the instruction stream, they need to be set dynamically in place of the instruction normally found at this instruction address. The key mechanism for a debugger is then to exchange the instruction where to set a break point, hit the breakpoint and replace again the break point instruction with the original instruction when the breakpoint is encountered. Upon continuation and a still valid breakpoint for that address, the break point needs to be set after the execution of the original instruction.

Since a code is non-writeable, there needs to be a mechanism to allow the temporary modification of the instruction. One approach is to change the page type temporarily to a data page and do the modification and change it back again. Care has to be taken to ensure the caches are properly flushed, since a modification ends up in the data cache. Another approach is to allocate a debug bit for the page table. The continuous instruction privilege check would also check the debug bit and in this case allow to write to a code page. All these modifications only take place at the highest privilege level. When resuming the debugger, the original instruction executes and after execution, the break point instruction needs to be set again, if desired. This would be the second  trap, but this time only the break point is set again.

Data breakpoints are traps that are taken when a data reference to the page is done. They do not modify the instruction stream. Depending on the data access, the trap could happen before or after the instruction that accesses the data. A write operation is trapped before the data is written, a read instruction is trapped after the instruction took place.


<!--------------------------------------------------------------------------------------------------------->

<div style="page-break-before: always;"></div>

## Instruction Set Overview

This chapter gives a brief overview on the instruction set. The instruction set is divided into five general groups of instructions. There are memory reference, immediate, branch, computational and system control type instructions. This chapter will present an overview on the general layout, instruction encoding and the high level way of describing the instruction operation in the chapters to follow.

### General Instruction Encoding

The instruction uses fixed word length instruction format. Instructions are always one machine word. In general there is a 6 bit field to encode the instruction opCode. The instruction opCode field is encoded as follows:

```
    0                 6                                                                          31
   :--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:
   : opCode          : opCode specific portion                                                     :
   :-----------------:-----------------------------------------------------------------------------:
```

In the interest of a simplified hardware design, the opCode and opCode specific instruction fields are regular and on fixed positions whenever possible. As shown above, the opCode is always at bits 0 .. 5. But also registers, modes and offsets are at the same locations. The benefit is that the decoding logic can just extract the fields without completely decoding the instruction upfront. For some instructions not all bits instruction word are used. They are reserved fields and should be filled with a zero for now. Throughout the remainder of this document numbers are shown in three numeric formats. A decimal number is a number starting with the digits 1 .. 9. An octal number starts with the number 0, and a hex number starts with the "0x" prefix.

### Operand Encoding

The **operand mode** field in instructions that use operand modes for specifying one argument indicates mode is used. There are operand modes to define immediate values, registers or an address. Furthermore, the operand mode also encodes the data length of the operand. There are 32 operand modes for instructions that use the operand encoding format. They can be grouped in four sets. Modes 0 .. 3 contains the immediate and register modes, modes 4 ..7 are the register indexed modes and 8 .. 15, 16 .. 23 and 24 .. 31 are the indexed adressing groups. The following figure gives an overview of the instruction layout for instructions with an operand encoding. 

```
                      <-  res  -> <-opt -> <-   mode   -> <--------- operand --------------------->
    0                 6           10       13             18                24          28       31
   :--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:
   : opCode          : r         :        : 0            : val                                  :s :  immediate
   :-----------------:-----------------------------------------------------------------------------:
   : opCode          : r         :        : 1            : 0                           : b         :  one register
   :-----------------:-----------------------------------------------------------------------------:
   : opCode          : r         :        : 2            : 0               : a         : b         :  two registers
   :-----------------:-----------------------------------------------------------------------------:
   : opCode          : r         :        : 3            : 0                                       :  undefined
   :-----------------:-----------------------------------------------------------------------------:

   :-----------------:-----------------------------------------------------------------------------:
   : opCode          : r         :        : 4            : seg : 0         : a         : b         :  register indexed, word
   :-----------------:-----------------------------------------------------------------------------:
   : opCode          : r         :        : 5            : seg : 0         : a         : b         :  register indexed, half
   :-----------------:-----------------------------------------------------------------------------:
   : opCode          : r         :        : 6            : seg : 0         : a         : b         :  register indexed, byte
   :-----------------:-----------------------------------------------------------------------------:
   : opCode          : r         :        : 7            : 0                                       :  undefined
   :-----------------:-----------------------------------------------------------------------------:
   
   :-----------------:-----------------------------------------------------------------------------:
   : opCode          : r         :        : 8  - 15      : seg : ofs                            :S :  indexed, word
   :-----------------:-----------------------------------------------------------------------------:
   : opCode          : r         :        : 16 - 23      : seg : ofs                            :S :  indexed, half
   :-----------------:-----------------------------------------------------------------------------:
   : opCode          : r         :        : 24 - 31      : seg : ofs                            :S :  indexed, byte
   :-----------------:-----------------------------------------------------------------------------:
```

There is one **immediate operand mode**, which supports a signed 14-bit value. The sign bit is in the rightmost position of the field and removed from the value to encode. Depending on the sign, the remaining value im the field is sign or zero extended. The **register modes** specify one or two registers for the operation. Operand mode 1 sets "a" to zero and passes the other argument in "b". This mode is typically used for operations that use a zero value as one of the arguments. For example, the operation "zero - b" can be used in the  SUB instruction, which will subtract the "b" register from a zero value. This is essentially a negate operation. Operand mode 2 will use "a" and "b" as input to the operation of the instruction.

The machine addresses memory with a byte address. There are indexed and register indexed operand modes. Operand modes 4 to 6 are the **register indexed address modes**s, which will use a base register "b" and add an offset in "a" to it, forming the final byte address offset. Mode 4 refers to a date word, mode 5 to a date half-word and mode 6 to a datde byte. Modes 8 to 31 are the **indexed address mode**s structured in three groups, which access a word, a half-word or a byte. The mode group map to an index register R8 to R15, i.e. mode 8, 16 and 24 to R8, 9, 17 and 25 to R9 and so on. For the indexed operand modes, the computed address must be aligned with the size of data to fetch. 

The **seg** field manages the segment register selection. A value of zero will use the upper two bits of the computed offset to select from SR4 to SR7. A value of 1 to 3 will select SR1 to SR3. SR0 is not used in this operand scheme. It is used as a target register for an external branch or a segment scratch register.

There are no auto pre/post offset adjustment or indirect modes. Indirect addressing modes are not pipeline friendly, they require an additional memory access. Pre or post offset adjustments have not been implemented so far. Nevertheless, auto pre/post adjustment would be an option for the load/store type instructions.

### Instruction Notation

The instructions described in the following chapters contain the instruction word layout, a short description of the instruction and also a high level pseudo code of what the instruction does. The pseudo code uses a register notation describing by their class and index. For example, GR[5] labels general register 5. SR[x] represents the segment registers and CR[x] the control register. In addition, some control register also have dedicated names, such as for example the shift amount control register, labelled "shamt".

Instruction operation are described in a pseudo C style language using assignment and simple control structures to describe the instruction operation. In addition there are functions that perform operations and compute expressions. The following table lists these functions.

| Function | Description |
|:---|:----|
| **cat( x, y )** | concatenates the value of x and y. |
| **catImm( x, y )** | Assembles the immediate fields of an instruction. There are several formats for encoding the immediate. The individual fields are concatenated with "x" being the left most number of bits, followed by "y". |
| **lowSignExtend( x, len )** | performs a sign extension of the value "x". The sign bit is stored in the rightmost position and applied to the extracted field of the remainning left side bits. |
| **signExtend( x, len )** | performs a sign extension of the value "x". The "len" parameter specifies the number of bits in the immediate. The sign bit is the leftmost bit. |
| **zeroExtend( x, len )** | performs a zero bit extension of the value "x". The "len" parameter specifies the number of bits in the immediate.|
| **segSelect( x )** | returns the segment register number based on the leftmost three bits of the argument "x". |
| **operandAdrSeg( instr )** | computes the segment Id from the instruction and mode information. ( See the operand encoding diagram for modes ) |
| **operandAdrOfs( instr )** | computes the offset portion from the instruction and mode information. ( See the operand encoding diagram for modes ) |
| **operandBitLen( instr )** | computes the operand bit length from the instruction and mode information. ( See the operand encoding diagram for modes ) |
| **ofsSelect( x )** | returns the 30-bit offset portion of the argument "x". |
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

Memory reference instruction operate on memory and a general register with a unit of transfer of a word, a half-word or a byte. In that sense the architecture is a typical load/store architecture. However, in contrast to a load/store architecture VCPU-32 load / store instructions are not the only instructions that access memory. Being a register/memory architecture, all instructions with an operand field encoding, will access memory as well. There are however no instructions that will access data memory access for reading and writing back an operand in the same instruction. Due to the operand instruction format and the requirement to offer a fixed length instruction word, the offset for an address operation is limited but still covers a large address range. 

The instructions use a **W** for word, a **H** for half-word and a **B** for byte operand size. The **LDx** and **STx** instruction are load and store using the operand encoding to specify the actual address. Using logical addresses restricts the segment size to 30-bit address range. To address the entire 32-bit range possible in a segment, the segment selector needs to specify one of the SR1 to SR3 registers. The **LDAW**, **LDWAX**  and **STAW** instruction implement access to the physical memory computing a physical address to access. For supporting atomic operations two instructions are provided. The **LDWR** instruction loads a value form memory and remember this access. The **STWC** instruction will store a value to the same location that the LR instructions used and return a failure if the value was modified since the last LR access. This CPU pipeline friendly pair of instructions allow to build higher level atomic operations on top.

Memory reference instructions can be issued when data translation is on and off. When address translation is turned off, the memory reference instructions will ignore the segment part and replace it with a zero value. It is an architectural requirement that a virtual address with a segment Id of zero maps to an absolute address with the same offset. The absolute address mode instruction also works with translation turned on and off. The extended address mode instructions will raise a trap.

<!--------------------------------------------------------------------------------------------------------->

<div style="page-break-before: always;"></div>

### LDW, LDH, LDB

<hr>

Loads a memory value into a general register.

#### Format

```
   LDx GR[r], GR[a] ( GR[b] )           ; opMode 4, 5, 6
   LDx GR[r], <ofs> ( GR[b] )           ; opMode 8 .. 15, 16 .. 23, 24 .. 31
   LDx GR[r], <ofs> ( seg, GR[b]> )     ; opMode 8 .. 15, 16 .. 23, 24 .. 31
```

```
    0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31
   :--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:
   : LDx    ( 0x18 ) : r         : 0      : opMode       :  opArg                                  :
   :-----------------:-----------------------------------------------------------------------------:
```

#### Description

The load instruction will load the operand into the general register "r". See the section on operand encoding for the defined operand modes. OpModes 0 to 3 are undefined and result in an illegal instruction trap.

#### Operation

```
   if ( opMode < 4 ) illegalInstructionTrap( );

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
   STx GR[a] ( GR[b] ), GR[r]        ; opMode 4, 5, 6
   STx ofs ( GR[b] ), GR[r]          ; opMode 8 .. 15, 16 .. 23, 24 .. 31
   STx ofs ( seg, GR[b] ), GR[r]     ; opMode 8 .. 15, 16 .. 23, 24 .. 31
```

```
    0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31
   :--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:
   : STx    ( 0x19 ) : r         : 0      : opMode       :  opArg                                  :
   :-----------------:-----------------------------------------------------------------------------:
```

#### Description

The load instruction will store the data in general register "r" to memory. See the section on operand encoding for the defined operand modes. Operand modes 0 - 7, 8, 16, and 24 are undefined for this instruction and result in illegal instruction trap. Note that there is no register indexed mode for the STx instruction. In contrast to the LDx instruction, the STx instruction would need to access three registers ( the value to store, the base and index register ), which would require a greater hardware effort.

#### Operation

```
   if ( opMode < 8 ) illegalInstructionTrap( );

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

### LDWA

<hr>

Loads the memory content into a general register using an absolute address.

#### Format

```
   LDWA GR[r], ofs ( GR[b] )
```

```
    0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31
   :--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:
   : LDWA   ( 0x3C ) : r         : ofs                                                 : b         :
   :-----------------:-----------------------------------------------------------------------------:
```

#### Description

The load absolute instruction will load the content of the physical memory address into the general register "r". The absolute 32-bit address is computed from adding the signed offset to general register "b". The LDwA instructions is a privileged instructions.

#### Operation

```
   if ( ! ST.[ PRIV ] ) privilegedOperationTrap( );

   GR[r] <- zeroExtend( memLoad( 0, GR[b] + lowSignExtend( ofs, 18 ), 32 ));
```

#### Exceptions

- Privileged operation trap
- Alignment trap

#### Notes

None.


<!--------------------------------------------------------------------------------------------------------->

<div style="page-break-before: always;"></div>

### LDWAX

<hr>

Loads the memory content into a general register using an absolute address.

#### Format

```
   LDWAX GR[r], GR[a] ( GR[b] )
```

```
    0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31
   :--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:
   : LDWAX  ( 0x3D ) : r         : ofs                                     : a         : b         :
   :-----------------:-----------------------------------------------------------------------------:
```

#### Description

The load absolute indexed instruction will load the content of the physical memory address into the general register "r". The absolute 32-bit address is computed from adding the signed offset in register "a" to general register "b". The LDWAX instructions is a privileged instructions.

#### Operation

```
   if ( ! ST.[ PRIV ] ) privilegedOperationTrap( );

   GR[r] <- zeroExtend( memLoad( 0, GR[b] + GR[a], 32 ));
```

#### Exceptions

- Privileged operation trap
- Alignment trap

#### Notes

None.


<!--------------------------------------------------------------------------------------------------------->

<div style="page-break-before: always;"></div>

### STWA

<hr>

Stores a general register value into memory using an absolute physical address.

#### Format

```
   STWA ofs ( <SR a>, <GR b> ), <GR r>
```

```
    0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31
   :--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:
   : STWA   ( 0x3E ) : r         : ofs                                                 : b         :
   :-----------------:-----------------------------------------------------------------------------:
```

#### Description

The store absolute instruction will store the target register into memory using a physical address. The absolute 32-bit address is computed from adding the signed offset to general register "b". The STWA instructions are privileged instructions.

#### Operation

```
   if ( ! ST.[ PRIV ] ) privilegedOperationTrap( );

   memStore( 0, GR[b] + signExtend( catImm( ofs1, ofs2 ), 18 ), GR[r], 32 );
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
   LDWR GR[r], GR[a] ( GR[b] )           ; opMode 4, 5, 6
   LDWR GR[r], <ofs> ( GR[b] )           ; opMode 8 .. 15, 16 .. 23, 24 .. 31
   LDWR GR[r], <ofs> ( seg, GR[b]> )     ; opMode 8 .. 15, 16 .. 23, 24 .. 31
```

```
    0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31
   :--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:
   : LDWR   ( 0x1A ) : r         : 0      : opMode       : opArg                                   :
   :-----------------:-----------------------------------------------------------------------------:
```

#### Description

The LDWR instruction is used for implementing semaphore type operations. See the section on operand encoding for the defined operand mode encoding. The first part of the instruction behaves exactly like the LDW instruction. A logical memory address is computed. Next, the memory content is loaded into general register "r". The second part remembers the address and will detect any modifying reference to it.

#### Operation

```
   if (( opMode < 8 ) && ( opMode > 15 )) illegalInstructionTrap( );

   seg <- operandAdrSeg( instr );
   ofs <- operandAdrOfs( instr );

   GR[r] <- zeroExtend( memLoad( seg, ofs, len ), 32 );
      
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
   STWC GR[a] ( GR[b] ), GR[r]        ; opMode 4, 5, 6
   STWC ofs ( GR[b] ), GR[r]          ; opMode 8 .. 15, 16 .. 23, 24 .. 31
   STWC ofs ( seg, GR[b] ), GR[r]     ; opMode 8 .. 15, 16 .. 23, 24 .. 31
```

```
    0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31
   :--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:
   : STWC   ( 0x1B ) : r         : 0      : opMode       : opArg                                   :
   :-----------------:-----------------------------------------------------------------------------:
```

#### Description

The STWC conditional instruction will store a value in "r" to the memory location specified by the operand address. The store is however only performed when the data location has not been written to since the last load reference instruction execution for that address. If the operation is successful a value of zero is returned otherwise a value of one. See the section on operand encoding for the defined operand mode encoding. 

#### Operation

```
   if ( opMode < 4 ) illegalInstructionTrap( );

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

## Immediate Instructions

Fixed length instruction word size architectures have one issue in that there is not enough room to embed a full word immediate value in the instruction. Typically, a combination of two instructions that concatenate the value from two fields is used. The **LDIL** instruction will place an 22-bit value in the left portion of a register, padded with zeroes to the right. The register content is then paired with an instruction that sets the right most 10-bit value. The **LDO** instruction that computes an offset is an example of such an instruction. 
The **ADDIL** instructions add the left side of a register argument to a register. In combination, the two instructions LIDL and ADDIL allow to generate a 32-bit offset value. 


<!--------------------------------------------------------------------------------------------------------->

<div style="page-break-before: always;"></div>

### LDIL

<hr>

Loads an immediate value left aligned into the target register.

#### Format

```
   LDIL <GR r>, <val>
```

```   
    0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31
   :--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:
   : LDIL   ( 0x02 ) : r         : val                                                             :
   :-----------------:-----------------------------------------------------------------------------:
```

#### Description

The load immediate left instruction loads a 22-bit immediate value left aligned into the general register "r", padded with zeroes on the right. 

#### Operation

```
   GR[r] = val << 10;
```

#### Exceptions

None.

#### Notes

The LDIL instruction will place an 22-bit value in the left portion of a register, padded with zeroes to the right. The register content is then paired with an instruction that sets the right most 10-bit value. The LDO instruction that computes an offset is an example of such an instruction. The instruction sequence

```
   LDIL  R10, L%val
   LDO   R2, R%val(R10)
```

will load the left hand side of the 32-bit value "val" into R10 and use the LOD instruction to add the right side part of "val"". The result is stored in R2. 

<!--------------------------------------------------------------------------------------------------------->

<div style="page-break-before: always;"></div>

### ADDIL

<hr>

Adds an immediate value left aligned into the target register.

#### Format

```
   ADDIL <GR r>, <val>
```

```
    0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31
   :--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:
   : ADDIL  ( 0x03 ) : r         : val                                                             :
   :-----------------:-----------------------------------------------------------------------------:
```

#### Description

The add immediate left instruction loads a 22-bit immediate value padded with zeroes on the right left aligned to the general register "r". Any overflows are ignored.

#### Operation

```
   GR[r] = GR[r] + ( val << 10 );
```

#### Exceptions

None.

#### Notes

The ADDIL instruction is typically used to produce a 32bit address offset in combination with the load and store instruction. The following example will use a 32-bit offset for loading a value into general register one. GR10 holds a logical address. The ADDIL instruction will add the left 22-bit portion padded with zeroes to the right to a register. The instruction sequence

```
   ADDIL  R1, R11, L%ofs
   LDW    R3, R%ofs(R11)
```

<!--------------------------------------------------------------------------------------------------------->

<div style="page-break-before: always;"></div>

### LDO

<hr>

Loads the effective address offset of the operand.

#### Format

```
   LDO GR[r], val                         : opMode 0  
   LDO GR[r], ofs ( GR[b] )               ; opMode 8 .. 15, 16 .. 23, 24 .. 31
   LDO GR[r], ofs ( SRn, GR[b] )          ; opMode 8 .. 15, 16 .. 23, 24 .. 31
```

```
    0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31
   :--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:
   : LDO    ( 0x1C ) : r         : 0      : opMode       : opArg                                   :
   :-----------------:-----------------------------------------------------------------------------:
```

#### Description

The LDO instruction loads an offset into general register "r". For operand modes 0 the immediate value is loaded. For operand modes 4 .. 7 and 8 .. 31, the instruction just performs the address computation part comparable to the **LDx** instruction, however with no memory access. See the section on the defined operand modes. Note that the does not matter whether the word, half-word or byte related operand mode is used for the address offset computation. In the assembler notation the "W", "H" and "B" option is therefore omitted.

#### Operation

```
   if (( opMode >= 1 ) && ( opMode <= 7 )) illegalInstructionTrap( );

   GR[r] <- operandAdrOfs( instr );
```

#### Exceptions

- Illegal Instruction Trap

#### Notes

The assembler uses the LDO instruction in mode zero for a "LDI r, val" pseudo instruction to load an immediate value into a general register.


<!--------------------------------------------------------------------------------------------------------->

<div style="page-break-before: always;"></div>

## Branch Instructions

The control flow instructions are divided into unconditional and conditional branch type instructions. The unconditional branch type instructions allow in addition to altering the control flow, the save of the return point to a general register. A subroutine call would for example compute the target branch address and store this address + 4 in that register. Upon subroutine return, there is a branch instruction to return via a general register content.

The **B** and **BL** instruction are the unconditional branch type instruction within a program segment. They add a signed offset to the current instruction address. The BL instruction additionally saves a return link in a general register. The **BR** and **BLR** instruction behave similar, except that a general register contains the instruction address relative offset. The **BV** and **BVR** are instruction segment absolute instructions. The branch address for the BV instruction is the segment relative value specified in a general register. The BVR instruction features two register, one being the segment relative base and the other a signed offset to be added.

The **BE** and **BLE** instruction are the inter-segment branches. The BE instruction branches to a segment absolute address encoded in the segment and general offset register. In addition, a signed offset encoded in the instruction can is added to form the target segment offset. The BLE instruction will in addition return the return address in the segment register SR0 and the offset on the general register GR0.

Segment relative and external branches potentially branch to pages with a different privilege level. The **GATE** instruction will promote the privilege level to the page where the GATE instruction resides. 

The conditional branch instructions combine an operation such as comparison or test with a local instruction address relative branch if the comparison or test condition is met. The **CBR** instruction will compare two general registers for a condition encoded in the instruction. If the condition is met, an instruction address relative branch is performed. The target address is formed by adding an offset encoded in the instruction to the instruction address. In a similar way, the **TBR** instruction will test a general register for a condition and perform an instruction relative branch if the condition is met. Conditional branches are implemented with a static branch prediction scheme. Forward branches are predicted not taken, backward branches are predicted taken. The decision is predicted during the fetch and decode stage and if predicted correctly will result in no pipeline penalty.


<!--------------------------------------------------------------------------------------------------------->

<div style="page-break-before: always;"></div>

### B

<hr>

Perform an unconditional IA-relative branch with a static offset.

#### Format

```
   B ofs
```

```
    0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31
   :--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:
   : B     ( 0x20 )  : 0         : ofs                                                             :
   :-----------------:-----------------------------------------------------------------------------:
```

#### Description

The branch instruction performs a branch to an instruction address relative location. The target address is shifted by 2 to the left and added to the current instruction offset. The virtual target address is built from the instruction address segment and offset. If code translation is disabled, the target address is the absolute physical address.

#### Operation

```
   IA-OFS <- IA-OFS + ( lowSignExt( ofs << 2 ), 22 );
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
   BL GR[r], ofs
```

```
    0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31
   :--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:
   : BL    ( 0x21 )  : r         : ofs                                                             :
   :-----------------:-----------------------------------------------------------------------------:
```

#### Description

The branch instruction performs a branch to an instruction address relative location. The target address is shifted by 2 to the left and added to the current instruction offset. The virtual target address is built from the instruction address segment and offset. If code translation is disabled, the target address is the absolute physical address. The current instruction address offset + 4 is returned in general register "r". If code translation is disabled, the return value is the absolute physical address.

#### Operation

```
   GR[r]  <- IA-OFS + 4;
   IA-OFS <- IA-OFS + ( lowSignExt( ofs << 2 ), 22 );
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
   BR GR[b]
```

```
    0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31
   :--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:
   : BR     ( 0x23 ) : 0                                                               : b         :
   :-----------------:-----------------------------------------------------------------------------:
```

#### Description

The branch register instruction performs an unconditional IA-relative branch. The target address is formed by adding the content of register "b" shifted by two bits to the left to the current instruction address. If code translation is disabled, the target address is the absolute physical address.

#### Operation

```
   IA-OFS <- IA-OFS + ( GR[b] << 2 );
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
      BL GR[r], GR[b]
```

```
    0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31
   :--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:
   : BLR    ( 0x24 ) : r         : 0                                                   : b         :
   :-----------------:-----------------------------------------------------------------------------:
```

#### Description

The branch register instruction performs an unconditional IA-relative branch. The target address is formed adding the content of register "b" shifted by two bits to the left to the current instruction address. If code translation is disabled, the target address is the absolute physical address. The current instruction address offset + 4 is returned in general register "r".

#### Operation

```
   GR[r]  <- IA-OFS, 4 ;
   IA-OFS <- IA-OFS + ( GR[b] << 4 );
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
   BV GR[b]
```

```
    0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31
   :--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:
   : BV     ( 0x25 ) : 0                                                               : b         :
   :-----------------:-----------------------------------------------------------------------------:
```

#### Description

The branch vectored instruction performs an unconditional branch to the target address in register in "b". Register "b" is interpreted as an instruction address offset in the current code segment. This unconditional jump allows to reach the entire code address range. If code translation is disabled, the resulting offset is the absolute physical address.

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
   BVR GR[b], GR[a]
```

```
    0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31
   :--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:
   : BVR    ( 0x26 ) : 0                                                   : a         : b         :
   :-----------------:-----------------------------------------------------------------------------:
```

#### Description

The branch vectored instruction performs an unconditional branch by adding the offset register in "a" shifted by two bits to the base address in register in "b". The result is interpreted as an instruction address in the current code segment. This unconditional jump allows to reach the entire code address range. If code translation is disabled, the resulting offset is the absolute physical address.

Since the BVR instruction is a segment base relative branch, a branch to page with a different privilege level is possible. A branch from a lower level to a higher level result in an instruction protection trap. A branch from a higher privilege to a lower privilege level result in the privilege level adjusted in the status register. Otherwise, the privilege level remains unchained.

#### Operation

```
   GR[r]  <- IA-OFS + 4;
   IA-OFS <- GR[b] + ( GR[a] << 2 );
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
   BE ofs ( SR[a], GR[b] )
```

```
    0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31
   :--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:
   : BE     ( 0x26 ) : ofs                                                 : a         : b         :
   :-----------------:-----------------------------------------------------------------------------:
```

#### Description

The branch external instruction branches to a segment relative location in another code segment. The target address is built from the segment register in field "a" and the base register "b" to which the sign extended offset is added. If code translation is disabled, the offset is the absolute physical address and the "a" field ignored.

Since the BE instruction is a segment base relative branch, a branch to page with a different privilege level is possible. A branch from a lower level to a higher level result in an instruction protection trap. A branch from a higher privilege to a lower privilege level result in the privilege level adjusted in the status register. Otherwise, the privilege level remains unchained.

#### Operation

```
   IA-SEG <- GR[a];
   IA-OFS <- GR[b] + lowSignExt(( ofs1 << 2 ), 20 );
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
   BLE ofs ( SR[a], GR[b] )
```

```
    0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31
   :--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:
   : BLE    ( 0x27 ) : ofs                                                 : a         : b         :
   :-----------------:-----------------------------------------------------------------------------:
```

#### Description

The branch and link external instruction branches to an absolute location in another code segment. The target address is built from the segment register in field "a" and the base register "b" to which the sign extended offset "ofs" is added. The return address is saved in SR0 and GR0. If code translation is disabled, the offset is the absolute physical address and the "a" field being ignored.

Since the BLE instruction is a segment base relative branch, a branch to page with a different privilege level is possible. A branch from a lower level to a higher level result in an instruction protection trap. A branch from a higher privilege to a lower privilege level result in the privilege level adjusted in the status register. Otherwise, the privilege level remains unchained.

#### Operation

```
   SR[0] <- IA-SEG;
   GR[0] <- IA-OFS + 4;

   IA-SEG <- GR[a];
   IA-OFS <- GR[b] + lowSignExt(( ofs << 2 ), 20 );
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
   : GATE   ( 0x28 ) : r         : ofs                                                             :
   :-----------------:-----------------------------------------------------------------------------:
```

#### Description

The GATE instruction computes the target address by adding "ofs" shifted by 2 bits to the left to the current instruction address offset. If code translation is enabled, the privilege level is changed to the privilege field in the TLB entry for the page from which the GATE instruction is fetched. The change is only performed when it results in a higher privilege level, otherwise the current privilege level remains. If code translation is disabled, the privilege level is set to zero. The resulting privilege level is deposited in the P bit of the GR "r". Execution continues at the target address with the new privilege level.

#### Operation

```
   GR[r] <- IA-OFS.[P];

   if ( ST.[C] ) {

      searchInstructionTlbEntry( seg, ofs, &entry );

      if ( entry.[PageType] == 3 ) priv <- entry.[PL1];
      else                         priv <- IA-OFS.[P];
   
   } else priv <- 0;

   IA-OFS     <- IA-OFS + signExt(( catImm( ofs1, ofs2 ) << 2 ), 24 );
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
   CBR [ .<cond> ] GR[a], GR[b], ofs
```

```
    0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31
   :--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:
   : CBR    ( 0x29 ) : cond   : ofs                                        : a         : b         :
   :-----------------:-----------------------------------------------------------------------------:
```

#### Description

The CBR instruction compares general registers "a" and "b" for the condition specified in the "cond" field. If the condition is met, the target branch address is the offset "ofs" shifted by two to the left and added to the current instruction address, otherwise execution continues with the next instruction. The conditional branch is predicted taken for a backward branch and predicted not taken for a forward branch.

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

   if ( res ) IA-OFS <- IA-OFS + lowSignExt(( ofs << 2 ), 17 );
   else       IA-OFS <- IA-OFS + 4;
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
   TBR [ .<cond> ] GR[b], ofs
```

```
    0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31
   :--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:
   : TBR    ( 0x2A ) : cond   : ofs                                        : 0         : b         :
   :-----------------:-----------------------------------------------------------------------------:
```

#### Description

The TBR instruction tests general register "b" for the condition specified in the "cond" field. If the condition is met, the target address is the offset shifted by two to the left and added to the current instruction address. Otherwise, execution continues with the next instruction. The conditional branch is predicted taken for a backward branch and predicted not taken for a forward branch.

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

   if ( res ) IA-OFS <- IA-OFS + lowSignExt(( ofs << 2 ), 17 );
   else       IA-OFS <- IA-OFS + 4;
```

#### Exceptions

None.

#### Notes

Often a test is followed by a branch in an instruction stream. VCPU-32 therefore features conditional branch instructions that both offer the combination of a register evaluation and branch depending on the condition specified.


<!--------------------------------------------------------------------------------------------------------->

<div style="page-break-before: always;"></div>

## Computational Instructions

The arithmetic, logical and bit field operations instruction represent the computation type instructions. Most of the computational instructions use the operand instruction format, where the operand fields, opMode and opArg, define one of the instruction operands. The computation instructions are divided into the numeric instructions **ADD** and **SUB**, the logical instructions **AND**, **OR**, **XOR** and the bit field operational instructions **EXTR**, **DEP** and **DSR**. The numeric and logical instructions encode the second operand in the operand field. This allows for immediate values, register values and values accessed via their logical address. The numeric instructions allow for a carry/borrow bit for implementing multi-precision operations as well as the distinction between signed and unsigned operation overflow detection traps. The logical instructions allow to negate the operand as well as the output. The bit field instructions will perform bit extraction and deposit as well as double register shifting. These instruction not only implement bit field manipulation, they are also the base for all shift and rotate operations.

The **CMP** instruction compares two values for a condition and returns a result of zero or one. The **SHLA** instruction is a combination of shifting an operand and adding a value to it. Simple integer multiplication with small values can elegantly be done with the help of this instruction. The LDI instruction allows to load a 16-bit value into the right or left half of a register. There are options to clear the register upfront, and to deposit the respective half without modifying the other half of the register. This way immediate values with a full 32-bit range can be constructed.

The **LDSID** instruction will return the segment id resulting from the operand mode and argument. 


<!--------------------------------------------------------------------------------------------------------->

<div style="page-break-before: always;"></div>

### ADD

<hr>

Adds the operand to the target register.

#### Format

```
   ADD[.<opt>]          GR[r>], val                    ; opMode 0
   ADD[.<opt>]          GR[r], GR[b]                   ; opMode 2
   ADD[(W|H|B)][.<opt>] GR[r], GR[a] ( GR[b] )         ; opMode 4 .. 6
   ADD[(W|H|B)][.<opt>] GR[r], ofs ( GR[b] )           ; opMode 8 .. 15, 16 .. 23, 24 .. 31
   ADD[(W|H|B)][.<opt>] GR[r], ofs ( SRn, GR[b] )      ; opMode 8 .. 15, 16 .. 23, 24 .. 31
```

```
    0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31
   :--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:
   : ADD    ( 0x10 ) : r         :C :L :O : opMode       : opArg                                   :
   :-----------------:-----------------------------------------------------------------------------:
```

#### Description

The instruction fetches the operand and adds it to the general register "r". If the C bit is set, the status register carry bit is added too. The "L" bit set specifies an unsigned add. If the "O" bit is set, a numeric overflow will cause an overflow trap. See the section on operand encoding for the defined operand modes. The operations will set the carry/borrow bits in the processor status word. 

#### Operation

```
   switch( opMode ) {

      case 0:        tmpA <- GR[r]; tmpB <- lowSignExtend( opArg, 14 ); break;
      case 1:        tmpA <- 0 ;    tmpB <- GR[b];                      break;
      case 2:        tmpA <- 0;     tmpB <- GR[b];                      break;

      case 8 .. 31:  seg <- operandAdrSeg( instr );
                     ofs <- operandAdrOfs( instr );
                     len <- operandBitLen( instr );

                     tmpA <- GR[r];
                     tmpB <- zeroExtend( memLoad( seg, ofs, len ), 32 - len ); 
                     break;

      default: illegalInstructionTrap( );
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
   SUB[.<opt>]          GR[r>], val                    ; opMode 0
   SUB[.<opt>]          GR[r], GR[b]                   ; opMode 2
   SUB[(W|H|B)][.<opt>] GR[r], GR[a] ( GR[b] )         ; opMode 4 .. 6
   SUB[(W|H|B)][.<opt>] GR[r], ofs ( GR[b] )           ; opMode 8 .. 15, 16 .. 23, 24 .. 31
   SUB[(W|H|B)][.<opt>] GR[r], ofs ( SRn, GR[b] )      ; opMode 8 .. 15, 16 .. 23, 24 .. 31
```

```
    0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31
   :--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:
   : SUB    ( 0x11 ) : r         :C :L :O : opMode       : opArg                                   :
   :-----------------:-----------------------------------------------------------------------------:
```

#### Description

The instruction fetches the operand and subtracts it from the general register "r". If the "C" bit is set, the status register carry bit is added too. The "L" bit specifies that his is an unsigned subtraction. If the "O" bit is set, a numeric overflow will cause an overflow trap. See the section on operand encoding for the defined operand modes. The operations will set the carry/borrow bits in the processor status word.

#### Operation

```
     switch( opMode ) {

      case 0:        tmpA <- GR[r]; tmpB <- lowSignExtend( opArg, 14 ); break;
      case 1:        tmpA <- 0 ;    tmpB <- GR[b];                      break;
      case 2:        tmpA <- 0;     tmpB <- GR[b];                      break;

      case 8 .. 31:  seg <- operandAdrSeg( instr );
                     ofs <- operandAdrOfs( instr );
                     len <- operandBitLen( instr );

                     tmpA <- GR[r];
                     tmpB <- zeroExtend( memLoad( seg, ofs, len ), 32 - len ); 
                     break;

      default: illegalInstructionTrap( );
   }

   if ( instr.[C] ) res <- tmpA - tmpB + instr.[C];
   else res <- tmpA - tmpB;

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

### AND

<hr>

Performs a bitwise AND of the operand and the target register and stores the result into the target register.

#### Format

```
   AND[.<opt>]          GR[r>], val                    ; opMode 0
   AND[.<opt>]          GR[r], GR[b]                   ; opMode 2
   AND[(W|H|B)][.<opt>] GR[r], GR[a] ( GR[b] )         ; opMode 4 .. 6
   AND[(W|H|B)][.<opt>] GR[r], ofs ( GR[b] )           ; opMode 8 .. 15, 16 .. 23, 24 .. 31
   AND[(W|H|B)][.<opt>] GR[r], ofs ( SRn, GR[b] )      ; opMode 8 .. 15, 16 .. 23, 24 .. 31
```

```
    0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31
   :--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:
   : AND    ( 0x12 ) : r         :N :C :0 : opMode       : opArg                                   :
   :-----------------:-----------------------------------------------------------------------------:
```

#### Description

The instruction fetches the data specified by the operand and performs a bitwise AND of the general register "r" and the operand fetched. The result is stored in general register "r". The "N" bit allows to negate the result making the AND a NAND operation. The "C" bit allows to complement ( 1's complement ) the operand input, which is the "b" register. See the section on operand encoding for the defined operand modes.

#### Operation

```
   switch( opMode ) {

      case 0:        tmpA <- GR[r]; tmpB <- lowSignExtend( opArg, 14 ); break;
      case 1:        tmpA <- 0 ;    tmpB <- GR[b];                      break;
      case 2:        tmpA <- 0;     tmpB <- GR[b];                      break;

      case 8 .. 31:  seg <- operandAdrSeg( instr );
                     ofs <- operandAdrOfs( instr );
                     len <- operandBitLen( instr );

                     tmpA <- GR[r];
                     tmpB <- zeroExtend( memLoad( seg, ofs, len ), 32 - len ); 
                     break;

      default: illegalInstructionTrap( );
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

Complementing the operand input allows to perform a bit clear in a register word by complementing the bit mask stored in the operand before performing the AND. Typically this is done in a program in two steps, which are first to complement the mask and then AND to the target variable. The C option allows to do this more elegantly in one step. 


<!--------------------------------------------------------------------------------------------------------->

<div style="page-break-before: always;"></div>

### OR

<hr>

Performs a bitwise OR of the operand and the target register and stores the result into the target register.

#### Format

```
   OR[.<opt>]          GR[r>], val                    ; opMode 0
   OR[.<opt>]          GR[r], GR[b]                   ; opMode 2
   OR[(W|H|B)][.<opt>] GR[r], GR[a] ( GR[b] )         ; opMode 4 .. 6
   OR[(W|H|B)][.<opt>] GR[r], ofs ( GR[b] )           ; opMode 8 .. 15, 16 .. 23, 24 .. 31
   OR[(W|H|B)][.<opt>] GR[r], ofs ( SRn, GR[b] )      ; opMode 8 .. 15, 16 .. 23, 24 .. 31
```

```
    0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31
   :--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:
   : OR     ( 0x13 ) : r         :N :C :0 : opMode       : opArg                                   :
   :-----------------:-----------------------------------------------------------------------------:
```

#### Description

The instruction fetches the data specified by the operand and performs a bitwise OR of the general register "r" and the operand fetched. The result is stored in general register "r". The N bit allows to negate the result making the OR a NOR operation. The C bit allows to complement ( 1's complement ) the operand input, which is the regB register. See the section on operand encoding for the defined operand modes.

#### Operation

```
   switch( opMode ) {

      case 0:        tmpA <- GR[r]; tmpB <- lowSignExtend( opArg, 14 ); break;
      case 1:        tmpA <- 0 ;    tmpB <- GR[b];                      break;
      case 2:        tmpA <- 0;     tmpB <- GR[b];                      break;

      case 8 .. 31:  seg <- operandAdrSeg( instr );
                     ofs <- operandAdrOfs( instr );
                     len <- operandBitLen( instr );

                     tmpA <- GR[r];
                     tmpB <- zeroExtend( memLoad( seg, ofs, len ), 32 - len ); 
                     break;

      default: illegalInstructionTrap( );
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

Using operand mode one will result in OR-ing a zero with general register "b", which is a copy of general register "b" to the general register "r".


<!--------------------------------------------------------------------------------------------------------->

<div style="page-break-before: always;"></div>

### XOR

<hr>

Performs a bitwise XORing the operand and the target register and stores the result into the target register.

#### Format

```
   XOR[.<opt>]          GR[r>], val                    ; opMode 0
   XOR[.<opt>]          GR[r], GR[b]                   ; opMode 2
   XOR[(W|H|B)][.<opt>] GR[r], GR[a] ( GR[b] )         ; opMode 4 .. 6
   XOR[(W|H|B)][.<opt>] GR[r], ofs ( GR[b] )           ; opMode 8 .. 15, 16 .. 23, 24 .. 31
   XOR[(W|H|B)][.<opt>] GR[r], ofs ( SRn, GR[b] )      ; opMode 8 .. 15, 16 .. 23, 24 .. 31
```

```
    0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31
   :--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:
   : XOR    ( 0x14 ) : r         :N :C :0 : opMode       : opArg                                   :
   :-----------------:-----------------------------------------------------------------------------:
```

#### Description

The instruction fetches the data specified by the operand and performs a bitwise XOR of the general register "r" and the operand fetched. The result is stored in general register "r". The "N" bit allows to negate the result making the OR a XNOR operation. The "C" bit allows to complement ( 1's complement ) the operand input, which is the "b"" register. See the section on operand encoding for the defined operand modes.

#### Operation

```
   switch( opMode ) {

      case 0:        tmpA <- GR[r]; tmpB <- lowSignExtend( opArg, 14 ); break;
      case 1:        tmpA <- 0 ;    tmpB <- GR[b];                      break;
      case 2:        tmpA <- 0;     tmpB <- GR[b];                      break;

      case 8 .. 31:  seg <- operandAdrSeg( instr );
                     ofs <- operandAdrOfs( instr );
                     len <- operandBitLen( instr );

                     tmpA <- GR[r];
                     tmpB <- zeroExtend( memLoad( seg, ofs, len ), 32 - len ); 
                     break;

      default: illegalInstructionTrap( );
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
- Alignment trap

#### Notes

None.


<!--------------------------------------------------------------------------------------------------------->

<div style="page-break-before: always;"></div>

### CMP

<hr>

Compares a register and an operand and stores the comparison result in the target register.

#### Format

```
   CMP[.<opt>]          GR[r>], val                     ; opMode 0
   CMP[.<opt>]          GR[r], GR[b]                    ; opMode 2
   CMP[(W|H|B)][.<cond>] GR[r], GR[a] ( GR[b] )         ; opMode 4 .. 6
   CMP[(W|H|B)][.<cond>] GR[r], ofs ( GR[b] )           ; opMode 8 .. 15, 16 .. 23, 24 .. 31
   CMP[(W|H|B)][.<cond>] GR[r], ofs ( SRn, GR[b] )      ; opMode 8 .. 15, 16 .. 23, 24 .. 31
```

```
    0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31
   :--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:
   : CMP    ( 0x15 ) : r         : cond   : opMode       : opArg                                   :
   :-----------------:-----------------------------------------------------------------------------:
```

#### Description

The compare instruction will compare two operands for the condition specified in the "cond" field. A value of one is stored in "r" when the condition is met, otherwise a zero value is stored. A typical code pattern would be to issue the compare instruction with the comparison result in general register "r", followed by a conditional branch instruction. See the section on operand encoding for the defined operand modes.

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

      case 0:        tmpA <- GR[r]; tmpB <- lowSignExtend( opArg, 14 ); break;
      case 1:        tmpA <- 0 ;    tmpB <- GR[b];                      break;
      case 2:        tmpA <- 0;     tmpB <- GR[b];                      break;

      case 8 .. 31:  seg <- operandAdrSeg( instr );
                     ofs <- operandAdrOfs( instr );
                     len <- operandBitLen( instr );

                     tmpA <- GR[r];
                     tmpB <- zeroExtend( memLoad( seg, ofs, len ), 32 - len ); 
                     break;

      default: illegalInstructionTrap( );
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

Test a general register for a condition and conditionally move a register value to the target register.

#### Format

```
   CMR [ .<cond> ] GR[r],  GR[b],  GR[a]
   CMR [ .<cond> ] GR[r],  GR[b], val
```

```
    0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31
   :--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:
   : CMR    ( 0x08 ) : r         : cond   : 0                              : a         : b         :
   :-----------------:-----------------------------------------------------------------------------:
```

#### Description

The conditional move instruction will test register "b" for the condition. If the condition is met, general register "a" is moved to "r". 


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

The CMR instruction is a rather specialized instruction. It highly depends on a good peephole optimizer to detect such a situation.


<!--------------------------------------------------------------------------------------------------------->

<div style="page-break-before: always;"></div>

### EXTR

<hr>

Performs a bit field extract from a general register and stores the result in the targetReg.

#### Format

```
   EXTR [ .<opt> ] GR[r], GR[b], pos, len
```

```
    0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31
   :--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:
   : EXTR   ( 0x04 ) : r         :S :A : 0               : len          : pos          : b         :
   :-----------------:-----------------------------------------------------------------------------:
```

#### Description

The instruction performs a bit field extract specified by the position and length instruction data from general register "b". The "pos" field specifies the rightmost bit of the bitfield to extract. It is encoded as 31- pos. The "len" field specifies the bit size if the field to extract. The extracted bit field is stored right justified in the general register "r". If set, the "S" bit allows to sign extend the extracted bit field. If the "A" bit is set, the shift amount control register is used for obtaining the position value.

#### Operation

```
   if ( instr.[A] ) tmpPos <- SHAMT.[27..31];
   else tmpPos <- pos;

   GR[r] <= extract( GR[b], pos, len );

   if ( instr.[S] ) signExtend( GR[r], len );
   else zeroExtend( GR[r], len );
```

#### Exceptions

None.

#### Notes

The VCPU-32 instruction set does not have dedicated instructions for left and right shift operations. They can easily be realized with the EXTR and DEP instructions. Refer to the chapter for pseudo instructions.


<!--------------------------------------------------------------------------------------------------------->

<div style="page-break-before: always;"></div>

### DEP

<hr>

Performs a bit field deposit of the value extracted from a bit field in reg "B" and stores the result in the targetReg.

#### Format

```
   DEP [ .<opt> ] GR[r], GR[b], pos, len
   DEP [ .<opt> ] GR[r], val, pos, len
```

```
       0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31
      :--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:
      : DEP    ( 0x05 ) : r         :Z :A :I : 0            : len          : pos          : b         :
      :-----------------:-----------------------------------------------------------------------------:
```

#### Description

The instruction extracts the right justified bit field of length "len" in general register "b" and deposits this field in the general register "r" at the specified position. The "pos" field specifies the rightmost bit for the bit field to deposit. It is encoded as 31- pos. The "len" field specifies the bit size if the field to extract. The extracted bit field is stored right justified in the general register "r". The Z bit clears the target register "r" before storing the bit field, the I bit specifies that the instruction bits 28..31 contain an immediate value instead of a register. If the "A" bit is set, the shift amount control register is used for obtaining the position value.

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

None.

#### Notes

The VCPU-32 instruction set does not have dedicated instructions for left and right shift operations. They can easily be realized with the EXTR and DEP instructions. Refer to the chapter for pseudo instructions.


<!--------------------------------------------------------------------------------------------------------->

<div style="page-break-before: always;"></div>

### DSR

<hr>

Performs a right shift of two concatenated registers for shift amount bits and stores the result in the target register.

#### Format

```
   DSR [ .<opt> ] GR[r], GR[b], GR[a], shAmt
```

```
    0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31
   :--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:
   : DSR    ( 0x06 ) : r         :0 :A : 0               : shamt        :0 : a         : b         :
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
   SHLA [ .<opt> ] GR[r], GR[a], GR[b], smAmt
   SHLA [ .<opt> ] GR[r], GR[a], val, smAmt
```

```
    0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31
   :--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:
   : SHLA   ( 0x07 ) : r         :Z :L :O : 0                     : sa  :0 : a      : b            :
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
   : LSID   ( 0x01 ) : r         : 0                                                   : b         :
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

The system control instructions are intended for the runtime designer to control the CPU resources such as TLB, Cache and so on. There are instruction to load a segment or control registers as well as instructions to store a segment or control register. The TLB and the cache modules are controlled by instructions to insert and remove data in these two entities. Finally, the interrupt and exception system needs a place to store the address and instruction that was interrupted as well as a set of shadow registers to start processing an interrupt or exception. Most of the instructions found in this group require that the processor runs in privileged mode.

The **MR** instructions is used to transfer data to and from a segment register or a control register. The processor status instruction **MST** allows for setting or clearing an individual an accessible bit of the status word. The **LDPA** and **PRB** instructions are used to obtain information about the virtual memory system. The LDPA instruction returns the physical address of the virtual page, if present in memory. The PRB instruction tests whether the desired access mode to an address is allowed.

The TLB and Caches are managed by the **ITLB**, **PTLB** and **PCA** instruction. The ITLB and PTLB instructions insert into the TLB and removes translations from the TLB. The PCA instruction manages the instruction and data cache and features to flush and / or just purge a cache entry. There is no instruction that inserts data into a cache as this is completely controlled by hardware. The **RFI** instruction is used to restore a processor state from the control registers holding the interrupted instruction address, the instruction itself and the processor status word. Optionally, general registers that have been stored into the reserved control registers will be restored.

The **DIAG** instruction is a control instructions to issue hardware specific implementation commands. Example is the memory barrier command, which ensures that all memory access operations are completed after the execution of this DIAG instruction.  The **BRK** instruction is transferring control to the breakpoint trap handler.


<!--------------------------------------------------------------------------------------------------------->

<div style="page-break-before: always;"></div>

### MR

<hr>

Copy data from or to a segment or control register.

#### Format

```
   MR targetReg, sourceReg
```

```
    0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31
   :--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:
   : MR     ( 0x09 ) : r         :Z :D :M : 0                                    : s               :
   :-----------------:-----------------------------------------------------------------------------:
```

#### Description

The move register instruction MR copies data from a segment or control register "s" to a general register "r" and vice versa. If the "D" bit is set, the copy direction is from "r" to a segment or control register in "s", otherwise from "s" to "r". The "M" bit indicates whether a segment register or a control register is moved. The "Z" option uses a zero value for the source. Setting a value for a privileged segment or control register is a privileged operation.

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
   
   } else {

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
   MST GR[b]
   MST.S <val>
   MST.C >val>
```

```
    0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31
   :--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:
   : MST    ( 0x0A ) : r         :mode : 0                                       : val / b         :
   :-----------------:-----------------------------------------------------------------------------:
```

#### Description

The MST instruction sets the status bits 26.. 31 of the processor status word. The previous bit settings are returned in "r". There are three modes. Mode 0 will replace the user status bits 26..31 with the bits in the general register "b". Mode 1 and 2 will interpret the 6-bit field bits 26..31 as the bits to set or clear with a bit set to one will perform the set or clear operation. Modifying the status register is a privileged instruction. 

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
   LDPA GR[r], ( GR[b] )
   LDPA GR[r], ( SR[a], GR[b] )
```

```
    0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31
   :--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:
   : LDPA   ( 0x30 ) : r         : seg : 0                                 : a         : b         :
   :-----------------:-----------------------------------------------------------------------------:
```

#### Description

The LDPA instruction returns the physical address for a logical or virtual address encoded in "a" and "b" into register "r". The "L" bit will specify whether the address is a full virtual address with the space in "a" and the offset in "b". The "seg" field selects the segment register. A value of zero indicates a logical address and the the upper two bits of "b" select SR4 to SR7. A value of 1 to 3 select SR1 to SR3. If the virtual address is not in main memory, a zero result is returned. The result is ambiguous for a virtual address that translates to a zero address. LDPA is a privileged operation.

#### Operation

```
   if ( ! ST.[ PRIV ] ) privilegedOperationTrap( );

   GR[r] <- loadPhysAdr( SR[ segSelect( GR[b] )], GR[b] );
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
   PRB <opt> GR[r], ( GR[b] )
   PRB <opt> GR[r], ( SR[a],GR[b] )
```

```
    0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31
   :--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:
   : PRB    ( 0x31 ) : r         :seg  :M : 0                              : a         : b         :
   :-----------------:-----------------------------------------------------------------------------:
```

#### Description

The PRB instruction determines whether data access at the requested privilege level stored in the general register "r" is allowed. If the probe operation succeeds, a value of one is returned in GR "r", otherwise a zero is returned. The "M" bit specifies whether a read or write access is requested. A value of zero is a read access, a value of one a read/write access. The "seg" field selects the segment register. A value of zero indicates a logical address and the the upper two bits of "b" select SR4 to SR7. A value of 1 to 3 select SR1 to SR3. If the protection ID is enabled in the processor status word, the protection ID is checked as well. The instruction performs the necessary virtual to physical data translation regardless of the processor status bit for data translation.

#### Operation

```
   if ( ! searchDataTlbEntry( SR[ segSelect( seg, GR[b] )], GR[b], &entry )) {

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
   ITLB <opt> GR[r], ( SR[a], GR[b] )
```

```
    0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31
   :--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:
   : ITLB   ( 0x32 ) : r         :T :M : 0                                 : a         : b         :
   :-----------------:-----------------------------------------------------------------------------:
```

#### Description

The ITLB instruction inserts a translation into the instruction or data TLB. The virtual address is encoded in "a" for the segment register and "b" for the offset. The data to be inserted is grouped into two steps. The first step will enter the virtual address and physical address is associated with. The second step will enter the access rights and protection information and marks the entry valid. The TLB entry is set to invalid and the address fields are filled. The "T" bit specifies whether the instruction or the data TLB is addressed. A value of zero references the instruction TLB, a value of one refers to the data TLB.

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

   if ( instr.[T] )
      if ( ! searchDataTlbEntry( SR[a], GR[b], &entry )) allocateDataTlbEntry( SR[a], GR[b], &entry );
   else
      if ( ! searchInstructionTlbEntry( SR[a], GR[b],, &entry )) allocateInstructionTlbEntry( SR[a], GR[b], &entry );

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
   PTLB <opt> ( GR[b] )
   PTLB <opt> ( SR[a], GR[b] )
```

```
    0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31
   :--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:
   : PTLB   ( 0x33 ) : 0         :T : 0                                    : a         : b         :
   :-----------------:-----------------------------------------------------------------------------:
```

#### Description

The DTLB instruction removes a translation from the instruction or data TLB by marking the entry invalid. The virtual address is encoded in "a" for the segment register and "b" for the offset. The "T" bit indicates whether the instruction or the data TLB is addressed. A value of zero references the instruction TLB, a value of one refers to the data TLB. 


#### Operation

```
   if ( ! ST.[ PRIV ] ) privilegedOperationTrap( );

   if ( instr.[T] )
      if ( searchDataTlbEntry( SR[a], GR[b], &entry )) purgeDataTlbEntry( SR[a], GR[b], &entry );
   else
      if ( searchInstructionTlbEntry( SR[a], GR[b], &entry )) purgeInstructionTlbEntry( SR[a], GR[b], &entry );
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
   PCA <opt> ( GR[b] )
   PCA <opt> ( SR[a], GR[b] )
```

```
       0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31
   :--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:
   : PCA    ( 0x34 ) : 0         :T :F : 0                                 : a         : b         :
   :-----------------:-----------------------------------------------------------------------------:
```

#### Description

The PCA instruction flushes or purges a cache line from the instruction or data cache. The virtual address is encoded in "a" for the segment register and "b" for the offset. An instruction cache line can only be purged, a data cache line can also be written back to memory and then optionally purged. A flush operation is only performed when the cache line is dirty. The "T" bit indicates whether the instruction or the data cache is addressed. A value of zero references the instruction cache. The "F" bit will indicate whether the data cache is to be purged without flushing it first to memory. If "F" is zero, the entry is first flushed and then purged, else just purged. The "F" bit has no meaning for an instruction cache.

#### Operation

```  
   if ( instr.[T] ) {

      if ( ! instr.[F] ) flushDataCache( SR[a], GR[b] );
      purgeDataCache( SR[a], GR[b] );
   
   } else purgeInstructionCache( SR[a], GR[b] );
```

#### Exceptions

- privileged operation trap
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
   : RFI    ( 0x3F ) : 0                                                                           :
   :-----------------:-----------------------------------------------------------------------------:
```

#### Description

The RFI instruction restores the instruction address segment, instruction address offset and the processor status register from the control registers I-IA-SEG, I-IA-OFS and I-STAT.

// ??? **note** perhaps an option to also restore some shadow regs ? ( GR and SRs ? )

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
   : DIAG   ( 0x35 ) : r         :  0                                      : a         : b         :
   :-----------------:-----------------------------------------------------------------------------:
```

#### Description

The DIAG instruction sends a command to an implementation hardware specific components. The instruction accepst two arguments in the "a" and "b" field and returns a result in "r".

// ??? **note** under construction... as we go along with hardware. In general, we should use "a" and "b" for passing arguments and "r" for a return status.

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
   : BRK    ( 0x00 ) : info1     : 0               : info2                                         :
   :-----------------:-----------------------------------------------------------------------------:
```

#### Description

The BRK instruction raises a debug breakpoint trap and enters the debug trap handler. The "info1" and "info2" field are passed to the debug subsystem.

#### Operation

```
   debugBreakpointTrap( info1, info2 );
```

#### Exceptions

- debug breakpoint trap

#### Notes

The instruction opCode for BRK is the opCode value of zero. A zero instruction word result is a BRK #0 instruction which raises a trap.


<!--------------------------------------------------------------------------------------------------------->

<div style="page-break-before: always;"></div>

## Pseudo Instructions

The instruction set allows for a rich set of options on the individual instruction functions. Setting a defined option bit in the instruction adds useful capabilities to an instruction with little additional overhead to the overall data path. For better readability, pseudo operations could be defined that allows for an easier usage. Furthermore, some instructions have no assembler format counterpart. The only way to execute them is through the pseudo operation. This applies for example to the operand mode one case in computational instructions. For pseudo instructions, options and traps are those specified by the actually used instructions.

| Pseudo Instruction | Assembler Syntax |  Possible Implementation | Purpose |
|:---|:---|:---|:---|
| **NOP** | NOP | OR  GR0, #0 | There are many instructions that can be used for a NOP. The idea is to pick one that does not affect the prgram state. |
| **LDI** | LDI val | | |
| **CLR** | CLR GRn | OR  GRn, #0 | Clears a general register. There are many instructions that can be used for a CLR. |
| **MR** | MR GRx, GRy | OR GRx, GRy ; using operand mode 1 | Copies a general register to another general register. This overlaps MR for using two general registers. |
| **ASR** | ASR GRx, sa | EXTR.S Rx, Rx, 31 - sa, 32 - sa | For the right shift operations, the shift amount is transformed into the bot position of the bit field and the length of the field to shift right. For arithmetic shifts, the result is sign extended. |
| **LSR** | LSR GRx, sa | EXTR   Rx, Rx, 31 - sa, 32 - sa | For the right shift operations, the shift amount is transformed into the bot position of the bit field and the length of the field to shift right. |
| **LSL** | LSR GRx, sa | DEP.Z  Rx, Rx, 31 - sa, 32 - sa | |
| **ROL** | ROL GRx, GRy, cnt | DSR Rx, Rx, cnt | |
| **ROR** | ROR GRx, GRy, cnt | DSR Rx, Rx, 32 - cnt | |
| **INC** | INC [ .<opt> ] GRx, val | ADD [ .<opt> ] GR x, val | |
| **DEC** | DEC [ .<opt> ] GRx, val | SUB [ .<opt> ] GR x, val | |
| **NEG** | NEG Grx | SUB [ .<opt> ] GRx, GRx, opMode 1 | |
| **COM** | COM Grx | OR.N  GRx, GRx, opMode 1 | |


<!--------------------------------------------------------------------------------------------------------->

<div style="page-break-before: always;"></div>

## Instruction Set Summary

This appendix lists all instructions by instruction group.

### Computational Instructions

```
    0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31
   :--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:
   : ADD     ( 0x10 ): r         :C :L :O : opMode       : opArg                                   :
   :-----------------:-----------------------------------------------------------------------------:
   : SUB     ( 0x11 ): r         :C :L :O : opMode       : opArg                                   :
   :-----------------:-----------------------------------------------------------------------------:
   : AND     ( 0x12 ): r         :N :C :0 : opMode       : opArg                                   :
   :-----------------:-----------------------------------------------------------------------------:
   : OR      ( 0x13 ): r         :N :C :0 : opMode       : opArg                                   :
   :-----------------:-----------------------------------------------------------------------------:
   : XOR     ( 0x14 ): r         :N :C :0 : opMode       : opArg                                   :
   :-----------------:-----------------------------------------------------------------------------:
   : CMP     ( 0x15 ): r         : cond   : opMode       : opArg                                   :
   :-----------------:-----------------------------------------------------------------------------:
```

```
    0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31
   :--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:
   : LSID    ( 0x01 ): r         : b         : 0                                                   :
   :-----------------:-----------------------------------------------------------------------------:
   : EXTR    ( 0x04 ): r         :S :A :0               : len          : pos           : b         :
   :-----------------:-----------------------------------------------------------------------------:
   : DEP     ( 0x05 ): r         :Z :A :I :             : len          : pos           : b         :
   :-----------------:-----------------------------------------------------------------------------:
   : DSR     ( 0x06 ): r         : 0                    : shamt        :0 : a          : b         : 
   :-----------------:-----------------------------------------------------------------------------:
   : SHLA    ( 0x07 ): r         :I :L :O : 0                     : sa  :0 : a         : b         : 
   :-----------------:-----------------------------------------------------------------------------:
   : CMR     ( 0x08 ): r         :cond    : 0                              : a         : b         : 
   :-----------------:-----------------------------------------------------------------------------:
```

### Memory Reference Instruction

```
    0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31
   :--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:
   : LDW/H/B ( 0x18 ): r         : 0      : opMode       : opArg                                   :
   :-----------------:-----------------------------------------------------------------------------:
   : STW/H/B ( 0x19 ): r         : 0      : opMode       : opArg                                   :
   :-----------------:-----------------------------------------------------------------------------:
   : LDWR    ( 0x1A ): r         : 0      : opMode       : opArg                                   :
   :-----------------:-----------------------------------------------------------------------------:
   : STWC    ( 0x1B ): r         : 0      : opMode       : opArg                                   :
   :-----------------:-----------------------------------------------------------------------------:
```

### Immediate Instructions

```
   :-----------------:-----------------------------------------------------------------------------:
   : LDIL    ( 0x02 ): r         : val                                                             :
   :-----------------:-----------------------------------------------------------------------------:
   : ADDIL   ( 0x03 ): r         : val                                                             :
   :-----------------:-----------------------------------------------------------------------------:
   : LOD     ( 0x16 ): r        : 0       : opMode       : opArg                                   :   
   :-----------------:-----------------------------------------------------------------------------:
```

### Absolute Memory Reference Instructions

```
    0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31
   :--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:
   : LDWA    ( 0x20 ): r         : ofs                                                 : b         : 
   :-----------------:-----------------------------------------------------------------------------:
   : LDWAX   ( 0x21 ): r         : 0                                       : a         : b         : 
   :-----------------:-----------------------------------------------------------------------------:
   : STWA    ( 0x1C ): r         : ofs                                                 : b         :
   :-----------------:-----------------------------------------------------------------------------:
```

### Control Flow Instructions

```
    0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31
   :--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:
   : B       ( 0x20 ): 0         : ofs                                                             :
   :-----------------:-----------------------------------------------------------------------------:
   : BL      ( 0x21 ): r         : ofs                                                             :
   :-----------------:-----------------------------------------------------------------------------:
   : GATE    ( 0x28 ): r         : ofs                                                             :
   :-----------------:-----------------------------------------------------------------------------:
   : BR      ( 0x22 ): 0                                                               : b         :
   :-----------------:-----------------------------------------------------------------------------:
   : BLR     ( 0x23 ): r         : 0                                                   : b         :
   :-----------------:-----------------------------------------------------------------------------:
   : BV      ( 0x24 ): 0                                                               : b         :
   :-----------------:-----------------------------------------------------------------------------:
   : BVR     ( 0x25 ): 0                                                   : a         : b         :
   :-----------------:-----------------------------------------------------------------------------:
   : BE      ( 0x26 ): ofs                                                 : a         : b         :
   :-----------------:-----------------------------------------------------------------------------:
   : BLE     ( 0x27 ): ofs                                                 : a         : b         :      
   :-----------------:-----------------------------------------------------------------------------:
   : CBR     ( 0x29 ): cond   : ofs                                        : a         : b         : 
   :-----------------:-----------------------------------------------------------------------------:
   : TBR     ( 0x2A ): cond   : ofs                                        : 0         : b         :
   :-----------------:-----------------------------------------------------------------------------:
```

### System Control Instructions

```
    0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31
   :--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:
   : MR      ( 0x09 ): r         :Z :D :M : 0                                    : s               :
   :-----------------:-----------------------------------------------------------------------------:
   : MST     ( 0x0A ): r         :mode : 0                                       : s               :
   :-----------------:-----------------------------------------------------------------------------:
   : LDPA    ( 0x30 ): r         :seg  : 0                                 : a         : b         :         
   :-----------------:-----------------------------------------------------------------------------:
   : PRB     ( 0x31 ): r         :seg  :R : 0                              : a         : b         :         
   :-----------------:-----------------------------------------------------------------------------:
   : ITLB    ( 0x32 ): r         :T :M : 0                                 : a         : b         :
   :-----------------:-----------------------------------------------------------------------------:
   : PTLB    ( 0x33 ): 0         :T : 0                                    : a         : b         :      
   :-----------------:-----------------------------------------------------------------------------:
   : PCA     ( 0x34 ): 0         :T :F : 0                                 : a         : b         :     
   :-----------------:-----------------------------------------------------------------------------:
   : DIAG    ( 0x35 ): r         : 0                                       : a         : b         :
   :-----------------:-----------------------------------------------------------------------------:
   : RFI     ( 0x3F ): 0                                                                           :
   :-----------------:-----------------------------------------------------------------------------:
   : BRK     ( 0x00 ): info1     : 0                    : info2                                    :
   :-----------------:-----------------------------------------------------------------------------:
```

<!--------------------------------------------------------------------------------------------------------->

<div style="page-break-before: always;"></div>

## TLB and Cache Models

A key part of the CPU is a cache and a TLB mechanism. The caches bridge the performance gap between a main memory and the CPU processing elements. In modern CPUs, there is even a hierarchy of cache layers. In addition, a virtual memory system needs a way to translate a virtual address to a physical address on each instruction. The translation look-aside buffers are therefore an indispensable component of such systems. Not surprisingly, VCPU-32 has caches and TLBs too.

### Instruction and Data L1 Cache

The pipeline design makes a reference to memory during instruction fetch and then optional data access. Since both operations potentially take place  for different instructions but in the same cycle, a separate **instruction cache** and **data cache** is a key part of the overall architecture. These two caches are called **L1 caches**.

- direct mapped model
- set associative model

### Unified L2 Cache

In addition to the L! caches, there could be a joint L2 cache to serve both L1 caches.

- instruction L1 cache requests have priority over L2 data requests
- 

### Instruction to manage caches


### Instruction and Data TLBs

Computers with virtual addressing simply cannot work without a **translation look-aside buffer** (TLB). For each instruction using a virtual address for instruction fetch and data access a translation to the physical memory address needs to be performed. The TLB is a kind of cache for translations and comparable to the data caches, separate instruction and data TLBs are the common implementation.

### Separate instruction and data TLB

- essentially like a cache
- direct mapped model
- set associative model

### Joint TLBs

- models with a small fully associative I-TLB fed from a joint TLB for code and data pages. 

### Instructions to manage TLBs


<!--------------------------------------------------------------------------------------------------------->

<div style="page-break-before: always;"></div>

## VCPU-32 Runtime Environment


### Registers

### Stack Frame

### Parameter passing

### Local Calls

### External Calls

### Privilege level changes

### Traps and Interrupt handling

### Debug

### Startup sequence

<!--------------------------------------------------------------------------------------------------------->

<div style="page-break-before: always;"></div>

### A pipelined CPU

The first VCPU-32 implementation uses a three stage pipeline model. There stages are the **instruction fetch and decode stage**, the **memory access** stage and the **execute** stage. This section gives a brief overview on the pipelining considerations using the three-stage model. The architecture does not demand that particular model. It is just the first implementation of VCPU-32. The typical challenges such as structural hazards and data hazards will be identified and discussed.

- **Instruction fetch and decode**. The first stage will fetch the instruction word from memory and decode it. There are two parts. The first part of the stage will use the instruction address and attempt to fetch the instruction word from the instruction cache. At the same time the translation look-aside buffer will be checked whether the virtual to physical translation is available and if so whether the access rights match. The second part of the stage will decode the instruction and also read the general registers from the register file.

- **Memory access**. The memory access stage will take the instruction decoded in the previous stage and compute the address for the memory data access. This also the stage where any segment or control register are accessed. In addition, unconditional branches are handled at this stage. Memory data item are read or stored depending on the instruction. Due to the nature of a register/memory architecture, the memory access has to be performed before the execute stage. This also implies that there needs to be an address arithmetic unit at this state. The classical 5-stage RISC pipeline with memory access past the execute stage uses the ALU for this purpose.

- **Execute**. The Execute Stage will primarily do the computational work using the values passed from the MA stage. The computational result will be written back to the registers on the next clock cycle.

Note that this is perhaps one of many ways to implement a pipeline. The three major stages could also be further divided internally. For example, the fetch and decode stage could consist of two sub stages. Likewise, the memory access stages could be divided into an address calculation sub-stage and the actual data access. Dividing into three stages however simplifies the bypass logic as there are only two spots to insert any register overriding. This is especially important for the memory access stage, which uses the register content to build addresses. Two separate stages, i.e. address computation and memory access, would require options to redo the address arithmetic when detecting a register interlock at the memory access stage.

<!--------------------------------------------------------------------------------------------------------->

<div style="page-break-before: always;"></div>

## Table of Contents

[TOC]

<!--------------------------------------------------------------------------------------------------------->
