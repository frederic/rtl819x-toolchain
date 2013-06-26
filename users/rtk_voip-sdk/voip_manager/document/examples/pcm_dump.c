#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

#define RTP_HEADER_TYPE  0x80
#define RTP_PAYLOAD_TYPE_ALAW 0x8
#define RTP_PAYLOAD_TYPE_ULAW 0x0
#define RTP_SSRC 0x12345678
#define RTP_TIMESTAMP 80
#define RTP_PAYLOAD_LENGTH 80

/*

-----RTP payload 
byte 0    : header
byte 1    : payload type
byte 2-3  : Sequence number
byte 4-7  : time stamp
byte 8-11 : SSRC
*/
struct rtp_data
{
 char version;
 char payload_type;
 short sequence;
 int timestamp;
 int ssrc;
 char data[RTP_PAYLOAD_LENGTH];	
};

#define DEV_TX0 "/dev/voip/pcmtx0"
#define DEV_RX0 "/dev/voip/pcmrx0"

#define DEV_TX1 "/dev/voip/pcmtx1"
#define DEV_RX1 "/dev/voip/pcmrx1"

#define SEG_SHIFT	4
#define	QUANT_MASK  0xf
#define	BIAS		0x84
static  short  seg_end[8] = {0xFF, 0x1FF, 0x3FF, 0x7FF, 0xFFF, 0x1FFF, 0x3FFF, 0x7FFF};
static unsigned char linear2alaw(int pcm_val);
static unsigned char linear2ulaw(int pcm_val);

static int fill_rtp_data(short* raw, int type, struct rtp_data* rtp, int sequence, int timestamp);



int main(int argc, char *argv[]) {
  
  int sd, rc, ch;
  struct sockaddr_in cliAddr, ServAddr;
  struct hostent *h;
  int port = 0;
  int type = 0;
  int dir  = 0;
  int sequence = 0;
  int timestamp = 0;
  
  FILE *inp=NULL;
  
  int state;
  struct rtp_data* payload = (struct rtp_data*)malloc(sizeof(struct rtp_data));
  short data[80]={0};

  if(argc != 6) {
   goto wrong_input;
  }

  h = gethostbyname(argv[1]);
  
  if(h==NULL) {
    printf("%s: unknown host '%s' \n", argv[0], argv[1]);
    goto wrong_input;
  }
  port = atoi(argv[2]);
  if(!port)
  	goto wrong_input;

  if(!strcmp(argv[3],"ALAW"))
  	type = 1;
  else if(!strcmp(argv[3],"ULAW"))
  	type = 0;
  else
  	goto wrong_input;

  if(!strcmp(argv[4],"RX"))
  	dir = 1;
  else if(!strcmp(argv[4],"TX"))
  	dir = 0;
  else
  	goto wrong_input;
  
  ch = atoi(argv[5]);
  
  if( (ch<0) || (ch>1) ) {
    printf("%s: error channel%d, only support 0, 1 \n", argv[0], ch);
    return -1;
  }

  printf("%s: sending data to '%s' Port : %d (%s) (%s) for channel %d\n", argv[0], h->h_name,port,type?"ALAW":"ULAW",dir?"RX":"TX", ch);

  ServAddr.sin_family = h->h_addrtype;
  memcpy((char *) &ServAddr.sin_addr.s_addr, h->h_addr_list[0], h->h_length);
  ServAddr.sin_port = port;

  /* socket creation */
  sd = socket(AF_INET,SOCK_DGRAM,0);
  if(sd<0) {
    printf("%s: cannot open socket \n",argv[0]);
    return -1;
  }
  
  /* bind any port */
  cliAddr.sin_family = AF_INET;
  cliAddr.sin_addr.s_addr = htonl(INADDR_ANY);
  cliAddr.sin_port = htons(0);
  
  rc = bind(sd, (struct sockaddr *) &cliAddr, sizeof(cliAddr));
  
  if(rc<0) {
  
    printf("%s: cannot bind port\n", argv[0]);
    return -1;
  }
  
  if (ch == 0)
  	inp = fopen(dir?DEV_RX0:DEV_TX0, "rb");
  else if (ch == 1)
  	inp = fopen(dir?DEV_RX1:DEV_TX1, "rb");
  	
  
  if(inp == 0)
  {
  	if (ch == 0)
  		printf("Read %s Fail \n",dir?DEV_RX0:DEV_TX0);
  	else if (ch == 1)
  		printf("Read %s Fail \n",dir?DEV_RX1:DEV_TX1);
  	close(sd);
  	return -1;
  }
  
  srand((unsigned)time( NULL ));

  sequence = rand();
	
  timestamp= rand(); 
  
 while(1){
 
   	state  = fread(data, 1 , 160,inp);
  
  	if(state < 160)
  	{
  		printf("Read Error %d\n",state);
  		continue;
  	}
  	
  	fill_rtp_data(data, type, payload, sequence, timestamp);
    
  	rc = sendto(sd, payload, sizeof(struct rtp_data), 0,(struct sockaddr *) &ServAddr, sizeof(ServAddr));
    	if(rc<0) {
      		printf("%s: cannot send data\n",argv[0]);
      		return -1;
    	}
    	sequence++;
    	timestamp += RTP_TIMESTAMP;
  }
  fclose(inp);
  close(sd);
  return 0;
  
wrong_input:

	printf("usage : %s <server> <port> <ALAW/ULAW> <RX/TX> <channel>\n", argv[0]);
	return -1;
}

