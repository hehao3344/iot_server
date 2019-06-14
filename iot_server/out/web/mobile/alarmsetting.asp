<html>
<head>
<title></title>
<meta http-equiv="Pragma" content="no-cache">
<link rel="stylesheet" href="../css/mb.css" type="text/css">
<meta http-equiv="Content-Type" content="text/html; charset=gb2312">

<script language="javascript">
document.oncontextmenu=new Function("event.returnValue=false");
//document.onselectstart=new Function("event.returnValue=false");
var mailmatch = /^([\w-\.]+)@((\[[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}\.)|(([\w-]+\.)+))([a-zA-Z]{2,4}|[0-9]{1,3})(\]?)$/;

function saveSmtp()
{
	var uname=alarmSet.smtpuname.value;
    var send=alarmSet.smtpsendaddr.value;
    var rcv1=alarmSet.smtprecaddr1.value;
    var rcv2=alarmSet.smtprecaddr1.value;

    var reg = uname.match(mailmatch);
    if (reg == null)
    {
        alert("用户不合法，必须包含@126.com或@qq.com!");
        return false;
    }
    var reg = send.match(mailmatch);
    if (reg == null)
    {
        alert("发送Email地址不合法!");
        return false;
    }
    reg = rcv1.match(mailmatch);
    if (reg == null)
    {
        alert("接收Email地址1不合法!");
        return false;
    }
    reg = rcv2.match(mailmatch);
    if (reg == null)
    {
        alert("接收Email地址2不合法!");
        return false;
    }
    alarmSet.action="/goform/PhoneSaveSmtp";
}

function setSmtpInterval()
{
    alarmIntervalForm.action="/goform/PhoneSetSmtpSendInterval";
	return true;
}
</script>
</head>

<body topmargin="30" leftmargin="0">
<table width="100%" border="0" cellspacing="0" cellpadding="0">
    <tr>
      <td width="2%"></td>
      <td weith="40%" class="title1"><b>SMTP设置</b></td>
      <td></td>
    </tr>
</table>
<table width="100%" border="0" cellspacing="0" cellpadding="0">
  <form action="" name="alarmSet" method=POST>
  <tr>
    <td width="2%">&nbsp;</td>
    <td colspan="3" class="title2"><u><b>SMTP设置（目前支持126和QQ邮箱，请在邮箱设置中开启SMTP）</b></u></td>
  </tr>
  <tr>
    <td>&nbsp;</td>
    <td width="32%">首选服务器:</td>
    <td rowspan="6">
	    <% DisplaySmtpInfo(); %>
	</td>
    <td colspan="-4">&nbsp;</td>
  </tr>
  <tr>
    <td>&nbsp;</td>
    <td>备用服务器:</td>
    <td colspan="-4">&nbsp;</td>
  </tr>
  <tr>
    <td>&nbsp;</td>
    <td>用户名:</td>
    <td>&nbsp;</td>
  </tr>
  <tr>
    <td>&nbsp;</td>
    <td class="item3">密码:</td>
    <td colspan="-4">&nbsp;</td>
  </tr>
  <tr>
    <td>&nbsp;</td>
    <td>发送Email地址:</td>
    <td>&nbsp;</td>
  </tr>
  <tr>
    <td>&nbsp;</td>
    <td>接收Email地址:</td>
    <td>
	  <input nowrap type=submit name=smTest onClick="saveSmtp()" style="height:23px; width:40px;" value="确定" title="">
	  <input nowrap type=submit name=clSmtp style="height:23px; width:40px;" value="取消">
	</td>
  </tr>
  </form>
  <form action="" name="alarmIntervalForm" method=POST>
  <tr>
    <td>&nbsp;</td>
    <td colspan="3" class="title2"><u><b>SMTP定时任务</b></u></td>
  </tr>
  <tr>
      <td>&nbsp;</td>
      <td>定时任务:</td>
      <td rowspan="2"><% DisplaySmtpSendInterval(); %></td>
      <td rowspan="2">
        <input nowrap type=submit name=smEnable onClick="setSmtpInterval()" style="height:23px; width:40px;" value="确定" title="">
        <input nowrap type=submit name=clEnable style="height:23px; width:40px;" value="取消" title="取消"></td>
      <td>&nbsp;</td>
    </tr>
  </form>
</table>

</body>
</html>

