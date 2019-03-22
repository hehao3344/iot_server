#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>


#include "log/debug_util.h"
#include "log/msg_util.h"

#include "core/core.h"
#include "db/id_mgr.h"
#include "device_manage/dev_mgr.h"
#include "device_manage/websockets.h"
#include "db/dev_db_mgr.h"

int main(int argc, char** argv)
{
    debug_init(IOT_MODULE_IOT_SERVER, IOT_DEBUG_LEVEL_INFO, (char *)"iot_server");

    websockets_unit_test();

    //id_mgr_unit_test();
    //dev_db_mgr_unit_test();
    //dev_mgr_unit_test();

    return 0;
}


