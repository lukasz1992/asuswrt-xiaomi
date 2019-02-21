<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<head>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
<meta HTTP-EQUIV="Pragma" CONTENT="no-cache">
<meta HTTP-EQUIV="Cache-Control" CONTENT="no-cache">
<meta HTTP-EQUIV="Expires" CONTENT="-1">
<link rel="shortcut icon" href="images/favicon.png">
<link rel="icon" href="images/favicon.png">
<title>BT Setting</title>
<link rel="stylesheet" type="text/css" href="index_style.css"> 
<link rel="stylesheet" type="text/css" href="form_style.css">
<!--<link rel="stylesheet" type="text/css" href="usp_style.css">-->
<link href="other.css"  rel="stylesheet" type="text/css">
<link rel="stylesheet" type="text/css" href="routercss.css">
<link rel="stylesheet" type="text/css" href="ext/css/ext-all.css">
<!--<link href="multiLanguageCss/english.css" rel="stylesheet" type="text/css" id="languageCss" />
<link href="multiLanguageCss/english_1.css" rel="stylesheet" type="text/css" id="languageCss_1" />-->
<script type="text/javascript" src="jquery.js?ver=104"></script>
<script>
var httpTag = 'https:' == document.location.protocol ? false : true;
var $j = jQuery.noConflict();
var multi_INT = 0;
var generalbt_data_tmp;
var AUTOLOGOUT_MAX_MINUTE_TMP=0;
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
	generalbt_data_tmp=data;
	AUTOLOGOUT_MAX_MINUTE_TMP=array[30];
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
<script type="text/javascript" src="multiLanguage_setting.js?ver=104"></script>
<script type="text/javascript" src="multiLanguage_task.js?ver=104"></script>
<script type="text/javascript" src="multiLanguage_all.js?ver=104"></script>
<script type="text/javascript" src="state.js?ver=104"></script>
<!--<script type="text/javascript" src="help.js?ver=104"></script>-->
<script type="text/javascript" src="general.js?ver=104"></script>
<script type="text/javascript" src="popup.js?ver=104"></script>
<!-- delete by alan <script type="text/javascript" src="md5.js?ver=104"></script>-->
<script type="text/javascript" src="ext/ext-base.js?ver=104"></script>
<script type="text/javascript" src="ext/ext-all.js?ver=104"></script>
<script type="text/javascript" src="detect.js?ver=104"></script>
<script type="text/javascript" src="plugin/jquery.iphone-switch.js?ver=104"></script>
<script type="text/javascript" src="Setting.js?ver=104"></script>
<style type="text/css">
a:focus{outline:none;}
</style>
<script>
function initial(){
	show_menu();
	initial_bt();
	//document.getElementById("select_lang_"+multi_INT).selected = true;
	get_Refresh_time(generalbt_data_tmp);
	AUTOLOGOUT_MAX_MINUTE=AUTOLOGOUT_MAX_MINUTE_TMP;
	updateStatus_AJAX();
}
function get_Refresh_time(data){
	/*var url = "dm_apply.cgi";
	var action_mode = "initial";
	var type = "General";
	url += "?action_mode=" + action_mode + "&download_type=" +type+ "&t=" +Math.random();
	$j.ajax({
			url:url,
			success:function(data){initial_Refresh_time(data)},
			error:function(XMLHttpRequest, textStatus, errorThrown){error_Refresh_time()}
			});*/
	var initial_array = new Array();
	eval("initial_array="+data);
if(httpTag) {
	document.getElementById("helpAddress").href = "http://"+ location.host +"/downloadmaster/help.asp";
		if(location.host.split(":")[0] != initial_array[10]){
			if(document.getElementById("handToPhone"))
			document.getElementById("handToPhone").href = "http://"+ location.host.split(":")[0] +":" + initial_array[13]+"/downloadmaster/task_hand.asp";
			}
		else{
			if(document.getElementById("handToPhone"))
				document.getElementById("handToPhone").href = "http://"+ location.host +"/downloadmaster/task_hand.asp";
			}
} else {
		document.getElementById("helpAddress").href = "https://"+ location.host +"/downloadmaster/help.asp";
		if(location.host.split(":")[0] != initial_array[10]){
			if(document.getElementById("handToPhone"))
			document.getElementById("handToPhone").href = "https://"+ location.host.split(":")[0] +":" + initial_array[34]+"/downloadmaster/task_hand.asp";
			}
		else{
			if(document.getElementById("handToPhone"))
				document.getElementById("handToPhone").href = "https://"+ location.host +"/downloadmaster/task_hand.asp";
}
}
}

