#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <sys/time.h>
#include <pthread.h>
#include <fcntl.h>
#include <signal.h>

#include <dirent.h>
#include <semaphore.h>
#include <sys/prctl.h>

#include "debug_util.h"
#include "msg_util.h"

#include "log.h"

#define LOG_TRUE        1
#define LOG_FALSE       0
#define LOG_OK          0
#define LOG_FAIL        -1
#define ARRAY_SIZE(x)   ((int)(sizeof(x)/sizeof((x)[0])))


static LOG_FILE_S   g_config;
static int          g_iRunThread = LOG_FALSE;
static sem_t        smsg_queue;
static sem_t        smsg_file_del;
pthread_mutex_t     pqueue_mutex = PTHREAD_MUTEX_INITIALIZER;


static LOG_MOD_COUNT_S g_stWriteFlashCount[] =
{
    {0, IOT_MODULE_NONE},
    {0, IOT_MODULE_DEBUG},
    {0, IOT_MODULE_IOT_SERVER},
};

static int log_create_dir(char *path, char *dir_name)
{
    if ((!path) || (!dir_name))
    {
        return -1;
    }
    if (0 == access(path, F_OK))
    {
        if (-1 == access(dir_name, F_OK))
        {
            int iret = -1;

            iret = mkdir(dir_name, S_IXUSR);
            if (-1 == iret)
            {
                printf("mkdir log dir(%s) fail \n", dir_name);
                return -1;
            }
        }
    }
    return 0;
}

static int log_file_num_get(char *pCmdBuf)
{
    char cmd[LOG_COMMAND_MAXSIZE] = {0};
    char result[LOG_COMMAND_MAXSIZE] = {0};
    FILE *fp = NULL;
    int file_num = 0;

    if (pCmdBuf == NULL)
    {
        return 0;
    }
    snprintf(cmd, sizeof(cmd), "%s", pCmdBuf);  //converity
    fp = popen(cmd, "r");
    if (fp == NULL)
    {
        printf("\r |popen error! [%s]\n", cmd);
        return LOG_FAIL;
    }

    if (fgets(result, LOG_COMMAND_MAXSIZE, fp) == NULL)
    {
        printf("\r |fgets error\n");
        pclose(fp);
        return LOG_FAIL;
    }

    pclose(fp);
    if (0 == strlen(result))
    {
        file_num = 0;
    }
    else
    {
        file_num = atoi(result);
    }

    return file_num;
}

static void log_file_num_check(int file_num, char * pcmdBuf)
{
    char cmd[LOG_COMMAND_MAXSIZE] = {0};

    if (pcmdBuf == NULL)
    {
        return;
    }

    if (file_num > g_config.log_file_maxcount)
    {
        snprintf(cmd, sizeof(cmd), "%s", pcmdBuf);    //converity
        system(cmd);
    }
}

/* create a new log file and open it */
static FILE* log_file_new(void)
{
    FILE* fp;
    char filename[LOG_FILE_NAME_MAXSIZE] = {0};
    char timebuf[LOG_TIME_MAXSIZE];
    time_t t = time(NULL);

    log_create_dir(LOG_DIR_FILE_PATH, LOG_DIR_FILE_PATH_SUB);

    snprintf(filename, LOG_FILE_NAME_MAXSIZE - 1, "%s", g_config.log_file_path);

    strftime(timebuf, sizeof(timebuf), "%Y%m%d%H%M%S", localtime(&t));
    strcat(filename, timebuf);
    strcat(filename, ".log");

    fp = fopen(filename, "a+");
    if (fp == NULL)
    {
        printf("Create file \"%s\" fail!\n", filename);
        return NULL;
    }

    snprintf(g_config.log_file_name, sizeof(g_config.log_file_name), "%s", filename);   //converity
    printf("log_file_path = %s, maxcount = %d, log_file_name=%s\n",
             g_config.log_file_path, g_config.log_file_maxcount, g_config.log_file_name);

    return fp;
}

static int log_msg_queue_init(void)
{
    int queue_id = -1;

    queue_id = msg_queue_get(IOT_MSG_SQUEUE_DEBUG);
    if (queue_id == LOG_FAIL)
    {
        printf("iot_msg_queue_get error!\n");
        return LOG_FAIL;
    }
    return queue_id;
}

