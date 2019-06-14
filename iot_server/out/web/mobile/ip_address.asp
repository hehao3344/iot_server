<html>
<head>
<title></title>
<meta http-equiv="Pragma" content="no-cache">
<link rel="stylesheet" href="../css/mb.css" type="text/css">
<meta http-equiv="Content-Type" content="text/html; charset=gb2312">

<script language="javascript">
var exp  = /^([1-9]|[1-9]\d|1\d\d|2[0-4]\d|25[0-5])\.(([0-9]|[1-9]\d|1\d\d|2[0-4]\d|25[0-5])\.){2}([1-9]|[1-9]\d|1\d\d|2[0-4]\d|25[0-5])$/;

function GoToDhcpPage()
{
 	var optionValue = document.getElementById("SetAddrMethod").value;
    if (optionValue == "DhcpMethod")
	{
		document.all.ServerIpAddr.disabled= true;
		document.all.NetMask.disabled     = true;
		document.all.DftGateway.disabled  = true;
        BasicIpAddr.BasicIpAddrSet.value  = "确定";
	}
	else
	{
		document.all.ServerIpAddr.disabled= false;
		document.all.NetMask.disabled     = false;
		document.all.DftGateway.disabled  = false;
		BasicIpAddr.BasicIpAddrSet.value  = "提交";
	}

	return true;
}

function GoToWlanDhcpPage()
{
 	var optionValue = document.getElementById("SetWlanAddrMethod").value;
    if (optionValue == "WlanDhcpMethod")
	{
		document.all.WlanIP.disabled= true;
		document.all.WlanNetmask.disabled  = true;
		document.all.WlanGateway.disabled  = true;
        BasicWlanIPAddr.BasicWlanIPAddrSet.value  = "确定";
	}
	else
	{
		document.all.WlanIP.disabled= false;
		document.all.WlanNetmask.disabled  = false;
		document.all.WlanGateway.disabled  = false;
		BasicWlanIPAddr.BasicWlanIPAddrSet.value  = "提交";
	}

	return true;
}

function ConformBasicIpAddr()
{
    // support chrome and ie.
    var ipobj=BasicIpAddr.ServerIpAddr.value;
    var nmobj=BasicIpAddr.NetMask.value;
    var gwobj=BasicIpAddr.DftGateway.value;

    var getIPMethod=BasicIpAddr.BasicIpAddrSet.value;

    if ( "提交" == getIPMethod )
    {
        var nmexp= /(\d+)\.(\d+)\.(\d+)\.(\d+)/g;
        var reg = ipobj.match(exp);
        if ( reg == null )
        {
            alert("IP地址不合法!");
            return false;
        }

        reg = nmobj.match(nmexp);
        if (reg == null)
        {
            alert("子网掩码不合法!");
            return false;
        }

        reg = gwobj.match(exp);
        if (reg == null)
        {
            alert("默认网关地址不合法!");
            return false;
        }
    }
    else
	{
		if (confirm("确定使用DHCP方式获取IP?"))
		{
			BasicIpAddr.action="/goform/PhoneSetBasicIpAddr";
			return true;
		}
	}

	if ( confirm("确定修改?") )
	{
		BasicIpAddr.action="/goform/PhoneSetBasicIpAddr";
		return true;
	}
}

function ConformBasicWlanIPAddr()
{
    var ipobj=BasicWlanIPAddr.WlanIP.value;
    var nmobj=BasicWlanIPAddr.WlanNetmask.value;
    var gwobj=BasicWlanIPAddr.WlanGateway.value;

    var nmexp= /(\d+)\.(\d+)\.(\d+)\.(\d+)/g;

    var getIPMethod=BasicWlanIPAddr.BasicWlanIPAddrSet.value;

    if ( "提交" == getIPMethod )
    {
		var reg = ipobj.match(exp);
		if (reg == null)
		{
			alert("IP地址不合法!");
			return false;
		}

		// netmask
		reg = nmobj.match(nmexp);
		if (reg == null)
		{
			alert("子网掩码不合法!");
			return false;
		}
		// gateway
		reg = gwobj.match(exp);
		if (reg == null)
		{
			alert("默认网关地址不合法!");
			return false;
		}
	}
	else
	{
		if(	BasicWlanIPAddr.BasicWlanIPAddrSet.value == "确定")
		{
			if (confirm("确定使用DHCP方式获取IP?"))
			{
				BasicWlanIPAddr.action="/goform/PhoneSetBasicWlanIPAddr";
				return true;
			}
		}
	}
	if ( confirm("确定修改?") )
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
        alert("首选DNS地址不合法!!!");
        return false;
    }
    // second dns.
    var reg = secobj.match(exp);
    if (reg == null)
    {
        alert("备用DNS地址不合法!!!");
        return false;
    }
    if (confirm("确定修改?"))
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
        alert("端口不合法");
        return false;
    }
    formHttpPort.action="/goform/PhoneSetHttpPort";
}

function SetDevName()
{
    var dev_name = devName.DeviceName.value;
    if ( null == dev_name )
    {
        alert("设备名长度不合法");
        return false;
    }
    devName.action="/goform/SetDeviceName";
}
</script>