function initial_Refresh_time(data){
	var initial_array = new Array();
	eval("initial_array="+data);
	document.getElementById("helpAddress").href = "http://"+initial_array[10]+":8081/downloadmaster/help.asp";
	}

function select_port(value) {
	updateStatusCounter=0;
	if(value == 0){
		document.getElementById('Incoming_port').value = 51413;	
		document.getElementById('Incoming_port').readOnly= true;
		document.getElementById('Enable_peer_port').value = 0;
	}
	else{
		document.getElementById('Incoming_port').readOnly= false;
		document.getElementById('Enable_peer_port').value = 1;
		}
	}
function Maximum_download_speed(){
	updateStatusCounter=0;
	if(document.getElementById('Down_limit').value == 0)
	{
	$j("#Down_limit").attr("value","1");
	$j("#Down_limit_checkbox").attr("value","0");
	//$j("#Down_limit_checkbox").attr("checked","checked");
	document.getElementById('Down_limit_checkbox').checked = false;
	$j("#download_speed_limit").show();
	$j("#download_speed_span").show();
	}
	else{
	//document.getElementById('download_speed_limit').value = "";
	$j("#Down_limit").attr("value","0");
	$j("#Down_limit_checkbox").attr("value","1");
	//$j("#Down_limit_checkbox").attr("checked","");
	document.getElementById('Down_limit_checkbox').checked = true;
	$j("#download_speed_limit").hide();
	$j("#download_speed_span").hide();
	}
	//alert(document.getElementById("Down_limit").value);
	}
function Maximum_upload_speed(){
	updateStatusCounter=0;
	if(document.getElementById('Up_limit').value == 0){
	$j("#Up_limit").attr("value","1");
	$j("#Up_limit_checkbox").attr("value","0");
	document.getElementById('Up_limit_checkbox').checked = false;
	$j("#upload_speed_limit").show();
	$j("#upload_speed_span").show();
	}
	else{
	//document.getElementById('upload_speed_limit').value = "";
	$j("#Up_limit").attr("value","0");
	$j("#Up_limit_checkbox").attr("value","1");
	document.getElementById('Up_limit_checkbox').checked = true;
	$j("#upload_speed_limit").hide();
	$j("#upload_speed_span").hide();
	//document.getElementById('upload_speed_limit').readOnly = false;
	}
	//alert(document.getElementById("Up_limit").value);
	}
function documentkeydown(){
	var userAgent = navigator.userAgent.toLowerCase();
	if(window.ActiveXObject){
		if(event.keyCode==8 && document.getElementById('Incoming_port').readOnly){
			event.keyCode = 0;
			event.returnValue = false;
		}
	}
	else if(/chrome/i.test(userAgent) && /webkit/i.test(userAgent) && /mozilla/i.test(userAgent)){
		if(event.keyCode==8 && document.getElementById('Incoming_port').readOnly){
			event.keyCode = 0;
			event.returnValue = false;
		}
		}
	}
	
function judgeDigit(){
	var digit = /^\d*$/
	var port = document.getElementById("Incoming_port").value;
	if(!digit.test(port)){
		Ext.MessageBox.alert(multiLanguage_setting_array[multi_INT][39],multiLanguage_setting_array[multi_INT][96]);
		document.getElementById("Incoming_port").value = "";
	}
	}
</script>
</head>

<body onload="initial();" onunLoad="return unload_body();">
<div id="TopBanner"></div>

<div id="Loading" class="popup_bg"></div>


<iframe name="hidden_frame" id="hidden_frame" width="0" height="0" frameborder="0"></iframe>
<form method="get" name="form" action="dm_apply.cgi" target="hidden_frame" id="Setting_BT_form">
<input type="hidden" name="action_mode" value="DM_APPLY" />
<input type="hidden" name="download_type" value="BT" />
<input id="Enable_peer_port" name="Enable_peer_port" type="hidden" value="" />
<input id="peer_port_change" name="peer_port_change" type="hidden" value="0" />
<input id="bt_port_tmp" name="bt_port_tmp" type="hidden" value="" />

