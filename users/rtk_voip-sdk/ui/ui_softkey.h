#ifndef __UI_SOFTKEY_H__
#define __UI_SOFTKEY_H__

#ifdef SOFTKEY_SUPPORT

#include "gs_types.h"

#if KEYPAD_MAP_VENDOR == VENDOR_WCO2
#define SOFTKEY_TEXT_LOC_Y		48
#define SOFTKEY_INS_TEXT_WIDTH	14		/* 14 pixels per words */
#endif

/* Reset or initialize softkey configuration */
extern void ResetSoftkeyConfiguration( void );

/* Clear instruction text rectangle, if a frame doesn't screen full screen. */
extern void ClearSoftkeyInstructionTextRetangle( rect_t *prect );

/* softkey configuration */
extern int SoftkeyConfiguration( unsigned char vkeySource, unsigned char vkeyTarget );

/* softkey configuration with insruction text */
extern void SoftkeyConfigurationWithInstructionText( unsigned char vkeySource, unsigned char vkeyTarget, const unsigned char *pszInstructionText );

/* softkey translation core (called by UI_KeypadInput() only) */
extern void DoSoftkeyTranslation( unsigned char *pkey );

/* Left softkey --> VKEY_INS_B1 */
extern void LeftSoftkeyConfigurationWithInstructionText( unsigned char vkeyTarget, const unsigned char *pszInstructionText );

/* Middle softkey --> VKEY_INS_B2 (allow long instruction text) */
extern void MiddleSoftkeyConfigurationWithInstructionText( unsigned char vkeyTarget, const unsigned char *pszInstructionText );

/* Middle2 softkey --> VKEY_INS_B3 (don't allow long instruction text) */
extern void Middle2SoftkeyConfigurationWithInstructionText( unsigned char vkeyTarget, const unsigned char *pszInstructionText );

/* Right softkey --> VKEY_INS_B4 */
extern void RightSoftkeyConfigurationWithInstructionText( unsigned char vkeyTarget, const unsigned char *pszInstructionText );

/* === Standard Configuration Part === */
/* Left: OK, Right: Back */
extern void SoftkeyStandardConfiguration_OkBack( void );

/* Left: OK, Right: Clear */
extern void SoftkeyStandardConfiguration_OkClear( void );

/* Left: none, Right: Back */
extern void SoftkeyStandardConfiguration_Back( void );

/* Left: Yes, Right: No */
extern void SoftkeyStandardConfiguration_YesNo( void );

/* Left: Detail, Right: Back */
extern void SoftkeyStandardConfiguration_DetailBack( void );

/* === Context Save/Restore Part === */
extern void SoftkeyContextSave( void );
extern void SoftkeyContextRestore( int bDraw );
extern void DrawAllInstructionTextByContext( void );

#else

#define ResetSoftkeyConfiguration()						{}

#define ClearSoftkeyInstructionTextRetangle( prect )	{ if( (prect) ) { ((rect_t *)(prect)) ->left = ((rect_t *)(prect)) ->top = ((rect_t *)(prect)) ->right = ((rect_t *)(prect)) ->bottom = 0; } }

#define SoftkeyConfiguration( vks, vkt )				{}

#define SoftkeyConfigurationWithInstructionText( vks, vkt, psz )		{}

#define DoSoftkeyTranslation( pk )						{}

#define LeftSoftkeyConfigurationWithInstructionText( vkt, psz )			{}

#define MiddleSoftkeyConfigurationWithInstructionText( vkt, psz )		{}

#define Middle2SoftkeyConfigurationWithInstructionText( vkt, psz )		{}

#define LeftSoftkeyConfigurationWithInstructionText( vkt, psz )			{}

#define RightSoftkeyConfigurationWithInstructionText( vkt, psz )		(0)

#define SoftkeyStandardConfiguration_OkBack()			{}

#define SoftkeyStandardConfiguration_OkClear()			{}

#define SoftkeyStandardConfiguration_Back()				{}

#define SoftkeyStandardConfiguration_YesNo()			{}

#define SoftkeyStandardConfiguration_DetailBack()		{}

#define SoftkeyContextSave()							{}

#define SoftkeyContextRestore( draw )					{}

#define DrawAllInstructionTextByContext()				{}

#endif /* SOFTKEY_SUPPORT */



#endif /* __UI_SOFTKEY_H__ */
