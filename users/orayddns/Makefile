RM = rm
#CC = gcc
CC = rsdk-linux-gcc
#OBJS = challenge.o digest.o hmac-md5.o  mechinfo.o  server.o client.o gc-gnulib.o  md5.o memxor.o
#OBJS = digest.o hmac-md5.o  gc-gnulib.o  md5.o memxor.o test.o blowfish.o oray_util.o oray.o libexception_handle.o
OBJS = hmac-md5.o  md5.o memxor.o oray_util.o oray.o
bin = orayddns 
CFLAGS += -I. -D_MIPS_ -Os -mips16
all : $(bin)

$(bin) : $(OBJS)
	$(CC) $(EXTRA_CFLAGS) -Wall  $(OBJS) $(LIBERROR) -o $(bin)  $(LIBS)

clean :
	$(RM) -f $(OBJS)
	$(RM) -f $(bin)

romfs:
	$(ROMFSINST) $(bin) /bin/$(bin)