<table class="content" align="center" cellpadding="0" cellspacing="0">
  <tr>
	<td width="17">&nbsp;</td>
	
	<!--=====Beginning of Main Menu=====-->
	<td valign="top" width="202">
	  <div id="mainMenu"></div>
	  <div id="subMenu"></div>
	</td>
	
	<td valign="top">
	 <!--delete by alan <div id="tabMenu" class="submenuBlock"></div> -->
	<div style="margin-top:-150px; padding-left:0px;">
<!--===================================Beginning of Main Content===========================================-->
<table width="98%" border="0" align="left" cellpadding="0" cellspacing="0">
  <tr>
	<td align="left" valign="top" >
	  <table width="760px" border="0" cellpadding="4" cellspacing="0" class="FormTitle" id="FormTitle">
		<tbody>
		<tr>
		  <td bgcolor="#4D595D">
		  <div>&nbsp;</div>
		  <!--  deletd by alan<div class="formfonttitle"><#menu5_1#> - <#menu5_1_1#></div>-->
			<div class="formfonttitle" id="multiSetting_13"></div><!-- add by alan -->
      <div style="margin-left:5px;margin-top:10px;margin-bottom:10px"><img src="images/New_ui/export/line_export.png"></div>
      <!-- delete by alan <div class="formfontdesc"><#Layer3Forwarding_x_ConnectionType_sectiondesc#></div> -->
		</td>
        </tr>
        <tr>
        <td bgcolor="#4D595D">
			<table width="99%" border="1" align="center" cellpadding="4" cellspacing="0" class="FormTable" bordercolor="#6b8fa3">
				<thead>
            	<tr><td colspan="2" id="multiSetting_14"></td></tr>
          		</thead>
                <tr><td colspan="2"><input type="radio" name="Select_port" class="input" value="0" id="Select_port1" onclick="select_port(this.value);"/><span id="multiSetting_15" style="color:#FFF;"></span><br />
                <input  type="radio" class="input" value="1" name="Select_port" id="Select_port2" onclick="select_port(this.value);"/><span id="multiSetting_16" style="color:#FFF;"></span></td></tr>
				<tr><td colspan="2" style="height:35px;"><span style="margin-right:10px; margin-left:8px; color:#FFF;" id="multiSetting_17"></span>
                <input id="Incoming_port" name="Peer_port" type="text" maxlength="5" class="input_15_table" readonly="readonly" value="" onkeydown="documentkeydown();" onkeyup="judgeDigit();" /></td></tr>
			</table>
           </td>
         </tr>
          <tr>
        <td bgcolor="#4D595D">
			<table width="99%" border="1" align="center" cellpadding="4" cellspacing="0" class="FormTable" bordercolor="#6b8fa3">
				<thead>
            	<tr><td colspan="2" id="multiSetting_18"></td></tr>
          		</thead>
                <tr>
                <th id="multiSetting_19"></th>
                <td>
                &nbsp;<input type="checkbox" value="1" checked="checked" id="Down_limit_checkbox" onclick="Maximum_download_speed();" />
                <input type="hidden" name="Down_limit" id="Down_limit" value="0" ><span id="multiSetting_20" style="color:#FFF;"></span>
                <input type="text" value="" name="Down_rate" id="download_speed_limit" maxlength="6" class="input_6_table" /><span id="download_speed_span"><font color="#FFFFFF" id="multiSetting_22"></font></span></td></tr>
				<tr><th id="multiSetting_24"></th>
                <td>
                &nbsp;<input value="1" id="Up_limit_checkbox" checked="checked" type="checkbox" onclick="Maximum_upload_speed();" />
                <input type="hidden" name="Up_limit" id="Up_limit" value="0" /><span id="multiSetting_21" style="color:#FFF;"></span>
                <input name="Up_rate" type="text" value="" id="upload_speed_limit" maxlength="6" class="input_6_table" /><span id="upload_speed_span"><font color="#FFFFFF" id="multiSetting_23"></font></span></td></tr>
			</table>
           </td>
         </tr>
          <tr>
        	<td bgcolor="#4D595D">
			<table width="99%" border="1" align="center" cellpadding="4" cellspacing="0" class="FormTable" bordercolor="#6b8fa3">
				<thead>
            	<tr><td colspan="2" id="multiSetting_25"></td></tr>
          		</thead>
                <tr>
                	<th id="multiSetting_26"></th>
                    <td>
					<!--<div class="left" style="width: 94px;" id="radio_wl_closed"></div>
					<div class="clear"></div>	-->				
					<select class="input_option" onchange="" name="Auth_type">
                    	<option value="0" id="Auth_type0"></option>
                        <option value="1" id="Auth_type1"></option>
                        <option value="2" id="Auth_type2"></option>
                    </select>
                    </td>
                </tr>
                <tr>
                	<th id="multiSetting_27"></th>
                    <td>
                    	<input type="text" maxlength="5" class="input_15_table" name="Max_torrent_peer" id="Max_torrent_peer" value="100"/>
                    </td>
                </tr>
                <tr>
                	<th id="Max_peer_desc">Global Maximum connection</th>
                    <td>
                    	<input type="text" maxlength="5" class="input_15_table" name="Max_peer" id="Max_peer" value="100"/>
                    </td>
                </tr>
                <tr>
                	<th id="multiSetting_28" style="height:48px;"></th>
                    <td style="word-break:normal|break-all|keep-al">
                    <div class="left" style="width: 94px;" id="radio_wl2_closed" title=""></div>
                    <div style="display:table; position:absolute; margin-left:100px; margin-top:-35px; height:40px; width:350px; color:#FFF;">
                    <span id="multiSetting_29" style="color:#FFF;vertical-align:middle;display:table-cell;"></span>
                    </div><input type="hidden" name="Enable_dht" id="Enable_dht" value="" />					
                    </td>
                </tr>
                <tr>
                	<th id="radio_pex_closed_desc">PEX network</th>
                    <td>
                    <div class="left" style="width: 94px;" id="radio_pex_closed" title=""></div>
		<input type="hidden" name="Enable_pex" id="Enable_pex" value="" />					
                    </td>
                </tr>
                <!--<tr>
                	<th>DHT UDP port</th>
                    <td>
                    	<input type="text" maxlength="15" class="input_15_table"  value="100"/>
                    </td>
                </tr>-->
			</table>
			  
					<div class="apply_gen">
						<!--input type="button" class="button_gen" value="<#GO_5G#>" onclick="submit();"-->
						<input type="button" id="applyButton" class="button_gen" value="" onclick="BTapplyRule();">
					</div>			  	
			  	
		  	</td>
		</tr>
		</tbody>
		
	  </table>
	</td>
