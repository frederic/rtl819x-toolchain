
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <sys/time.h>

#include <time.h>
#include <linux/videodev.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <signal.h>
#include <pthread.h>
#include <sys/fcntl.h>

#include "tcputils.h"
#include "utils.h"
#include "avilib.h"
#include "spcav4l.h"
#include "spcaframe.h"
#include "ushare.h"
#include "metadata.h"
#include "grab_avi.h"
#include "mime.h"
struct vdIn videoIn;
int frame_size;
avi_t *out_fd;

#define SHARE_DIR "/mnt"//"/home/share/media"//"/mnt"
pthread_mutex_t grab_ctrl_mutex, Mov_mutex, Pic_mutex;
pthread_cond_t grab_ctrl_cond, Mov_cond, Pic_cond;
int camloop = 1;

unsigned char* prv_frame;
int framecount = 0;
int webcam_exit_flag = 0;
int Mov_exit_flag = 1;
int Pic_exit_flag = 1;
int FPS = 10;
double delay_time = 0;


int serv_sock = 0;
int width = 352;
int height = 288;

pthread_mutex_t WebcamFileMutex;
int readcnt =0;
int writecnt =0;
char vodSec = 5; //預設攝影區間5秒
char *saveName_tmp;
char *saveName[2];
int nowFile = 0;
int PicTime = 60;//預設60秒抓一次圖
int PicNumByTime = 1;

typedef struct _Msg//control interface傳回的訊息
{
	char ok[2];
	char incWin;
	char decWin;
	char brigFlag;
	char contFlag;
	char brig[2];
	char cont[2];
	char secFlag;
	
	char PicTime[2];
	//char PicNumByTime;
	char MovFlag;
	char PicFlag;
	
	
} Msg;


void stop()
{
	webcam_exit_flag = 1;
	Pic_exit_flag =1;
	Mov_exit_flag =1;
	camloop = 0;
}

void dump_frame( struct frame_t* f )
{
	int i;
	printf("Header:\t" );
	for( i = 0; i <5; i++ )
	{
		printf("0x%x ", f->header[i] );
	}
	printf("\n");
	printf("Number of frame:\t%d\n",f->nbframe);
	printf("Time:\t%f\n", f->seqtimes);
	printf("deltatimes:\t%d\n", f->deltatimes);
	printf("Width:\t%d\n", f->w);
	printf("Height:\t%d\n", f->h);
	printf("Size:\t%d\n", f->size);
	if( f->format == 21 )
		printf("Format:\tJPEG\n");
	else
		printf("Format:\t%d\n", f->format);
	printf("bright:\t%d\n",f->bright);
	printf("contrast:\t%d\n",f->contrast);
	printf("colors:\t%d\n",f->colors);
	printf("exposure:\t%d\n",f->exposure);
	printf("wakeup:\t%d\n",f->wakeup);
	
	
}
int init_dev(  char* videodevice, int grabmethod)
{
	printf("%s, %d\n", __FUNCTION__, __LINE__);

	memset (&videoIn, 0, sizeof (struct vdIn));
	extern struct ushare_t* ut;
	frame_size = width*height;
	if( videodevice ==NULL )
		videodevice = strdup("/dev/video0");
	printf("%s, %d\n", __FUNCTION__, __LINE__);	
	if( init_videoIn(&videoIn, videodevice, width, height, VIDEO_PALETTE_JPEG,grabmethod) < 0 )
		return -1;
	//v4lGrab ( &videoIn );
	//printf( "Cam=>%s", videoIn.cameraname );
	printf("%s, %d\n", __FUNCTION__, __LINE__);
	usleep(1000);
	printf("%s, %d\n", __FUNCTION__, __LINE__);
	return 0;
}

int newfile( char* fileName )
{
  	if( fileName == NULL )
		fileName = strdup("webcam.avi");
  	if ((out_fd = AVI_open_output_file (fileName)) == NULL)
	{
			printf ("cannot open write file ? \n");
			return -1;
	}
	AVI_set_video (out_fd, width, height, 1, "MJPG");
	return 0;
}

