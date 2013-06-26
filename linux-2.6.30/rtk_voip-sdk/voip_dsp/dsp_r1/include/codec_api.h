
#ifndef CODEC_API_H
#define CODEC_API_H

//g723 function , call init function in system startup, and between each VoIP call
/*

	input:rate 1:6.3kbps, 0:5.3kbps
	      vad 1: vad enable, 0: vad disable
*/
void g723_init(unsigned int sid, unsigned int rate, unsigned int vad);
/*

input waveinbuf 480byte, 240sample 30ms farme voice

output:packetoutbuf
24byte for 6.3kbps rate
20byte for 5.3kbps rate
*/
int g723_en_start(unsigned int sid, short* waveinbuf, char* packetoutbuf)


/*
bad_frame_flag 1:packet lost, 0=> packet ok

*/
int g723_de_start(unsigned int sid, short* waveoutbuf, char* packetinbuf, unsigned int bad_frame_flag)



//g729 function , call init function in system startup, and between each VoIP call

/*
	      vad 1: vad enable, 0: vad disable
*/
void g729_init(unsigned int sid, unsigned int vad)

/*
input waveinbuf 160byte, 80sample 10ms frame voice
output:packetoutbuf
10byte for active packet
2byte for voice non active(sid)
0byte for silence
*/
int g729_en_start(unsigned int sid, short* waveinbuf, char* packetoutbuf)

/*
bad_frame_flag 1:packet lost, 0=> packet ok
active packet:packet_len = 10
sid packet: packet_len = 2
silence no packet packet_len = 0
*/
int g729_de_start(unsigned int sid, short* waveoutbuf, char* packetinbuf, unsigned int packet_len, unsigned int bad_frame_flag)



//g711 function , call init function in system startup, and between each VoIP call
/*

	input:law 1:mu-law, 0:a-law
	      vad 1: vad enable, 0: vad disable
*/
void g711_init(unsigned int sid, unsigned int law, unsigned int vad);
/*

input: waveinbuf 160byte, 80sample 10ms frame voice

output: packetoutbuf
80byte for active voice
0byte for Not tx
1byte for sid packet
*/
int g711_en_start(unsigned int sid, short* waveinbuf, char* packetoutbuf)


/*
bad_frame_flag 1:packet lost, 0=> packet ok

*/
int g711_de_start(unsigned int sid, short* waveoutbuf, char* packetinbuf, unsigned int packet_len, unsigned int bad_frame_flag)


#endif

