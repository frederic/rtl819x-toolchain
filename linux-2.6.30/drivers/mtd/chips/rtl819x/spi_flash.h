#ifndef _MTD_SPI_PROBE_H_
#define _MTD_SPI_PROBE_H_

struct spi_chip_mtd
{
	unsigned int 		chip_id;
	unsigned int		extra_id;
	unsigned int		sectorSize;
	unsigned int 		deviceSize;
	unsigned int 		uiClkMhz;
	char*				name;
};

struct spi_chip_info 
{
	char* name;
	unsigned int chip_select;
	struct spi_chip_mtd *flash;
	
	void (*destroy)(struct spi_chip_info *chip_info);

	unsigned int (*read)(unsigned int  from, unsigned int  to, unsigned int  size, unsigned int uiChip);
	unsigned int (*write)(unsigned int  from, unsigned int  to, unsigned int  size, unsigned int uiChip);
	int (*erase)(unsigned int  addr, unsigned int uiChip);
};

#endif /* _MTD_SPI_PROBE_H_ */


