#ifndef __PLATFORM_H
#define __PLATFORM_H

#ifdef __cplusplus
extern "C" {
#endif

#undef ESP_PLATFORM

#ifndef ESP_PLATFORM
#define ICACHE_FLASH_ATTR
#define os_strncmp strncmp
#define os_strlen  strlen
#define os_malloc  malloc
#define os_free    free
#define os_memcpy  memcpy
#define os_strstr  strstr
#define os_zalloc  malloc
#define os_malloc  malloc
#define os_strcpy  strcpy
#define os_memset  memset
#define os_strcmp  strcmp
#endif

#ifdef __cplusplus
}
#endif

#endif // __PLATFORM_H