static int fill_rtp_data(short* raw, int type, struct rtp_data* rtp, int sequence, int timestamp)
{
	
	int index; 

	memset(rtp,sizeof(struct rtp_data),0);	
	
	rtp->version = RTP_HEADER_TYPE;
	rtp->payload_type= type?RTP_PAYLOAD_TYPE_ALAW:RTP_PAYLOAD_TYPE_ULAW;
	rtp->sequence = htons(sequence);
	rtp->timestamp = htonl(timestamp);
	rtp->ssrc = htonl(RTP_SSRC);
	
	for(index = 0; index <80 ; index++)
	{
		if(type)
			rtp->data[index] = linear2alaw(raw[index]);
		else
			rtp->data[index] = linear2ulaw(raw[index]);
		
	}
	return 0;
}




static int search(int val, short *table, int size)
{
        int     i;

        for (i = 0; i < size; i++)
        {
                if (val <= *table++)
                        return (i);
    }
        return (size);
}


unsigned char linear2alaw(int pcm_val)        /* 2's complement (16-bit range) */
{
        int     mask;
        int     seg;
        unsigned char   aval;

        if (pcm_val >= 0)
        {
                mask = 0xD5;        /* sign (7th) bit = 1 */
        }
        else
        {
                mask = 0x55;        /* sign bit = 0 */
                pcm_val = ~pcm_val;
    }

        /* Convert the scaled magnitude to segment number. */
        seg = search(pcm_val, seg_end, 8);

        /* Combine the sign, segment, and quantization bits. */

        if (seg >= 8)       /* out of range, return maximum value. */
                return (0x7F ^ mask);
        else
        {
                aval = seg << SEG_SHIFT;
                if (seg < 2)
                        aval |= (pcm_val >> 4) & QUANT_MASK;
                else
                        aval |= (pcm_val >> (seg + 3)) & QUANT_MASK;
                return (aval ^ mask);
    }
}

unsigned char linear2ulaw(int pcm_val)    /* 2's complement (16-bit range) */
{
        int     mask;
        int     seg;
        unsigned char   uval;

        /* Get the sign and the magnitude of the value. */
        if (pcm_val < 0)
        {
                pcm_val = BIAS - pcm_val;
                mask = 0x7F;
        }
        else
        {
                pcm_val += BIAS;
                mask = 0xFF;
    }

        /* Convert the scaled magnitude to segment number. */
        seg = search(pcm_val, seg_end, 8);

    /*
     * Combine the sign, segment, quantization bits;
     * and complement the code word.
     */
        if (seg >= 8)       /* out of range, return maximum value. */
                return (0x7F ^ mask);
        else
        {
                uval = (seg << 4) | ((pcm_val >> (seg + 3)) & 0xF);
                return (uval ^ mask);
    }
}

