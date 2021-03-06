#if 0

#include "apu.h"

#include <cassert>
#include <cstdio>

APU::~APU() {}

APU::APU(Memory& mem, InterruptLines& interrupt)
: interrupt(interrupt)
, mem(mem)
{
  this->power_cycle();
}

// https://wiki.nesdev.com/w/index.php/CPU_power_up_state
void APU::power_cycle() {
  this->cycles = 0;

  this->seq_step = 0;

  // clear internal registers (through mem interface, for convenience)
  for (u16 addr = 0x4000; addr <= 0x4013; addr++)
    (*this)[addr] = 0x00;
  (*this)[0x4015] = 0x00;
  (*this)[0x4017] = 0x00;
}

// https://wiki.nesdev.com/w/index.php/CPU_power_up_state
void APU::reset() {
  this->cycles = 0;

  this->seq_step = 0;

  // silence APU
  (*this)[0x4015] = 0x00;
}

u8 APU::read(u16 addr) {
  if (addr == 0x4015) return this->reg.state.val;

  fprintf(stderr, "[APU] Reading from Write-Only register: 0x%04X\n", addr);
  return 0x00;
}

u8 APU::peek(u16 addr) const {
  if (addr == 0x4015) return this->reg.state.val;

  fprintf(stderr, "[APU] Peeking from Write-Only register: 0x%04X\n", addr);
  return 0x00;
}

void APU::write(u16 addr, u8 val) {
  if (addr == 0x4017) {
    if (this->reg.frame_counter.disable_frame_irq != !!(val & 0x40))
      fprintf(stderr, "[APU] IRQ: %s\n", (val & 0x40) ? "OFF" : "ON");
    if (this->reg.frame_counter.disable_frame_irq != !!(val & 0x80))
      fprintf(stderr, "[APU] mode: %s\n", (val & 0x80) ? "5" : "4");
  }

  switch (addr) {
  case 0x4000: this->reg.pulse1.byte_0 = val; break;
  case 0x4001: this->reg.pulse1.byte_1 = val; break;
  case 0x4002: this->reg.pulse1.byte_2 = val; break;
  case 0x4003: this->reg.pulse1.byte_3 = val; break; // does additional stuff

  case 0x4004: this->reg.pulse2.byte_0 = val; break;
  case 0x4005: this->reg.pulse2.byte_1 = val; break;
  case 0x4006: this->reg.pulse2.byte_2 = val; break;
  case 0x4007: this->reg.pulse2.byte_3 = val; break; // does additional stuff

  case 0x4008: this->reg.triangle.byte_0 = val; break;
  case 0x400A: this->reg.triangle.byte_2 = val; break;
  case 0x400B: this->reg.triangle.byte_3 = val; break; // does additional stuff

  case 0x400C: this->reg.noise.byte_0 = val; break;
  case 0x400E: this->reg.noise.byte_2 = val; break;
  case 0x400F: this->reg.noise.byte_3 = val; break; // does additional stuff

  case 0x4010: this->reg.dmc.byte_0 = val; break;
  case 0x4011: this->reg.dmc.byte_1 = val; break;
  case 0x4012: this->reg.dmc.byte_2 = val; break;
  case 0x4013: this->reg.dmc.byte_3 = val; break;

  case 0x4015: this->reg.state.val = val; break;

  case 0x4017: this->reg.frame_counter.val = val; break;

  default:
    // fprintf(stderr, "[APU] Writing to Read-Only register: 0x%04X\n", addr);
    break;
  }
}

// mode 0:    mode 1:       function
// ---------  -----------  -----------------------------
//  - - - f    - - - - -    IRQ (if bit 6 is clear)
//  - l - l    l - l - -    Length counter and sweep
//  e e e e    e e e e -    Envelope and linear counter


void APU::cycle() {
  this->cycles++;

  // The APU is cycled at 240Hz.
  // The CPU runs at 1789773 Hz
  // Thus, only step frame counter CPUSPEED / 240 cycles
  if (this->cycles % (1789773 / 240) == 0) {
    if (this->reg.frame_counter.five_frame_seq) {
      switch(this->seq_step % 5) {
      case 0: /* do stuff */ break;
      case 1: /* do stuff */ break;
      case 2: /* do stuff */ break;
      case 3: /* do stuff */ break;
      case 4: /* do stuff */ break;
      }
    } else {
      switch(this->seq_step % 4) {
      case 0: /* do stuff */ break;
      case 1: /* do stuff */ break;
      case 2: /* do stuff */ break;
      case 3:
        // if (!this->reg.frame_counter.disable_frame_irq && /* something */)
        //   this->interrupt.request(Interrupts::IRQ);
        break;
      }
    }
    this->seq_step++;
  }
}

#endif
