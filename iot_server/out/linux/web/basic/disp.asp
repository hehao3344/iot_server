<html>
<head>
<title></title>
<meta http-equiv="Pragma" content="no-cache">
<link rel="stylesheet" href="../css/mb.css" type="text/css">
<meta http-equiv="Content-Type" content="text/html; charset=utf-8">

<script language="javascript">

function confirmDelDev() {
    var result = confirm("确定删除?");
    if (result == true) {
    } else {
        return false;
    }
    delDevForm.action = "/action/dev_list_del";
}
</script>

</head>

<body topmargin="30" leftmargin="0" onload=loadXMLDoc()>

<table width="100%" border="0" cellspacing="0" cellpadding="0">
    <tr>
        <td width="2%"></td>
        <td class="title1"><b>设备状态管理</b></td>
    </tr>
</table>

<table width="100%" border="0" cellspacing="0" cellpadding="0">
    <tr>
        <td width="5%"></td>
        <td width="5%">序号</td>
        <td width="10%">设备ID</td>
        <td width="10%">OpenID</td>
        <td width="10%">设备名</td>
		<td width="10%">product_key</td>
		<td width="10%">dev_secret</td>		
        <td width="10%">上线时间</td>
        <td width="10%">离线时间</td>
        <td width="8%">状态</td>
    </tr>
</table>

<table width="100%" border="0" cellspacing="0" cellpadding="0">
    <% dev_list(); %>
</table>
</body>
</html>
