#include "parse_rom.h"

#include "rom_file.h"

#include <cstdio>

static ROM_File* parse_iNES(const u8* data, uint data_len) {
  (void)data_len;

  const u8 prg_rom_pages = data[4];
  const u8 chr_rom_pages = data[5];

  fprintf(stderr, "[File Parsing][iNES] PRG-ROM pages: %d\n", prg_rom_pages);
  fprintf(stderr, "[File Parsing][iNES] CHR-ROM pages: %d\n", chr_rom_pages);

  // Can't use a ROM with no prg_rom!
  if (prg_rom_pages == 0) {
    fprintf(stderr, "[File Parsing][iNES] Invalid ROM! "
                    "Can't have rom with no PRG ROM!\n");
    return nullptr;
  }

  // Cool, this seems to be a valid iNES rom.
  // Let's allocate the rom_file, and get to work!
  ROM_File* rom_file = new ROM_File();
  rom_file->data     = data;
  rom_file->data_len = data_len;

  // 7       0
  // ---------
  // NNNN FTBM

  // N: Lower 4 bits of the Mapper
  // F: has_4screen
  // T: has_trainer
  // B: has_battery
  // M: mirror_type (0 = horizontal, 1 = vertical)

  rom_file->meta.has_trainer = nth_bit(data[6], 2);
  rom_file->meta.has_battery = nth_bit(data[6], 1);

  if (nth_bit(data[6], 3)) {
    rom_file->meta.mirror_mode = Mirroring::FourScreen;
  } else {
    if (nth_bit(data[6], 0)) {
      rom_file->meta.mirror_mode = Mirroring::Vertical;
    } else {
      rom_file->meta.mirror_mode = Mirroring::Horizontal;
    }
  }

  if (rom_file->meta.has_trainer)
    fprintf(stderr, "[File Parsing][iNES] ROM has a trainer\n");
  if (rom_file->meta.has_battery)
    fprintf(stderr, "[File Parsing][iNES] ROM has a battery\n");

  fprintf(stderr, "[File Parsing][iNES] Initial Mirroring Mode: %s\n",
    Mirroring::toString(rom_file->meta.mirror_mode));

  // 7       0
  // ---------
  // NNNN xxPV

  // N: Upper 4 bits of the mapper
  // P: is_PC10
  // V: is_VS
  // x: is_NES2 (when xx == 10)

  const bool is_NES2 = nth_bit(data[7], 3) && !nth_bit(data[6], 2);
  if (is_NES2) {
    // Ideally, use this data, but for now, just log a message...
    fprintf(stderr, "[File Parsing][iNES] ROM has NES 2.0 header.\n");
  }

  rom_file->meta.is_PC10 = nth_bit(data[7], 1);
  rom_file->meta.is_VS   = nth_bit(data[7], 0);

  if (rom_file->meta.is_PC10)
    fprintf(stderr, "[File Parsing][iNES] This is a PC10 ROM\n");
  if (rom_file->meta.is_VS)
    fprintf(stderr, "[File Parsing][iNES] This is a VS ROM\n");

  rom_file->meta.mapper = data[6] >> 4 | (data[7] & 0xFF00);

  fprintf(stderr, "[File Parsing][iNES] Mapper: %d\n", rom_file->meta.mapper);

  // I'm not parsing the rest of the header, since it's not that useful


  // Finally, use the header data to get pointers into the data for the various
  // chunks of ROM

  // iNES is laid out as follows:

  // Section             | Multiplier    | Size
  // --------------------|---------------|--------
  // Header              | 1             | 0x10
  // Trainer ROM         | has_trainer   | 0x200
  // Program ROM         | prg_rom_pages | 0x4000
  // Character ROM       | chr_rom_pages | 0x2000
  // PlayChoice INST-ROM | is_PC10       | 0x2000
  // PlayChoice PROM     | is_PC10       | 0x10

  const u8* data_p = data + 0x10; // move past header

  // Look for the trainer
  if (rom_file->meta.has_trainer) {
    rom_file->rom.misc.trn_rom = data_p;
    data_p += 0x200;
  } else {
    rom_file->rom.misc.trn_rom = nullptr;
  }

  rom_file->rom.prg.len = prg_rom_pages * 0x4000;
  rom_file->rom.prg.data = data_p;
  data_p += rom_file->rom.prg.len;

  rom_file->rom.chr.len = chr_rom_pages * 0x2000;
  rom_file->rom.chr.data = data_p;
  data_p += rom_file->rom.chr.len;

  if (rom_file->meta.is_PC10) {
    rom_file->rom.misc.pci_rom = data_p;
    rom_file->rom.misc.pc_prom = data_p + 0x2000;
  } else {
    rom_file->rom.misc.pci_rom = nullptr;
    rom_file->rom.misc.pc_prom = nullptr;
  }

  return rom_file;
}

static ROMFileFormat::Type rom_type(const u8* data, uint data_len) {
  (void)data_len;

  // Can't parse data if there is none ;)
  if (data == nullptr) {
    fprintf(stderr, "[File Parsing] ROM file is nullptr!\n");
    return ROMFileFormat::INVALID;
  }

  // Try to determine ROM format
  const bool is_iNES = (data[0] == 'N' &&
                        data[1] == 'E' &&
                        data[2] == 'S' &&
                        data[3] == 0x1A);

  if (is_iNES) {
    fprintf(stderr, "[File Parsing] ROM has iNES header.\n");

    // Double-check that it's not NES2
    const bool is_NES2 = nth_bit(data[7], 3) && !nth_bit(data[6], 2);
    if (is_NES2) {
      fprintf(stderr, "[File Parsing] ROM has NES 2.0 header.\n");
      return ROMFileFormat::NES2;
    }

    return ROMFileFormat::iNES;
  }

  fprintf(stderr, "[File Parsing] Cannot identify ROM type!\n");
  return ROMFileFormat::INVALID;
}

ROM_File* parse_ROM(const u8* data, uint data_len) {
  // Determine ROM type, and parse it
  switch(rom_type(data, data_len)) {
  case ROMFileFormat::iNES: return parse_iNES(data, data_len);
  case ROMFileFormat::NES2: return parse_iNES(data, data_len);
  case ROMFileFormat::INVALID: return nullptr;
  }
}
