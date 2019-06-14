#include "goahead.h"
#include "gbk_utf8.h"
#include "db/id_mgr.h"
#include "goahead_apps.h"

static void dev_list_del_action(Webs *wp);
static void dev_add_action(Webs *wp);

//static void dev_list_modify_action(Webs *wp);
static int  dev_list_jst(int jid, Webs *wp, int argc, char **argv);
static int  utf8_2_gbk(char *dst, char *src, int *out_buf_len);
static void db_got_fn(void * arg, int index, char * dev_uuid, char * openid, char * dev_name, char * online_time, char * offline_time, int online);

static ID_MGR_HANDLE hid_mgr = NULL;

int gohead_apps_init(void)
{
    if (NULL == hid_mgr)
    {
        hid_mgr = id_mgr_create();
    }

    id_mgr_add_device(hid_mgr, "10001122334455");
    id_mgr_update_dev_name(hid_mgr, "10001122334455", "湖南长沙");
    id_mgr_update_online_time(hid_mgr, "10001122334455", "2019-06-08 18:00:00");
    id_mgr_update_offline_time(hid_mgr, "10001122334455", "2019-06-08 18:10:26");
    id_mgr_add_group_openid(hid_mgr, "10001122334455", "abcdefghkdkjikjd");

    id_mgr_add_device(hid_mgr, "1084f3eb83a7aa");
    id_mgr_update_dev_name(hid_mgr, "1084f3eb83a7aa", "广东深圳");
    id_mgr_update_online_time(hid_mgr, "1084f3eb83a7aa", "2019-06-08 18:00:00");


    websDefineJst("dev_list", dev_list_jst);
    websDefineAction("dev_list_del", dev_list_del_action);
    websDefineAction("dev_add",      dev_add_action);

    //websDefineAction("dev_list_modify", dev_list_modify_action);

    return 0;
}

static void db_got_fn(void * arg, int index, char * dev_uuid, char * openid, char * dev_name, char * online_time, char * offline_time, int online)
{
    Webs *wp = (Webs *)arg;

    logmsg(2, "db_got_fn called ");

    websWrite(wp, "<tr>");
    websWrite(wp, "<td width=\"60\"></td>");
    websWrite(wp, "<td width=\"80\">%06d</td>", index);
    websWrite(wp, "<td width=\"160\">%s</td>", dev_uuid);
    websWrite(wp, "<td width=\"160\">%s</td>", openid);

#if 0
    char gbk_dev_name[128] = {0};
    char utf_dev_name[128] = {0};
    int out_len = sizeof(utf_dev_name);
    memset(utf_dev_name, 0, sizeof(utf_dev_name));
    snprintf(utf_dev_name, sizeof(utf_dev_name), "<td width=\"160\">%s</td>", dev_name);
    memset(gbk_dev_name, 0, sizeof(gbk_dev_name));
    utf8_2_gbk(gbk_dev_name, utf_dev_name, &out_len);
    websWrite(wp, gbk_dev_name);
#endif

    websWrite(wp, "<td width=\"160\">%s</td>", dev_name);

    websWrite(wp, "<td width=\"160\">%s</td>", online_time);
    websWrite(wp, "<td width=\"160\">%s</td>", offline_time);
    websWrite(wp, "<td width=\"80\">%d</td>", online);


    websWrite(wp, "<td width=\"10\">");

    websWrite(wp, "<form action=\"/action/dev_list_del\" method=\"post\">");
    //websWrite(wp, "<form action=\"\" name=\"delDevForm\" method=\"post\">");
    //websWrite(wp, "<input type=\"submit\" value=\"del\" onClick=\"confirmDelDev();\"/>");
    websWrite(wp, "<input type=\"submit\" value=\"del\" \"/>");
    websWrite(wp, "<input type=\"hidden\" name=\"id\" value=\"%s\" />", dev_uuid);
    websWrite(wp, "</form>");

    websWrite(wp, "</td>");
    websWrite(wp, "</tr>");
}

static int dev_list_jst(int jid, Webs *wp, int argc, char **argv)
{
    id_mgr_get_uuid(hid_mgr, db_got_fn, (void *)wp);
    return 0;
}

static void dev_list_del_action(Webs *wp)
{
    const char * dev_id = websGetVar(wp, "id", NULL);

    logmsg(2, "dev_list_del_action called = %s ", dev_id);

    websSetStatus(wp, 200);
    websWriteHeaders(wp, -1, 0);
    websWriteEndHeaders(wp);

    websWrite(wp, "<form action=/basic/disp.asp method=POST>");
    if (0 == id_mgr_del_device(hid_mgr, (char *)dev_id))
    {
        websWrite(wp, "Success, Press<input type=submit name=GoFormRet value=return>");
    }
    else
    {
        websWrite(wp, "Failed, Press<input type=submit name=GoFormRet value=return>");
    }
    websWrite(wp, "</form>");

    websFlush(wp, 0);
    websDone(wp);
}

static void dev_add_action(Webs *wp)
{
    const char * dev_id = websGetVar(wp, "devID", NULL);
    const char * dev_name = websGetVar(wp, "devName", "DevName");

    logmsg(2, "dev_add_action called1 = %s %s ", dev_id, dev_name);

    websSetStatus(wp, 200);
    websWriteHeaders(wp, -1, 0);
    websWriteEndHeaders(wp);

    websWrite(wp, "<form action=/basic/add.asp method=POST>");
    if ((NULL == dev_id) || (NULL == dev_name))
    {
        websWrite(wp, "Failed(invalid devID/devName length), Press<input type=submit name=GoFormRet value=return> to return add device page");
    }
    else if ((14 == strlen(dev_id)) && (strlen(dev_name) < 32))
    {
        if ((0 == id_mgr_add_device(hid_mgr, (char *)dev_id)) && (0 == id_mgr_update_dev_name(hid_mgr, (char *)dev_id, (char *)dev_name)))
        {
            websWrite(wp, "Success, Press<input type=submit name=GoFormRet value=return>to return add device page");
        }
        else
        {
            websWrite(wp, "Falied, Press<input type=submit name=GoFormRet value=return>to return add device page");
        }
    }
    else
    {
        websWrite(wp, "Failed, Press<input type=submit name=GoFormRet value=return>to return add device page");
    }
    websWrite(wp, "</form>");

    websWrite(wp, "<form action=/basic/disp.asp method=POST>");

    websWrite(wp, "Press<input type=submit name=GoFormRet2Disp value=return>to return display device page");

    websWrite(wp, "</form>");

    websFlush(wp, 0);
    websDone(wp);
}

#if 0
static void dev_list_modify_action(Webs *wp)
{
    const char * dev_id = websGetVar(wp, "id", NULL);

    logmsg(2, "dev_list_modify_action called = %s ", dev_id);

    websSetStatus(wp, 200);
    websWriteHeaders(wp, -1, 0);
    websWriteEndHeaders(wp);

    websWrite(wp, "<form action=/basic/disp.asp method=POST>");
	websWrite(wp, "Press<input type=submit name=GoFormRet value=return>");
	websWrite(wp, "</form>");

    websFlush(wp, 0);
    websDone(wp);
}
#endif

static int utf8_2_gbk(char *dst, char *src, int *out_buf_len)
{
    int ret = -1;
    gbk_utf8_init();
    unsigned int from_len = strlen(src);
    if (0 == utf8_to_gbk(src, from_len, &dst, (unsigned int *)out_buf_len))
    {
        ret = 0;
    }

    return ret;
}