static void log_config_init(int argc, char **argv)
{
    int ch;
    int iret = -1;
    int tmp_maxcount = 0;

    if (0 == access(LOG_DIR_FILE_PATH, F_OK))
    {
        if (-1 == access(LOG_DIR_FILE_PATH_SUB, F_OK))
        {
            iret = mkdir(LOG_DIR_FILE_PATH_SUB, S_IXUSR);
            if (-1 == iret)
            {
                printf("mkdir log dir(%s) fail \n", LOG_DIR_FILE_PATH_SUB);
            }
        }
    }

    strcpy(g_config.log_file_name, LOG_DEFAULT_FILE_PREFIX);
    g_config.log_file_maxsize = LOG_DEFAULT_FILE_MAXSIZE;

    while ((ch = getopt(argc, argv, "S:n:")) != -1)
    {
        switch (ch)
        {
            case 'S':
            {
                g_config.log_file_maxsize = atoi(optarg) < 1 ? LOG_DEFAULT_FILE_MAXSIZE : atoi(optarg);
                printf("log_file_maxsize:%d\n", g_config.log_file_maxsize);
                break;
            }
            case 'n':
            {
                tmp_maxcount = atoi(optarg) < 1 ? LOG_DEFAULT_FILE_MAXCOUNT : atoi(optarg);
                printf("log_file_maxcount:%d\n", tmp_maxcount);
                break;
            }
        }
    }
    g_config.log_file_maxcount = (tmp_maxcount > LOG_DEFAULT_FILE_MAXCOUNT) ? tmp_maxcount : LOG_DEFAULT_FILE_MAXCOUNT;
    snprintf(g_config.log_file_path, LOG_FILE_PATH_MAXSIZE - 1, LOG_DIR_FILE_PATH_SUB);
    g_config.log_file_maxsize *= 1024;
    printf("g_config.log_file_maxcount = %d, g_config.log_file_path = %s\n",LOG_DEFAULT_FILE_MAXCOUNT, g_config.log_file_path);
}

static int log_do_write(IOT_MSG *recv_msg, FILE *way_out)
{
    time_t t;
    unsigned int  iret = 0;
    char timebuf[LOG_TIME_MAXSIZE] = {0};
    char szModuleCount[64] = {0};
    int  i = 0;

    if ((!recv_msg) || (!way_out))
    {
        return LOG_FAIL;
    }


    if (stdout != way_out)
    {
        if (access(g_config.log_file_name, F_OK) < 0)
        {
            printf("log_do_write: file(%s) not exist!\n", g_config.log_file_name);
            return LOG_FAIL;
        }
    }

    for(i = 0; i < ARRAY_SIZE(g_stWriteFlashCount); i++)
    {
        if (g_stWriteFlashCount[i].i_stModuleId == recv_msg->src_mod)
        {
            if(2147483647 == g_stWriteFlashCount[i].i_stCount)
            {
                g_stWriteFlashCount[i].i_stCount = 0;
            }
            g_stWriteFlashCount[i].i_stCount ++;
            memset(szModuleCount, 0, sizeof(szModuleCount));
            snprintf(szModuleCount, sizeof(szModuleCount), "-%u-", g_stWriteFlashCount[i].i_stCount);
        }
    }

    t = time(NULL);
    strftime(timebuf, sizeof(timebuf), "%Y-%m-%d %H:%M:%S", localtime(&t));
    iret = fwrite(timebuf, sizeof(char), strlen(timebuf), way_out) ;

    if (iret != strlen(timebuf))
    {
        return LOG_FAIL;
    }

    iret = fwrite(szModuleCount, sizeof(char), strlen(szModuleCount), way_out);
    if(iret != strlen(szModuleCount))
    {
        return LOG_FAIL;
    }

    iret = fwrite(recv_msg->body + 1, sizeof(char), recv_msg->body_len - 1, way_out);
    if (iret != recv_msg->body_len - 1)
    {
        return LOG_FAIL;
    }

    iret = fwrite("\n", sizeof(char), strlen("\n"), way_out);
    if (iret != strlen("\n"))
    {
        return LOG_FAIL;
    }

    if (fflush(way_out))
    {
        return LOG_FAIL;
    }

    return LOG_OK;
}

static int log_write(IOT_MSG *recv_msg, FILE *fp)
{
    int ret = LOG_OK;

    if ((!recv_msg) || (!fp))
    {
        return -1;
    }

    ret = log_do_write(recv_msg, fp);

    return ret;
}

