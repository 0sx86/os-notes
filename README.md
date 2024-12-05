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
=> MSRs provide a way on Intel systems to support an number of feature flags. The list is so big that Intel eventually split it out into Volume 4 of the manuals.

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

### Segment Descriptors
Each entry in the GDT and LDT is a data structure called a Segment Descriptor. Each Segment Descriptor describes a segment.

Each segment descriptor has an associated segment selector. A segment selector provides the software that uses it with an index into the GDT or LDT (the offset of its associated segment descriptor), a global/local flag (determines whether the selector points to the GDT or the LDT), and access rights information.

![](imgs/20241117183313.png)

To access a byte in a segment, a segment selector and an offset must be supplied. The segment selector provides access to the segment descriptor for the segment (in the GDT or LDT). From the segment descriptor, the processor obtains the base address of the segment in the linear address space. The offset then provides the location of the byte relative to the base address. This mechanism can be used to access any valid code, data, or stack segment, provided the segment is accessible from the current privilege level (CPL) at which the processor is operating. The CPL is defined as the protection level of the currently executing code segment.

The linear address of the base of the GDT is contained in the GDT register (GDTR); the linear address of the LDT is contained in the LDT register (LDTR).

![](imgs/20241117183818.png)

- L Flag (bit 21): Whether this is a 64-bit segment or not (if not, it's Compatibility Mode, which exhibits 32-bit protected mode behavior) => CS.L=1 (64bits) - CS.L=0 (CM)

- Base (32bits): linear address where the segment start, **for Compatibility Mode**. For 64-bit mode, the base is juste always treated as 0 for CS, SS, DS, ES. For FS and GS, the hidden part of the segment registers is mapped into the **IA32_FS_BASE** (0xc0000100) and **IA32_GS_BASE** (0xC0000101) MSRs respectively. So a full 64bit address can be written there instead of in the segment descriptor

- Limit (20bits): size of the segment (in bytes or 4kb blocks), for Compatibility Mode. Limits are not actulaly checked anymore in 64 bit mode, even for FS and GS.

- G Flag: whether the Limit is specified in bytes or 4096 byte chunks, for Compatibility Mode. Not  used in 64-bit mode.

- P Flag: 0 for not present. 1 for present.
	if this flag is clear, the processor generates a segment-not-present exception (#NP) when a segment selector that points to the segment descriptor is loaded into a segment register

- S Flag: 0 for System segment. 1 for Code or Data segment

- Type (4bits): Different types for System and non-System (i.e. Code & Data) descriptors. <br> **What was the point of Expand-Down segment ?** => Originally for Stack Segments, to allow for growth towards lower addresses. Although a read-only && expand-down segment can't be used for stack segments... Loading the SS register with a segment generates a general-protection exception (#GP). <br> <br> **Conforming vs Non-Conforming Segments**: Conforming segments allow lower privilege code to execute them (ring 3 code could just jump into Ring 0 conforming segments and keep running). Non-conforming segments behave how you'd expect from a security perspective, and will throw a general protection fault if someone from a lower privilege level tries to execute them

- D/B Flag: used for completely different stuff based on the descriptor type. <br>**Code Segment:** "D" (Default Opcode Size) flag, this is what actually controls whether an overloaded opcode is interpreted as dealing with 16 or 32 but register/memory sizes. The processor fetched a 0x25 in the isntruction stream, how would it know whether it should be followed by 2 bytes (imm16) or 4 bytes (imm32) ? => if the D flag == 0 it's 16bits, and if D == 1 it's 32bits. The instruction prefix 66H can be used to select and operand size other than the default. ![](imgs/20241117234207.png) <br><br>**Stack Segment (data segment pointed to by the SS register):** B (Big) Flag, whether implicit stack pointer usage (e.g. push/pop/call) moves stack pointer by 16 bits (if B is 0), or 32 bits (if B is 1).<br>**Expand-Down Data Segment:** B Flag, 0 = upper bound of 0xFFFF, 1 = upper bound of 0xFFFFFFFF. No one really uses expand down segments ... 

- DPL Flags (2 bits): PRivilege Ring of the segment for access controls. If this is a non-conforming segment descriptor && DPL == 0, only ring 0 code can execute from within this segment. If this i s a data segment selector && DPL == 0, only ring 0 code can read/write data from/to this segment

- Available Flag: No specific usage defined. Available for OSes to use it or not use it as they see fit.

***Other types of descriptors in 64-bit Mode***
![](imgs/20241117234532.png)

The system segment descriptor for the TSS and the LDT, in the GDT there is an entry that points to the DLT and that entry has been expanded to be 16 bytes large, so that the base address which previously could only hold a 32-bit value is expanded to a 64-bit value.
<br><br><br>If attempting a control flow transition via JMP/Jcc/CALL/RET from one segment into a different segment, the hardware will check the DPL of the target segment and allow the access only if CPL <= DPL. 
<br>Some of the system instructions (called “privileged instructions”) are protected from use by application programs. The privileged instructions control system functions (such as the loading of system registers). They can be executed only when the CPL is 0 (most privileged). If one of these instructions is executed when the CPL is not 0, a general-protection exception (#GP) is generated.<br>The following system instructions are privileged instructions:
- LTR — Load task register.
- LIDT — Load IDT register.
- MOV (control registers) — Load and store control registers.
- LMSW — Load machine status word.
- CLTS — Clear task-switched flag in register CR0.
- MOV (debug registers) — Load and store debug registers.
- INVD — Invalidate cache, without writeback.
- WBINVD — Invalidate cache, with writeback.
- INVLPG —Invalidate TLB entry.
- HLT— Halt processor.
- RDMSR — Read Model-Specific Registers.
- WRMSR —Write Model-Specific Registers.
- RDPMC — Read Performance-Monitoring Counter.
- RDTSC — Read Time-Stamp Counter.
The PCE and TSD flags in register CR4 (bits 4 and 2, respectively) enable the RDPMC and RDTSC instructions, respectively, to be executed at any CPL.
<br> The MOV instruction cannot be used to load CS register. Attempting to do so results in an invalid opcode exception. Also, there is no "POP CS" instruction like there is for SS, DS, ES, FS, GS.

### Call Gates 
A Call Gates is a way to transfer control from on segment to another segment at a different privilege level.
<br>They are typically used only in operating systems or executives that use the privilege-level protection mechanism. Call gates are also useful for transferring program control between 16-bit and 32-bit code segments.

![](imgs/20241118002925.png)

Call-gate descriptors in 32-bit mode provide a 32-bit offset for the instruction pointer (EIP); 64-bit extensions double the size of 32-bit mode call gates in order to store 64-bit instruction pointers (RIP).

![](imgs/20241118002757.png).

To transition from CPL 3 to CPL 0, we would issue a CALL instruction, with a far pointer, that had a Segment Selector, that pointed at a Call Gate Segment Descriptor.

To access a call gate, a far pointer to the gate is provided as a target operand in a CALL or JMP instruction. The segment selector from this pointer identifies the call gate; the offset from the pointer is required, but not used or checked by the processor. (The offset can be set to any value.)<br>
![](imgs/20241118004127.png)

When the processor has accessed the call gate, it uses the segment selector from the call gate to locate the segment descriptor for the destination code segment. (This segment descriptor can be in the GDT or the LDT.) It then combines the base address from the code-segment descriptor with the offset from the call gate to form the linear address of the procedure entry point in the code segment.<br><br>Four different privilege levels are used to check the validity of a program control transfer
through a call gate:
- The CPL (current privilege level).
- The RPL (requestor privilege level) of the call gate’s selector.
- The DPL (descriptor privilege level) of the call gate descriptor.
- The DPL of the segment descriptor of the destination code segment.
The C flag (conforming) in the segment descriptor for the destination code segment is also checked.

![](imgs/20241118003523.png)

The privilege checking rules are different depending on whether the control transfer was initiated with a CALL or a JMP instruction.

![](imgs/20241118003619.png)

The DPL field of the call-gate descriptor specifies the numerically highest privilege level from which a calling procedure can access the call gate; that is, to access a call gate, the CPL of a calling procedure must be equal to or less than the DPL of the call gate. For example, in Figure 6-15, call gate A has a DPL of 3. So calling procedures at all CPLs (0 through 3) can access this call gate, which includes calling procedures in code segments A, B, and C. Call gate B has a DPL of 2, so only calling procedures at a CPL or 0, 1, or 2 can access call gate B, which includes calling procedures in code segments B and C. The dotted line shows that a calling procedure in code segment A cannot access call gate B.<br><br>
The RPL of the segment selector to a call gate must satisfy the same test as the CPL of the calling procedure; that is, the RPL must be less than or equal to the DPL of the call gate. In the example in Figure 6-15, a calling procedure in code segment C can access call gate B using gate selector B2 or B1, but it could not use gate selector B3 to access call gate B.<br>
![](imgs/20241118003916.png)

If the privilege checks between the calling procedure and call gate are successful, the  processor then checks the DPL of the code-segment descriptor against the CPL of the calling procedure. Here, the privilege check rules vary between CALL and JMP instructions. Only CALL instructions can use call gates to transfer program control to more privileged (numerically lower privilege level) nonconforming code segments; that is, to nonconforming code segments with a DPL less than the CPL. A JMP instruction can use a call gate only to transfer program control to a non conforming code segment with a DPL equal to the CPL. CALL and JMP instruction can both transfer program control to a more privileged conforming code segment; that is, to a conforming code segment with a DPL less than
or equal to the CPL.
<br>
How can i return from a call through a Call Gate ?<br>A normal CALL pushes RIP, a inter-privilege far CALL through a Call Gate pushes SS:RSP and CS:RIP, RET can pop those back off the stack to return from an inter-privilege far call.

![](imgs/20241118013953.png)
<br><br>

### Interrupts and Exceptions
Interrupts and exceptions are events that indicate that a condition exists somewhere in the system, the processor, or within the currently executing program or task that requires the attention of a processor.<br>
When an interrupt is received or an exception is detected, the currently running procedure or task is suspendedwhile the processor executes an interrupt or exception handler. When execution of the handler is complete, theprocessor resumes execution of the interrupted procedure or task. The resumption of the interrupted procedure ortask happens without loss of program continuity, unless recovery from an exception was not possible or an inter-rupt caused the currently running program to be terminated.<br>
The processor receives interrupts from two sources:
- External (hardware generated) interrupts
- Software-generated interrupts

![](imgs/20241120000837.png)


**Difference between Interrupt and Exception**<br>
Exceptions typically indicate error conditions, whereas interrupts typically indicate events from external hardware (Exception - E is for error). Interrupts clear the Interrupt Flag (IF), Exceptions do not. <br>
3 categories of exception:
- Fault - recoverable - pushed RIP points to **the faulting instruction**
- Trap - recoverable - pushed RIP points to **the instruction following the trapping instruction**
- Abort - unrecoverable -  **may not be able to save RIP where abort occurred**

#### Saving state

Previous 32-bit behavior
![](imgs/20241120002114.png)

64-bit behavior
![](imgs/20241120004722.png)

An interrupt (whether hardward or software) "PUSHes" save state onto the stack, so it can resume the previous execution after handling it. <br>
The ```IRET (Interrupt Return)``` instruction "POPs" it back off into the relevant registers, in the same sense that RET "POPs" a saved return address into RIP.

#### Software Generated Interrupts

- ```Int n```: Invoke Interrupt handler n. Important: while we can invoke anything in the IDT, some interrupts except an error code. INT does not push any error code, and therefore a handler may not behave correctly.
- ```IRET```: returns from an interrupt, popping all saved state back into the correct registers
- ```INT3```: There is a special one-byte "0xCC" opcode form that can be used to invoke interrupt 3 for software debug breakpoints.
- ```INT1```: There is a special one-byte "0xF1" opcode form that can be used to invoke interrupt 1 to fake hardware debug breakpoints
- ```INTO```: Invoke overflow interrupt if the overflow flag (OF) in RFLAGS is set to 1 (like a conditionnal ```INT 4```)
- ```UD2```: Invoke invalid opcode interrupt (same as ```INT 6```)
<br><br>Interrupts are another way to transfer control from one segment to another segment at a different privilege level.<br>How does the hardware decide where to push the interrupted procedure's saved state ?<br>=> To understand that, we need to learn about Tasks and Task-State Segment (TSS)

#### Tasks
The intel documentation often references the notion of a "task". It is a hardware mechanism to support multi-tasking, by saving/restoring task state, like registers. But it wasn't getting used for that purpose, so just like with segmentation, a lot of things were removed with the x86-64 extensions. But the Task-State Segment (TSS) is something which still **must** be used by the OS b virtue of being consulted on privilege changing interrupts, so we will at least talk about that.

![](imgs/20241120013604.png)

Like the 16-bit segment registers, and LDTR, the 16-bit TR has a visible part, the segment selector, and a hidden part, the cached segment info which specifies the size of the Task-State Segment (TSS). Special instructions used to load a value into the register or store the value out to memory. ```LTR - Load 16 bit segment into TR``` and ```STR - Store 16 bit segment selector of TR to memory```. <br>Call gates facilitate transitions between 64-bit mode and compatibility mode. Task gates are not supported in IA-32e mode. On privilege level changes, stack segment selectors are not read from the TSS. Instead, they are set to NULL.

![](imgs/20241120014302.png)

![](imgs/20241120014416.png)

![](imgs/20241120014629.png)

When interrupt is occuring and changing state into ring 0, ```RSP0``` will be used as the rsp to decided where to throw stuff on the stack. So the kernel needs to set this up, this needs to be in some kernel space location so that people can't tamper with it and this is were state will be saved. If processor was changing to ring 1, ```RSP1``` will be used instead and similarly if it was changing to ring 2, ```RSP2``` would be used instead.<br>There is also new elements for the x86-64 extensions called the interrupt stack table **IST**. It is a list of seven possible value that can be used for different interrupts. Interruptions can precise which stack they want to use.<br>
**TSS is still required because he holds a pointer to the stack, which is where the stack is going to be consider to start before the informations gets pushed on into the stack. So TSS helps the hardware to know where to push informations.**

#### Interrupt Decriptor Table (IDT)
There is a specific register which points at the base (0th entry) of the IDT. The **IDT R**egister is named ITDR. When interrupt/exception occurs, the hardware automatically : 
1. consults the IDTR
2. finds the appropriate offset in the IDT
3. pushes the saved state onto the stack (at a location determined by the TSS)
4. changes CS:RIP to the address of the interrupt handler, as read from the IDT entry (interrupt descriptor).

![](imgs/20241120021945.png)


The IDTR register holds the base address (32 bits in protected mode; 64 bits in IA-32e mode) and 16-bit table limit for the IDT. The base address (upper 64bits of the register) specifies the linear address of byte 0 of the IDT; the table limit (the lower 16 bits) specifies the number of bytes in the table. The LIDT and SIDT instructions load and store the IDTR register, respectively. On power up or reset of the processor, the base address is set to the default value of 0 and the limit is set to 0FFFFH. The base address and limit in the register can then be changed as part of the processor initialization process.<br>Special instructions used to load a value into the register or store the value out to memory: 
- ```LIDT - Load 10 bytes from memory into IDTR```
- ```SIDT - Store 10 bytes of IDTR to memory```

![](imgs/20241120022415.png)

The IDT is an array of <= 256 16-byte decriptor entries. 0 through 31 are reserved for architecture-specific exceptions and interrupts. 32-255 are user-defined. While it interacts with segments, we can think of it as being an array of function (far) pointers, and when interrupt n is invoked by software or hardware, execution transfers to the address pointed to by the nth descriptor in the table. <br>
![](imgs/20241120022734.png)

Here, base address isn't used. It's always going to be 0. But access control bits like DPL, will be checked.

![](imgs/20241120023107.png)
![](imgs/20241120023117.png)

#### Interrupt Descriptors
The descriptors in the IDT describe one of two gate types:
- Interrupt Gate
- Trap Gate
The only difference between an interrupt gate and a trap gate is the way the processor handles the IF flag in the EFLAGS register. 

![](imgs/20241120024200.png)

IDT entries contain a segment selector and a 64-bit offset. That looks like a "logical address" aka "far pointer".
- Type is binary 1110 for Interrupt Gate and 1111 for Trap Gate
- P(resent) bit should be set to 1 to indicate this is a valid / filled in entry, and 0 otherwise
- DPL is only checked when a descriptor is accessed by a software interrupt (ignored on hardware interrupts). Only allowed if CPL <= DPL
- IST != 0 specifies a specific index to use for the RSP as pulled from the TSS. This is a location where saved context will be pushed ("I want my RSP to be in that table").

#### Interrupt Masking
It is sometimes useful to disable some interrupts. (E.g. to stop an interrupt from occurring in the middle of handling another interrupt.) This is called "masking" the interrupt. The Interrupt Flag (IF) in the RFLAGS register is cleared automatically whenever an interrupt occurs through an Interrupt Gate. But, it is not cleared if we go through a Trap Gate. **This is the only difference between the two types of gates**. Maskable interrupts can be manually masked by clearing IF (```CLI - Clear IF``` and ```STI - Set IF```)<br><br>The IF does not mask the explicit invocation of an interrupt with the INT N/INT1/INT3/INT0/UD2 instructions. <br>The IF does not mask a Non Maskable Interrupt - IDT[2]

<<<<<<< HEAD

### System calls
The addition of privilege separation (e.g. Intel privilege rings), necessitates some way to transfer control between different execution domains. 
Multiple mechanisms can be used, in historical order : Call Gates, Interrupts, Syscalls

![](20241122110604.png)

Interrupt Gate is the most compatible, but also the slowest. The instructions syscall and sysenter are the best. This why operating systems generally try to use these syscalls.
Dedicated system cal instructions are another way to transfer control from one segment to another segment at a different privilege level.

Preferred for 32-bit: SYSENTER/SYSEXIT (intel invented) - Preferred for 64-bit: SYSCALL/SYSRET (amd invented).

![](imgs/20241122112120.png)

SYSCALL Enable: IA32_EFER.SCE (R/W) Enables SYSCALL/SYSRET instructions in 64-bit mode.


**SYSCALL - System Call**<br>Save RIP of address after SYSCALL into RCX
Change RIP to value stored in IA32_LSTAR MSR (0xc0000082)
Save RFLAGS into R11
Clears the each bit in RFLAGS that has its bit set in IA32_FMASK MSR (0xC0000084) 
> Effectively : RFLAGS &= ~IA32_FMASK
> Any bit that is set to one this IA32_MASK MSR will be cleared in the RFLAGS at the time that it gets into kernelspace

Loads CS with value stored in IA32_STAR MSR (0xC0000081) bits [47:32] and SS = (CS+8)
> Easiest to just read the instruction Operation pseudocode in the manual to see what is does

Doesn't save RSP. Either kernel or userspace SYSCALL handler is responsible for that.

**SYSRET - System Call Return**
Restore RIP from RCX
Restore RFLAGS from R11
Loads CS with value stored in IA32_STAR MSR bits [63:48] and adds 16
Loads SS with value stored in IA32_STAR MSR bits [63:48] and adds 8
Whichever side of kernel/user saved RSP is responsible for restoring it

![](imgs/20241122114027.png)

The SYSCALL instruction does not save the stack pointer, and the SYSRET instruction does not restore it. It is likely that the OS system-call handler will change the stack pointer from the user stack to the OS stack. If so, it is the responsibility of software first to save the user stack pointer. This might be done by user code, prior to executing SYSCALL, or by the OS system-call handler after SYSCALL.

Because the SYSRET instruction does not modify the stack pointer, it is necessary for software to switch back to the user stack. The OS may load the user stack pointer (if it was saved after SYSCALL) before executing SYSRET; alter-natively, user code may load the stack pointer (if it was saved before SYSCALL) after receiving control from SYSRET.

If the OS loads the stack pointer before executing SYSRET, it must ensure that the handler of any interrupt or exception delivered between restoring the stack pointer and successful execution of SYSRET is not invoked with the user stack.

![](imgs/20241122140233.png)
From the details of SYSRET, we can see ```CS.SELECTOR := CS.SELECTOR OR 3;```. The microcode makes always sure returning back to userspace => it is forcing use into ring 3. This is one of theses consequences of rings 1 and 2 never being used.

![](imgs/20241122141533.png)


SYSENTER/SYSEXIT are preferred on 32-bit systems, because they're supported on both Intel and AMD (was created before AMD's x86-64 extensions).

##### Syscall-adjacent Tech (swapgs, {rd,wr}{fs,gs}base)

**SWAPGS**
When using SYSCALL to implement system calls, there is no kernel stack at the OS entry point. Neither is there a straightforward method to obtain a pointer to kernel structures from which the kernel stack pointer could be read. Thus, the kernel cannot save general purpose registers or reference memory.
=> The SWAPGS instruction is designed to help with the above issue  
 Exchanges the GS base linear address (mapped to IA32_GS_BASE) with the one found in the IA32_KERNEL_GS_BASE MSR (0xC0000102). Useful for interrupt handlers as wel as SYSCALL handlers.
 
![](imgs/20241122160011.png)


Generally speaking OSes use FS and GS to point at global data structures that they want to easily be able to access.

x86-64:
- Userspace: FS => Thread Local Storage (TLS), GS => The GS segment has no common use
- Kernel: FS => Unused, GS => Per-CPU variables
x86-32: 
- Userspace: FS => Unused, GS => Thread Local Storage (TLS)
- Kernel: FS => Unused, GS => Unused

if CPUID.07H.0H:EBX.FSGBASE[bit 0] = 1 && CR4.FSGSBASE = 1
=> it means that the processor supports the following instructions :
- ```RDFSBASE```/```WRFSBASE```
- ```RDGSBASE```/```WRGSBASE```
Which can be used instead of ```RDMSR```/```WRSMR``` to ```read```/```write``` the FS and GS base addresses.
["A possible end to the FSGSBASE saga"](https://lwn.net/Articles/821723/)


#### Read the TimeStamp Counter (RDTSC)
What's the time ? Time to ReaD (the) Time-Stamp Counter
The TSC is a 64bit counter introduced in the Pentium
Set to 0 when processor reset, incremented on each clock cycle thereafter.

![](imgs/20241122172451.png)

Reads the current value of the processor’s time-stamp counter (a 64-bit MSR) into the EDX:EAX registers. The EDX register is loaded with the high-order 32 bits of the MSR and the EAX register is loaded with the low-order 32 bits. (On processors that support the Intel 64 architecture, the high-order 32 bits of each of RAX and RDX are cleared.)

- The counter also can be read out of IA32_TIME_STAMP_COUNTER (0x10) MSR
- It can set bit 2 in Control Register 4 so that only Ring 0 can use the instruction.
- Processor families increment the timestamp counter differently (see Vol 3, Sect 17.17)
- VM Extensions have special provisions to help hypervisors lie to VMs about RDTSC contents

The processor monotonically increments the time-stamp counter MSR every clock cycle and resets it to 0 whenever the processor is reset.

The time stamp disable (TSD) flag in register CR4 restricts the use of the RDTSC instruction as follows. When the flag is clear, the RDTSC instruction can be executed at any privilege level; when the flag is set, the instruction can only be executed at privilege level 0.

The time-stamp counter can also be read with the RDMSR instruction, when executing at privilege level 0.

The RDTSC instruction is not a serializing instruction. It does not necessarily wait until all previous instructions have been executed before reading the counter. Similarly, subsequent instructions may begin execution before the read operation is performed. The following items may guide software seeking to order executions of RDTSC:
- If software requires RDTSC to be executed only after all previous instructions have executed and all previous loads are globally visible,1 it can execute LFENCE immediately before RDTSC.
- If software requires RDTSC to be executed only after all previous instructions have executed and all previous loads and stores are globally visible, it can execute the sequence MFENCE;LFENCE immediately before RDTSC.
- If software requires RDTSC to be executed prior to execution of any subsequent instruction (including any memory accesses), it can execute the sequence LFENCE immediately after RDTSC.

### Paging
- Logical Address: Far pointer that specifies a segment selector + offset in that segment
- Linear Address: Flat 32/64 notional address-space, post-segmentation
- Virtual Address: Address that takes paging into account
- Physical Address: Offset into physical RAM

![](imgs/20241205230717.png)

It's called "paging" because physical memory is divided into fixed size chunks called _pages_.

When paging is enabled, a linear address is the same thing as a virtual memory address or virtual address. 
Memory Management Unit => chunk of hardware in a processor which uses the current execution mode, segmentation information, and paging information to perform the overall process of translating logical to physical memory addresses.
The Translation Lookaside Buffer (TLB) is a cache of Virtual -> Physical mappings the MMU consults.
Paging makes memory access virtual in that no longer does the linear address correspond to the exact same physical address sent out on the bus lines to RAM (as it does when paging is disabled). Low addresses can map to high addresses, or low addresses, or no addresses.

There are 5 Control Registers (CR0-CR4) which are used for paging control as well as enabling/disabling other features.

![](imgs/20241128013711.png)