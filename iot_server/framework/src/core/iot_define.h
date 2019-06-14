#ifndef __IOT_DEFINE_H
#define __IOT_DEFINE_H

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x) ((int)(sizeof(x)/sizeof((x)[0])))
#endif

#define UNUSED_VALUE(x) ((void)(x))

#ifndef MIN
    #define MIN(a, b)   (((a) < (b)) ? (a) : (b))
#endif

#ifndef MAX
    #define MAX(a, b)   (((a) > (b)) ? (a) : (b))
#endif

#endif
