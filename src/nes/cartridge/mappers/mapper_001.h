#pragma once

#include "nes/generic/ram/ram.h"
#include "nes/generic/rom/rom.h"
#include "common/bitfield.h"
#include "common/util.h"
#include "../mapper.h"

// https://wiki.nesdev.com/w/index.php/MMC1
class Mapper_001 final : public Mapper {
private:
  // Banked ROMs
  struct {
    struct {
      uint  len;
      ROM** bank; // Only ever ROM
    } prg;

    struct {
      uint     len;
      Memory** bank; // Might be RAM
    } chr;
  } banks;

  // CPU Memory Space
  RAM  prg_ram; // 0x6000 ... 0x7FFF - Fixed RAM (can be disabled though)
  ROM* prg_lo;  // 0x8000 ... 0xBFFF - Switchable
  ROM* prg_hi;  // 0xC000 ... 0xFFFF - Switchable

  // PPU Memory Space
  Memory* chr_lo;  // 0x0000 ... 0x0FFF - Switchable
  Memory* chr_hi;  // 0x1000 ... 0x1FFF - Switchable

  struct { // Registers
    // Control - 0x8000 ... 0x9FFF
    // 4   0
    // -----
    // CPPMM
    // |||||
    // |||++- Mirroring (0: one-screen, lower bank; 1: one-screen, upper bank;
    // |||               2: vertical; 3: horizontal)
    // |++--- PRG ROM bank mode
    // |      (0, 1: switch 32 KB at $8000, ignoring low bit of bank number;
    // |       2: fix first bank at $8000 and switch 16 KB bank at $C000;
    // |       3: fix last bank at $C000 and switch 16 KB bank at $8000)
    // +----- CHR ROM bank mode
    //        (0: switch 8 KB at a time; 1: switch two separate 4 KB banks)
    union {
      u8 val;
      BitField<4>    chr_bank_mode;
      BitField<2, 2> prg_bank_mode;
      BitField<0, 2> mirroring;
    } control;

    // CHR Bank 0 - 0xA000 ... 0xBFFF
    // CHR Bank 1 - 0xC000 ... 0xDFFF
    // 4   0
    // -----
    // CCCCC
    // |||||
    // +++++- Select 4 KB or 8 KB CHR bank at PPU $0000 / 4 KB CHR bank at $1000
    //        (low bit ignored in 8 KB mode)
    //
    // MMC1 can do CHR banking in 4KB chunks.
    // Known carts with CHR RAM have 8 KiB, so that makes 2 banks.
    // RAM vs ROM doesn't make any difference for address lines.
    //
    // For carts with 8 KiB of CHR (be it ROM or RAM), MMC1 follows the common
    // behavior of using only the low-order bits: the bank number is in effect
    // ANDed with 1.
    union {
      u8 val;
      BitField<0, 5> bank;
    } chr0, chr1;

    // PRG Bank - 0xE000 ... 0xFFFF
    // 4   0
    // -----
    // RPPPP
    // |||||
    // |++++- Select 16 KB PRG ROM bank (low bit ignored in 32 KB mode)
    // +----- PRG RAM chip enable (0: enabled; 1: disabled; ignored on MMC1A)
    union {
      u8 val;
      BitField<0, 4> bank;
      BitField<4>    ram_enable;
    } prg;

    // Shift Register
    u8 sr;
  } reg;

  // ---- Emulation Vars and Helpers ---- //

  uint write_just_happened;

  SERIALIZE_START(3 + this->banks.chr.len, "Mapper_001")
    SERIALIZE_SERIALIZABLE(prg_ram)
    SERIALIZE_CUSTOM() {
      for (uint j = 0; j < this->banks.chr.len; j++) {
        RAM* bank = dynamic_cast<RAM*>(this->banks.chr.bank[j]);
        SERIALIZE_SERIALIZABLE_PTR(bank)
      }
    }
    SERIALIZE_POD(reg)
    SERIALIZE_POD(write_just_happened)
  SERIALIZE_END(3 + this->banks.chr.len)

  void update_banks() override;

public:
  Mapper_001(const ROM_File& rom_file);
  ~Mapper_001();

  // <Memory>
  u8 peek(u16 addr) const override;
  void write(u16 addr, u8 val) override;
  // <Memory/>

  Mirroring::Type mirroring() const override;

  void cycle() override;

  const Serializable::Chunk* getBatterySave() const override {
    return this->prg_ram.serialize();
  }
  void setBatterySave(const Serializable::Chunk* c) override {
    this->prg_ram.deserialize(c);
  }
};
