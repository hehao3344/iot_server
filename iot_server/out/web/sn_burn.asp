<html>
<head>
<title></title>
<meta http-equiv="Pragma" content="no-cache">
<link rel="stylesheet" href="../css/mb.css" type="text/css">
<meta http-equiv="Content-Type" content="text/html; charset=gb2312">

<script language="javascript">

function SetID()
{
    var id = IdForm.id.value;
    if ( null == id )
    {
        alert("设备名长度不合法");
        return false;
    }
	
    IdForm.action="/goform/SetID";
}


</script>

</head>
<body topmargin="30" leftmargin="0">
  <table width="100%" border="0" cellspacing="0" cellpadding="0">
    <tr>
      <td width="2%">&nbsp;</td>
      <td class="title1"><div align="left"><b>序列号设置</font></div></td>
    </tr>
  </table>
  <table width="100%" border="0" cellspacing="0" cellpadding="0">
	<form name="IdForm" method="post" action="">
	<tr>
      <td width="2%">&nbsp;</td>
      <td colspan="6" class="title2"><u><b>序列号</b></u></td>
    </tr>
	<tr>
      <td>&nbsp;</td>
      <td width="5%">序列号:</td>
      <td colspan="3">
	    <% DisplayID(); %>
	  </td>
      <td>
	    <input type=submit name=set_dev_name onClick="SetID()" style="height:23px; width:60px;" value="确定" title="submit">
        <input type=submit name=cancel style="height:23px; width:60px;" value="取消" title="cancel">
      </td>
	</tr>
	</form>
	
  </table>
<p>&nbsp;</p>
</body>
</html>
