#include <asm.h>
#include <drivers/apic.h>
#include <lock.h>
#include <stdint.h>
#include <sys/idt.h>
#include <sys/irq.h>

static lock_t irq_lock = {0};

static void (*irq_routines[16])(uint64_t rsp) = {0};

extern void pit_handler();

extern void irq0();
extern void irq1();
extern void irq2();
extern void irq3();
extern void irq4();
extern void irq5();
extern void irq6();
extern void irq7();
extern void irq8();
extern void irq9();
extern void irq10();
extern void irq11();
extern void irq12();
extern void irq13();
extern void irq14();
extern void irq15();
extern void schedule_irq();

int init_irq() {
  LOCK(irq_lock);

  idt_set_gate(&idt[32 + 0], 1, 0, irq0);
  idt_set_gate(&idt[32 + 1], 1, 0, irq1);
  idt_set_gate(&idt[32 + 2], 1, 0, irq2);
  idt_set_gate(&idt[32 + 3], 1, 0, irq3);
  idt_set_gate(&idt[32 + 4], 1, 0, irq4);
  idt_set_gate(&idt[32 + 5], 1, 0, irq5);
  idt_set_gate(&idt[32 + 6], 1, 0, irq6);
  idt_set_gate(&idt[32 + 7], 1, 0, irq7);
  idt_set_gate(&idt[32 + 8], 1, 0, irq8);
  idt_set_gate(&idt[32 + 9], 1, 0, irq9);
  idt_set_gate(&idt[32 + 10], 1, 0, irq10);
  idt_set_gate(&idt[32 + 11], 1, 0, irq11);
  idt_set_gate(&idt[32 + 12], 1, 0, irq12);
  idt_set_gate(&idt[32 + 13], 1, 0, irq13);
  idt_set_gate(&idt[32 + 14], 1, 0, irq14);
  idt_set_gate(&idt[32 + 15], 1, 0, irq15);
  idt_set_gate(&idt[32 + 16], 1, 0, schedule_irq);

  UNLOCK(irq_lock);

  return 0;
}

void irq_install_handler(int irq, void (*handler)(uint64_t rsp)) {
  LOCK(irq_lock);
  irq_routines[irq] = handler;
  UNLOCK(irq_lock);
}

void irq_uninstall_handler(int irq) {
  LOCK(irq_lock);
  irq_routines[irq] = 0;
  UNLOCK(irq_lock);
}

void c_irq_handler(uint64_t irqno, uint64_t rsp) {
  if (irq_routines[irqno])
    irq_routines[irqno](rsp);
  lapic_eoi();
}
