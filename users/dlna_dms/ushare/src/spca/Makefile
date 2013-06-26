##############################
# spcaview Makefile
##############################
include ../../config.mak

SERVFLAGS= -I/home/joychen/桌面/x86_ushare/include -I..  -static -O2 -DLINUX $(WARNINGS)
MATH_LIB=-lm 
SERVLIBS= $(MATH_LIB) -lpthread

#WARNINGS = -Wall \
#           -Wundef -Wpointer-arith -Wbad-function-cast \
#           -Wcast-align -Wwrite-strings -Wstrict-prototypes \
#           -Wmissing-prototypes -Wmissing-declarations \
#           -Wnested-externs -Winline -Wcast-qual -W \
#           -Wno-unused
#           -Wunused



OBJAVI= spcav4l.o utils.o tcputils.o picture.o pargpio.o encoder.o huffman.o marker.o quant.o avilib.o


# Makefile commands:
#libjpgenc:	$(LIB_ENCODE_OBJECTS)
#		ld -r $(LIB_ENCODE_OBJECTS) -o $(LIB_ENCODE)
		
all:	grab_avi.o $(OBJAVI)
clean:
	@echo "Cleaning up directory."
	rm -f *.a *.o   *~ log errlog

# Applications:


#catavi: $(OBJAVI)
	#$(CC) $(SERVFLAGS) -o catavi $(OBJAVI) $(SERVLIBS)

	

grab_avi.o: grab_avi.c grab_avi.h
		$(CC) $(SERVFLAGS) -c -o $@ $<	
			
avilib.o: avilib.c avilib.h
		$(CC) $(SERVFLAGS) -c -o $@ $<
	
server.o:	server.c
		$(CC) $(SERVFLAGS) -c -o $@ $<
		
spcacat.o:	spcacat.c
		$(CC) $(SERVFLAGS) -c -o $@ $<
	
spcav4l.o:	spcav4l.c spcav4l.h
		$(CC) $(SERVFLAGS) -c -o $@ $<
		
utils.o:	utils.c utils.h
		$(CC) $(SERVFLAGS) -c -o $@ $<
		
picture.o:	picture.c picture.h
		$(CC) $(SERVFLAGS) -c -o $@ $<
		
pargpio.o:	pargpio.c pargpio.h
		$(CC) $(SERVFLAGS) -c -o $@ $<
		
tcputils.o:	tcputils.c tcputils.h
		$(CC) $(SERVFLAGS) -c -o $@ $<
		
encoder.o:	encoder.c encoder.h
		$(CC) $(SERVFLAGS) -c -o $@ $<
		
huffman.o:	huffman.c huffman.h
		$(CC) $(SERVFLAGS) -c -o $@ $<
		
marker.o:	marker.c marker.h
		$(CC) $(SERVFLAGS) -c -o $@ $<
		
quant.o:	quant.c quant.h
		$(CC) $(SERVFLAGS) -c -o $@ $<
		

