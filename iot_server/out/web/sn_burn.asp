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
        alert("�豸�����Ȳ��Ϸ�");
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
      <td class="title1"><div align="left"><b>���к�����</font></div></td>
    </tr>
  </table>
  <table width="100%" border="0" cellspacing="0" cellpadding="0">
	<form name="IdForm" method="post" action="">
	<tr>
      <td width="2%">&nbsp;</td>
      <td colspan="6" class="title2"><u><b>���к�</b></u></td>
    </tr>
	<tr>
      <td>&nbsp;</td>
      <td width="5%">���к�:</td>
      <td colspan="3">
	    <% DisplayID(); %>
	  </td>
      <td>
	    <input type=submit name=set_dev_name onClick="SetID()" style="height:23px; width:60px;" value="ȷ��" title="submit">
        <input type=submit name=cancel style="height:23px; width:60px;" value="ȡ��" title="cancel">
      </td>
	</tr>
	</form>
	
  </table>
<p>&nbsp;</p>
</body>
</html>
