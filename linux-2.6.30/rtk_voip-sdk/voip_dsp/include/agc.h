#ifndef AGC_H
#define AGC_H

#define AGC_RX_DIR	0 /* phone mic -> dsp -> network */
#define AGC_TX_DIR	1 /* network -> dsp -> phone speaker */

/*
 * chid: channel id 0 ~ 15
 * dir: direction, 0:AGC_RX_DIR ; 1:AGC_TX_DIR
 */

void rtk_agc_reset(unsigned int chid, unsigned int dir);

/*
 *  lvl: target level 0~9, 0 -> min level, 9 -> max level
 *    maping 0-9 : 1024, 1448, 2048, 2896, 4096, 5793, 8192, 11585, 16384, 23170
 *  max gain up: inpuat range 0~15,  0 -> 0db, 1 -> 1db, ... , 15 -> 15db
 *  max gain down: input range 0~9, 0 -> 0db, 1 -> -1db, ... , 9 -> -9db
 */
void rtk_agc_set_lvl(unsigned int chid, unsigned int dir, unsigned int lvl, unsigned int max_gain_up, unsigned int max_gain_down);

/*
 * average threshold for voice and non-voice detect.
 * peak threashold for voice and non-voice detect.
 *
 */
void rtk_agc_set_threshold(unsigned int chid, unsigned int dir, short average, short peak);

/*
 * max_increase: max increase gain config, 0 not update, def (long)(1.0139*(1<<13))
 * max_decrease: max decrease gain config, 0 net update, def (long)(0.9550*(1<<13))
 */
void rtk_agc_set_step(long max_increase, long max_decrease);


/*
 * addr: address of voice, 80 sample (160byte)
 */
void rtk_agc(unsigned int chid, unsigned int dir, short* addr);



#endif