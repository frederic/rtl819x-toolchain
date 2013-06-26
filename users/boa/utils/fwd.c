#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <signal.h>
#include <fcntl.h>

//keith_fwd
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
//keith_fwd
#include <unistd.h>



int main(int argc, char *argv[])
{
	unsigned char *shm_memory=NULL;
	int shm_id = 0;
	FILE *fp, *fp2;
	FILE *fp_watchdog;
	char  buf[150]={0};		
				
	while(1)
	{  	  


		sleep(3);
		

		fp = fopen("/var/fwd.ready", "r");
    if (fp)
		{
   		fgets(buf,150,fp);
			shm_id = atoi(buf);
			
			/* Attach the shared memory segment */
			shm_memory = (char *)shmat(shm_id, NULL, 0);
			

			
			fp2 = fopen("/var/fwd.conf", "r");
			if(fp2)
			{
				int head_offset=0;
				unsigned char mtd_name[20]={0};
				unsigned char offset_str[20]={0};
				unsigned char size_str[20]={0};
				unsigned char fw_offset_str[20]={0};
				
				int numWrite;
				int fw_offset=0;
				
				
				fgets(mtd_name,20,fp2);
				fgets(offset_str,20,fp2);
				fgets(size_str,20,fp2);
				fgets(fw_offset_str,20,fp2);
								
				do{
					
					int fh;
					int offset = atoi(offset_str);
				 	int size = atoi(size_str);
				 	int fw_offset = atoi(fw_offset_str);				 	

				 	mtd_name[strlen(mtd_name)-1]='\0';
				 	
				 	fh = open(mtd_name, O_RDWR);
				 	if(fh == -1)
				 	{
						//printf("\r\n Open [%s] fail.__[%s-%u]\r\n",mtd_name,__FILE__,__LINE__);				 		
						fp_watchdog = fopen("/proc/watchdog_reboot","w+"); //reboot
						if(fp_watchdog)
						{
							fputs("111", fp_watchdog);
							fclose(fp_watchdog);
						}
						
						for(;;);
				 	}
				 	lseek(fh, offset, SEEK_SET);
				 	numWrite = write(fh, &(shm_memory[fw_offset]), size);
				 	
				 	if(numWrite != size)
				 	{
						//printf("\r\n Write firmware size incorrect.__[%s-%u]\r\n",__FILE__,__LINE__);				 		
						fp_watchdog = fopen("/proc/watchdog_reboot","w+"); //reboot
						if(fp_watchdog)
						{
							fputs("111", fp_watchdog);
							fclose(fp_watchdog);
						}
						
						for(;;);			 		
				 	}
				 	sync();		
					close(fh);					 
				 	
				 	memset(mtd_name,0x00,sizeof(mtd_name));
				 	memset(offset_str,0x00,sizeof(offset_str));
				 	memset(size_str,0x00,sizeof(size_str));
				 	memset(fw_offset_str,0x00,sizeof(fw_offset_str));
				 	
				 	fgets(mtd_name,20,fp2);
					fgets(offset_str,20,fp2);
					fgets(size_str,20,fp2);
					fgets(fw_offset_str,20,fp2);
					
				}while( !feof(fp2) );
				
				fclose(fp2);
												
				fp_watchdog = fopen("/proc/watchdog_reboot","w+"); //reboot
				if(fp_watchdog)
				{
					fputs("111", fp_watchdog);
					fclose(fp_watchdog);
				}
				for(;;);
			}
			else
			{
				fp_watchdog = fopen("/proc/watchdog_reboot","w+"); //reboot
				if(fp_watchdog)
				{
					fputs("111", fp_watchdog);
					fclose(fp_watchdog);
				}
				
				for(;;);
				
			}

			
				
			fclose(fp);
		}
		

	}
}
	
