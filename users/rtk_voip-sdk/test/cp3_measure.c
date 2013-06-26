#include <stdlib.h>
#include "voip_manager.h"

#define M_CP3_ITEM( x )		&cp3_voip_param.bCp3Count_ ##x, #x

static st_CP3_VoIP_param cp3_voip_param;

static const struct {
	int * item_field_ptr;
	const char * item_name;
} item_list[] = {
	{ M_CP3_ITEM( PCM_ISR ), }, 
	{ M_CP3_ITEM( PCM_RX ), },
	{ M_CP3_ITEM( LEC ), },
	{ M_CP3_ITEM( G711Enc ), },
	{ M_CP3_ITEM( G711Dec ), },
	{ M_CP3_ITEM( G729Enc ), },
	{ M_CP3_ITEM( G729Dec ), },
	{ M_CP3_ITEM( G7231Enc ), },
	{ M_CP3_ITEM( G7231Dec ), },
	{ M_CP3_ITEM( G726Enc ), },
	{ M_CP3_ITEM( G726Dec ), },
	{ M_CP3_ITEM( G722Enc ), },
	{ M_CP3_ITEM( G722Dec ), },
	{ M_CP3_ITEM( GSMFREnc ), },
	{ M_CP3_ITEM( GSMFRDec ), },
	{ M_CP3_ITEM( iLBC20Enc ), },
	{ M_CP3_ITEM( iLBC20Dec ), },
	{ M_CP3_ITEM( iLBC30Enc ), },
	{ M_CP3_ITEM( iLBC30Dec ), },
	{ M_CP3_ITEM( T38Enc ), },
	{ M_CP3_ITEM( T38Dec ), },
	{ M_CP3_ITEM( AMRNBEnc ), },
	{ M_CP3_ITEM( AMRNBDec ), },
	{ M_CP3_ITEM( SpeexNBEnc ), },
	{ M_CP3_ITEM( SpeexNBDec ), },
	{ M_CP3_ITEM( G7111NBEnc ), },
	{ M_CP3_ITEM( G7111NBDec ), },
	{ M_CP3_ITEM( G7111WBEnc ), },
	{ M_CP3_ITEM( G7111WBDec ), },
	{ M_CP3_ITEM( PCM_HANDLER ), },
};

#define SIZE_OF_ITEM_LIST	( sizeof( item_list ) / sizeof( item_list[ 0 ] ) )