</head>
<body onLoad="GoToDhcpPage();GoToWlanDhcpPage();" topmargin="30" leftmargin="0">
  <table width="100%" border="0" cellspacing="0" cellpadding="0">
    <tr>
      <td width="2%">&nbsp;</td>
      <td class="title1"><div align="left"><b>IP地址设置</font></div></td>
    </tr>
  </table>
  <table width="100%" border="0" cellspacing="0" cellpadding="0">
	<tr>
      <td width="2%">&nbsp;</td>
      <td colspan="6"  class="title2"><u><b>IP地址设置</b></u></td>
    </tr>
    <tr>
      <td>&nbsp;</td>
      <td width="30%">IP地址设置方式:	  </td>
        <td colspan="2"><% DisplayMethodOfGetIp(); %></td>
        <td>&nbsp;</td>
        <td colspan="2">&nbsp;</td>
    </tr>
    <form name="BasicIpAddr" method="post" action="">
    <tr>
      <td>&nbsp;</td>
      <td>IP地址:  </td>
      <td colspan="2" rowspan="3"><% DisplayIPAddress(); %></td>
      <td>&nbsp;</td>
      <td colspan="2">&nbsp;</td>
    </tr>
    <tr>
      <td>&nbsp;</td>
      <td>子网掩码:</td>
      <td>&nbsp;</td>
      <td colspan="2">&nbsp;</td>
    </tr>
    <tr>
      <td>&nbsp;</td>
      <td>默认网关:</td>
      <td>&nbsp;</td>
      <td>&nbsp;</td>
      <td>
	    <input type=submit name=BasicIpAddrSet onClick="ConformBasicIpAddr()" style="height:23px; width:40px;" value="提交" title="submit">
        <input type=submit name=BasicIpAddrCancel style="height:23px; width:40px;" value="取消" title="cancel">	  </td>
    </tr>
	</form>
    <tr>
      <td>&nbsp;</td>
      <td colspan="6"  class="title2"><u><b>无线网络IP地址设置</b></u></td>
    </tr>
    <tr>
      <td>&nbsp;</td>
      <td>IP地址设置方式:	  </td>
      <td colspan="2"><% DisplayMethodOfGetWlanIp(); %></td>
      <td>&nbsp;</td>
      <td colspan="2">&nbsp;</td>
    </tr>
    <form name="BasicWlanIPAddr" method="post" action="">
    <tr>
      <td>&nbsp;</td>
      <td>无线网络IP地址:  </td>
      <td colspan="2" rowspan="3"><% DisplayWlanIPAddress(); %></td>
      <td>&nbsp;</td>
      <td colspan="2">&nbsp;</td>
    </tr>
    <tr>
      <td>&nbsp;</td>
      <td>子网掩码:</td>
      <td>&nbsp;</td>
      <td colspan="2">&nbsp;</td>
    </tr>
    <tr>
      <td>&nbsp;</td>
      <td>默认网关:</td>
      <td>&nbsp;</td>
      <td>&nbsp;</td>
      <td>
	    <input type=submit name=BasicWlanIPAddrSet onClick="ConformBasicWlanIPAddr()" style="height:23px; width:40px;" value="提交" title="submit">
        <input type=submit name=BasicWlanIPAddrCancel style="height:23px; width:40px;" value="取消" title="cancel">	  </td>
    </tr>
	</form>
	<form name="dnsIpAddr" method="post" action="">
    <tr>
      <td>&nbsp;</td>
      <td colspan="6" class="title2"><u><b>DNS服务器</b></u></td>
    </tr>
    <tr>
      <td>&nbsp;</td>
      <td>首选DNS:</td>
      <td colspan="2" rowspan="2">
	      <% DisplayDnsAddr(); %>
	  </td>
      <td>&nbsp;</td>
      <td colspan="2">&nbsp;</td>
    </tr>
    <tr>
      <td>&nbsp;</td>
      <td>备用DNS:</td>
      <td>&nbsp;</td>
      <td>&nbsp;</td>
      <td>
	    <input type=submit name=setDns onClick="setDnsAddr()" style="height:23px; width:40px;" value="确定" title="submit">
        <input type=submit name=cancelDns style="height:23px; width:40px;" value="取消" title="cancel">
    </td>
    </tr>
	</form>
	<form name="formHttpPort" method="post" action="">
	<tr>
      <td>&nbsp;</td>
      <td colspan="6" class="title2"><u><b>HTTP服务端口</b></u></td>
    </tr>
	<tr>
      <td>&nbsp;</td>
      <td>端口号:</td>
      <td colspan="2">
	    <% DisplayHttpPort(); %>
	  </td>
      <td>&nbsp;</td>
      <td>&nbsp;</td>
      <td>
	    <input type=submit name=setHttpPort onClick="SaveHttpPort()" style="height:23px; width:40px;" value="确定" title="submit">
        <input type=submit name=cancelDns style="height:23px; width:40px;" value="取消" title="cancel">	  </td>
	</tr>
	</form>
	<form name="devName" method="post" action="">
	<tr>
      <td>&nbsp;</td>
      <td colspan="6" class="title2"><u><b>设备名</b></u></td>
    </tr>
	<tr>
      <td>&nbsp;</td>
      <td>设备名:</td>
      <td colspan="2">
	    <% DisplayDeviceName(); %>
	  </td>
      <td>&nbsp;</td>
      <td>&nbsp;</td>
      <td>
	    <input type=submit name=set_dev_name onClick="SetDevName()" style="height:23px; width:40px;" value="确定" title="submit">
        <input type=submit name=cancel style="height:23px; width:40px;" value="取消" title="cancel">
      </td>
	</tr>
	</form>
  </table>
<p>&nbsp;</p>
</body>
</html>
