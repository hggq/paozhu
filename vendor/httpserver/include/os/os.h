#ifndef __OPERATING_SYSTEM_H__
#define __OPERATING_SYSTEM_H__

#if defined(_WIN32) || defined(__MINGW32__)
 char DIRECTORY_SEPARATOR = '\\';
#else
 char DIRECTORY_SEPARATOR = '/';
#endif

#endif
