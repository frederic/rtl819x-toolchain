#ifndef __BASE_I2C_WM8510_H__
#define __BASE_I2C_WM8510_H__


extern void init_i2c_gpio(void);
unsigned short int read_WM8510(unsigned char Reg);
void write_WM8510(unsigned char Reg,unsigned short int data);


#endif /* __BASE_I2C_WM8510_H__ */
