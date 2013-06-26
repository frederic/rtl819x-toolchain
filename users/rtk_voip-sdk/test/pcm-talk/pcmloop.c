/*
 *      Realtek PCM Controller Loopback Application sample.
 *
 *      Linear Mode     : 8-bit PCM raw data.
 *      a-law or u-law  : 16-bit PCM raw data.
 *
 *                      Ziv Huang
*/

#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

//--------------------------------------------------------
//	PCM Controller IOCTL
//--------------------------------------------------------
#define PCM_IOCTL_LINEAR        0xBE01
#define PCM_IOCTL_ALAW          0xBE02
#define PCM_IOCTL_ULAW          0xBE03
#define PCM_IOCTL_GET_SIZE      0xBE04  //Get page size.


int main(int argc, char** argv)
{


	int fdpcm = open("/dev/pcmctrl0", O_RDWR);

	if(fdpcm == -1)
	{
		printf("Can't open PCM device.\n");
		return -1;
	}

#if 0
        //set a-law
        if(ioctl(fdpcm, PCM_IOCTL_ALAW))
        {
                perror("PCM_IOCTL_ALAW failed!!\n");
                return -1;
        }
#endif

	//get page size
        unsigned int pagesize = 0;
        if(ioctl(fdpcm, PCM_IOCTL_GET_SIZE, &pagesize) == -1)
        {
                perror("PCM_IOCTL_GET_SIZE faile!!\n");
                return -1;
        }
	printf("PCM Controller Page Size = %d\n", pagesize);

	unsigned char buffer[pagesize];

	ssize_t readsize;
	ssize_t wsize, wcount;
	ssize_t n;
	int i;

	while(1)
	{
		wsize = 0;
		wcount = 0;
		n = 0;

		if((readsize = read(fdpcm, buffer, pagesize)) > 0)
		{
			while(readsize > 0)
			{
				n = write(fdpcm, buffer + wsize, readsize);
				wsize += n;
				readsize -= n;
			}

		}
	}

	return 0;
}
