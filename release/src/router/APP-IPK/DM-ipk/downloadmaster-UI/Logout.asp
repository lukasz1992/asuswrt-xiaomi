<html>
<head>
<title></title>
<meta http-equiv="Content-Type" content="text/html; charset=UTF-8">
<meta HTTP-EQUIV="Pragma" CONTENT="no-cache">
<meta HTTP-EQUIV="Expires" CONTENT="-1">
<link rel="shortcut icon" href="images/favicon.png">
<link rel="icon" href="images/favicon.png">
<link href="other.css"  rel="stylesheet" type="text/css">
<style type="text/css">
body {
	background: #CCC;
	margin:50px auto;
}
.erTable {
	background: #000;
	}
</style>
<script type="text/javascript" src="jquery.js"></script>
<script type="text/javascript" src="jquery-scrolltotop.min.js"></script>
<script>
var httpTag = 'https:' == document.location.protocol ? false : true;
var $j = jQuery.noConflict();
var multi_INT = 0;
	var url = "dm_apply.cgi";
	var action_mode = "initial";
	var type = "General";
	url += "?action_mode=" + action_mode + "&download_type=" +type+ "&t=" +Math.random();
	$j.ajax({url: url,
			async: false,
			success: function(data){initial_multi_INT_status(data)},
			error: function(XMLHttpRequest, textStatus, errorThrown){
			if(XMLHttpRequest.status==598){
				if(httpTag)
					self.location = "http://"+ location.host +"/Main_Login.asp";
				else
					self.location = "https://"+ location.host +"/Main_Login.asp";
			}}});
function initial_multi_INT_status(data){
	var array = new Array();
	eval("array="+data);
	var lang = array[14];
	if(lang == "EN")
	multi_INT = 0;
	else if(lang == "TW")
	multi_INT = 1;
	else if(lang == "CN")
	multi_INT = 2;
	else if(lang == "RU")
	multi_INT = 3;
	else if(lang == "FR")
	multi_INT = 4;
	else if(lang == "DE")
	multi_INT = 5;
	else if(lang == "BR")
	multi_INT = 6;
	else if(lang == "CZ")
	multi_INT = 7;
	else if(lang == "DA")
	multi_INT = 8;
	else if(lang == "FI")
	multi_INT = 9;
	else if(lang == "MS")
	multi_INT = 10;
	else if(lang == "NO")
	multi_INT = 11;
	else if(lang == "PL")
	multi_INT = 12;
	else if(lang == "SV")
	multi_INT = 13;
	else if(lang == "TH")
	multi_INT = 14;
	else if(lang == "TR")
	multi_INT = 15;
	else if(lang == "JP")
	multi_INT = 16;
	else if(lang == "IT")
	multi_INT = 17;
	else if(lang == "HU")
	multi_INT = 18;
	else if(lang == "RO")
	multi_INT = 19;
	else if(lang == "UK")
	multi_INT = 20;
	else if(lang == "ES")
	multi_INT = 21;
	else
	multi_INT = 0;
	}
</script>
<script type="text/javascript" src="multiLanguage_all.js"></script>
</head>

<body>
<table width="500" border="0" align="center" cellpadding="10" cellspacing="0" class="erTable">
  <tr>
    <th align="left" valign="top" background="images/er_bg.gif">
		<div class="drword"><span id="logout_des"></span>
			<!--You have Logout successfully.-->
			<div style="float:right;">
				<!--<a href="javascript:window.close();">If you want to login again, please close this window first.</a>-->
				<!--a href="/index.asp"><#Not_authpage_re_login#></a-->
			</div>
			<br/><br/>
		</div>
		<div class="drImg"><img src="images/alertImg.gif"></div>		
		<div style="height:70px; "></div>
	  	</th>
  </tr>		
</table>
<script>
$j("#logout_des").html(multiLanguage_all_array[multi_INT][8]);
</script>
</body>
</html>
