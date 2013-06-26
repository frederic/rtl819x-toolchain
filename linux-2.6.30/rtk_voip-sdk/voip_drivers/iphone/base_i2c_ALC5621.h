#ifndef __BASE_I2C_ALC5621_H__
#define __BASE_I2C_ALC5621_H__

extern void init_i2c_gpio(void);
extern unsigned short int read_ALC5621(unsigned char Reg);
extern void write_ALC5621(unsigned char Reg,unsigned short int data);
extern unsigned short int read_ALC5621_hidden(unsigned char index);
extern void write_ALC5621_hidden(unsigned char index, unsigned short int data);

#endif /* __BASE_I2C_ALC5621_H__ */