static int msg_push(LOG_MSG_QUEUE_S *pQueueData, IOT_MSG *pBufData)
{
    if(pQueueData == NULL || pBufData == NULL)
    {
        return 0;
    }
    if (pQueueData->full)
    {
        printf("queue full \n");
        return 0;
    }
    if(pQueueData->iMsgLen >= LOG_CMD_BUF_LEN)
    {
        printf("queue full \n");
        return 0;
    }

    memcpy(pQueueData->pLogBuf+pQueueData->tail, pBufData, sizeof(IOT_MSG));

    pQueueData->tail++;
    pQueueData->iMsgLen ++;

    if(pQueueData->tail >= LOG_CMD_BUF_LEN)
    {
        pQueueData->tail  = 0;
    }
    if(pQueueData->head == pQueueData->tail)
    {
        pQueueData->full = 1;
    }

    return 1;

}

static int msg_pop(LOG_MSG_QUEUE_S *pQueueData, IOT_MSG *pBufData)
{
    if (NULL == pQueueData || NULL == pBufData)
    {
        return 0;
    }
    if(pQueueData->iMsgLen == 0)
    {
        printf("queue is none \n");
        return 0;
    }

    memcpy((char*)pBufData, (char*)&(pQueueData->pLogBuf[pQueueData->head]), sizeof(IOT_MSG));
    memset((char*)&(pQueueData->pLogBuf[pQueueData->head]), 0, sizeof(IOT_MSG));

    pQueueData->head++;

    if(pQueueData->iMsgLen > 0)
    {
        pQueueData->iMsgLen --;
    }

    if (pQueueData->head >= LOG_CMD_BUF_LEN)
    {
         pQueueData->head = 0;
    }

    pQueueData->full = 0;

    return 1;
}

static int log_msg_routine(void *arg)
{
    int queue_id = -1;
    int ret = 0;
    IOT_MSG recv_msg;

    printf("success : log_msg_routine  thread 1 ok \n");
    if(arg == NULL)
    {
        return 0;
    }

    prctl(PR_SET_NAME, "log_recv");
    while (queue_id < 0)
    {
        queue_id = log_msg_queue_init();
        printf("log_msg_queue_init error\n");
        usleep(1000 * 10);
        continue;
    }

    while (!g_iRunThread)
    {
        memset(&recv_msg, 0, sizeof(IOT_MSG));
        recv_msg.dst_mod = IOT_MODULE_DEBUG;
        ret = msg_recv_log_wait(queue_id, &recv_msg);
        if (ret == LOG_FAIL)
        {
            printf("iot_msg_recv_wait error!\n");
            continue;
        }
        pthread_mutex_lock(&pqueue_mutex);
        ret = msg_push((LOG_MSG_QUEUE_S*)arg,&recv_msg);
        pthread_mutex_unlock(&pqueue_mutex);
        if (ret)
        {
            sem_post(&smsg_queue);
        }
    }

    return LOG_FALSE;

}

static int log_file_size_check()
{
    struct stat file_info;

    if (stat(g_config.log_file_name, &file_info))
    {
        printf("stat file \"%s\" error! (%s)\n", g_config.log_file_name, strerror(errno));
        return LOG_FAIL;
    }

    if (file_info.st_size > g_config.log_file_maxsize)
    {
        return LOG_TRUE;
    }

    return LOG_FALSE;
}

static int log_write_file_thread(void * arg)
{
    int iRet = 0;
    IOT_MSG   stIotBuf;
    FILE *fp = NULL;
    int count = 0;

    printf("success : log_Write_File_Thread  thread 2 ok \n");
    if (arg == NULL)
    {
        return 0;
    }

    prctl(PR_SET_NAME, "log_write");

    while(1)
    {
        fp = log_file_new();
        if (fp == NULL)
        {
            printf("log_file_new error!\n");
            usleep(1000 * 10);
            continue;
        }
        break;
    }
    while (!g_iRunThread)
    {
        memset(stIotBuf.body, 0, IOT_MSG_BODY_LEN_MAX);     //coverity
        sem_wait(&smsg_queue);
        pthread_mutex_lock(&pqueue_mutex);

        iRet = msg_pop((LOG_MSG_QUEUE_S*)arg,(IOT_MSG*)&stIotBuf);

        pthread_mutex_unlock(&pqueue_mutex);

        if (iRet)
        {
            iRet = log_write(&stIotBuf, fp);
            if(iRet < 0)
            {
                goto newloop;       //coverty
            }
            count++;
         }
         if (count == LOG_CHECKFILESIZE_INTERVAL)
         {
            count = 0;
            if (log_file_size_check())
            {
                goto newloop;
            }
         }
         continue;

newloop:
        fclose(fp);
        sem_post(&smsg_file_del);

        while(1)
        {
            fp = log_file_new();
            if (fp == NULL)
            {
                printf("log_file_new error!\n");
                usleep(1000 * 200);
                continue;
            }
            break;
        }
        sem_post(&smsg_file_del);
    }

    if (fp != NULL)      //converity
    {
        fclose(fp);
    }

    return LOG_OK;
}

