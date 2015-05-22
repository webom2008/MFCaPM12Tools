#pragma once

#define _PROMPT_        "bf\\>"
#define _RESETSCREEN_   "\f\e[0m"
#define _CLEARSCREEN_   "\e[2J"
#define _ERASELINE_     "\e[K"
#define _NEWLINE_       "\n"
#define _CARRIAGE_      "\r"
#define _VTAB_          "\v"
#define _HTAB_          "\t"
#define _CURSORUP_      "\e[A"
#define _CURSORDN_      "\e[B"
#define _CURSORFW_      "\e[C"
#define _CURSORBW_      "\e[D"
#define _CURSORUPX_     "\e[%dA" // requires the number of lines as the 1st parameter
#define _CURSORDNX_     "\e[%dB" // requires the number of lines as the 1st parameter
#define _CURSORFWX_     "\e[%dC" // requires the number of lines as the 1st parameter
#define _CURSORBWX_     "\e[%dD" // requires the number of lines as the 1st parameter
#define _CURSORPOSXY_   "\e[%d;%dH"
#define _CURSORPOSSAVE_ "\e[s"
#define _CURSORPOSREST_ "\e[u"
#define _INVERSEON_     "\e[7m"
#define _INVERSEOFF_    "\e[27m"
#define _NORMALTEXT_    "\e[0m"
#define _BOLDTEXT_      "\e[1m"
#define _ITALICTEXT_    "\e[3m"
#define _BLINKTEXT_     "\e[5m"
#define _REDTEXT_       "\e[31m"
#define _GREENTEXT_     "\e[32m"
#define _YELLOWTEXT_    "\e[33m"
#define _BLUETEXT_      "\e[34m"
#define _MAGENTATEXT_   "\e[35m"
#define _CYANTEXT_      "\e[36m"
#define _WHITETEXT_     "\e[37m"
#define _BLACKTEXT_     "\e[30m"
#define _TEST_          "\e[=3h"
#define _NL_    _NEWLINE_
#define _CR_    _CARRIAGE_
#define _EL_    _ERASELINE_
#define _CS_    _CLEARSCREEN_

//static char crsuw[] = { '\e', '[', 'A' }; // Cursor Up
//static char crsdw[] = { '\e', '[', 'B' }; // Cursor Down
//static char crsfw[] = { '\e', '[', 'C' }; // Cursor Forward
//static char crsbw[] = { '\e', '[', 'D' }; // Cursor Backward
//static char crstb[] = { '\e', '[', '4', 'C' }; // Tab: Cursor Forward by 4

#define __VERBOSITY__               2

#if (__VERBOSITY__ > 0)
#define CONTROL(format,...) printf(format, ##__VA_ARGS__)
#else
#define CONTROL(format,...) do { } while(0)
#endif

#if (__VERBOSITY__ > 0)
//#define DEBUG_INFO(format,...)  printf(_NORMALTEXT_ _CR_ "File: "__FILE__", Line: %05d: "format"\n", __LINE__, ##__VA_ARGS__)
#define DEBUG_INFO(format,...)  printf("File: "__FILE__", Line: %05d: "format"\n", __LINE__, ##__VA_ARGS__)
#else
#define DEBUG_INFO(format,...) do { } while(0)
#endif

#if (__VERBOSITY__ > 0)
//#define INFO(format,...)  printf(_BOLDTEXT_ _GREENTEXT_ _CR_ format, ##__VA_ARGS__)
#define INFO(format,...)  printf(format, ##__VA_ARGS__)
#else
#define INFO(format,...) do { } while(0)
#endif

#if (__VERBOSITY__ > 0)
//#define WARNING(format,...)  printf(_BOLDTEXT_ _YELLOWTEXT_ _CR_ "警告:"format, ##__VA_ARGS__)
#define WARNING(format,...)  printf("警告:"format, ##__VA_ARGS__)
#else
#define WARNING(format,...) do { } while(0)
#endif


#if (__VERBOSITY__ > 0)
//#define ERROR_INFO(format,...)  printf(_BOLDTEXT_ _REDTEXT_ _CR_ "错误:"format, ##__VA_ARGS__)
#define ERROR_INFO(format,...)  printf("错误:"format, ##__VA_ARGS__)
#else
#define ERROR_INFO(format,...) do { } while(0)
#endif

#if (__VERBOSITY__ > 0)
//#define MSG(format,...)  printf(_BOLDTEXT_ _WHITETEXT_ _CR_ format, ##__VA_ARGS__)
#define MSG(format,...)  printf("提示:"format, ##__VA_ARGS__)
#else
#define MSG(format,...) do { } while(0)
#endif
