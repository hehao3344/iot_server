<html>
<head>
<title></title>
<meta http-equiv="Pragma" content="no-cache">
<link rel="stylesheet" href="../css/mb.css" type="text/css">
<meta http-equiv="Content-Type" content="text/html; charset=gb2312">

<script language="javascript">

function ChangeWlanPasswordBtn()
{
	var pwd1    =   WlanPasswordForm.WlanPassword.value;		
	var pwd2    =   WlanPasswordForm.WlanPasswordConfirm.value;		
	if(pwd1 == "")
	{
		alert("���벻��Ϊ��");
		return false;
	}
	else if( (pwd1.length < 1) || (pwd1.length > 31) )
	{
		alert("���볤�ȷǷ�");
		return false;
	}
	else if( pwd1 != pwd2 )
	{
		alert("������������벻ƥ��");
		return false;
	}
	else
	{	
		WlanPasswordForm.action="/goform/PhoneChangeWlanPassword";
	}
	
	return true;
}

function goToWlanPageBtn()
{	
	alert("������������벻ƥ��");
	WlanPasswordForm.action="/advanced/wlan.asp";
}

</script>
</head>

<body topmargin="30" leftmargin="0">
<table width="100%" border="0" cellspacing="0" cellpadding="0">
<tr>
  <td width="5%"></td>
  <td class="title1"><b>��������</b></td>
</tr>
</table>
  
<table width="100%" border="0" cellspacing="0" cellpadding="0">
  <form action="" name="WlanPasswordForm" method=POST>
  <tr>
    <td width="5%">&nbsp;</td>
    <td colspan="6" class="title2"><u><b>����������������</b></u></td>
  </tr>
  
  <tr>
    <td>&nbsp;</td>
    <td>Essid:</td>
    <td colspan="3">
	  <input type=password name=WlanPassword size=40 style="height:20px; width:150px;" title="����������" value="">	</td>
    <td>&nbsp;</td>
  </tr>
    
  <tr>
    <td>&nbsp;</td>
    <td class="item3">����:</td>
    <td colspan="3">
	  <input type=password name=WlanPassword size=40 style="height:20px; width:150px;" title="����������" value="">	</td>
    <td>&nbsp;</td>
  </tr>
  <tr>
    <td>&nbsp;</td>
    <td class="item3">ȷ������:</td>
    <td colspan="3">
	    <input type=password name=WlanPasswordConfirm size=40 style="height:20px; width:150px;" title="���ٴ���������" value="">	    </td>
    <td>&nbsp;</td>
  </tr>  
  <tr>
    <td>&nbsp;</td>
    <td>&nbsp;</td>
    <td width="1%">&nbsp;</td>
    <td width="20%">
	  <input nowrap type=submit name=ok onClick="ChangeWlanPasswordBtn()" style="height:23px; width:40px;" value="����" title="">
      <input nowrap type=submit name=ok style="height:23px; width:40px;" value="ȡ��" title="">	
  	  <input nowrap type=submit name=ok onClick="goToWlanPageBtn()" style="height:23px; width:40px;" value="����" title="">	</td>
    <td>&nbsp;</td>
    <td>&nbsp;</td>
  </tr>
  </form>
</table>

</body>
</html>
