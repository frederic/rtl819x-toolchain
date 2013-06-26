#ifndef __UI_HOST_H__
#define __UI_HOST_H__

#include "ui_state.h"
#include "ui_vkey.h"
#include "ioctl_softrtc.h"
#include "com_edit.h"
#include "com_ime.h"
#include "com_select.h"
#include "res.h"

/* Related to state */
typedef enum {
	STATE_BACK_LEVEL_INVALID = 0,
	STATE_BACK_LEVEL_ONE = 1,
	STATE_BACK_LEVEL_TWO = 2,
	STATE_BACK_LEVEL_THREE = 3,
	STATE_BACK_LEVEL_FOUR = 4,
	STATE_BACK_LEVEL_FIVE = 5,
	STATE_BACK_LEVEL_SIX = 6,
} state_back_level_t;

extern ui_state_t uiState;

extern void UI_StateJump( ui_state_t newState, int param );
extern void UI_StateTransition( ui_state_t newState, int param );
extern void UI_StateTransitionWithBackHelp( ui_state_t newState, int param, int backParam );
extern void UI_StateTransitionToPrompt( ui_state_t nextState, int paramForNextState );
extern void UI_StateBack( state_back_level_t level );
extern void UI_PeekStateStack( state_back_level_t level, ui_state_t *state, int *param );

/* parameters */
#define PARAM_NOT_DRAW		0x7FFFFFEE	/* Only for special states. */
#define PARAM_RESTART		0x7FFFFFCE
#define PARAM_SPECIAL		0x7FFFFFDE
#define PARAM_PROMPT_BACK	0x7FFFFFBE
#define PARAM_BORROW		0x7FFFFFAE
#define PARAM_VOL_BACK		0x7FFFFF9E

/* Draw centralized string */
#define SOFTKEY_L_OK			0x00000001	/* Left: OK */
#define SOFTKEY_R_BACK			0x00010000	/* Right: Back */
#define SOFTKEY_R_CLEAR			0x00020000	/* Right: Clear */
#define SOFTKEY_LR_OK_BACK		0x80000000	/* bit 31 = 1 */
#define SOFTKEY_LR_OK_CLEAR		0x80000001
#define SOFTKEY_LR_NONE_BACK	0x80000002
#define SOFTKEY_LR_YES_NO		0x80000003
typedef unsigned long std_skm_t;	/* softkey mask */

extern void DrawCentralizedString( const unsigned char *pszPromptText );
extern void SetupPromptFrame( const unsigned char *pszPromptText );
extern void SetupFrameWithCentralizedString( const unsigned char *pszPromptText );
extern void SetupFrameWithCentralizedStringAndSoftkey( const unsigned char *pszPromptText, std_skm_t std_skm );

extern int nSelectedItem;

/* ui_sup */
extern void ChangeTimerPeriodForApplication( void );
extern uptime_t CheckIfTimeoutInMillisecond( uptime_t *pPrevUptime, 
												unsigned long nTimeoutMS );
extern int GetTextStringFromBcd( unsigned char *pszText, int maxLen, const unsigned char *pBCD );
extern int GetBcdStringFromText( unsigned char *pBCD, int maxBytes, const unsigned char *pszText );

extern int EnableHandfreeFunctionalityAndFlag( int bEnable );

#define FLASH_FEATURES		0x00000000

#define MY_FIELD_OFFSET( type, field )		( ( unsigned long )&( ( ( type * )0 ) ->field ) )
#define MY_FIELD_SIZE( type, field )		( sizeof( ( ( type * )0 ) ->field ) )

/* Provide interfaces to main loop */
extern void UI_BasicInitialization( void );
extern void UI_Termination( void );
extern void UI_KeypadInput( unsigned char key );
extern void UI_TimerEvent( void );

/* Phone call realted functions (Implement in ui_call.c) */
//extern void UI_IncomingCallEvent( unsigned char *pszPhonenumber );

/* Key related functions */
#define IsDialKey( key )		( ( ( key ) >= VKEY_0 && ( key ) <= VKEY_9 ) || ( ( key ) == VKEY_STAR ) || ( ( key ) == VKEY_POUND ) )
#define IsNumberKey( key )		( ( ( key ) >= VKEY_0 && ( key ) <= VKEY_9 ) )

extern int CheckIfPlayKeypressTone( void ); 

#endif /* __UI_HOST_H__ */

