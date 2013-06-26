/*
 *	Realtek PCM Controller Application sample.
 *
 *	Linear Mode 	: 8-bit PCM raw data.
 *	a-law or u-law	: 16-bit PCM raw data.
 *
 *			Ziv Huang - 2005.4.8
*/


#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

//ioctl number
#define PCM_IOCTL_SET_LINEAR	0xBE01
#define PCM_IOCTL_SET_ALAW	0xBE02
#define PCM_IOCTL_SET_ULAW	0xBE03
#define PCM_IOCTL_GET_SIZE	0xBE04	//Get page size.
#define PCM_IOCTL_SET_EX_CLK	0xBE05	//External clock source from codec
#define PCM_IOCTL_SET_IN_CLK	0xBE06	//Clock source from internal PLL (Output to codec)

//read sample file and write data to PCM controller
int rwtest(int dev_fd, int file_fd, size_t buffer_size)
{
	unsigned char buf[buffer_size];
	size_t readlen, count, n, wcount;
	int result = 0;

	do
	{
		if((count = readlen = read(file_fd, buf, buffer_size)) == -1)
		{
			perror("Read file failed.\n");
			result = -1;
			break;
		}

		wcount = 0;

		while(count > 0)
		{
			if((n = write(dev_fd, buf + wcount, count)) < 0)
			{
				perror("write pcm dev error.\n");
				return -1;
			}
			
			wcount += n;
			count -= n;
		}

	}while(readlen > 0);

	return result;
}

int main(int argv, char** argc)
{

	if(argv == 3)
	{
	}
	else
	{
		printf("USAGE: %s file_for_linear(8-bit) file_for_a-law( or u-law, 16-bit)\n", argc[0]);
		exit(1);
	}


	int result = 0;

	int fd_dev = open("/dev/pcmctrl0", O_RDWR);	
	
	if(fd_dev == -1)
	{
		perror("Can't open PCM device 0\n");
		exit(1);
	}

	int fd_8bit = open(argc[1], O_RDONLY);

	if(fd_8bit == -1)
	{
		perror("Can't open %s .\n", argc[1]);
		close(fd_dev);
		exit(1);
	}

	int fd_16bit = open(argc[2], O_RDONLY);

	if(fd_16bit == -1)
	{
		perror("Can't open %s .\n", argc[2]);
		close(fd_dev);
		close(fd_8bit);
		exit(1);
	}

	
	//You should get page size first for buffering size.
	//You should use this size for good quality and performance.
	unsigned int pagesize = 0;
	if( (result = ioctl(fd_dev, PCM_IOCTL_GET_SIZE, &pagesize)) == -1)
	{
		perror("PCM_IOCTL_GET_SIZE faile!!\n");
		goto failed;
	}

	printf("PCM device page size = %d\n", pagesize);

	printf("Going to test a-law mode...\n");
	//set a-law
	if( (result = ioctl(fd_dev, PCM_IOCTL_SET_ALAW)) == -1)
	{
		perror("PCM_IOCTL_SET_ALAW failed!!\n");
		goto failed;
	}

	//use 16-bit PCM raw data as source.
	if(rwtest(fd_dev, fd_16bit, pagesize))
	{
		perror("A-Law : Read %s and write PCM device failed.\n", argc[2]);
		result = -1;
		goto failed;
	}

	printf("Going to test linear mode...\n");
	//set linear
	if( (result = ioctl(fd_dev, PCM_IOCTL_SET_LINEAR)) == -1)
	{
		perror("PCM_IOCTL_SET_LINEAR failed!!\n");
		goto failed;
	}

	//use 8-bit PCM raw data as source.
	if(rwtest(fd_dev, fd_8bit, pagesize))
	{
		perror("Linear : Read %s and write PCM device failed.\n", argc[1]);
		result = -1;
		goto failed;
	}

	printf("Going to test u-law mode...\n");
	//set u-law
	if( (result = ioctl(fd_dev, PCM_IOCTL_SET_ULAW)) == -1)
	{
		perror("PCM_IOCTL_SET_ULAW failed!!\n");
		goto failed;
	}

	//use 16-bit PCM raw data as source.
	lseek(fd_16bit, 0, SEEK_SET);
	if(rwtest(fd_dev, fd_16bit, pagesize))
	{
		perror("U-Law : Read %s and write PCM device failed.\n", argc[2]);
		result = -1;
		goto failed;
	}

	printf("Going to test external clock source.\n");
	//set external clock source.
	if( (result = ioctl(fd_dev, PCM_IOCTL_SET_EX_CLK)) == -1)
	{
		perror("PCM_IOCTL_SET_EX_CLK failed!!\n");
		goto failed;
	}

	printf("Going to test internal clock source.\n");
	//set external clock source.
	if( (result = ioctl(fd_dev, PCM_IOCTL_SET_IN_CLK)) == -1)
	{
		perror("PCM_IOCTL_SET_IN_CLK failed!!\n");
		goto failed;
	}

	//use 16-bit PCM raw data as source.
	printf("Play sound.\n");
	lseek(fd_16bit, 0, SEEK_SET);
	if(rwtest(fd_dev, fd_16bit, pagesize))
	{
		perror("U-Law : Read %s and write PCM device failed.\n", argc[2]);
		result = -1;
		goto failed;
	}


	printf("Test OK!!\n");


failed:
	close(fd_dev);
	close(fd_8bit);
	close(fd_16bit);
	return result;
}
