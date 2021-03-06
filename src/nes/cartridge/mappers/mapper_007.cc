#include "mapper_007.h"

#include <cassert>
#include <cstdio>
#include <cstring>

#include "common/debug.h"

Mapper_007::Mapper_007(const ROM_File& rom_file)
: Mapper(7, "AxROM")
{
  // Clear registers
  memset(&this->reg, 0, sizeof this->reg);

  // ---- PRG ROM ---- //

  if (rom_file.rom.prg.len == 0x4000) {
    // There was only 1 16K bank.
    // In this special case, we need to mirror it twice.
    this->banks.prg.len = 1;
    this->banks.prg.bank = new ROM* [1];

    fprintf(stderr, "[Mapper_007] Special case: mirroring single 16K ROM Bank");

    // this is a bit jank, but it will have to do for now...
    u8* tmp = new u8 [0x8000];
    memcpy(tmp + 0x0000, rom_file.rom.prg.data, 0x4000);
    memcpy(tmp + 0x4000, rom_file.rom.prg.data, 0x4000);
    this->banks.prg.bank[0] = new ROM (0x8000, tmp, "Mapper_007 PRG");
    delete[] tmp;
  } else {
    // Split PRG ROM into 32K banks
    this->banks.prg.len = rom_file.rom.prg.len / 0x8000;
    this->banks.prg.bank = new ROM* [this->banks.prg.len];

    fprintf(stderr, "[Mapper_007] 32K PRG ROM Banks: %u\n", this->banks.prg.len);

    const u8* prg_data_p = rom_file.rom.prg.data;
    for (uint i = 0; i < this->banks.prg.len; i++) {
      this->banks.prg.bank[i] = new ROM (0x8000, prg_data_p, "Mapper_007 PRG");
      prg_data_p += 0x8000;
    }
  }

  // Set default banks
  this->prg_rom = this->banks.prg.bank[0];

  // ---- CHR ROM ---- //

  if (rom_file.rom.chr.len == 0)
    fprintf(stderr, "[Mapper_007] No CHR ROM detected. Using 8K CHR RAM\n");

  // If there is no chr_rom, then intitialize chr_ram
  this->chr_mem = (rom_file.rom.chr.len == 0x2000)
    ? (Memory*)new ROM (0x2000, rom_file.rom.chr.data)
    : (Memory*)new RAM (0x2000);
}

Mapper_007::~Mapper_007() {
  for (uint i = 0; i < this->banks.prg.len; i++)
    delete this->banks.prg.bank[i];
  delete[] this->banks.prg.bank;

  delete this->chr_mem;
}

u8 Mapper_007::peek(u16 addr) const {
  // Wired to the PPU MMU
  if (in_range(addr, 0x0000, 0x1FFF)) return this->chr_mem->peek(addr);

  // Wired to the CPU MMU
  if (in_range(addr, 0x4020, 0x5FFF)) return 0x00; // Nothing in "Expansion ROM"
  if (in_range(addr, 0x6000, 0x7FFF)) return 0x00; // Nothing in SRAM
  if (in_range(addr, 0x8000, 0xFFFF)) return this->prg_rom->peek(addr - 0x8000);

  assert(false);
  return 0;
}

void Mapper_007::write(u16 addr, u8 val) {
  // TODO: handle bus conflicts?

  // Since there is potentially CHR RAM, try to write to it (if in range)
  if (in_range(addr, 0x0000, 0x1FFF)) {
    this->chr_mem->write(addr, val);
  }

  // Otherwise, handle writing to registers

  if (in_range(addr, 0x8000, 0xFFFF)) {
    this->reg.bank_select.val = val;
    this->update_banks();
  }
}

Mirroring::Type Mapper_007::mirroring() const {
  return (this->reg.bank_select.vram_page == 0)
    ? Mirroring::SingleScreenLo
    : Mirroring::SingleScreenHi;
};

void Mapper_007::update_banks() {
  const u8 bank = this->reg.bank_select.prg_bank;
  this->prg_rom = this->banks.prg.bank[bank % this->banks.prg.len];
}
