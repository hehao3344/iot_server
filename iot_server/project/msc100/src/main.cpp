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
#include "https_client/openid.h"
#include "db/dev_db_mgr.h"

#include "httpd/goahead.h"

int main(int argc, char** argv)
{
    //debug_init(IOT_MODULE_IOT_SERVER, IOT_DEBUG_LEVEL_INFO, (char *)"iot_server");

    //https_client_unit_test();
    //openid_unit_test();
    goahead_init();

    //return 0;

    //websockets_unit_test();

    //id_mgr_unit_test();
    //dev_db_mgr_unit_test();
    //dev_mgr_unit_test();

    DEV_MGR_HANDLE devm_hanle = dev_mgr_create();
    if (NULL == devm_hanle)
    {
        debug_error("dev_mgr_create failed \n");
        return -1;
    }

    OPENID_HANDLE openid_handle = openid_create();
    if (NULL == openid_handle)
    {
        debug_error("openid_create failed \n");
        return -1;
    }

    CLT_MGR_HANDLE cltm_handle = clt_mgr_create(dev_mgr_get_dev_param_handle(devm_hanle));
    if (NULL == cltm_handle)
    {
        debug_error("clt_mgr_create failed \n");
        return -1;
    }

    debug_info("enter event loop \n");
    goahead_eventloop();

    while(1)
    {
        sleep(1);
    }

    return 0;
}