int size[] = { 352,288,320,240,192,144,176,144,160,120 };//640,480,384,288,

void newServer(void* p)
{
	pthread_t server_th;
	int *pp = (int*)p;
	int port = *pp;
	int new_sock;
	int i;
	struct sockaddr_in cli_addr;
	struct sigaction sa;
	int addrlen;

	int j;
	int index =0;
	struct frame_t *frame_index;
	int maxfd = 0;
	

	
	serv_sock = open_sock(port);
	signal(SIGPIPE, SIG_IGN);
	//sa.sa_handler = stop;
	//sigemptyset(&sa.sa_mask);
	//sa.sa_flags = SA_RESTART;
	printf("Waiting client connect to port:%d \n",port);
	addrlen = sizeof(struct sockaddr_in);
	while( !webcam_exit_flag )
	{
		
		if(	(new_sock = accept( serv_sock, &cli_addr, &addrlen )) >= 0 )
		{
			printf("new_sock=%d\n",new_sock);
			printf("New sock connect from %s:%d\n", inet_ntoa(cli_addr.sin_addr), cli_addr.sin_port );
			
			pthread_create(&server_th, NULL, &loopSock, &new_sock);
		}
	}
}

void loopSock(void *cli)
{
	int *id = (int*)cli;
	int cli_sock = *id;
	int i, index =1;
	struct frame_t *frame_index;
	
	//unsigned char* frame_tmp = (unsigned char*)malloc (frame_size); 
	//struct client_t message;
	Msg message;
	int maxw = videoIn.videocap.maxwidth;
	int minw = videoIn.videocap.minwidth;
	int maxh = videoIn.videocap.maxheight;
	int minh = videoIn.videocap.minheight;
	int sizeIndex;
	//unsigned char* frame_tmp = (unsigned char*)malloc (frame_size);
	int cntl;
	printf("client: %d enter\n",cli_sock);
	//cntl = fcntl( cli_sock, F_GETFL );
	//cntl |= O_NONBLOCK;
	//fcntl( cli_sock, F_SETFL,cntl );
	while(!webcam_exit_flag)
	{
		
	 	
	redo:
		index = ((videoIn.frame_cour-1) < 0)?3:(videoIn.frame_cour-1);
		//while ((index == videoIn.frame_cour) && !webcam_exit_flag) usleep(500);
		//videoIn.framelock[index]++;
		//memcpy( frame_tmp, videoIn.ptframe[index], frame_size );
		//videoIn.framelock[index]--;
		if(!webcam_exit_flag)
		{
			
			frame_index = (struct frame_t *) videoIn.ptframe[index];
			if(frame_index->size==0)
			{
			
				index = (index+1)% OUTFRMNUMB;  
				goto redo;
			}
			//dump_frame( frame_index);
			frame_index->movflag = !Mov_exit_flag;
			frame_index->picflag = !Pic_exit_flag;
			if(write_sock(cli_sock, (unsigned char *)frame_index, sizeof(struct frame_t))<0)
				break;
			if(write_sock(cli_sock, (unsigned char *)(videoIn.ptframe[index]+sizeof(struct frame_t)), frame_index->size)<0 )
				break;
			
		}else
			break; 
			
		memset(&message,0,sizeof(Msg));
	 	if(read(cli_sock,(unsigned char*)&message,sizeof(Msg)) < 0 )
	 	{
	 		if( errno == EWOULDBLOCK )
	 			continue;
	 		
	 		break;
	 	}
	 	//printf("Read header from socket %d\n",cli_sock );
	 	if( message.ok[0] == 'O' )
	 	{
	 		//printf( "brig=%d cont=%d\n",message.brig,message.cont);
	 		if(message.decWin)
	 		{
	 			for( i = 0; i < 10; i+=2 ) //find now size
	 				if( (videoIn.hdrwidth == size[i]) &&(videoIn.hdrheight == size[i+1]) )
	 					break;
	 			if( i < 12 )
	 			{
	 				i +=2;
	 				if((size[i] >= minw) && (size[i+1] >= minh ) )
	 				{
	 					width = size[i];height = size[i+1];
		 				videoIn.hdrwidth = size[i];videoIn.hdrheight = size[i+1];
		 				changeSize (&videoIn);
	 				}
	 			}
	 		}
			
			if(message.incWin)
			{
				for( i = 0; i < 10; i+=2 ) //find now size
	 				if( (videoIn.hdrwidth == size[i]) &&(videoIn.hdrheight == size[i+1]) )
	 					break;
	 			if( i >= 2 )
	 			{
	 				i -=2;
	 				if((size[i] <= maxw) && (size[i+1] <= maxh ) )
	 				{
	 					width = size[i];height = size[i+1];
		 				videoIn.hdrwidth = size[i];videoIn.hdrheight = size[i+1];
		 				changeSize (&videoIn);
	 				}
	 			}
			}
			int b = 0;
			int c=0;
			int pict =0;
			b = (int)message.brig[1] << 8;
			b = b+(int)message.brig[0];
			c = (int)message.cont[1] << 8;
			c = c+(int)message.cont[0];
			pict = (int)message.PicTime[0] << 8;
			pict = pict+(int)message.PicTime[1];
			PicTime = pict;
			if(message.brigFlag) SpcaSetBrightness(&videoIn, b);
			if(message.contFlag) SpcaSetContrast(&videoIn, c);
			vodSec = message.secFlag;
			if ( message.MovFlag &&( Mov_exit_flag==1 ) )//啟動錄影功能
			{
				Mov_exit_flag=0;
				pthread_mutex_lock (&Mov_mutex);
				pthread_cond_signal (&Mov_cond);
				pthread_mutex_unlock (&Mov_mutex);
			}
			if ( message.PicFlag &&( Pic_exit_flag==1 ) )//啟動照相功能
			{
				Pic_exit_flag=0;
				pthread_mutex_lock (&Pic_mutex);
				pthread_cond_signal (&Pic_cond);
				pthread_mutex_unlock (&Pic_mutex);
			}
			if ( !message.MovFlag &&( Mov_exit_flag==0 ) )
				Mov_exit_flag=1;
			if ( !message.PicFlag &&( Pic_exit_flag==0 ) )
				Pic_exit_flag=1;
				 			
	 	}
		//index = (index+1)% OUTFRMNUMB;  
	}

	printf("client: %d exit\n",cli_sock);
	close_sock(cli_sock);
	//free(frame_tmp);
	pthread_exit(NULL);
}

