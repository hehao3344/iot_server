#ifndef __BASE64_h
#define __BASE64_h

#include <stdio.h>

#if __cplusplus
extern "C"{
#endif

int base64_encode(const char *buf, const long size, char *out_buf, int out_buf_len);

#if __cplusplus
}
#endif

#endif /* base64_h */
