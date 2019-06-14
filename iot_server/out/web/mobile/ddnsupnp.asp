<html>
<head>
<title>DDNS/UPnP</title>
<meta  http-equiv="refresh" content="30">
<meta  http-equiv="Pragma"  content="no-cache">
<link rel="stylesheet" href="../css/mb.css" type="text/css">
<meta http-equiv="Content-Type" content="text/html; charset=gb2312">

<script language="javascript">
function setDdnsInfo()
{
    var ddnsName=document.getElementById("ddnsUserName").value;
    var ddnsPw=document.getElementById("ddnsPassword").value;
    var server=document.getElementById("DdnsServer").value;

	if ( ddnsName == "" )
	{
		alert("�û�������Ϊ��!");
		return false;
	}
	if ( ddnsPw == "" )
	{
		alert("���벻��Ϊ��!");
		return false;
	}

	DdnsFrom.action="/goform/PhoneSetDdnsInfo";

	return true;
}

function setUpnpInfo()
{
    var outPort  = document.getElementById("UpnpHttpOutPort").value;
	var num 	 = "^[0-9]*[1-9][0-9]*$";
	var regPort  = outPort.match(num);;
	if( regPort == null )
	{
		alert("�˿�ֻ��������!");
		return false;
	}
	if( outPort.length >  5 )
	{
		alert("�˿ڱ���С��65535!");
		return false;
	}

    UpnpFrom.action="/goform/PhoneSetUpnpInfo";

	return true;
}

function GoToSelectDdns()
{
	var wsEl = document.getElementById("whichServer");
	var ws 	 = wsEl.value;
	var dsEl = document.getElementById("DdnsServer");
	if (ws == "DynDns")
	{
		dsEl.value   = "DynDns";
	}
	else
	{
		dsEl.value   = "net3322";
	}
}

</script>

</head>
<body topmargin="30" leftmargin="0">
<table width="100%" border="0" cellspacing="0" cellpadding="0">
    <tr>
        <td width="2%"></td>
        <td class="title1"><div align="left"><b>DDNS&amp;UPnP����</b></div></td>
    </tr>
</table>
<table width="100%" border="0" cellspacing="0" cellpadding="0">
    <tr>
        <td width="2%">&nbsp;</td>
        <td colspan="6" class="title2"><u><b>DDNS����</b></u></td>
    </tr>
    <form name="DdnsFrom" method="post">
    <tr>
        <td>&nbsp;</td>
        <td width="30%">�����ṩ��:</td>
        <td colspan="2"><% DisplayDdnsServer(); %></td>
        <td>&nbsp;</td>
        <td colspan="2">&nbsp;</td>
    </tr>
    <tr>
        <td>&nbsp;</td>
        <td>�û���:</td>
        <td colspan="3" rowspan="3"><% DisplayDdnsAddr(); %></td>
        <td colspan="2">&nbsp;</td>
    </tr>
    <tr>
        <td>&nbsp;</td>
        <td>����:</td>
        <td colspan="2">&nbsp;</td>
    </tr>
    <tr>
        <td>&nbsp;</td>
        <td>����״̬:</td>
        <td colspan="2">&nbsp;</td>
      </tr>
	<tr>
        <td>&nbsp;</td>
        <td>�Ƿ���:</td>
        <td colspan="2"><% DisplayDdnsEnable(); %></td>
        <td>&nbsp;</td>
        <td>
	      <input type=submit name=setDdns id=setDdns onClick="setDdnsInfo()" style="height:23px; width:40px;" value="ȷ��" title="">
          <input type=submit name=clDdns id=clDdns style="height:23px; width:40px;" value="ȡ��" title="ȡ��">
		</td>
        <td>&nbsp;</td>
	</tr>
    </form>
    <tr>
        <td>&nbsp;</td>
        <td colspan="6" class="title2"><u><b>UPnP����</b></u></td>
    </tr>
    <form name="UpnpFrom" method="POST" action="">
    <tr>
        <td>&nbsp;</td>
        <td>�ⲿ�˿�:</td>
        <td colspan="3" rowspan="2"><% DisplayUpnpInfo(); %></td>
        <td colspan="2">&nbsp;</td>
    </tr>
    <tr>
        <td>&nbsp;</td>
        <td>״̬:</td>
        <td colspan="2">&nbsp;</td>
      </tr>
	<tr>
        <td>&nbsp;</td>
        <td>�Ƿ���:</td>
        <td colspan="2"><% DisplayUpnpEnable(); %></td>
        <td>&nbsp;</td>
        <td>
   		   <input type=submit name=setUpnp id=setUpnp onClick="setUpnpInfo()" style="height:23px; width:40px;" value="ȷ��" title="Upnp">
           <input type=submit name=clUpnp id=clUpnp style="height:23px; width:40px;" value="ȡ��" title="ȡ��">
		</td>
        <td>&nbsp;</td>
	</tr>
    </form>
</table>
</body>
</html>
