<html>
<head>
<title></title>
<meta http-equiv="Pragma" content="no-cache">
<link rel="stylesheet" href="../css/mb.css" type="text/css">
<meta http-equiv="Content-Type" content="text/html; charset=gb2312">

<script language="javascript">

function GotoSetAPPassword()
{
  	var optionValue = document.getElementById("WifiAPEnc").value;
    if ( optionValue == "share" )
	{
		document.all.WifiAPPassword.disabled= true;
	}
	else
	{		
		document.all.WifiAPPassword.disabled= false;
	}
}

function WifiModeChanged()
{
 	var mode = document.getElementById("WifiModeSelect").value;
    if ( mode == "WifiAPMode")
	{
	    WifiAPParamForm.WlanAPSSID.disabled = false;
		WifiAPParamForm.WifiAPEnc.disabled = false;
		WifiAPParamForm.WifiAPPassword.disabled = false;
		
		WifiEssidPasswordForm.authModeSelect.disabled = true;
		WifiEssidPasswordForm.WifiEssid.disabled = true;
		WifiEssidPasswordForm.WifiPassword.disabled = true;

		WifiAPParamForm.smWifiAP.disabled = false;
		WifiEssidPasswordForm.smWifiClient.disabled = true;		
	}
	else
	{
	    WifiAPParamForm.WlanAPSSID.disabled = true;
		WifiAPParamForm.WifiAPEnc.disabled = true;
		WifiAPParamForm.WifiAPPassword.disabled = true;

		WifiEssidPasswordForm.authModeSelect.disabled = false;
		WifiEssidPasswordForm.WifiEssid.disabled = false;
		WifiEssidPasswordForm.WifiPassword.disabled = false;
		
		WifiAPParamForm.smWifiAP.disabled = true;
		WifiEssidPasswordForm.smWifiClient.disabled = false;	
	}

	return true;
}

function SetWifiMode()
{
    WifiModeForm.action="/goform/PhoneSetWifiMode";
}

function SetWifiAPParam()
{
    WifiAPParamForm.action="/goform/PhoneSetWifiAPParam";
}

function SetWifiEssidPassword()
{
    WifiEssidPasswordForm.action="/goform/PhoneSetWifiEssidPassword";
}


</script>

</head>
<body onLoad="WifiModeChanged()" topmargin="30" leftmargin="0">
    <table width="100%" border="0" cellspacing="0" cellpadding="0">
        <tr>
            <td width="2%">&nbsp;</td>
            <td width="%" class="title1">
                <div align="left"><b>Wifi����ģʽ����</b></div>
            </td>
        </tr>
    </table>
    <table width="100%" border="0" cellspacing="0" cellpadding="0">
        <tr>
            <td width="2%">&nbsp;</td>
            <td colspan="4" class="title2"><u><b>Wifi����ģʽ</b></u></td>
        </tr>
        <form name="WifiModeForm" method="post" action="">
        <tr>
            <td>&nbsp;</td>
            <td width="30%">����ģʽ:</td>
            <td><% DisplayWlanWorkMode(); %></td>
            <td>
              <input type=submit name=smWifiMode onClick="SetWifiMode()" style="height:23px; width:40px;" value="ȷ��" title="submit">
              <input type=submit name=clWifiMode style="height:23px; width:40px;" value="ȡ��" title="cancel">            </td>
            <td>&nbsp;</td>
        </tr>
        </form>
        <tr>
            <td>&nbsp;</td>
            <td colspan="4" class="title2"><u><b>SSID����(APģʽ����Ч)</b></u></td>
        </tr>
	    <form name="WifiAPParamForm" method="post" action="">
        <tr>
            <td>&nbsp;</td>
            <td>SSID:</td>
            <td rowspan="3"><% DisplayWlanAPParam(); %></td>
            <td colspan="2" height="23">&nbsp;</td>
          </tr>
        <tr>
          <td>&nbsp;</td>
          <td>���ܷ�ʽ:</td>
          <td colspan="2" height="23">&nbsp;</td>
          </tr>
        <tr>
          <td>&nbsp;</td>
          <td>����:</td>
          <td>
  		    <input type=submit name=smWifiAP id=smWifiAP onClick="SetWifiAPParam()" style="height:23px; width:40px;" value="ȷ��" title="submit">
            <input type=submit name=clWifiAP style="height:23px; width:40px;" value="ȡ��" title="cancel">		  </td>
          <td>&nbsp;</td>
        </tr>
        </form>
        <tr>
            <td>&nbsp;</td>
            <td colspan="4" class="title2"><u><b>Essid�š���������(Clientģʽ����Ч)</b></u></td>
        </tr>
        <form name="WifiEssidPasswordForm" method="post" action="">
        <tr>
            <td>&nbsp;</td>
            <td>���ܷ�ʽ:</td>
            <td rowspan="3"><% DisplayWlanEssidPassword(); %></td>
            <td colspan="2" height="23">&nbsp;</td>
          </tr>
        <tr>
          <td>&nbsp;</td>
          <td>Essid:</td>
          <td colspan="2" height="23">&nbsp;</td>
          </tr>
        <tr>
          <td>&nbsp;</td>
          <td>����:</td>
          <td>
  		    <input type=submit name=smWifiClient id=smWifiClient onClick="SetWifiEssidPassword()" style="height:23px; width:40px;" value="ȷ��" title="submit">
            <input type=submit name=clWifiClient style="height:23px; width:40px;" value="ȡ��" title="cancel">		  </td>
          <td>&nbsp;</td>
        </tr>
        </form>
    </table>
</body>
</html>
