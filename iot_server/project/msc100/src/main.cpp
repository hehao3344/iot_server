#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "core/core.h"
#include "db/id_mgr.h"
#include "device_manage/dev_mgr.h"
#include "db/dev_db_mgr.h"

int main(int argc, char** argv)
{
    //id_mgr_unit_test();
    //dev_db_mgr_unit_test();
    dev_mgr_unit_test();

    return 0;
}
