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
        alert("�û����Ϸ����������@126.com��@qq.com!");
        return false;
    }
    var reg = send.match(mailmatch);
    if (reg == null)
    {
        alert("����Email��ַ���Ϸ�!");
        return false;
    }
    reg = rcv1.match(mailmatch);
    if (reg == null)
    {
        alert("����Email��ַ1���Ϸ�!");
        return false;
    }
    reg = rcv2.match(mailmatch);
    if (reg == null)
    {
        alert("����Email��ַ2���Ϸ�!");
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
      <td weith="40%" class="title1"><b>SMTP����</b></td>
      <td></td>
    </tr>
</table>
<table width="100%" border="0" cellspacing="0" cellpadding="0">
  <form action="" name="alarmSet" method=POST>
  <tr>
    <td width="2%">&nbsp;</td>
    <td colspan="3" class="title2"><u><b>SMTP���ã�Ŀǰ֧��126��QQ���䣬�������������п���SMTP��</b></u></td>
  </tr>
  <tr>
    <td>&nbsp;</td>
    <td width="32%">��ѡ������:</td>
    <td rowspan="6">
	    <% DisplaySmtpInfo(); %>
	</td>
    <td colspan="-4">&nbsp;</td>
  </tr>
  <tr>
    <td>&nbsp;</td>
    <td>���÷�����:</td>
    <td colspan="-4">&nbsp;</td>
  </tr>
  <tr>
    <td>&nbsp;</td>
    <td>�û���:</td>
    <td>&nbsp;</td>
  </tr>
  <tr>
    <td>&nbsp;</td>
    <td class="item3">����:</td>
    <td colspan="-4">&nbsp;</td>
  </tr>
  <tr>
    <td>&nbsp;</td>
    <td>����Email��ַ:</td>
    <td>&nbsp;</td>
  </tr>
  <tr>
    <td>&nbsp;</td>
    <td>����Email��ַ:</td>
    <td>
	  <input nowrap type=submit name=smTest onClick="saveSmtp()" style="height:23px; width:40px;" value="ȷ��" title="">
	  <input nowrap type=submit name=clSmtp style="height:23px; width:40px;" value="ȡ��">
	</td>
  </tr>
  </form>
  <form action="" name="alarmIntervalForm" method=POST>
  <tr>
    <td>&nbsp;</td>
    <td colspan="3" class="title2"><u><b>SMTP��ʱ����</b></u></td>
  </tr>
  <tr>
      <td>&nbsp;</td>
      <td>��ʱ����:</td>
      <td rowspan="2"><% DisplaySmtpSendInterval(); %></td>
      <td rowspan="2">
        <input nowrap type=submit name=smEnable onClick="setSmtpInterval()" style="height:23px; width:40px;" value="ȷ��" title="">
        <input nowrap type=submit name=clEnable style="height:23px; width:40px;" value="ȡ��" title="ȡ��"></td>
      <td>&nbsp;</td>
    </tr>
  </form>
</table>

</body>
</html>

