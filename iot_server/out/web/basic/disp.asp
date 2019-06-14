<html>
<head>
<title></title>
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
var sysYear   = 2013;
var sysGetTimeStr, secAdd = 0;

function GetServerTime()
{
    var sysTime = document.getElementById("SystemTime").value;
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

	SystemTime.value = year + "-" + month + "-" + day + " " + hour + ":" + minute + ":" + second;

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

function SetRecorderPlan()
{
	RecorderPlanFrom.action="/goform/PhoneRecorderPlan";
	return true;
}

function HandSetIOSwitch()
{
	HandSetIOForm.action="/goform/PhoneHandSetIOSwitch";
	return true;
}

function HandSetIOSwitchTimer()
{
	HandSetIOTimerForm.action="/goform/PhoneSetIOSwitchTimer";
	return true;
}

</script>
</head>

<body topmargin="30" leftmargin="0">

<table width="100%" border="0" cellspacing="0" cellpadding="0">
    <tr>
        <td width="2%"></td>
        <td class="title1"><b>设备状态管理</b></td>
    </tr>
</table>

<table width="100%" border="0" cellspacing="0" cellpadding="0">
    <tr>
        <td width="5%"></td>
        <td width="10%">序号</td>
        <td width="25%">设备ID</td>
        <td width="30%">设备名</td>
        <td width="30%">上线时间</td>
    </tr>
</table>

<table width="100%" border="0" cellspacing="0" cellpadding="0">
    <tr>
        <td colspan="18"><% DisplayDevInfo(); %></td>
    </tr>
</table>
</body>
</html>
