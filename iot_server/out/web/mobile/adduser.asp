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
		alert("用户名不能为空");
		return false;
	}
	else if( (uname.length < 1) || (uname.length > 31) )
	{
        alert("用户名长度应该为1到31个字符之间");
        return false;
	}
	else if(pwd1 == "")
	{
		alert("密码不能为空");
		return false;
	}
	else if( (pwd1.length < 1) || (pwd1.length > 31) )
	{
		alert("密码长度应该为1到31个字符之间");
		return false;
	}
	else if( pwd1 != pwd2 )
	{
		alert("两次输入的密码不匹配");
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
        <td class="title1"><div align="left"><strong>用户管理</strong></div></td>
    </tr>
</table>

<table width="100%" border="0" cellspacing="0" cellpadding="0">
  <tr>
    <td width="2%">&nbsp;</td>
    <td colspan="5" class="title2"><u><b>添加用户</b></u></td>
  </tr>
  <form action="" name="adduser" method=POST>
  <tr>
    <td width="2%">&nbsp;</td>
    <td width="8%">用户名:</td>
    <td colspan="2">
	    <input type=text name=AddedUserName title="请输入用户名" style="height:20px; width:150px;" size=40 value="">	</td>
    <td>&nbsp;</td>
    <td rowspan="4">&nbsp;</td>
  </tr>
  <tr>
    <td>&nbsp;</td>
    <td>密码:</td>
    <td colspan="2"><input type=password name=AddedUserPassWord size=40 style="height:20px; width:150px;" title="请输入密码" value=""></td>
    <td>&nbsp;</td>
    </tr>
  <tr>
    <td>&nbsp;</td>
    <td>确认密码:</td>
    <td colspan="2">
	    <input type=password name=AddedUserPassWordconf size=40 style="height:20px; width:150px;" title="请再次输入密码" value="">	    </td>
    <td>&nbsp;</td>
    </tr>
  <tr>
    <td>&nbsp;</td>
    <td>&nbsp;</td>
    <td width="2%">&nbsp;</td>
    <td>
	    <input nowrap type=submit name=ok onClick="adduserCheck()" style="height:23px; width:40px;" value="添加" title="点击添加新用户">
        <input nowrap type=submit name=ok style="height:23px; width:40px;" value="取消" title="取消添加">
		<input nowrap type=submit name=ok onClick="goRet()" style="height:23px; width:40px;" value="返回" title="">	</td>
    <td>&nbsp;</td>
    </tr>
  </form>
</table>

</body>
</html>

