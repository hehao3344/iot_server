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
		ugrebootFrom.action="/goform/PhoneRebootSystem";
	    return true;
    }
}
function recoverDefault()
{
    if (confirm("�ָ�����������ǰ����?"))
	{
		ugrebootFrom.action="/goform/PhoneRecoverDefault";
	    return true;
    }
}
</script>

</head>
<body topmargin="30" leftmargin="0">
    <table width="100%" border="0" cellspacing="0" cellpadding="0">
        <tr>
            <td width="2%"></td>
            <td class="title1"><div align="left"><b>����&amp;����</b></div></td>
        </tr>
    </table>

    <table width="100%" border="0" cellspacing="0" cellpadding="0">
    <tr>
        <td width="2%">&nbsp;</td>
        <td colspan="3" class="title2"><u><b>����&amp;�ָ�ȱʡ����</b></u></td>
    </tr>
    <form name="ugrebootFrom" method="POST" action="">
        <tr>
            <td width="2%">&nbsp;</td>
            <td>
                <input nowrap type=submit name=smReboot onClick="rebootSystem()" style="height:23px; width:40px;" value="����" title="">
            </td>
            <td>����ϵͳ</td>
            <td>&nbsp;</td>
        </tr>
        <tr>
            <td>&nbsp;</td>
            <td>
                <input nowrap type=submit name=smReDefault onClick="recoverDefault()" style="height:23px; width:40px;" value="ȱʡ" title="">
            </td>
            <td>�ָ��������õ���������</td>
            <td>&nbsp;</td>
        </tr>
    </form>
    </table>

</body>
</html>

