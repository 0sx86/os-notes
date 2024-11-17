# os-notes


### Intel Processor Execution Modes

![](imgs/20241117000054.png)


- **Real Mode**:
This mode implements the programming environment of the Intel 8086 processor with extensions (such as the ability to switch to protected or system management mode). The processor is placed in real-address mode following power-up or a reset.

No virtual memory, no privilege rings, 16 but mode. For example, DOS runs in real mode.
If we reset our processor, it is going to go back into real mode. So everything always starts in real mode every time we power on and power off the processor.
It is the job of the BIOS to get us out of real mode as soon as possible.

=> For more informations, let's see : [Architecture 4021: Introduction UEFI](https://www.youtube.com/watch?v=HgGwqG1Fiao&list=PLUFkSN0XLZ-ltETI20mpXOCdqC8rdven6)


- **Protected Mode:**
This mode is the native state of the processor. Among the capabilities of protected mode
is the ability to directly execute “real-address mode” 8086 software in a protected, multi-tasking environment. This feature is called virtual-8086 mode, although it is not actually a processor mode. Virtual-8086 mode is actually a protected mode attribute that can be enabled for any task.


- **System management mode (SMM):**
This mode provides an operating system or executive with a transparent mechanism for implementing platform-specific functions such as power management and system security. The processor enters SMM when the external SMM interrupt pin (SMI#) is activated or an SMI is received from the advanced programmable interrupt controller (APIC).


### CPUID

WIP

### Model Specific Registers (MSRs)
The mnemonic cpuid is the way that software finds out what features are supported, but how to enable it ?
=> MSRs provide a way on Intel systems to support an number of feature flags. The list is so big that Intel eventuyally split it out into Volume 4 of the manuals.

Naming Caveat : architectural MSRs were given the prefix "IA32_". However, it's not specific limited to 32 bits execution.

Using the instructions ```RDMSR``` and ```WRMSR```, the instruction can only be run in kernel mode.

```UA32_EFER``` is the Extended Feature Enables. The bit 8 (LME -- Long Mode Enable) is used to go from protected mode into 64-bit mode. The bit 10 (LMA) telling us if the mode is active right now or not. 

### Segmentation
Segmentation provides a mechanism of isolating individual code, data, and stack modules so that multiple programs (or tasks) can run on the same processor without interfering with one another. Paging provides a mechanism for implementing a conventional demand-paged, virtual-memory system where sections of a program’s execution environment are mapped into physical memory as needed. Paging can also be used to provide isolation between multiple tasks. When operating in protected mode, some form of segmentation must be used. There is no mode bit to disable segmentation. The use of paging, however, is optional.

We are going to use a logical address to locate a byte in a particular segment. A logical address consists of a segment selector and an offset. The physical address space is defined as the range of addresses that the processor can generate on its address bus. Linear address space is a flat 64 bit space.

- **32 bit Near Pointer vs Far Pointer**
A Far Pointer is a 32 bit value with a selector put in a front of it 1(16bit value).

![](imgs/20241117040546.png)

- **64 bit Near Pointer vs Far Pointer**
A Far Pointer is a 64 bit Near Pointer value with a 16 bit Segment Selector.

![](imgs/20241117040600.png)

Segmentation translates logical addresses to linear addresses, automatically, in hardware, by using table lookups. But software fills in the tables. Logical address (far pointer) = 16 bit segment selector + 32/64 bit offset.

![](imgs/20241117041107.png)

If paging is not used, the processor maps the linear address directly to a physical address (that is, the linear address goes out on the processor’s address bus). If the linear address space is paged, a second level of address translation is used to translate the linear address into a physical address.

To translate a logical address into a linear address, the processor does the following:
1. Uses the offset in the segment selector to locate the segment descriptor for the segment in the GDT or LDT and reads it into the processor. (This step is needed only when a new segment selector is loaded into a segment register.)
2. Examines the segment descriptor to check the access rights and range of the segment to ensure that the segment is accessible and that the offset is within the limits of the segment.
3. Adds the base address of the segment from the segment descriptor to the offset to form a linear address.

A segment selector is a 16-bit identifier for a segment (see Figure 3-6). It does not point directly to the segment, but instead points to the segment descriptor that defines the segment.

![](imgs/20241117041506.png)

It is used to select a different data structure from one of two tables : GDT, LDT. The index is actually 13 bits not 16, so the tables can each hold a maximum of $2^{13} = 8191$ data structures 


  
![](imgs/20241117035402.png)


In IA-32e mode of Intel 64 architecture, the effects of segmentation depend on whether the processor is running in compatibility mode or 64-bit mode. In compatibility mode, segmentation functions just as it does using legacy 16-bit or 32-bit protected mode semantics.

In 64-bit mode, segmentation is generally (but not completely) disabled, creating a flat 64-bit linear-address space. The processor treats the segment base of CS, DS, ES, SS as zero, creating a linear address that is equal to the effective address. The FS and GS segments are exceptions. These segment registers (which hold the segment base) can be used as additional base registers in linear address calculations. They facilitate addressing local data
and certain operating system data structures. 
Note that the processor does not perform segment limit checks at runtime in 64-bit mode.

- **Basic Flat Model**
The simplest memory model for a system is the basic “flat model,” in which the operating system and application programs have access to a continuous, unsegmented address space. To the greatest extent possible, this basic flat model hides the segmentation mechanism of the architecture from both the system designer and the application programmer.

![](imgs/20241117035626.png)

To implement a basic flat memory model with the IA-32 architecture, at least two segment descriptors must be created, one for referencing a code segment and one for referencing a data segment (see Figure 3-2). Both of these segments, however, are mapped to the entire linear address space: that is, both segment descriptors have the same base address value of 0 and the same segment limit of 4 GBytes. By setting the segment limit to 4 GBytes, the segmentation mechanism is kept from generating exceptions for out of limit memory references, even if no physical memory resides at a particular address. ROM (EPROM) is generally located at the top of the physical address space, because the processor begins execution at FFFF_FFF0H. RAM (DRAM) is placed at the bottom of the address space because the initial base address for the DS data segment after reset initialization is 0.


- **Protected Flat Model**
The protected flat model is similar to the basic flat model, except the segment limits are set to include only the range of addresses for which physical memory actually exists (see Figure 3-3). A general-protection exception (#GP) is then generated on any attempt to access nonexistent memory. This model provides a minimum level of hardware protection against some kinds of program bugs.

![](imgs/20241117035656.png)

**Reading/Writing Segment Registers with MOV**
![](imgs/20241117042920.png)

**Reading/Writing Segment Registers with PUSH/POP**
![](imgs/20241117043048.png)

![](imgs/20241117043141.png)


Every segment register has a “visible” part and a “hidden” part. (The hidden part is sometimes referred to as a “descriptor cache” or a “shadow register.”) When a segment selector is loaded into the visible part of a segment register, the processor also loads the hidden part of the segment register with the base address, segment limit, and access control information from the segment descriptor pointed to by the segment selector. The information cached in the segment register (visible and hidden) allows the processor to translate addresses without taking extra bus cycles to read the base address and limit from the segment descriptor. In systems in which multiple processors have access to the same descriptor tables, it is the responsibility of software to reload the segment registers when the descriptor tables are modified. If this is not done, an old segment descriptor cached in a segment register might be used after its memory-resident version has been modified.


![](imgs/20241117043319.png)

The hidden part is especially used by the segment descriptor (LDT or GDT) to write informations (Base, Limit, => Access <=). 
Segmentation is not optional, it is still used, but it is used in a much more limited form than what it was originally designed for.

### GDT & LDT

- GDTR

![](imgs/20241117124157.png)

The upper 64bits ("base address") of the register specifies the linear address where the GDT is stored. The lower 16bits ("table limit") specify the size of the table in bytes (with base+limit specifying the address of the last valid/included byte). Special instructions used to load a value into the register or store the value out to memory (```LGDT```: load 10 bytes from memory into GDTR, ```SGDT```: store 10 bytes of GDTR to memory)

- LDTR

![](imgs/20241117142638.png)

Like the 16-bit segment registers, the 16-bit LDTR has a visible part, the segment selector, and a hidden part, the cached segment info which specifies the size of the LDT.
	The selector's Table Indicator (T) bit must be set to 0 to specify that it's selecting from GDT, not from itself.
Special instructions used to load a value into the register or store the value out to memory (```LLDT```: load 16 bit segment selector into LDTR, ```SLDT```: store 16 bit segment selector of LDTR to memory)

![](imgs/20241117143243.png)

A segment descriptor table is an array of segment descriptors (see Figure 3-10). A descriptor table is variable in length and can contain up to 8192 (213 ) 8-byte descriptors. There are two kinds of descriptor tables:
- The global descriptor table (GDT).
- The local descriptor tables (LDT).

Each system must have one GDT defined, which may be used for all programs and tasks in the system. Optionally, one or more LDTs can be defined. For example, an LDT can be defined for each separate task being run, or some or all tasks can share the same LDT.

The GDT is not a segment itself; instead, it is a data structure in linear address space. The base linear address and limit of the GDT must be loaded into the GDTR register (see Section 2.4, “Memory-Management Registers”). The base address of the GDT should be aligned on an eight-byte boundary to yield the best processor performance. The limit value for the GDT is expressed in bytes. As with segments, the limit value is added to the base address to get the address of the last valid byte. A limit value of 0 results in exactly one valid byte. Because segment descriptors are always 8 bytes long, the GDT limit should always be one less than an integral multiple of eight (that is, 8N – 1). 

The first descriptor in the GDT is not used by the processor. A segment selector to this “null descriptor” does not generate an exception when loaded into a data-segment register (DS, ES, FS, or GS), but it always generates a general-protection exception (#GP) when an attempt is made to access memory using the descriptor. By initializing the segment registers with this segment selector, accidental reference to unused segment registers can be guaranteed to generate an exception. 

The LDT is located in a system segment of the LDT type. The GDT must contain a segment descriptor for the LDT segment. If the system supports multiple LDTs, each must have a separate segment selector and segment descriptor in the GDT. The segment descriptor for an LDT can be located anywhere in the GDT. See Section 3.5, “System Descriptor Types,” for information on the LDT segment-descriptor type.

An LDT is accessed with its segment selector. To eliminate address translations when accessing the LDT, the segment selector, base linear address, limit, and access rights of the LDT are stored in the LDTR register (see Section 2.4, “Memory-Management Registers”).

When the GDTR register is stored (using the SGDT instruction), a 48-bit “pseudo-descriptor” is stored in memory. To avoid alignment check faults in user mode (privilege level 3), the pseudo-descriptor should be located at an odd word address (that is, address MOD 4 is equal to 2). This causes the processor to store an aligned word, followed by an aligned doubleword. User-mode programs normally do not store pseudo-descriptors, but the possibility of generating an alignment check fault can be avoided by aligning pseudo-descriptors in this way. The same alignment should be used when storing the IDTR register using the SIDT instruc-tion. 

When storing the LDTR or task register (using the SLDT or STR instruction, respectively), the pseudo-descriptor should be located at a doubleword address (that is, address MOD 4 is equal to 0).

![](imgs/20241117143741.png)

