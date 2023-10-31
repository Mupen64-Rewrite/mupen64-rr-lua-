/**
 * Mupen64 - rom.c
 * Copyright (C) 2002 Hacktarux
 *
 * Mupen64 homepage: http://mupen64.emulation64.com
 * email address: hacktarux@yahoo.fr
 *
 * If you want to contribute to the project please contact
 * me first (maybe someone is already making what you are
 * planning to do).
 *
 *
 * This program is free software; you can redistribute it and/
 * or modify it under the terms of the GNU General Public Li-
 * cence as published by the Free Software Foundation; either
 * version 2 of the Licence, or any later version.
 *
 * This program is distributed in the hope that it will be use-
 * ful, but WITHOUT ANY WARRANTY; without even the implied war-
 * ranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public Licence for more details.
 *
 * You should have received a copy of the GNU General Public
 * Licence along with this program; if not, write to the Free
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139,
 * USA.
 *
**/

/* This is the functions that load a rom into memory, it loads a roms
 * in multiple formats, gzipped or not. It searches the rom, in the roms
 * subdirectory or in the path specified in the path.cfg file.
 */

#include <stdio.h>
#include <stdlib.h>
#include <zlib.h>
#include <string.h>
#include <ctype.h>

#include "rom.h"
#include "../memory/memory.h"
#include "unzip.h"
#include "guifuncs.h"
#include "md5.h"
#include "mupenIniApi.h"
#include "guifuncs.h"
#include "../main/win/Config.hpp"
#include <win/features/RomBrowser.hpp>
#include <win/main_win.h>

uint8_t* rom;
size_t rom_size;
char rom_md5[33];

t_rom_header ROM_HEADER;

void print_rom_info()
{
    printf("--- Rom Info ---\n");
    printf("%x %x %x %x\n", ROM_HEADER.init_PI_BSB_DOM1_LAT_REG,
           ROM_HEADER.init_PI_BSB_DOM1_PGS_REG,
           ROM_HEADER.init_PI_BSB_DOM1_PWD_REG,
           ROM_HEADER.init_PI_BSB_DOM1_PGS_REG2);
    printf("Clock rate: %x\n", sl((unsigned int)ROM_HEADER.ClockRate));
    printf("Version: %x\n", sl((unsigned int)ROM_HEADER.Release));
    printf("CRC: %x %x\n", sl((unsigned int)ROM_HEADER.CRC1), sl((unsigned int)ROM_HEADER.CRC2));
    printf("Name: %s\n", (char*)ROM_HEADER.nom);
    if (sl(ROM_HEADER.Manufacturer_ID) == 'N') printf("Manufacturer: Nintendo\n");
    else printf("Manufacturer: %x\n", (unsigned int)(ROM_HEADER.Manufacturer_ID));
    printf("Cartridge ID: %x\n", ROM_HEADER.Cartridge_ID);
    printf("Size: %d\n", rom_size);
    printf("PC: %x\n", sl((unsigned int)ROM_HEADER.PC));
    printf("Country: %s", country_code_to_country_name(ROM_HEADER.Country_code).c_str());
    printf("----------------\n");
}


void strip_extension(char* fname)
{
    char* end = fname + strlen(fname);

    while (end > fname && *end != '.' && *end != '\\' && *end != '/')
    {
        --end;
    }
    if ((end > fname && *end == '.') &&
        (*(end - 1) != '\\' && *(end - 1) != '/'))
    {
        *end = '\0';
    }
}

std::string country_code_to_country_name(uint16_t country_code)
{
    switch (country_code & 0xFF)
    {
    case 0:
        return "Demo";
    case '7':
        return "Beta";
    case 0x41:
        return "USA/Japan";
    case 0x44:
        return "Germany";
    case 0x45:
        return "USA";
    case 0x46:
        return "France";
     case 'I':
        return "Italy";
    case 0x4A:
        return "Japan";
    case 'S':
        return "Spain";
    case 0x55:
    case 0x59:
        return "Australia";
    case 0x50:
    case 0x58:
    case 0x20:
    case 0x21:
    case 0x38:
    case 0x70:
        return "Europe";
    default:
        return "Unknown (" + std::to_string(country_code & 0xFF) + ")";
    }
}

uint32_t get_vis_per_second(uint16_t country_code)
{
    switch (country_code & 0xFF)
    {
    case 0x44:
    case 0x46:
    case 0x49:
    case 0x50:
    case 0x53:
    case 0x55:
    case 0x58:
    case 0x59:
        return 50;
    case 0x37:
    case 0x41:
    case 0x45:
    case 0x4a:
        return 60;
    default:
        return 60;
    }
}


int rom_read(const char* argv)
{
    if (rom)
    {
        free(rom);    
    }
    
    // TODO: support zipped roms
    FILE* f = fopen(argv, "rb");
    fseek(f, 0, SEEK_END);
    rom_size = ftell(f);
    fseek(f, 0, SEEK_SET);
    
    unsigned long taille = rom_size;
    if (Config.use_summercart && taille < 0x4000000) taille = 0x4000000;
    rom = (unsigned char*)malloc(taille);

    fread(rom, rom_size, 1, f);
    fclose(f);

    uint8_t tmp;
    if (rom[0] == 0x37)
    {
        
        for (size_t i = 0; i < (rom_size / 2); i++)
        {
            tmp = rom[i * 2];
            rom[i * 2] = rom[i * 2 + 1];
            rom[i * 2 + 1] = (unsigned char)tmp;
        }
    }
    if (rom[0] == 0x40)
    {
        for (size_t i = 0; i < (rom_size / 4); i++)
        {
            tmp = rom[i * 4];
            rom[i * 4] = rom[i * 4 + 3];
            rom[i * 4 + 3] = (unsigned char)tmp;
            tmp = rom[i * 4 + 1];
            rom[i * 4 + 1] = rom[i * 4 + 2];
            rom[i * 4 + 2] = (unsigned char)tmp;
        }
    }
    else if ((rom[0] != 0x80)
        || (rom[1] != 0x37)
        || (rom[2] != 0x12)
        || (rom[3] != 0x40)

    )
    {
        printf("wrong file format !\n");
        free(rom);
        rom = nullptr;
        return 1;
    }
    printf("rom loaded succesfully\n");
    
    memcpy(&ROM_HEADER, rom, sizeof(t_rom_header));
    ROM_HEADER.unknown = 0;
    // Clean up ROMs that accidentally set the unused bytes (ensuring previous fields are null terminated)
    ROM_HEADER.Unknown[0] = 0;
    ROM_HEADER.Unknown[1] = 0;
   
    //trim header
    memcpy(ROM_HEADER.nom, trim((char*)ROM_HEADER.nom), sizeof(ROM_HEADER.nom));

    {
        md5_state_t state;
        md5_byte_t digest[16];
        md5_init(&state);
        md5_append(&state, rom, rom_size);
        md5_finish(&state, digest);

        char arg[256] = { 0 };
        for (size_t i = 0; i < 16; i++) sprintf(arg + i * 2, "%02X", digest[i]);
        strcpy(rom_md5, arg);

    }
    
    return 0;
}
