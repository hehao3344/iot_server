<html>
<head>
<title></title>
<meta http-equiv="Pragma" content="no-cache">
<link rel="stylesheet" href="../css/mb.css" type="text/css">
<meta http-equiv="Content-Type" content="text/html; charset=gb2312">
<script language="javascript">

function modifyWlanPassword()
{
    var essid = wlanInfo.Essid.value;
    if ( essid == ""  )
    {
        alert("请选择");
        return false;
    }
	wlanInfo.action="/goform/PhoneChangeWlanPassword";
	return true;
}

</script>

</head>
<body topmargin="30" leftmargin="0">

  <table width="100%" border="0" cellspacing="0" cellpadding="0">
    <tr>
      <td width="2%"></td>
      <td class="title1"><b>无线网络管理</b></td>
    </tr>
  </table>

<table width="100%" border="0" cellspacing="0" cellpadding="0">
  <tr>
    <td width="2%">&nbsp;</td>
    <td colspan="2" class="title2"  width="30%"><b>无线网络</b></td>
	<td class="title2">&nbsp;</td>
  </tr>
  <tr>
    <td>&nbsp;</td>
    <td width="30%">
	<table width="100%" border="0" cellspacing="0" cellpadding="0">
      <form action="" name="wlanInfo" method=POST>
        <tr>
          <td colspan="3"><% DisplayWlanInfo(); %></td>
		  <td>&nbsp;</td>
        </tr>
        <tr>
          <td>
		      <div align="right">
				  <input nowrap type=submit name=modifyPassword onClick="modifyWlanPassword()" style="height:25px; width:80px;" value="修改密码" title="">
				  <input nowrap type=submit name=flushWlan onClick="/advanced/wlan.asp" style="height:25px; width:80px;" value="刷新" title="">
			  </div>			</td>
			<td width="10%">&nbsp;</td>
			<td width="60%">&nbsp;</td>
			<td>&nbsp;</td>
        </tr>
      </form>
    </table>
	</td>
    <td width="8%">&nbsp;</td>
	<td>&nbsp;</td>
  </tr>
</table>

</body>
</html>

