<html>
<head>
<title></title>
<meta http-equiv="Pragma" content="no-cache">
<link rel="stylesheet" href="../css/mb.css" type="text/css">
<meta http-equiv="Content-Type" content="text/html; charset=gb2312">

<script language="javascript">
function adduserCheck()
{
	var uname   =   adduser.AddedUserName.value;
	var pwd1    =   adduser.AddedUserPassWord.value;
	var pwd2    =   adduser.AddedUserPassWordconf.value;
	if(uname == "")
	{
		alert("�û�������Ϊ��");
		return false;
	}
	else if( (uname.length < 1) || (uname.length > 31) )
	{
        alert("�û�������Ӧ��Ϊ1��31���ַ�֮��");
        return false;
	}
	else if(pwd1 == "")
	{
		alert("���벻��Ϊ��");
		return false;
	}
	else if( (pwd1.length < 1) || (pwd1.length > 31) )
	{
		alert("���볤��Ӧ��Ϊ1��31���ַ�֮��");
		return false;
	}
	else if( pwd1 != pwd2 )
	{
		alert("������������벻ƥ��");
		return false;
	}
	else
	{
		adduser.action="/goform/PhoneAddUser";
	}
	return true;
}

function goRet()
{
	adduser.action="/mobile/usermanage.asp";
}

</script></head>

<body topmargin="30" leftmargin="0">

<table width="100%" border="0" cellspacing="0" cellpadding="0">
    <tr>
        <td width="2%"></td>
        <td class="title1"><div align="left"><strong>�û�����</strong></div></td>
    </tr>
</table>

<table width="100%" border="0" cellspacing="0" cellpadding="0">
  <tr>
    <td width="2%">&nbsp;</td>
    <td colspan="5" class="title2"><u><b>����û�</b></u></td>
  </tr>
  <form action="" name="adduser" method=POST>
  <tr>
    <td width="2%">&nbsp;</td>
    <td width="8%">�û���:</td>
    <td colspan="2">
	    <input type=text name=AddedUserName title="�������û���" style="height:20px; width:150px;" size=40 value="">	</td>
    <td>&nbsp;</td>
    <td rowspan="4">&nbsp;</td>
  </tr>
  <tr>
    <td>&nbsp;</td>
    <td>����:</td>
    <td colspan="2"><input type=password name=AddedUserPassWord size=40 style="height:20px; width:150px;" title="����������" value=""></td>
    <td>&nbsp;</td>
    </tr>
  <tr>
    <td>&nbsp;</td>
    <td>ȷ������:</td>
    <td colspan="2">
	    <input type=password name=AddedUserPassWordconf size=40 style="height:20px; width:150px;" title="���ٴ���������" value="">	    </td>
    <td>&nbsp;</td>
    </tr>
  <tr>
    <td>&nbsp;</td>
    <td>&nbsp;</td>
    <td width="2%">&nbsp;</td>
    <td>
	    <input nowrap type=submit name=ok onClick="adduserCheck()" style="height:23px; width:40px;" value="���" title="���������û�">
        <input nowrap type=submit name=ok style="height:23px; width:40px;" value="ȡ��" title="ȡ�����">
		<input nowrap type=submit name=ok onClick="goRet()" style="height:23px; width:40px;" value="����" title="">	</td>
    <td>&nbsp;</td>
    </tr>
  </form>
</table>

</body>
</html>

