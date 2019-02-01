#ifndef __IOT_DEBUG_UTIL_H__
#define __IOT_DEBUG_UTIL_H__

#ifdef __cplusplus
extern "C"
{
#endif

#define LOG_CONFIG_FILE     "/tmp/log.conf"

typedef enum
{
    IOT_DEBUG_LEVEL_DEBUG           = 0x0,
    IOT_DEBUG_LEVEL_INFO            = 0x1,
    IOT_DEBUG_LEVEL_ERROR           = 0x2,
} IOT_DEBUG_LEVEL_E;

extern char g_module_name[32];

/****************************************************************************/
/*               Please use this in your code for logging                   */
/****************************************************************************/
int debug_printf(int way, const char* fmt, ...) __attribute__((format(printf, 2, 3)));

/* eg: iot_debug_print(IOT_DEBUG_LEVEL_WARNING,IOT_DEBUG_WAY_ALL,"%s","xx") */


/****************************************************************************/
/*               Call init first before using "debug_print"             */
/****************************************************************************/
int debug_init(int module, int level, char *tag);
/* eg: iot_debug_init(IOT_MODULE_MQTT, IOT_DEBUG_LEVEL_INFO, "iot_event_loop)*/


/****************************************************************************/
/*               Limit output by setting the level                          */
/****************************************************************************/
void debug_level_set(int level);
/* eg: iot_debug_level_set(IOT_DEBUG_LEVEL_WARNING)                         */
/* It mean INFO message were ignore, leaving WARNING and ERROR message send */

/****************************************************************************/
/*               Set module name in log                          */
/****************************************************************************/
void debug_tag_set(char *name);
/* eg: iot_debug_tag_set("My module name")                                  */



/****************************************************************************/
/*            Too complicated for "iot_debug_print", right?                 */
/*    The following is more easier but with more info, and is recommended   */
/*         For example, use "IOT_INFO" instead of "printf" directly         */
/****************************************************************************/
#define IOT_DEBUG(fmt, args...)\
        debug_printf(IOT_DEBUG_LEVEL_DEBUG, "%d [%s %s:%d DEBUG] "fmt, IOT_DEBUG_LEVEL_DEBUG, g_module_name, __func__, __LINE__, ##args)
#define IOT_INFO(fmt, args...)\
        debug_printf(IOT_DEBUG_LEVEL_INFO, "%d [%s %s:%d INFO] "fmt, IOT_DEBUG_LEVEL_INFO, g_module_name, __func__, __LINE__, ##args)
#define IOT_ERROR(fmt, args...)\
        debug_printf(IOT_DEBUG_LEVEL_ERROR, "%d [%s %s:%d ERROR]  "fmt, IOT_DEBUG_LEVEL_ERROR, g_module_name, __func__, __LINE__, ##args)


#ifdef __cplusplus
}
#endif

#endif
