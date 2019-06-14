<html>
<head>
<title></title>
<meta http-equiv="Pragma" content="no-cache">
<link rel="stylesheet" href="../css/mb.css" type="text/css">
<meta http-equiv="Content-Type" content="text/html; charset=gb2312">
<script language="javascript">
function adduserCheck()
{
    addDelUser.action="/mobile/adduser.asp";
}
function deluserCheck()
{
    var delUser = addDelUser.user.value;
    if (delUser == "")
    {
        alert("请选择要删除的用户");
        return false;
    }
    else if (delUser == "admin")
	{
    	alert("不能删除超级用户");
        return false;
    }
    else if (confirm("确定删除用户: " + delUser + "?"))
	{
    	addDelUser.action="/goform/PhoneDeleteUser";
    	return true;
    }
}

function changepwdCheck()
{
    var cpwdUser = addDelUser.user.value;
    if (cpwdUser == "")
    {
        alert("请选择用户");
        return false;
	}
	else
	{
	    addDelUser.action="/goform/PhoneChangePassword";
	    return true;
	}
}
</script>

</head>
<body topmargin="30" leftmargin="0">

  <table width="100%" border="0" cellspacing="0" cellpadding="0">
    <tr>
      <td width="2%"></td>
      <td class="title1"><div align="left"><b>用户管理</b></div></td>
    </tr>
  </table>

<table width="100%" border="0" cellspacing="0" cellpadding="0">
  <tr>
    <td width="2%">&nbsp;</td>
    <td colspan="4" class="title2"><b>添加/删除/修改用户</b></td>
  </tr>
  <tr>
    <td>&nbsp;</td>
    <td width="50%">
	<table width="20%" border="0" cellspacing="0" cellpadding="0">
      <form action="" name="addDelUser" method=POST>
        <tr>
          <td colspan="3"><% MakeUserList(); %></td>
        </tr>
        <tr>
          <td><input nowrap type=submit name=adduser onClick="adduserCheck()" style="height:25px; width:90px;" value="添加用户" title=""></td>
          <td><input nowrap type=submit name=deluser onClick="deluserCheck()" style="height:25px; width:100x;" value="删除用户" title=""></td>
          <td><input nowrap type=submit name=changepwd onClick="changepwdCheck()" style="height:25px; width:90px;" value="修改密码" title=""></td>
        </tr>
      </form>
    </table>
	</td>
    <td>&nbsp;</td>
  </tr>
</table>

</body>
</html>

