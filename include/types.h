//
// Created by yuchengye on 2021/11/17.
//

#ifndef JOBLIB_TYPES_H
#define JOBLIB_TYPES_H
#include "cstdio"
typedef int64_t DWORD;
typedef int32_t LWORD;
typedef int16_t DBYTE;
typedef int8_t BYTE;

#define FILENAME_ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#define __METHOD__ (strchr(__PRETTY_FUNCTION__, ' ')+1)

#define dprint(...) do{std::printf("\033[1m\033[31m%s\033[0m \033[36min\033[0m \033[1m\033[4m\033[32m%s:\033[1m\033[4m\033[35m%d\033[0m\n\t",FILENAME_,__METHOD__,__LINE__); \
std::printf(__VA_ARGS__);}while(0)

#endif //JOBLIB_TYPES_H
