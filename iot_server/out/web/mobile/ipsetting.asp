<html>
<head>
<title></title>
<meta http-equiv="Pragma" content="no-cache">
<link rel="stylesheet" href="../css/mb.css" type="text/css">
<meta http-equiv="Content-Type" content="text/html; charset=gb2312">

<script language="javascript">
var exp  = /^([1-9]|[1-9]\d|1\d\d|2[0-4]\d|25[0-5])\.(([0-9]|[1-9]\d|1\d\d|2[0-4]\d|25[0-5])\.){2}([1-9]|[1-9]\d|1\d\d|2[0-4]\d|25[0-5])$/;

function GoToWlanDhcpPage()
{
 	var optionValue = document.getElementById("SetWlanAddrMethod").value;
    if (optionValue == "WlanDhcpMethod")
	{
		document.all.WlanIP.disabled= true;
		document.all.WlanNetmask.disabled  = true;
		document.all.WlanGateway.disabled  = true;
        BasicWlanIPAddr.BasicWlanIPAddrSet.value  = "ȷ��";
	}
	else
	{
		document.all.WlanIP.disabled= false;
		document.all.WlanNetmask.disabled  = false;
		document.all.WlanGateway.disabled  = false;
		BasicWlanIPAddr.BasicWlanIPAddrSet.value  = "�ύ";
	}

	return true;
}

function ConformBasicWlanIPAddr()
{
    var ipobj=BasicWlanIPAddr.WlanIP.value;
    var nmobj=BasicWlanIPAddr.WlanNetmask.value;
    var gwobj=BasicWlanIPAddr.WlanGateway.value;

    var nmexp= /(\d+)\.(\d+)\.(\d+)\.(\d+)/g;

    var getIPMethod=BasicWlanIPAddr.BasicWlanIPAddrSet.value;

    if ( "�ύ" == getIPMethod )
    {
		var reg = ipobj.match(exp);
		if (reg == null)
		{
			alert("IP��ַ���Ϸ�!");
			return false;
		}

		// netmask
		reg = nmobj.match(nmexp);
		if (reg == null)
		{
			alert("�������벻�Ϸ�!");
			return false;
		}
		// gateway
		reg = gwobj.match(exp);
		if (reg == null)
		{
			alert("Ĭ�����ص�ַ���Ϸ�!");
			return false;
		}
	}
	else
	{
		if(	BasicWlanIPAddr.BasicWlanIPAddrSet.value == "ȷ��")
		{
			if (confirm("ȷ��ʹ��DHCP��ʽ��ȡIP?"))
			{
				BasicWlanIPAddr.action="/goform/PhoneSetBasicWlanIPAddr";
				return true;
			}
		}
	}
	if ( confirm("ȷ���޸�?") )
	{
		BasicWlanIPAddr.action="/goform/PhoneSetBasicWlanIPAddr";
		return true;
	}
}

function setDnsAddr()
{
    var firobj=dnsIpAddr.firstDns.value;
    var secobj=dnsIpAddr.secondDns.value;

    // first dns.
    var reg = firobj.match(exp);
    if (reg == null)
    {
        alert("��ѡDNS��ַ���Ϸ�!!!");
        return false;
    }
    // second dns.
    var reg = secobj.match(exp);
    if (reg == null)
    {
        alert("����DNS��ַ���Ϸ�!!!");
        return false;
    }
    if (confirm("ȷ���޸�?"))
    {
    	dnsIpAddr.action="/goform/PhoneSetDnsAddr";
    	return true;
    }
}

function SaveHttpPort()
{
    var portStr=formHttpPort.HttpPort.value;
    var port=parseInt(portStr);
    if ( (0 == port)  ||  (port > 65535) )
    {
        alert("�˿ڲ��Ϸ�");
        return false;
    }
    formHttpPort.action="/goform/PhoneSetHttpPort";
}

function SetDevName()
{
    var dev_name = devName.DeviceName.value;
    if ( null == dev_name )
    {
        alert("�豸�����Ȳ��Ϸ�");
        return false;
    }
    devName.action="/goform/SetDeviceName";
}

