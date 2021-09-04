#ifndef _version_h_
#define _version_h_

//#define DEMO
#ifdef DEMO
#  define DEMO_STRING " DEMO"
#else
#  define DEMO_STRING
#endif

#ifdef _DEBUG
#  define VERSION_CONFIG " Debug"
#else
#  define VERSION_CONFIG
#endif

#define VERSION_MAJOR 1
#define VERSION_MINOR 1
#define VERSION_REV   0

#define VERSION_STRING "1.1.0" DEMO_STRING VERSION_CONFIG

#endif
