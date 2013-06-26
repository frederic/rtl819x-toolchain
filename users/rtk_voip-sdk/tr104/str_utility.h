#ifndef __STR_UTILITY_H__
#define __STR_UTILITY_H__

/* .VoiceService.{i}.Capabilities.Codecs.{i}. */
extern const char *GetOneBasedInstanceNumber_Capabilities_Codecs( 
										const char *pszFullName, 
										unsigned int *pInstNumber );

/* .VoiceService.{i}.VoiceProfile.{i}. */
extern const char *GetOneBasedInstanceNumber_VoiceProfile( 
										const char *pszFullName, 
										unsigned int *pInstNumber );

/* .VoiceService.{i}.VoiceProfile.{i}.Line.{i}. */
extern const char *GetOneBasedInstanceNumber_VoiceProfile_Line( 
								const char *pszFullName, 
								unsigned int *pInstNumber_VoiceProfile,
								unsigned int *pInstNumber_Line );

/* .VoiceService.{i}.VoiceProfile.{i}.Line.{i}.Codec.List.{i} */
extern const char *GetOneBasedInstanceNumber_VoiceProfile_Line_List( 
								const char *pszFullName, 
								unsigned int *pInstNumber_VoiceProfile,
								unsigned int *pInstNumber_Line,
								unsigned int *pInstNumber_List );


#endif /* __STR_UTILITY_H__ */

