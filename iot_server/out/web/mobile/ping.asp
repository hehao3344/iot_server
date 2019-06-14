<html>
<head>
<title>PING</title>
<meta http-equiv="Pragma" content="no-cache">
<link rel="stylesheet" href="../css/normal.css" type="text/css">
<meta http-equiv="Content-Type" content="text/html; charset=gb2312">

<script language="javascript">

function StartPing()
{
 	var pingAddress=document.getElementById("textPingAddress").value;
    if ( pingAddress == null ) 
    { 
        alert("IP/Url不合法"); 
        return false;
    }     
	formPingAddress.action="/goform/PingAddress";		
	
	return true;	
}

</script>

</head>
<body topmargin="20" leftmargin="0">
  <table width="600" border="0" cellspacing="0" cellpadding="0">
    <tr>
      <td width="25">&nbsp;</td>
          <td width="181" height="20">
		  <div align="left"><b><font color="#004080">网络诊断</font></b></div>
	  </td>
      <td width="133"></td>
      <td width="182"></td>
      <td width="79">&nbsp;</td>
    </tr>
  </table>
  <table width="600" border="0" cellspacing="0" cellpadding="0">
	<tr>
      <td width="21">&nbsp;</td>
      <td colspan="4" class="title2"><u><b>网络诊断参数设置</b></u></td>
    </tr>
    <form name="formPingAddress" method="post" action="">
    <tr>
      <td>&nbsp;</td>
      <td width="145" class="item2">IP地址或域名:  </td>
      <td colspan="2"><input type=text name=textPingAddress value="192.168.1."></td>
      <td width="147"><input type=submit name=pingAddress onClick="StartPing()" style="height:23px; width:60px;" value="开始诊断" title="submit"></td>
      </tr>
    <tr>
      <td>&nbsp;</td>
      <td colspan="4" class="title2"><u><b></b></u></td>
    </tr>
	</form>	
  </table>
</body>
</html>
