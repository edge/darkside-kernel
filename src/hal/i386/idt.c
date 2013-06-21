#include <lib/libc/stdint.h>
#include <lib/libc/string.h>
#include <hal/i386/idt.h>

/* IDT and IDT Pointer */
struct idt_entry idt[256];
struct idt_ptr idtp;

/* This exists in loader.s, and is used to load our IDT */
extern void idt_load();

/* Set an entry in the IDT */
void idt_set_gate(uint8_t num, unsigned long base)
{
    /* The interrupt routine's base address */
    idt[num].base_lo = (base & 0xFFFF);
    idt[num].base_hi = (base >> 16) & 0xFFFF;

    /* The segment or 'selector' that this IDT entry will use is set here, along with any access flags */
    idt[num].sel = 0x08;
    idt[num].always0 = 0;
	idt[num].flags = 0xEE;
}

/* Install the IDT */
void idt_install()
{
    /* Sets the special IDT pointer up, just like in 'gdt.c' */
    idtp.limit = (sizeof(struct idt_entry) * 256) - 1;
    idtp.base = (uint32_t) &idt;

    /* Clear out the entire IDT, initializing it to zeros */
    memset(&idt, 0, sizeof(struct idt_entry) * 256);

    /* Points the processor's internal register to the new IDT */
    idt_load();
}