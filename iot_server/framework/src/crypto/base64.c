#include "base64.h"

#include <stdio.h>
#include <stdlib.h>

static const char *ALPHA_BASE = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

int base64_encode(const char *buf, const long size, char *out_buf, int out_buf_len)
{
    int a = 0;
    int i = 0;
    if (out_buf_len <= ((size+3)/3)*4)
    {
        printf("invalid out_buf_len %d should > %d \n", out_buf_len, (int)((size+3)/3)*4);
        return -1;
    }
    while (i < size)
    {
        char b0 = buf[i++];
        char b1 = (i < size) ? buf[i++] : 0;
        char b2 = (i < size) ? buf[i++] : 0;

        int int63  = 0x3F; //  00111111
        int int255 = 0xFF; // 11111111
        out_buf[a++] = ALPHA_BASE[(b0 >> 2) & int63];
        out_buf[a++] = ALPHA_BASE[((b0 << 4) | ((b1 & int255) >> 4)) & int63];
        out_buf[a++] = ALPHA_BASE[((b1 << 2) | ((b2 & int255) >> 6)) & int63];
        out_buf[a++] = ALPHA_BASE[b2 & int63];
    }
    switch (size % 3)
    {
        case 1:
            out_buf[--a] = '=';
        case 2:
            out_buf[--a] = '=';
    }

    return 0;
}