int cp3_measure_main(int argc, char *argv[])
{
	int item_no;
	
	memset(&cp3_voip_param, 0, sizeof(st_CP3_VoIP_param));
	
	if (argc == 7)
	{
		
		cp3_voip_param.cp3_counter1 = atoi(argv[1]);
		cp3_voip_param.cp3_counter2 = atoi(argv[2]);
		cp3_voip_param.cp3_counter3 = atoi(argv[3]);
		cp3_voip_param.cp3_counter4 = atoi(argv[4]);
		cp3_voip_param.cp3_dump_period = atoi(argv[5]);
		
		item_no = atoi(argv[6]);
		
		if( item_no < SIZE_OF_ITEM_LIST ) {
			*item_list[ item_no ].item_field_ptr = 1;
			goto label_bits_set_done;
		}
		
		switch ( item_no )
		{
			case 200:
				cp3_voip_param.bCp3Count_Temp200 = 1;
				break;
			case 201:
				cp3_voip_param.bCp3Count_Temp201 = 1;
				break;
			case 202:
				cp3_voip_param.bCp3Count_Temp202 = 1;
				break;
			case 203:
				cp3_voip_param.bCp3Count_Temp203 = 1;
				break;
			case 204:
				cp3_voip_param.bCp3Count_Temp204 = 1;
				break;
			case 205:
				cp3_voip_param.bCp3Count_Temp205 = 1;
				break;
			case 206:
				cp3_voip_param.bCp3Count_Temp206 = 1;
				break;
			case 207:
				cp3_voip_param.bCp3Count_Temp207 = 1;
				break;
			case 208:
				cp3_voip_param.bCp3Count_Temp208 = 1;
				break;
			case 209:
				cp3_voip_param.bCp3Count_Temp209 = 1;
				break;
			case 210:
				cp3_voip_param.bCp3Count_Temp210 = 1;			
				break;
			case 211:
				cp3_voip_param.bCp3Count_Temp211 = 1;
				break;
			case 212:
				cp3_voip_param.bCp3Count_Temp212 = 1;
				break;
			case 213:
				cp3_voip_param.bCp3Count_Temp213 = 1;
				break;
			case 214:
				cp3_voip_param.bCp3Count_Temp214 = 1;
				break;
			case 215:
				cp3_voip_param.bCp3Count_Temp215 = 1;
				break;
			case 216:
				cp3_voip_param.bCp3Count_Temp216 = 1;
				break;
			case 217:
				cp3_voip_param.bCp3Count_Temp217 = 1;
				break;
			case 218:
				cp3_voip_param.bCp3Count_Temp218 = 1;
				break;
			case 219:
				cp3_voip_param.bCp3Count_Temp219 = 1;
				break;			
			case 255:
				// disable cop3 counter
				break;
			
			default:
				goto CP3_ERROR;
				break;
		}
label_bits_set_done:
		
		rtk_cp3_measure(&cp3_voip_param);
	}
	else
	{

CP3_ERROR:
		printf("Error!\n\n");
		printf("Usage: cp3_measure counter1 counter2 counter3 counter4 dump_per_count test_item\n\n");
		printf(" - counterX:\n");
#ifdef CONFIG_ARCH_CPU_RLX5281
		printf("   - 1: count instruction fetches\n" );
		printf("   - 2: count I-cache misses\n" );
		printf("   - 3: count I-cache miss cycles\n" );
		printf("   - 4: count store instruction\n" );
		printf("   - 5: count load instruction\n" );
		printf("   - 6: count load or store instruction\n" );
		printf("   - 7: count completed instruction\n" );
		printf("   - 8: count cycle (in common use)\n" );
		printf("   - 9: count I-cache soft miss\n" );
		printf("   - 10: count D-cache misses\n" );
		printf("   - 11: count D-cache miss cycles\n" );
#else
		printf("   - 0: CP3CNT_CYCLES\n");
		printf("   - 1: CP3CNT_NEW_INST_FECTH\n");
		printf("   - 2: CP3CNT_NEW_INST_FETCH_CACHE_MISS\n");
		printf("   - 3: CP3CNT_NEW_INST_MISS_BUSY_CYCLE\n");
		printf("   - 4: CP3CNT_DATA_STORE_INST\n");
		printf("   - 5: CP3CNT_DATA_LOAD_INST\n");
		printf("   - 6: CP3CNT_DATA_LOAD_OR_STORE_INST\n");
		printf("   - 7: CP3CNT_EXACT_RETIRED_INST\n");
		printf("   - 8: CP3CNT_RETIRED_INST_FOR_PIPE_A\n");
		printf("   - 9: CP3CNT_RETIRED_INST_FOR_PIPE_B(LX5181 don't support)\n");
		printf("   - 10: CP3CNT_DATA_LOAD_OR_STORE_CACHE_MISS\n");
		printf("   - 11: CP3CNT_DATA_LOAD_OR_STORE_MISS_BUSY_CYCLE\n\n");
#endif
		printf(" - dump_per_count: dump the result per N times count\n\n");
		printf(" - test_item:\n");		
		for( item_no = 0; item_no < SIZE_OF_ITEM_LIST; item_no ++ ) {
			printf("   - %d: %s\n", item_no, item_list[ item_no ].item_name );
		}
		printf("   - 200~219: Temp\n\n");
		printf("   - 255: Disable cp3 counter\n");
		
		printf("Example:\n");
		printf("cp3_measure 0 1 2 3 500 2 (measure LEC, dump results per 500 counts)\n");
		printf("cp3_measure 0 7 3 11 1000 5 (measure G729Enc, dump results per 1000 counts)\n");
		
	}
		

	return 0;
}
