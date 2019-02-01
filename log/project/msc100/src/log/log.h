#ifndef _LOG_H_
#define _LOG_H_

#include <arpa/inet.h>
#include <netdb.h>

#include "msg_util.h"

#define LOG_TIME_MAXSIZE 32
#define LOG_COMMAND_MAXSIZE 255
#define LOG_CHECKFILESIZE_INTERVAL 100

//========== s_config ===========
#define LOG_FILE_NAME_MAXSIZE 255
#define LOG_FILE_PATH_MAXSIZE 255

typedef struct
{
    char log_file_name[LOG_FILE_NAME_MAXSIZE];
    char log_file_path[LOG_FILE_PATH_MAXSIZE];
    int  log_file_maxcount;
    int  log_file_maxsize;//unit:B
} LOG_FILE_S;

typedef struct
{
    int module;
    int level;
    char tag[32];
} LOG_MOD_TBL;

typedef struct
{
    int reg_mod_num;
    LOG_MOD_TBL module_tbl[IOT_MODULE_ID_NUM];
} LOG_MODULE_S;

#define LOG_DEFAULT_FILE_PREFIX     "mkit_"
#define LOG_DEFAULT_FILE_PATH       "logfile/"
#define LOG_DIR_FILE_PATH           "./"
#define LOG_DIR_FILE_PATH_SUB       "./logfile/"
#define LOG_TMP_FILE_PATH           "./"
#define LOG_DEFAULT_FILE_MAXCOUNT   10
#define LOG_DEFAULT_FILE_MAXSIZE    1024 // unit(KB)

typedef struct
{
    unsigned int    i_stCount;
    int             i_stModuleId;
} LOG_MOD_COUNT_S;


#define IOT_LOG_TAR_NUM (1000)
#define LOG_CMD_BUF_LEN  (2500)

typedef struct
{
    IOT_MSG *pLogBuf;
    int iMsgOffset;
    int iMsgLen;
    int head;
    int tail;
    int full;
} LOG_MSG_QUEUE_S;

#endif
