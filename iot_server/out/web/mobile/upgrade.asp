<html>
<head>
<title>upgrade reboot</title>
<meta http-equiv="Pragma" content="no-cache">
<link rel="stylesheet" href="../css/mb.css" type="text/css">
<meta http-equiv="Content-Type" content="text/html; charset=gb2312">
<script language="javascript">

function rebootSystem()
{
    if (confirm("ȷ������ϵͳ?"))
	{
		ugrebootFrom.action="/goform/RebootSystem";
	    return true;
    }
}
function recoverDefault()
{
    if (confirm("�ָ�����������ǰ����?"))
	{
		ugrebootFrom.action="/goform/RecoverDefault";
	    return true;
    }
}
</script>

</head>
<body topmargin="40" leftmargin="0">

<table width="600" border="0" cellspacing="0" cellpadding="0">
    <tr>
    <td width="2%"></td>
    <td class="title1"><div align="left"><b>����</font></div></td>
    </tr>
</table>
<table width="600" border="0" cellspacing="0" cellpadding="0">
    <form method='post' enctype='multipart/form-data' name="formUpgrade" action="up_process.asp">
    <tr>
       <td width="2%"></td>
 	   <td colspan="5" class="title2"><u><b>��������</b></u></td>
 	</tr>
    <tr>
        <td width="2%">&nbsp;</td>
        <td>�������:</td>
        <td width="40%"><input type="file" name="filePath"></td>
        <td width="50"><input type="submit" name="submit" value="����"></td>
        <td width="143">&nbsp;</td>
        <td>&nbsp;</td>
    </tr>
    <tr>
        <td width="2%">&nbsp;</td>
        <td>&nbsp;</td>
        <td colspan="3">&nbsp;</td>
        <td>&nbsp;</td>
    </tr>
    </form>
</table>

</body>
</html>

