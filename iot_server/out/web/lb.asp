<html>
<head>
<link rel="stylesheet" href="css/frame.css" type="text/css">
<meta http-equiv="Content-Type" content="text/html; charset=gb2312">
<title>leftbottom</title>
<script language="javascript">

document.oncontextmenu=new Function("event.returnValue=false");
document.onselectstart=new Function("event.returnValue=false");

function winHide(tableId, parentId)
{
	tbId = document.getElementById(tableId.toString());
	ptId = document.getElementById(parentId.toString());
	window.focus();
	if (tbId.style.display=='none')
	{
		tbId.style.display='block'
	}
	else
	{
		tbId.style.display='none'
	}
	ptId.style.color="#ffffff"
}
</script>
</head>

<body topmargin="0" leftmargin="0">

<table width="180" height="100%" border="0" cellpadding="0" cellspacing="0">
  <form action="" name="cmdCtl" method=POST target="hiddenArea">
  <tr>
    <td colspan="5" height="100"></td>
  </tr>
  <tr>
    <td height="40" colspan="5" style="BORDER-BOTTOM: #000000 0px solid; BORDER-LEFT: #000000 0px solid; BORDER-RIGHT: #000000 0px solid; BORDER-TOP: #000000 0px solid">
      <table width="180" height="40" border="0" cellpadding="0" cellspacing="0" bordercolor="#FFFFFF">
        <tr>
          <td height="20" align="left" id="cameraSet" style="cursor:pointer;" onClick="winHide('cameraSetTb','cameraSet');" name="cameraSet" target="mainFrame" onMouseOver="this.style.color='#d9d919';" onMouseOut="this.style.color='#ffffff';">            <span class="topTitle">
            <div style="margin-top:0px; margin-bottom:0px; margin-left:36px"><img src="images/drop_menu.gif">基本配置</div>
            </span>
          </td>
        </tr>
        <tr>
          <td colspan="0">
            <div id=cameraSetTb style="display: none; margin-top:0px; margin-bottom:0px; margin-left:45px" align="left">
			  <span style="font-size: 12px"><img src="images/drop_down_menu.gif"><a href="/basic/disp.asp" target="mainFrame">添加/删除</a></span><br>
			</td>
        </tr>
        <tr>
          <td colspan="0"></td>
        </tr>
        <tr>
          <td>&nbsp;</td>
        </tr>
        <tr>
          <td colspan="4"></td>
        </tr>
    </table></td>
  </tr>
  <tr>
	<td height="400" colspan="5" style="BORDER-BOTTOM: #000000 0px solid; BORDER-LEFT: #000000 0px solid; BORDER-RIGHT: #000000 0px solid; BORDER-TOP: #000000 0px solid">&nbsp;</td>
  </tr>
  </form>
</table>
<iframe name="hiddenArea" width="0%" height="0%">Your IE did not support iframe</iframe>
</body>
</html>
