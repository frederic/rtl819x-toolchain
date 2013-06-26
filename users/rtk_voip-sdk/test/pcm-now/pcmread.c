#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define BUF_SIZE	2048
#define FILE_SZ		(BUF_SIZE * 1024 * 4)	//2M

int main(int argc, char** argv)
{

	unsigned char buffer[BUF_SIZE];

	int fdw = open("/mnt/nfs/pcm/readfile", O_WRONLY | O_TRUNC | O_CREAT);

	if(fdw == -1)
	{
		printf("Can't open file for writing.\n");
		return -1;
	}

	int fdpcm = open("/dev/pcmctrl0", O_RDWR);

	if(fdpcm == -1)
	{
		printf("Can't open PCM device.\n");
		return -1;
	}

	unsigned int filesize = FILE_SZ;
	ssize_t readsize;
	ssize_t wsize;
	ssize_t n;
/*	int i;
	for(i = 0 ; i < 0xFFFF ; i++)
	{
		printf(".");
	}

	printf("\n");
*/

	while(1)
	{	
		wsize = 0;
		
		if((readsize = read(fdpcm, buffer, BUF_SIZE)) > 0)
		{

			while(readsize > 0)
			{
				n = write(fdw, buffer + wsize, readsize);
				wsize += n;
				readsize -= n;
			}

		}
		else
		{
			//printf("--------------------------------------\n");
		}
	}
	

	return 0;
}