int CreateDir(char *path)
{
	char Mkdir[128];

	struct stat dirStat;
	if(path)
	{
		
		if ( stat( path,&dirStat ) < 0 ){
			printf("error %d %s\n",errno,strerror(errno));
			if( errno == ENOENT ){
				printf("Create path: %s\n", path );
				sprintf(Mkdir, "mkdir -p %s", path);
				system(Mkdir);
			}
			else
				return -1;			
		}
		/*if( chdir( path ) == 0){
			printf("Change work path to %s\n", path );
		}*/
		
		return 0;		
	}
	else
	{
		printf("Can't find  dir\n");
		return -1;
	}
}
void loopFile(struct ushare_t *ut)
{
	int index = 0;
	int i;
	double start_t, end_t;
	struct upnp_entry_t *cam = NULL;
	struct stat fileStat;
	double Mov_start, Mov_end;
	char MovName[128];
	char MovDirPath[128];
	char DirName[64];
	char fullpath[256];
	char *shareDir = strdup(SHARE_DIR);
	avi_t *Mov_fd;
	
	unsigned char* frame_tmp = (unsigned char*)malloc (frame_size);
	//char buf[1024];
	
	while(!webcam_exit_flag)
	{
		pthread_mutex_lock (&Mov_mutex);//wait somebody to signal it
		pthread_cond_wait (&Mov_cond, &Mov_mutex);
		pthread_mutex_unlock (&Mov_mutex);
		sprintf(MovDirPath,"%s/WebCamMov",  shareDir);
		Mov_start = ms_time();
		Mov_end = Mov_start; 
		while(!Mov_exit_flag)
		{
			printf("Movie grab start!\n");
			start_t = ms_time();
			end_t = start_t;
			if(	newfile(saveName_tmp) != 0 )
				Mov_exit_flag = 1;
			while( (end_t-start_t) <  (vodSec*1000))//10 sec
			{
			
				struct frame_t *f;
				if( Mov_exit_flag == 1) break;
				if(webcam_exit_flag == 1){Mov_exit_flag =1;	return;}
			
				//v4lGrab ( &videoIn );
				//grab();
			
				bzero( frame_tmp, strlen( frame_tmp) );
				while ((index == videoIn.frame_cour) && !webcam_exit_flag) usleep(500);
				
				//index = ((&videoIn.frame_cour-1) < 0)?3:(&videoIn.frame_cour-1);
				videoIn.framelock[index]++;
				memcpy( frame_tmp, videoIn.ptframe[index], frame_size );
				videoIn.framelock[index]--;
				f = (struct frame_t *)frame_tmp;
				int fs = get_jpegsize (frame_tmp +sizeof(struct frame_t), frame_size);
				if (AVI_write_frame (out_fd, frame_tmp+sizeof(struct frame_t), fs) < 0)
					printf ("write error on avi out \n");
				sync();
			
		
				//f = (struct frame_t*)frame_tmp;
				delay_time = f->deltatimes;//end_time - start_time;
				int insFrame = (delay_time*FPS)/1000;
				int i;
				for( i = 0; i < insFrame; i++ )
				{
					AVI_write_frame (out_fd, frame_tmp+sizeof(struct frame_t), fs);
					sync();
				}
				
				index = (index+1)% OUTFRMNUMB;  
			
				if( webcam_exit_flag == 1){Mov_exit_flag =1;	return;}
				double wait = (1000*1000)/FPS - (delay_time*1000);
				if( wait > 0 )
					usleep( wait );
			
				end_t =ms_time();	
				//usleep((1000*1000)/FPS);
			}//end 30sec
			out_fd->fps = FPS;//(double) (framecount *1000) / (start_time-end_time);
			printf("FPS: %d\n",FPS);
			printf("Time: %fms\n",end_t-start_t);
			AVI_close (out_fd);
			sync();
		
			if( readcnt > 0 )
				nowFile = (nowFile+1)%2;
		
			pthread_mutex_lock (&WebcamFileMutex);
			writecnt++;
		
			char cp_file[256];
			bzero( cp_file, sizeof( cp_file));
			sprintf(cp_file, "cp -f %s %s", saveName_tmp, saveName[nowFile] );
			system(cp_file);
			stat(saveName[nowFile], &fileStat);
			writecnt--;
			pthread_mutex_unlock (&WebcamFileMutex); 
			
			
			cam = upnp_get_entry ( ut, WebCamID );
  			/*if(	stat(saveName[nowFile], &fileStat)  >=0)
  			{
  				if( cam != NULL )
  				{
  					cam->size = fileStat.st_size;
  					printf("File exist now name is %s\n", saveName[nowFile] );
  				}
  				else
  				{
  					cam = upnp_file_new (ut, WebCamName, saveName[nowFile], ut->root_entry);

					if( cam )
					{
				  	  upnp_entry_add_child (ut, ut->root_entry, cam);
					}
					printf("New file %s\n", saveName[nowFile] );
  				}
  				
  			}*/
			if( cam != NULL )
			{
				upnp_entry_free ( ut, cam );
				cam = NULL;
			}
		  		
		    cam = upnp_file_new (ut, WebCamName, saveName[nowFile], ut->root_entry);

			if( cam )
			{
		  	  upnp_entry_add_child (ut, ut->root_entry, cam);
			}
			printf("New file %s\n", saveName[nowFile] );
	    
	    }
	    
	}
	free(frame_tmp);
}


