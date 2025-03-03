#include <acpi/acpi.h>
#include <acpi/tables.h>
#include <asm.h>
#include <cpu_locals.h>
#include <drivers/apic.h>
#include <drivers/pit.h>
#include <lock.h>
#include <mm/pmm.h>
#include <printf.h>
#include <stddef.h>
#include <stdint.h>

#define LAPIC_REG_SPURIOUS 0x0f0
#define LAPIC_REG_TIMER 0x320
#define LAPIC_REG_TIMER_INITCNT 0x380
#define LAPIC_REG_TIMER_CURCNT 0x390
#define LAPIC_REG_TIMER_DIV 0x3e0
#define LAPIC_REG_EOI 0x0b0
#define LAPIC_REG_ICR0 0x300
#define LAPIC_REG_ICR1 0x310
#define LAPIC_APIC_ID 0x20

#define IOAPIC_REG_IOREGSEL 0
#define IOAPIC_REG_IOWIN 0x10
#define IOAPIC_REG_VER 1

static inline uintptr_t lapic_get_address() {
  return (uintptr_t)((rdmsr(0x1b) & 0xfffff000) + PHYS_MEM_OFFSET);
}

static inline uint32_t lapic_read(size_t reg) {
  return *((volatile uint32_t *)(lapic_get_address() + reg));
}

static inline void lapic_write(size_t reg, uint32_t data) {
  *((volatile uint32_t *)(lapic_get_address() + reg)) = data;
}

static inline uint32_t ioapic_read(uintptr_t addr, size_t reg) {
  *((volatile uint32_t *)(addr + IOAPIC_REG_IOREGSEL)) = reg;
  return *((volatile uint32_t *)(addr + IOAPIC_REG_IOWIN));
}

static inline void ioapic_write(uintptr_t addr, size_t reg, uint32_t data) {
  *((volatile uint32_t *)(addr + IOAPIC_REG_IOREGSEL)) = reg;
  *((volatile uint32_t *)(addr + IOAPIC_REG_IOWIN)) = data;
}

void disable_pic() {
  outb(0xa1, 0xff);
  outb(0x21, 0xff);
}

void lapic_enable(uint8_t vect) {
  lapic_write(LAPIC_REG_SPURIOUS,
              lapic_read(LAPIC_REG_SPURIOUS) | (1 << 8) | vect);
}

uint8_t lapic_get_id() { return (uint8_t)(lapic_read(LAPIC_APIC_ID) >> 24); }

void lapic_eoi() { lapic_write(LAPIC_REG_EOI, 0); }

void lapic_send_ipi(uint8_t lapic_id, uint8_t vect) {
  lapic_write(LAPIC_REG_ICR1, lapic_id << 24);
  lapic_write(LAPIC_REG_ICR0, vect);
}

void init_lapic() { lapic_enable(0xff); }

void lapic_timer_get_freq() {
  lapic_timer_stop();

  lapic_write(LAPIC_REG_TIMER, (1 << 16) | 0xff);
  lapic_write(LAPIC_REG_TIMER_DIV, 0);

  uint64_t initial_pit_tick = (uint64_t)pit_read_count();

  lapic_write(LAPIC_REG_TIMER_INITCNT, (uint32_t)0xfffff);
  while (lapic_read(LAPIC_REG_TIMER_CURCNT) != 0)
    ;

  uint64_t final_pit_tick = (uint64_t)pit_read_count();

  uint64_t pit_ticks = initial_pit_tick - final_pit_tick;
  cpu_locals_t *local = get_locals();
  local->lapic_timer_freq = (0xfffff / pit_ticks) * 1193182;

  lapic_timer_stop();
}

void lapic_timer_stop() {
  lapic_write(LAPIC_REG_TIMER_INITCNT, 0);
  lapic_write(LAPIC_REG_TIMER, (1 << 16));
}

void lapic_timer_oneshot(uint8_t intr, uint32_t us) {
  lapic_timer_stop();

  uint64_t ticks = us * (get_locals()->lapic_timer_freq / 1000000);

  lapic_write(LAPIC_REG_TIMER, intr);
  lapic_write(LAPIC_REG_TIMER_DIV, 0);
  lapic_write(LAPIC_REG_TIMER_INITCNT, (uint64_t)ticks);
}

uint32_t ioapic_get_gsi_count(uintptr_t addr) {
  return (ioapic_read(addr, IOAPIC_REG_VER) & 0xff0000) >> 16;
}

madt_ioapic_t *get_ioapic_by_gsi(uint32_t gsi) {
  for (size_t i = 0; i < (size_t)madt_ioapics.length; i++) {
    madt_ioapic_t *ioapic = madt_ioapics.data[i];

    if (ioapic->gsi <= gsi &&
        ioapic->gsi + ioapic_get_gsi_count(ioapic->address + PHYS_MEM_OFFSET) >
          gsi)
      return ioapic;
  }
  return NULL;
}

void ioapic_redirect_gsi(uint8_t lapic_id, uint32_t gsi, uint8_t vect,
                         uint64_t flags, int status) {
  madt_ioapic_t *ioapic = get_ioapic_by_gsi(gsi);
  uint64_t redirect = vect;

  if ((flags & (1 << 1)) != 0)
    redirect |= (1 << 13);
  if ((flags & (1 << 3)) != 0)
    redirect |= (1 << 5);
  if (!status)
    redirect |= (1 << 16);

  redirect |= (uint64_t)lapic_id << 56;
  uint32_t ioredtbl = (gsi - ioapic->gsi) * 2 + 16;
  ioapic_write(ioapic->address + PHYS_MEM_OFFSET, ioredtbl, (uint32_t)redirect);
  ioapic_write(ioapic->address + PHYS_MEM_OFFSET, ioredtbl + 1,
               (uint32_t)(redirect >> 32));
}

void ioapic_redirect_irq(uint8_t lapic_id, uint8_t irq, uint8_t vect,
                         int status) {
  for (size_t i = 0; i < (size_t)madt_isos.length; i++) {
    madt_iso_t *iso = madt_isos.data[i];
    if (iso->irq_source == irq)
      ioapic_redirect_gsi(lapic_id, iso->gsi, vect, iso->flags, status);
  }
  ioapic_redirect_gsi(lapic_id, irq, vect, 0, status);
}
