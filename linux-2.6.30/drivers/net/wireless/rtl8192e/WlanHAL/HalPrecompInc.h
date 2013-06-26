#ifndef __INC_PRECOMPINC_H
#define __INC_PRECOMPINC_H

#ifndef __ECOS
//HAL Shared with Driver
#include "StatusCode.h"
#include "HalDbgCmd.h"


//Prototype
#include "HalDef.h"

//MAC Header provided by SD1 HWSD
#include "HalHWCfg.h"
#include "HalComTXDesc.h"
#include "HalComRXDesc.h"
#include "HalComBit.h"
#include "HalComReg.h"

//Instance
#include "HalCommon.h"

#if IS_RTL88XX_GENERATION

#include "Hal88XXPwrSeqCmd.h"
#include "Hal88XXReg.h"
#include "Hal88XXDesc.h"
#include "Hal88XXTxDesc.h"
#include "Hal88XXRxDesc.h"
#include "Hal88XXFirmware.h"
#include "Hal88XXIsr.h"
#include "Hal88XXDebug.h"
#include "Hal88XXPhyCfg.h"


#if IS_RTL8881A_SERIES
#include "Hal8881APwrSeqCmd.h"
#include "Hal8881ADef.h"
#include "Hal8881APhyCfg.h"
#endif

#if IS_RTL8192E_SERIES
#include "Hal8192EPwrSeqCmd.h"
#include "Hal8192EDef.h"
#include "Hal8192EPhyCfg.h"
#endif

#if IS_EXIST_RTL8192EE
#include "Hal8192EEDef.h"
#endif


#include "Hal88XXDef.h"

#endif  //IS_RTL88XX_GENERATION

#else  

//HAL Shared with Driver
#include "Include/StatusCode.h"
#include "HalDbgCmd.h"


//Prototype
#include "HalDef.h"

//MAC Header provided by SD1 HWSD
#include "HalHeader/HalHWCfg.h"
#include "HalHeader/HalComTXDesc.h"
#include "HalHeader/HalComRXDesc.h"
#include "HalHeader/HalComBit.h"
#include "HalHeader/HalComReg.h"

//Instance
#include "HalCommon.h"

#if IS_RTL88XX_GENERATION

#include "RTL88XX/Hal88XXPwrSeqCmd.h"
#include "RTL88XX/Hal88XXReg.h"
#include "RTL88XX/Hal88XXDesc.h"
#include "RTL88XX/Hal88XXTxDesc.h"
#include "RTL88XX/Hal88XXRxDesc.h"
#include "RTL88XX/Hal88XXFirmware.h"
#include "RTL88XX/Hal88XXIsr.h"
#include "RTL88XX/Hal88XXDebug.h"
#include "RTL88XX/Hal88XXPhyCfg.h"

#if IS_RTL8881A_SERIES
#include "RTL88XX/RTL8881A/Hal8881APwrSeqCmd.h"
#include "RTL88XX/RTL8881A/Hal8881ADef.h"
#include "RTL88XX/RTL8881A/Hal8881APhyCfg.h"
#endif

#if IS_RTL8192E_SERIES
#include "RTL88XX/RTL8192E/Hal8192EPwrSeqCmd.h"
#include "RTL88XX/RTL8192E/Hal8192EDef.h"
#include "RTL88XX/RTL8192E/Hal8192EPhyCfg.h"
#endif

#if IS_EXIST_RTL8192EE
#include "RTL88XX/RTL8192E/RTL8192EE/Hal8192EEDef.h"
#endif


#include "RTL88XX/Hal88XXDef.h"
#endif  //IS_RTL88XX_GENERATION

#endif  //ECOS

#endif  //#ifndef __INC_PRECOMPINC_H
