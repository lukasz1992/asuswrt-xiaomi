<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<html xmlns:v>
<head>
<meta http-equiv="X-UA-Compatible" content="IE=Edge"/>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
<meta HTTP-EQUIV="Pragma" CONTENT="no-cache">
<meta HTTP-EQUIV="Expires" CONTENT="-1">
<link rel="shortcut icon" href="images/favicon.png">
<link rel="icon" href="images/favicon.png">
<style type="text/css">
.Desc {
    color: #000000;
    font-family: Segoe UI,Arial,sans-serif;
    font-size: 22px;
    font-weight: bolder;
    line-height: 40px;
    text-align: left;
    text-shadow: 0 1px 0 white;
}
</style>
<title>ASUS Login</title>
<script type="text/javascript" src="multiLanguage_all.js"></script>
<script type="text/javascript" src="jquery.js"></script>
<script>
var $j = jQuery.noConflict();
var multi_INT = 0;
	var url = "/downloadmaster/dm_apply.cgi";
	var action_mode = "initial";
	var type = "General";
	url += "?action_mode=" + action_mode + "&download_type=" +type+ "&t=" +Math.random();
	$j.ajax({url: url,
			async: false,
			success: function(data){initial_multi_INT_status(data)}
			});
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

function initial(){
	var flag=location.href.split("&ip=")[1];
	if(flag != ""){
		document.getElementById("logined_ip_str").innerHTML = flag;
	}
}
</script>
</head>
<body style="text-align:center;background: #DDD" onunload="return unload_body();" onload="initial();">
<div style="margin-top:100px;width:50%;margin-left:25%">
<div class="Desc"><span id='login_hint1' name='login_hint1'></span><span id="logined_ip_str"></span></div>
<div class="Desc"><span id='login_hint2' name='login_hint2'></span></div>
</div>
<script>
document.getElementById("login_hint1").innerHTML = multiLanguage_all_array[multi_INT][9];
document.getElementById("login_hint2").innerHTML = multiLanguage_all_array[multi_INT][10]
</script>
</body>
</html>
