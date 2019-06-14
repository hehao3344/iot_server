<html>
<head>
<title>System Time</title>
<meta http-equiv="Pragma" content="no-cache">
<link rel="stylesheet" href="../css/mb.css" type="text/css">
<meta http-equiv="Content-Type" content="text/html; charset=gb2312">

<script language="javascript">
function Div(exp1, exp2)
{
    var n1 = Math.round(exp1);
    var n2 = Math.round(exp2);
    var rslt = n1 / n2;
    if (rslt >= 0)
    {
        rslt = Math.floor(rslt);
    }
    else
    {
        rslt = Math.ceil(rslt);
    }
    return rslt;

}

function DateAdd(interval, number, date)
{
    number = parseInt(number);
    if (typeof(date)=="string")
    {
        date = date.split(/\D/);
        --date[1];
        eval("var date = new Date("+date.join(",")+")");
    }
    if (typeof(date)=="object")
    {
        var date = date
    }
    switch(interval)
    {
        case "y": date.setFullYear(date.getFullYear()+number); break;
        case "m": date.setMonth(date.getMonth()+number); break;
        case "d": date.setDate(date.getDate()+number); break;
        case "w": date.setDate(date.getDate()+7*number); break;
        case "h": date.setHours(date.getHour()+number); break;
        case "n": date.setMinutes(date.getMinutes()+number); break;
        case "s": date.setSeconds(date.getSeconds()+number); break;
        case "l": date.setMilliseconds(date.getMilliseconds()+number); break;
    }

    return date;
}

var isInitial = 0;
var sysYear = 2013;
var sysGetTimeStr, secAdd = 0;
var sysRunTime;
var runTime, secRun;

function GetServerTime()
{
    var sysTime = document.getElementById("SystemTime").value;
    var runDay, runHour, runMin, runSec;
    var newTime, year   = "",  month = "",  day = "",    hour = "",  minute = "",  second = "";

    if ( 0 == isInitial )
    {
        var sysMon, sysDay, sysHour, sysMin, sysSec;

        sysYear = sysTime.charAt(0)  +  sysTime.charAt(1) + sysTime.charAt(2) + sysTime.charAt(3);
        sysMon  = sysTime.charAt(4)  +  sysTime.charAt(5);
        sysDay  = sysTime.charAt(6)  +  sysTime.charAt(7);
        sysHour = sysTime.charAt(8)  +  sysTime.charAt(9);
        sysMin  = sysTime.charAt(10)  +  sysTime.charAt(11);
        sysSec  = sysTime.charAt(12)  +  sysTime.charAt(13);
        sysGetTimeStr = sysYear + "-" + sysMon + "-" + sysDay + " " + sysHour + ":" + sysMin + ":" + sysSec;

        runTime = document.getElementById("RunTime").value;
        secRun  = Number(runTime);
        isInitial = 1;
    }

    secAdd += 1;
    newTime = DateAdd("s", secAdd, sysGetTimeStr);
    year   = sysYear;// newTime.getYear();
    month  = add_zero(newTime.getMonth() + 1);
    day    = add_zero(newTime.getDate());
    hour   = add_zero(newTime.getHours());
    minute = add_zero(newTime.getMinutes());
    second = add_zero(newTime.getSeconds());

    runDay  = Div(secRun, 3600 * 24);
    runHour = Div(secRun - runDay  * 3600 * 24 , 3600);
    runMin  = Div(secRun - runDay  * 3600 * 24 - runHour * 3600, 60);
    runSec  = secRun % 60;
    secRun += 1;

    SystemTime.value = year + "-" + month + "-" + day + " " + hour + ":" + minute + ":" + second;
    RunTime.value    = runDay + "天" + runHour + "小时" + runMin + "分" + runSec + "秒";
    //RunTime.innerText    = runDay + "天" + runHour + "小时" + runMin + "分" + runSec + "秒";
}

function add_zero(temp)
{
    if(temp < 10)
    {
        return "0" + temp;
    }
    else
    {
        return temp;
    }
}
setInterval("GetServerTime()",  1000);