struct upnp_entry_t *PicDir_root_entry = NULL;
struct upnp_entry_t *PicDir_entry = NULL;

void PicSaver(struct ushare_t *ut)
{
	char PicName[128];
	char PicDirPath[128];
	char DirName[64];
	char fullpath[256];
	char *shareDir = strdup(SHARE_DIR);//strdup("/home/share");
	struct stat dirStat;
	int index = 0;
	int i;
	int dirID =-1;
	int sizeout = 0;
	struct frame_t *frame_index;
	
	struct tm *p;
	char *workPath =NULL;
	unsigned char* frame = (unsigned char*)malloc (frame_size); 
	unsigned char* picData;
	int picfp;
	
	while( !webcam_exit_flag)
	{
		pthread_mutex_lock (&Pic_mutex);//wait somebody to signal it
		pthread_cond_wait (&Pic_cond, &Pic_mutex);
		pthread_mutex_unlock (&Pic_mutex);
		sprintf(PicDirPath,"%s/WebCamPic",  shareDir);
		if( PicDir_root_entry == NULL )
		{
			PicDir_root_entry = upnp_dir_new (ut, "WebCamPic", PicDirPath, ut->root_entry);
			printf("0x%x\n",PicDir_root_entry);
			if( PicDir_root_entry )
			{
				/*if( stat( PicDirPath,&dirStat ) >=0)//檢查webPic資料夾是否存在
				{
					if(S_ISDIR (dirStat.st_mode))
					{
						metadata_add_container (ut, PicDir_root_entry, PicDirPath);
					}
				}*/
				upnp_entry_add_child (ut, ut->root_entry, PicDir_root_entry);
				dirID = PicDir_root_entry->id;
			}
		}
		
		while( !Pic_exit_flag )
		{
			printf("Picture grab start!\n");
			if( webcam_exit_flag == 1 ) {Pic_exit_flag = 1; return;}
			if(shareDir&&PicDir_root_entry)
			{
				for( i = 0; i < PicNumByTime; i++ )
				{
				ref:
					if( webcam_exit_flag == 1 ) {Pic_exit_flag = 1; return;}
					while ((index == videoIn.frame_cour) && !Pic_exit_flag) usleep(500);
					videoIn.framelock[index]++;
					memcpy( frame, videoIn.ptframe[index], frame_size );
					videoIn.framelock[index]--;
					frame_index = (struct frame_t *)frame;
					if(frame_index->size==0)
					{
			
						index = (index+1)% OUTFRMNUMB;  
						goto ref;
					}
					//
					//dump_frame( frame_index);
					time_t pic_t = (time_t)(frame_index->seqtimes/1000);
					p = localtime(&pic_t);
						
					sprintf(PicDirPath,"%s/WebCamPic/%d_%d_%d",  shareDir, (p->tm_year+1900),p->tm_mon+1, p->tm_mday);
					sprintf(PicName, "%02dh%02dm%02ds.jpg", p->tm_hour, p->tm_min, p->tm_sec );
					sprintf( DirName, "%d_%d_%d", (p->tm_year+1900),p->tm_mon+1, p->tm_mday+1);
					sprintf(fullpath, "%s/%s",PicDirPath, PicName );
					
					//workPath = get_current_dir_name();
					
					if( workPath)
					{
						if( strcmp( workPath, PicDirPath ) )//變更儲存目錄
						{
							CreateDir( PicDirPath );
							PicDir_entry = upnp_dir_new (ut, DirName, PicDirPath, PicDir_root_entry);
							if( PicDir_entry )
								upnp_entry_add_child (ut, PicDir_root_entry, PicDir_entry);
							printf("Chang path %s to %s\n", workPath,PicDirPath );
							workPath = strdup( PicDirPath );
						}
					}else
					{
							CreateDir( PicDirPath );
							PicDir_entry = upnp_dir_new (ut, DirName, PicDirPath, PicDir_root_entry);
							if( PicDir_entry )
								upnp_entry_add_child (ut, PicDir_root_entry, PicDir_entry);
							printf("Chang path to %s\n", PicDirPath );
							workPath = strdup( PicDirPath );
					}
					printf("Save %dx%d jpg file as %s size=%d\n",frame_index->w,frame_index->h,fullpath, sizeout);
					picData = frame+sizeof(struct frame_t);
					picfp = open( fullpath,O_RDWR|O_CREAT,0600);
					if(picfp < 0 )
					{
						printf("error %d %s\n",errno,strerror(errno));break;
					}
					sizeout = get_jpegsize( frame,frame_size)-sizeof(struct frame_t);
			
					
					if( write (picfp, picData, sizeout) < 0 )
					//if(fwrite (picData, sizeof (char), sizeout, picfp) <0 )
					{
						printf("error %d %s\n",errno,strerror(errno));
						printf("Pic write error! Disk full size?\n" );
						Pic_exit_flag = 1; 
						close (picfp);
						return;
					}
					sync();
			  		close (picfp);
			  		

		  			add_file (ut, PicDir_entry, fullpath, PicName);
			  		index = (index+1)% OUTFRMNUMB;  
		  		}
			}
			printf("PT=%dsec\n",PicTime);
			sleep(PicTime);			
		}
		printf("Picture grab stop!\n");
	}
	//if( dirID > 0 )
	//	PicDir_root_entry = upnp_get_entry ( ut, dirID );
	/*if(PicDir_root_entry)
	{
		upnp_entry_free( ut, PicDir_root_entry );PicDir_root_entry=NULL;
	}*/
	free(frame);
}

