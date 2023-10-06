#ifndef __HTTP_DEBUG_COLOR_H
#define __HTTP_DEBUG_COLOR_H

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)
#include <string.h>
// the following are UBUNTU/LINUX ONLY terminal color codes.
#define RESET "\033[0m"
#define BLACK "\033[30m"              /* Black */
#define RED "\033[31m"                /* Red */
#define GREEN "\033[32m"              /* Green */
#define YELLOW "\033[33m"             /* Yellow */
#define BLUE "\033[34m"               /* Blue */
#define MAGENTA "\033[35m"            /* Magenta */
#define CYAN "\033[36m"               /* Cyan */
#define WHITE "\033[37m"              /* White */
#define BOLDBLACK "\033[1m\033[30m"   /* Bold Black */
#define BOLDRED "\033[1m\033[31m"     /* Bold Red */
#define BOLDGREEN "\033[1m\033[32m"   /* Bold Green */
#define BOLDYELLOW "\033[1m\033[33m"  /* Bold Yellow */
#define BOLDBLUE "\033[1m\033[34m"    /* Bold Blue */
#define BOLDMAGENTA "\033[1m\033[35m" /* Bold Magenta */
#define BOLDCYAN "\033[1m\033[36m"    /* Bold Cyan */
#define BOLDWHITE "\033[1m\033[37m"   /* Bold White */

#undef ERROR

#ifdef DEBUG
#define ERROR(...)  \
do{ \
    const char *error_str=__FILE__;\
    unsigned int str_len_size=strlen(__FILE__)-1;\
    unsigned int pos_n_offset=0;\
    for(;str_len_size>0;str_len_size--)\
    {\
        if(error_str[str_len_size]=='/')\
        {\
            pos_n_offset+=1;\
            if(pos_n_offset>2)\
            break;\
        }\
    }\
    fprintf(stdout, "\033[1m\033[31m[ERROR  ]%s %s(Line %d): \033[0m",&error_str[str_len_size],__FUNCTION__,__LINE__);   \
    fprintf(stderr, __VA_ARGS__);   \
    fprintf(stdout, " \n");   \
}while(0)

#define WARNING(...)    \
do{   \
    const char *warning_str=__FILE__;\
    unsigned int str_len_size=strlen(__FILE__)-1;\
    unsigned int pos_n_offset=0;\
    for(;str_len_size>0;str_len_size--)\
    {\
        if(warning_str[str_len_size]=='/')\
        {\
            pos_n_offset+=1;\
            if(pos_n_offset>2)\
            break;\
        }\
    }\
    fprintf(stdout, "\033[1m\033[33m[WARNING]%s %s(Line %d): \033[0m",&warning_str[str_len_size],__FUNCTION__,__LINE__);   \
    fprintf(stdout, __VA_ARGS__);   \
    fprintf(stdout, " \n");   \
}while(0)

#define INFO(...)   \
do{ \
    fprintf(stdout, __VA_ARGS__);   \
    fprintf(stdout, " \n");   \
}while(0)

#define DEBUG_LOG(...)  \
do{ \
    const char *debug_str=__FILE__;\
    unsigned int str_len_size=strlen(__FILE__)-1;\
    unsigned int pos_n_offset=0;\
    for(;str_len_size>0;str_len_size--)\
    {\
        if(debug_str[str_len_size]=='/')\
        {\
            pos_n_offset+=1;\
            if(pos_n_offset>2)\
            break;\
        }\
    }\
    fprintf(stdout, "[DEBUG  ]%s \033[32m%s\033[0m(Line %d): ",&debug_str[str_len_size],__FUNCTION__,__LINE__);   \
    fprintf(stdout, __VA_ARGS__);   \
    fprintf(stdout, " \n");   \
}while(0)

#define OUT_LOG(...)  \
do{ \
    fprintf(stdout, __VA_ARGS__);   \
    fprintf(stdout, " \n");   \
}while(0)
#else
#define OUT_LOG(...)
#define DEBUG_LOG(...)
#define INFO(...)
#define WARNING(...)
#define ERROR(...)
#endif

#endif