function setNtpTime()
{
	var exp  = /^([1-9]|[1-9]\d|1\d\d|2[0-4]\d|25[0-5])\.(([0-9]|[1-9]\d|1\d\d|2[0-4]\d|25[0-5])\.){2}([1-9]|[1-9]\d|1\d\d|2[0-4]\d|25[0-5])$/;

	var ntpAddr1  =   NtpForm.NtpSvrAddr1.value;
	var ntpAddr2  =   NtpForm.NtpSvrAddr2.value;
	var reg1 = ntpAddr1.match(exp);
	var reg2 = ntpAddr2.match(exp);

	if(reg1 == null)
	{
		alert("请输入正确的NTP服务器地址");
		return false;
	}
	else
	{
		NtpForm.action="/goform/PhoneSetNtpTime";
	}
	return true;
}

function handSetTime()
{
	var handTime = handSetTimeForm.HandInputTime.value;

	if( handTime == "")
	{
		alert("时间非法");
		return false;
	}
	else
	{
		handSetTimeForm.action="/goform/PhoneHandSetSystemTime";
	}

	return true;
}

</script>
</head>
<body topmargin="30" leftmargin="0">
    <table width="100%" border="0" cellspacing="0" cellpadding="0">
      <tr>
        <td width="2%"></td>
        <td class="title1"><div align="left"><b>系统时间</b></div></td>
      </tr>
    </table>
<table width="100%"border="0" cellpadding="0" cellspacing="0">
    <tr>
      <td width="2%">&nbsp;</td>
      <td colspan="7" class="title2"><u><b>当前时间</b></u></td>
    </tr>
    <tr>
      <td width="2%">&nbsp;</td>
      <td width="25%">系统时间:</td>
      <td colspan="2" rowspan="2"><% DisplaySystemTime(); %></td>
      <td colspan="3">&nbsp;</td>
    </tr>
    <tr>
      <td>&nbsp;</td>
      <td class="item3">运行时长:</td>
      <td colspan="5">&nbsp;</td>
    </tr>
    <form action="" name="NtpForm" method=POST>
    <tr>
      <td height="20">&nbsp;</td>
      <td colspan="7" class="title2"><u><b>时区</b></u></td>
    </tr>
    <tr>
      <td>&nbsp;</td>
      <td>时区选择:</td>
      <td colspan="2"><% DisplayTimeZoneInfo(); %></td>
      <td colspan="3">&nbsp;</td>
    </tr>
    <tr>
      <td height="20">&nbsp;</td>
      <td colspan="7" class="title2"><u><b>时间服务器</b></u></td>
    </tr>
    <tr>
      <td>&nbsp;</td>
      <td>NTP 服务器1:</td>
      <td colspan="2">
	      <input type=text name=NtpSvrAddr1 title="" style="height:20px; width:150px;" size=40 value="132.163.4.102">	  </td>
      <td colspan="3">&nbsp;</td>
    </tr>
    <tr>
      <td>&nbsp;</td>
      <td>NTP 服务器2:</td>
      <td colspan="2"><input type=text name=NtpSvrAddr2 title="" style="height:20px; width:150px;" size=40 value="192.53.103.103"></td>
      <td>&nbsp;</td>
      <td>
	  <input nowrap type=submit name=smNtp onClick="setNtpTime()" style="height:23px; width:40px;" value="确定" title="Ntp Server">
	  <input nowrap type=submit name=clNtp style="height:23px; width:40px;" value="取消">
	  </td>
    </tr>
    </form>
    <form action="" name="handSetTimeForm" method=POST>
    <tr>
      <td>&nbsp;</td>
      <td colspan="7" class="title2"><u><b>手动设置系统时间</b></u></td>
    </tr>
    <tr>
      <td class="item3">&nbsp;</td>
      <td class="item3">系统时间:</td>
      <td colspan="2"><% DisplayHandInputTime(); %></td>
      <td width="30">&nbsp;</td>
      <td>
	  <input nowrap type=submit name=smHandSet onClick="handSetTime()" style="height:23px; width:40px;" value="确定" title="Ntp Server">
	  <input nowrap type=submit name=clHandSet style="height:23px; width:40px;" value="取消"></td>
    </tr>
    <tr>
      <td>&nbsp;</td>
      <td colspan="2">&nbsp;</td>
      <td>&nbsp;</td>
      <td>&nbsp;</td>
      <td>&nbsp;</td>
	  <td>&nbsp;</td>
    </tr>
    </form>
</table>
</body>
</html>
