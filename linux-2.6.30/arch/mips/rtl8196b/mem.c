/*
 *
 *  Copyright (c) 2011 Realtek Semiconductor Corp.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 */
#include <linux/config.h>
#include <linux/init.h>
#include <linux/mm.h>
#include <linux/bootmem.h>
#include <linux/string.h>

#include <asm/bootinfo.h>
#include <asm/page.h>

#include <prom.h>
#include <platform.h>

#define	DCR 0xb8001004

void __init prom_meminit(void)
{
	//char *ptr;
	unsigned int memsize;

	memsize = 0x02000000;  /* Default to 32MB */	
	/* Check the command line first for a memsize directive */
	//ptr = strstr(arcs_cmdline, "mem=");
#if 1
	/*now: alway believe DRAM configuration register*/
	{
		unsigned int DCRvalue = 0;
		unsigned int bus_width = 0, chip_sel = 0, row_cnt = 0, col_cnt = 0,bank_cnt = 0;

		DCRvalue = ( (*(volatile unsigned int *)DCR));

		/*bit 19,0:2 bank; 1: 4 bank*/
		switch(DCRvalue & 0x080000)
		{
			case 0x0:
				bank_cnt = 2;
				break;
			case 0x080000:
				bank_cnt = 4;
				break;
			default:
				bank_cnt = 0;
				break;
		}

		/*bit 22~24: colomn count*/
		switch(DCRvalue & 0x01C00000)
		{
			case 0x00000000:
				col_cnt = 256;
				break;
			case 0x00400000:
				col_cnt = 512;
				break;
			case 0x00800000:
				col_cnt = 1024;
				break;
			case 0x00C00000:
				col_cnt = 2048;
				break;
			case 0x01000000:
				col_cnt = 4096;
				break;
			default:
				printk("unknow colomn count(0x%x)\n",DCRvalue & 0x01C00000);
				break;
		}

		/*bit 25~26: row count*/
		switch(DCRvalue & 0x06000000)
		{
			case 0x00000000:
				row_cnt = 2048;
				break;
			case 0x02000000:
				row_cnt = 4096;
				break;
			case 0x04000000:
				row_cnt = 8192;
				break;
			case 0x06000000:
				row_cnt = 16384;
				break;
		}

		/*bit 27: chip select*/
		switch(DCRvalue & 0x08000000)
		{
			case 0x0:
				chip_sel = 1;
				break;
			case 0x08000000:
				chip_sel = 2;
				break;
		}

		/*bit 28~29: bus width*/
		switch(DCRvalue & 0x30000000)
		{
			case 0x0:
				bus_width = 8;
				break;
			case 0x10000000:
				bus_width = 16;
				break;
			case 0x20000000:
				bus_width = 32;
				break;
			default:
				printk("bus width is reseved!\n");
				break;
		}

		/*total size(Byte)*/
		memsize = (row_cnt * col_cnt *bank_cnt) * (bus_width >> 3) * chip_sel;		
		
	}
#endif      

   /*
    * call <add_memory_region> to register boot_mem_map
    * add_memory_region(base, size, type);
    * type: BOOT_MEM_RAM, BOOT_MEM_ROM_DATA or BOOT_MEM_RESERVED
    */
   add_memory_region(0, memsize, BOOT_MEM_RAM);
}

//unsigned long __init prom_free_prom_memory(void)
void prom_free_prom_memory(void)
{
	//unsigned long freed = 0;

	return;// freed;
}