function setServerAddr()
{
	var svr1 = ServerAddrForm.Addr1.value;
    var svr2 = ServerAddrForm.Addr2.value;
    var svr3 = ServerAddrForm.Addr3.value;
	var nmexp= /(\d+)\.(\d+)\.(\d+)\.(\d+)/g;
	var reg = svr1.match(exp);
	if ( reg == null )
	{
		alert("IP��ַ1���Ϸ�!");
		return false;
	}

	ServerAddrForm.action="/goform/SetServerAddr";
}

function changePasswordCheck()
{
	var pwone = changePassword.Passwordone.value;		
	var pwtwo = changePassword.Passwordtwo.value;
	if(pwone == "")
	{
		alert("���벻��Ϊ��");
		return false;
	}
	else if( pwone != pwtwo )
	{
		alert("������������벻һ��,����������");
		return false;
	}
	else
	{	
		changePassword.action="/goform/ChangeLoginInfo";		
		 
	}
	return true;
}

</script>

</head>
<body onLoad="GoToDhcpPage();GoToWlanDhcpPage();" topmargin="30" leftmargin="0">
  <table width="100%" border="0" cellspacing="0" cellpadding="0">
    <tr>
      <td width="2%">&nbsp;</td>
      <td class="title1"><div align="left"><b>IP��ַ����</font></div></td>
    </tr>
  </table>
  <table width="100%" border="0" cellspacing="0" cellpadding="0">
    <tr>
      <td width="2%">&nbsp;</td>
      <td colspan="6"  class="title2"><u><b>��������IP��ַ����</b></u></td>
    </tr>
    <tr>
      <td>&nbsp;</td>
      <td>IP��ַ���÷�ʽ:	  </td>
      <td colspan="2"><% DisplayMethodOfGetWlanIp(); %></td>
      <td>&nbsp;</td>
      <td colspan="2">&nbsp;</td>
    </tr>
    <form name="BasicWlanIPAddr" method="post" action="">
    <tr>
      <td>&nbsp;</td>
      <td>��������IP��ַ:  </td>
      <td colspan="2" rowspan="3"><% DisplayWlanIPAddress(); %></td>
      <td>&nbsp;</td>
      <td colspan="2">&nbsp;</td>
    </tr>
    <tr>
      <td>&nbsp;</td>
      <td>��������:</td>
      <td>&nbsp;</td>
      <td colspan="2">&nbsp;</td>
    </tr>
    <tr>
      <td>&nbsp;</td>
      <td>Ĭ������:</td>
      <td>&nbsp;</td>
      <td>&nbsp;</td>
      <td>
	    <input type=submit name=BasicWlanIPAddrSet onClick="ConformBasicWlanIPAddr()" style="height:23px; width:40px;" value="�ύ" title="submit">
        <input type=submit name=BasicWlanIPAddrCancel style="height:23px; width:40px;" value="ȡ��" title="cancel">	  </td>
    </tr>
	</form>
	<form name="dnsIpAddr" method="post" action="">
    <tr>
      <td>&nbsp;</td>
      <td colspan="6" class="title2"><u><b>DNS������</b></u></td>
    </tr>
    <tr>
      <td>&nbsp;</td>
      <td>��ѡDNS:</td>
      <td colspan="2" rowspan="2">
	      <% DisplayDnsAddr(); %>
	  </td>
      <td>&nbsp;</td>
      <td colspan="2">&nbsp;</td>
    </tr>
    <tr>
      <td>&nbsp;</td>
      <td>����DNS:</td>
      <td>&nbsp;</td>
      <td>&nbsp;</td>
      <td>
	    <input type=submit name=setDns onClick="setDnsAddr()" style="height:23px; width:40px;" value="ȷ��" title="submit">
        <input type=submit name=cancelDns style="height:23px; width:40px;" value="ȡ��" title="cancel">
    </td>
    </tr>
	</form>
	<form name="formHttpPort" method="post" action="">
	<tr>
      <td>&nbsp;</td>
      <td colspan="6" class="title2"><u><b>HTTP����˿�</b></u></td>
    </tr>
	<tr>
      <td>&nbsp;</td>
      <td>�˿ں�:</td>
      <td colspan="2">
	    <% DisplayHttpPort(); %>
	  </td>
      <td>&nbsp;</td>
      <td>&nbsp;</td>
      <td>
	    <input type=submit name=setHttpPort onClick="SaveHttpPort()" style="height:23px; width:40px;" value="ȷ��" title="submit">
        <input type=submit name=cancelDns style="height:23px; width:40px;" value="ȡ��" title="cancel">	  </td>
	</tr>
	</form>
	<form name="devName" method="post" action="">
	<tr>
      <td>&nbsp;</td>
      <td colspan="6" class="title2"><u><b>�豸��</b></u></td>
    </tr>
	<tr>
      <td>&nbsp;</td>
      <td>�豸��:</td>
      <td colspan="2">
	    <% DisplayDeviceName(); %>
	  </td>
      <td>&nbsp;</td>
      <td>&nbsp;</td>
      <td>
	    <input type=submit name=set_dev_name onClick="SetDevName()" style="height:23px; width:40px;" value="ȷ��" title="submit">
        <input type=submit name=cancel style="height:23px; width:40px;" value="ȡ��" title="cancel">
      </td>
	</tr>
	</form>
	<form name="ServerAddrForm" method="post" action="">
    <tr>
      <td>&nbsp;</td>
      <td colspan="6" class="title2"><u><b>��������ַ����</b></u></td>
    </tr>
    <tr>
      <td>&nbsp;</td>
      <td>��ַ1:</td>
      <td colspan="2" rowspan="3">
	      <% DisplayServerAddr(); %>
	  </td>
      <td>&nbsp;</td>
      <td colspan="2">&nbsp;</td>
    </tr>
	<tr>
      <td>&nbsp;</td>
      <td>��ַ2:</td>
      <td>&nbsp;</td>
      <td colspan="2">&nbsp;</td>
    </tr>
    <tr>
      <td>&nbsp;</td>
      <td>��ַ3:</td>
      <td>&nbsp;</td>
      <td>&nbsp;</td>
      <td>
	    <input type=submit name=setDns onClick="setServerAddr()" style="height:23px; width:40px;" value="ȷ��" title="submit">
        <input type=submit name=cancelDns style="height:23px; width:40px;" value="ȡ��" title="cancel">
    </td>
    </tr>
	</form>
	
	
	<form action="" name="changePassword" method=POST>
    <tr>
      <td>&nbsp;</td>
      <td colspan="6" class="title2"><u><b>�û���/�����޸�</b></u></td>
    </tr>
    <tr>
      <td>&nbsp;</td>
      <td>�û���:</td>
      <td colspan="2" rowspan="3">
	      <% DisplayUsernamePassword(); %>
	  </td>
      <td>&nbsp;</td>
      <td colspan="2">&nbsp;</td>
    </tr>
	<tr>
      <td>&nbsp;</td>
      <td>ԭ����:</td>
      <td>&nbsp;</td>
      <td colspan="2">&nbsp;</td>
    </tr>	
	<tr>
      <td>&nbsp;</td>
      <td>������:</td>
      <td>&nbsp;</td>
      <td colspan="2">&nbsp;</td>
    </tr>
    <tr>
      <td>&nbsp;</td>
      <td>���ٴ�����������:</td>
      <td>&nbsp;</td>
      <td>&nbsp;</td>
      <td>
	    <input type=submit name=setDns onClick="changePasswordCheck()" style="height:23px; width:40px;" value="ȷ��" title="submit">
        <input type=submit name=cancelDns style="height:23px; width:40px;" value="ȡ��" title="cancel">
      </td>
    </tr>
    </form>
	
	
	
  </table>
<p>&nbsp;</p>
</body>
</html>
