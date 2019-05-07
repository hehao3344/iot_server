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
#include "device_manage/clt_mgr.h"
#include "https_client/https_client.h"
#include "db/dev_db_mgr.h"

int main(int argc, char** argv)
{
    //debug_init(IOT_MODULE_IOT_SERVER, IOT_DEBUG_LEVEL_INFO, (char *)"iot_server");

    //https_client_unit_test();

    //websockets_unit_test();

    //id_mgr_unit_test();
    //dev_db_mgr_unit_test();
    //dev_mgr_unit_test();

#if 1
    DEV_MGR_HANDLE devm_hanle = dev_mgr_create();
    if (NULL == devm_hanle)
    {
        debug_error("dev_mgr_create failed \n");
        return -1;
    }
#endif

#if 1
    CLT_MGR_HANDLE cltm_handle = clt_mgr_create(dev_mgr_get_dev_param_handle(devm_hanle));
    if (NULL == cltm_handle)
    {
        debug_error("clt_mgr_create failed \n");
        return -1;
    }
#endif

    while(1)
    {
        sleep(1);
    }

    return 0;
}


