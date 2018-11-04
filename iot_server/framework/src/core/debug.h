#ifndef __DEBUG_H
#define __DEBUG_H

#ifdef  DEBUG
#define DEBUG_TEST 1
#define PR_HERE printf("%s %d \n", __FUNCTION__, __LINE__);
#else
#define DEBUG_TEST 0
#endif

#define filename(x) (strrchr(x,'/')?strrchr(x,'/')+1:x)

#define debug_print(fmt, ...) \
    do { if (DEBUG_TEST) fprintf(stderr, "debug,line:%d " fmt, \
			__LINE__, ##__VA_ARGS__); } while (0)

#define debug_info(fmt, ...) \
    do { if (DEBUG_TEST) printf("%s:%d:%s(): " fmt, filename(__FILE__), \
            __LINE__, __func__, ##__VA_ARGS__); } while (0)

#define debug_log(fmt, ...) \
	do { if (DEBUG_TEST) fprintf(stderr, "%s:%d:%s(): " fmt, filename(__FILE__), \
			__LINE__, __func__, ##__VA_ARGS__); } while (0)

#define debug_error(fmt, ...) \
	do { if (DEBUG_TEST) fprintf(stderr, "error: %s:%d:%s(): " fmt, filename(__FILE__), \
			__LINE__, __func__, ##__VA_ARGS__); } while (0)

#endif // __DEBUG_H
