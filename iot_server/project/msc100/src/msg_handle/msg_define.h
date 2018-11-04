// MSC: mobile smart(Senior) control.
#ifndef __MSG_DEFINE_H
#define __MSG_DEFINE_H

#include <core/types.h>

#ifdef __cplusplus
extern "C"
{
#endif

// cpu: little endian
typedef struct FrameHead
{
    unsigned short head;
    unsigned short length;
    unsigned char  from;
    unsigned char  msg;
} FrameHead;

#ifdef __cplusplus
}
#endif

#endif //__MSG_DEFINE_H