static int log_file_del_zip_thread()
{
    char cmd[LOG_COMMAND_MAXSIZE] = {0};
    int iRet = 0;

    printf("success : log_File_Del_Zip_Thread  thread 4 ok \n");

    prctl(PR_SET_NAME, "log_del");
    sem_post(&smsg_file_del);
    while(1)
    {
        sem_wait(&smsg_file_del);
        snprintf(cmd, sizeof(cmd), "ls %s |grep \"\\.log\"|wc -l", g_config.log_file_path);
        printf("cmd = %s \n",cmd);
        iRet = log_file_num_get(cmd);
        IOT_INFO("get file num (%s):(%d)\n", g_config.log_file_path, iRet);
        if (iRet > g_config.log_file_maxcount)
        {
            memset(cmd,0,LOG_COMMAND_MAXSIZE);
            snprintf(cmd, sizeof(cmd), "rm %s$(ls %s -rt |grep \"\\.log\"|sed -n \"1p\")", g_config.log_file_path, g_config.log_file_path );
            printf("cmd = %s \n",cmd);
            log_file_num_check(iRet, cmd);
        }
    }
    return 0;
}

int log_init_check()
{
    if (0 != access(LOG_DIR_FILE_PATH_SUB, F_OK))
    {
        return 0;
    }
    char cmd[LOG_COMMAND_MAXSIZE] = {0};
    snprintf(cmd, sizeof(cmd), "ls %s |grep \"\\.log\"|wc -l", g_config.log_file_path);
    printf("cmd = %s \n",cmd);
    if (log_file_num_get(cmd) >= g_config.log_file_maxcount)
    {
        memset(cmd,0,LOG_COMMAND_MAXSIZE);
        snprintf(cmd, sizeof(cmd), "rm %s$(ls %s -rt |grep \"\\.log\"|sed -n \"1p\")", g_config.log_file_path, g_config.log_file_path );
        printf("cmd = %s \n",cmd);
        system(cmd);
    }

    return 0;
}

int logs_main(int argc, char *argv[])
{
    LOG_MSG_QUEUE_S *pmsgQueue = NULL;
    IOT_MSG *pLogData = NULL;
    pthread_t tid[3];
    int queue_id = 0;

    signal(SIGPIPE, SIG_IGN);

    while(1)
    {
        pmsgQueue = (LOG_MSG_QUEUE_S*)malloc(sizeof(LOG_MSG_QUEUE_S));
        if (!pmsgQueue)
        {
            continue;
        }
        break;
    }
    while(1)
    {
        pLogData = (IOT_MSG*)malloc(sizeof(IOT_MSG)*LOG_CMD_BUF_LEN);
        if (!pLogData)
        {
            continue;
        }
        break;
    }
    memset((char*)pmsgQueue, 0, sizeof(LOG_MSG_QUEUE_S));
    memset((char*)pLogData, 0, sizeof(IOT_MSG)*LOG_CMD_BUF_LEN);

    pmsgQueue->pLogBuf = pLogData;

    while(1)
    {
        queue_id = log_msg_queue_init();
        if (queue_id < 0)
        {
           printf("log_msg_queue_init error\n");
           usleep(1000*10);
           continue;
        }
        break;
    }

    log_config_init(argc, argv);

    log_init_check();

    sem_init(&smsg_queue, 0, 0);
    sem_init(&smsg_file_del, 0, 0);

    debug_init(IOT_MODULE_LOG, IOT_DEBUG_LEVEL_INFO, "log");

    pthread_create(&tid[0], NULL, (void *)log_msg_routine, (void*)pmsgQueue);
    pthread_create(&tid[1], NULL, (void *)log_write_file_thread, (void*)pmsgQueue);
    pthread_create(&tid[2], NULL, (void *)log_file_del_zip_thread, (void*)NULL);

    pthread_join(tid[0],NULL);
    pthread_join(tid[1],NULL);
    pthread_join(tid[2],NULL);

    free(pmsgQueue->pLogBuf);
    free(pmsgQueue);

    return LOG_FAIL;
}
