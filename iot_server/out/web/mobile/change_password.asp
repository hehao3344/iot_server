<html>
<!- Copyright (c) Go Ahead Software Inc., 2000-2000. All Rights Reserved. ->
<head>
<title>change password</title>
<meta http-equiv="Pragma" content="no-cache">
<link rel="stylesheet" href="style/normal_ws.css" type="text/css">
<style type="text/css">
<!--
body {
	background-color: #FFFFFF;
}
.style3 {font-size: 12px}
-->
</style>
<meta http-equiv="Content-Type" content="text/html; charset=gb2312">


<script language="javascript">
function changePasswordCheck()
{
	var pwone = changePassword.Passwordone.value;		
	var pwtwo = changePassword.Passwordtwo.value;
	if(pwone == "")
	{
		alert("���벻��Ϊ��");
		return false;
	}
	else if(pwone != pwtwo)
	{
		alert("������������벻һ��,����������");
		return false;
	}
	else
	{	
		adduser.action="/goform/AddUser";		
		 
	}
	return true;
}
</script>
</head>
<body topmargin="40" leftmargin="0">

  <table width="800" border="0" cellspacing="0" cellpadding="0">
    <tr>
      <td width="15"></td>
      <td width="180"><div align="left"><strong>�޸�����</strong></div></td>
      <td width="160"><div align="center"><h2>&nbsp;</h2></div></td>
      <td width="220"></td>
      <td width="90">&nbsp;</td>
    </tr>
  </table>

<hr>
<table width="800" border="0" cellspacing="0" cellpadding="0">
  <tr>
    <td width="20">&nbsp;</td>
    <td width="240">&nbsp;</td>
    <td width="160">&nbsp;</td>
    <td width="220">&nbsp;</td>
    <td width="90">&nbsp;</td>
  </tr>
  <tr>
    <td>&nbsp;</td>
    <td><table width="240">
      <form action="" name="changePassword" method=POST>
        <tr>
          <td width="91" height="22">
            <div align="right" class="style3">�û���:</div></td>
          <td width="150">&nbsp;</td>
        </tr>
        <tr>
          <td>
            <div align="right" class="style3">�� ��:</div></td>
          <td>
            <input type=password name=Passwordone size=40 style="height:20px; width:150px;" title="����������" value="">
          </td>
        </tr>
        <tr>
          <td height="26">
            <div align="right" class="style3">ȷ������:</div></td>
          <td>
            <input type=password name=Passwordtwo size=40 style="height:20px; width:150px;" title="���ٴ���������" value="">
          </td>
        </tr>
        <tr>
          <td height="35" colspan="2">
            <div align="right">
              <input type=submit name=ok onClick="changePasswordCheck()" style="height:25px; width:50px;" value="ȷ��" title="���ȷ��">
              <input type=submit name=ok style="height:25px; width:50px;" value="ȡ��" title="ȡ��">
            </div>
		  </td>
        </tr>
      </form>
    </table></td>
    <td>&nbsp;</td>
    <td>&nbsp;</td>
    <td>&nbsp;</td>
  </tr>
</table>


</body>
</html>