void grab (void)
{
int err = 0;
  for (;;)
    {
      //printf("I am the GRABBER !!!!! \n");
      err = v4lGrab(&videoIn);
      if (webcam_exit_flag || (err < 0)){
      	printf("GRABBER going out !!!!! \n");
		return;
	 }
		//printf("Grab a frame \n");
    }
}

void grab_ctrl(struct ushare_t *ut)
{
	int sin_size;
  	int serverport = 7070;
  	int i;
  	char tmpName[256];
  	
	pthread_t grabPic, camSaver, server, picSaver;
	if(ut->contentlist->content[0])
	{
		sprintf( tmpName, "%s/webcam.avi", ut->contentlist->content[0] );
		saveName[0] = strdup(tmpName);
		sprintf( tmpName, "%s/webcam_.avi", ut->contentlist->content[0] );
		saveName[1] = strdup(tmpName);
		sprintf( tmpName, "%s/webcam_tmp.avi", ut->contentlist->content[0] );
		saveName_tmp = strdup(tmpName);
	}
	else
	{
		saveName[0] = strdup("webcam.avi");
		saveName[1] = strdup("webcam_.avi");
		saveName_tmp = strdup("webcam_tmp.avi");
	}
	
	pthread_mutex_init (&grab_ctrl_mutex, NULL);pthread_mutex_init (&Pic_mutex, NULL);pthread_mutex_init (&Mov_mutex, NULL);
  	pthread_cond_init (&grab_ctrl_cond, NULL);pthread_cond_init (&Pic_cond, NULL);pthread_cond_init (&Pic_cond, NULL);
	//if( init_dev( "/dev/video1", 0)== 0)
	//{
		//loopFile(ut);
	while( camloop )
	{
		webcam_exit_flag =0;
		pthread_mutex_lock (&grab_ctrl_mutex);//wait somebody to signal it
		pthread_cond_wait (&grab_ctrl_cond, &grab_ctrl_mutex);
		pthread_mutex_unlock (&grab_ctrl_mutex);
		if( !camloop ) break;
		pthread_create( &grabPic, NULL, &grab, NULL);
		pthread_create( &camSaver, NULL, &loopFile,ut);
		pthread_create( &server, NULL, &newServer, &serverport);
		pthread_create( &picSaver, NULL, &PicSaver,ut);
		
		//pthread_join( picSaver, NULL);
		pthread_join( camSaver, NULL); 
		pthread_join( grabPic, NULL);
		//pthread_kill( server, SIGINT );
		pthread_cancel( picSaver);
		pthread_join( picSaver, NULL);
		pthread_cancel( server );
		pthread_join( server, NULL);
		printf("kill stream server\n");
		close(serv_sock);	
    }
	close_v4l( &videoIn );
	pthread_cond_destroy (&grab_ctrl_cond);pthread_cond_destroy (&Pic_cond);pthread_cond_destroy (&Mov_cond);
  	pthread_mutex_destroy (&grab_ctrl_mutex);pthread_mutex_destroy (&Pic_mutex);pthread_mutex_destroy (&Mov_mutex);
}

