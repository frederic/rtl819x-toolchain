
#define __FUNCTION__	""

#include "cvcfg.c"


int checkFileType_win32(const char *filename)
{
	return checkFileType( filename );
}

int parseBinConfig_win32(int type, const char *filename, struct all_config *pMib)
{
	return parseBinConfig( type, filename, pMib );
}

int parseTxtConfig_win32(const char *filename, struct all_config *pMib)
{
	return parseTxtConfig( filename, pMib );
}

int generateBinFile_win32(int type, char *filename, int flag, struct all_config *allConfig)
{
	config = *allConfig;	// copy to static variable 

	return generateBinFile( type, filename, flag );
}

int generateTxtFile_win32(const char *filename, const struct all_config *mib_config)
{
	config = *mib_config;		// copy to static variable 

	return generateTxtFile( filename );
}







