#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define BUFFER_SIZE	1024

int main(int argv, char** argc)
{
	printf("Now open /dev/pcmctrl0\n");
	int fddev = open("/dev/pcmctrl0", O_RDWR);	
	
	if(fddev == -1)
	{
		printf("Can't open PCM device 0\n");
		return fddev;
	}
	
	int fdfile;

	if(argv == 2)
	{
		fdfile = open(argc[1], O_RDONLY);
		printf("Open PCM file - %s\n", argc[1]);
	}
	else
	{
		fdfile = open("/pcm/sig_big.snd", O_RDONLY);
		printf("Open default PCM file - /pcm/sig_big.snd\n");
	}

	if(fdfile == -1)
	{
		printf("Can't open PCM file\n");
		close(fddev);
		return -1;
	}

	printf("PCM device and PCM file are opened.\n");


	unsigned char buf[BUFFER_SIZE];
	unsigned int loopnum = 0;
	ssize_t count = 0;

	unsigned int playcount = 0;
	while(1)
	{
		while((count = read(fdfile, buf, BUFFER_SIZE)) > 0 )
		{
			//printf("read count = %d\n", count);
			ssize_t wcount = 0;
			ssize_t n = 0;

			while(count > 0)
			{
				if((n = write(fddev, buf + wcount, count)) < 0)
				{
					printf("write pcm dev error.\n");
				}
				
				wcount += n;
				count -= n;
				//printf("wcount = %d, count = %d\n", wcount, count);
			}
		}

		printf("Replay %d times", playcount);
		playcount++;
		lseek(fdfile, 0, SEEK_SET);
	}

	close(fddev);
	close(fdfile);

	return 0;
}
