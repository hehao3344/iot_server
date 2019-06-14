<html>
<head>
<title>添加设备</title>
<meta http-equiv="Pragma" content="no-cache">
<link rel="stylesheet" href="../css/mb.css" type="text/css">
<meta http-equiv="Content-Type" content="text/html; charset=utf-8">
</head>

<body topmargin="30" leftmargin="0">
  <table width="100%" border="0" cellspacing="0" cellpadding="0">
    <tr>
      <td width="5%">&nbsp;</td>
      <td class="title1"><div align="left"><b>添加设备</font></div></td>
    </tr>
  </table>
  <table width="100%" border="0" cellspacing="0" cellpadding="0">
	<form name="DevAddFrom" method="post" action="/action/dev_add">
    <tr>
      <td width="5%">&nbsp;</td>
      <td width="5%">设备ID:</td>
	  <td width="20%"><input width="200px" type="text" name=devID value="请输入14位设备ID"></td>
      <td></td>
    </tr>
    <tr>
      <td>&nbsp;</td>
      <td>设备名:</td>
	  <td><input width="200px" type="text" name=devName value="请输入设备名"></td>
      <td>
	  	<input type=submit value="添加" title="submit">
	  </td>
    </tr>
	</form>
  </table>
</body>
</html>