</form>
</tr>
</table>
<!--===================================Ending of Main Content===========================================-->
	</div>
	</td>
	
	<td width="10" align="center" valign="top"></td>
  </tr>
</table>

<div id="footer"></div>
<script>
$j("#multiSetting_13").html(multiLanguage_setting_array[multi_INT][12]);
$j("#multiSetting_14").html(multiLanguage_setting_array[multi_INT][13]);
$j("#multiSetting_15").html(multiLanguage_setting_array[multi_INT][14]);
$j("#multiSetting_16").html(multiLanguage_setting_array[multi_INT][15]);
$j("#multiSetting_17").html(multiLanguage_setting_array[multi_INT][16]);
$j("#multiSetting_18").html(multiLanguage_setting_array[multi_INT][17]);
$j("#multiSetting_19").html(multiLanguage_setting_array[multi_INT][18]);
$j("#multiSetting_20").html(multiLanguage_setting_array[multi_INT][19]);
$j("#multiSetting_22").html(multiLanguage_setting_array[multi_INT][20]);
$j("#multiSetting_24").html(multiLanguage_setting_array[multi_INT][21]);
$j("#multiSetting_21").html(multiLanguage_setting_array[multi_INT][19]);
$j("#multiSetting_23").html(multiLanguage_setting_array[multi_INT][20]);
$j("#multiSetting_25").html(multiLanguage_setting_array[multi_INT][22]);
$j("#multiSetting_26").html(multiLanguage_setting_array[multi_INT][23]);
$j("#Max_peer_desc").html(multiLanguage_setting_array[multi_INT][97]);
$j("#Auth_type0").html(multiLanguage_setting_array[multi_INT][24]);
$j("#Auth_type1").html(multiLanguage_setting_array[multi_INT][25]);
$j("#Auth_type2").html(multiLanguage_setting_array[multi_INT][26]);
$j("#multiSetting_27").html(multiLanguage_setting_array[multi_INT][27]);
$j("#multiSetting_28").html(multiLanguage_setting_array[multi_INT][28]);
$j("#multiSetting_29").html(multiLanguage_setting_array[multi_INT][29]);
$j("#radio_pex_closed_desc").html(multiLanguage_setting_array[multi_INT][111]);
$j("#applyButton").attr("value",multiLanguage_setting_array[multi_INT][2]);
</script>
</body>
</html>
