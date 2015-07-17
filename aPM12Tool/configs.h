#pragma once


#define CONFIG_RELEASE_FOR_ALL
//#define CONFIG_RELEASE_FOR_SMART_UPDATE


#ifdef CONFIG_RELEASE_FOR_ALL

#define CONFIG_SYSTEM_CFG_USED
#define CONFIG_SYSTEM_DEBUG_USED
#define CONFIG_NIBP_USED
#define CONFIG_WAVE_USED
#define CONFIG_FACTORY_USED
#define CONFIG_FILE_MAKER_USED
//#define CONFIG_SMART_UPDATE_USED
#define CONFIG_NORMAL_UPDATE_USED
#define CONFIG_CONSOLE_USED
#define CONFIG_LOGS_FILE_USED

#endif

#ifdef CONFIG_RELEASE_FOR_SMART_UPDATE
#define CONFIG_SYSTEM_CFG_USED
#define CONFIG_SMART_UPDATE_USED
#endif
