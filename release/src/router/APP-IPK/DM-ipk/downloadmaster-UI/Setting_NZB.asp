<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<html xmlns:v>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
<meta HTTP-EQUIV="Pragma" CONTENT="no-cache">
<meta HTTP-EQUIV="Cache-Control" CONTENT="no-cache">
<meta HTTP-EQUIV="Expires" CONTENT="-1">
<link rel="shortcut icon" href="images/favicon.png">
<link rel="icon" href="images/favicon.png">
<title>NZB Setting</title>
<link rel="stylesheet" type="text/css" href="index_style.css"> 
<link rel="stylesheet" type="text/css" href="form_style.css">
<link rel="stylesheet" type="text/css" href="routercss.css">
<link rel="stylesheet" type="text/css" href="ext/css/ext-all.css">
<!--<link href="multiLanguageCss/english.css" rel="stylesheet" type="text/css" id="languageCss" />
<link href="multiLanguageCss/english_1.css" rel="stylesheet" type="text/css" id="languageCss_1" />-->
<script type="text/javascript" src="jquery.js?ver=104"></script>
<script>
var httpTag = 'https:' == document.location.protocol ? false : true;
var $j = jQuery.noConflict();
var multi_INT = 0;
var generalnntp_data_tmp;
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
	generalnntp_data_tmp=data;
	var lang = array[14];
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
<script language="JavaScript" type="text/javascript" src="state.js?ver=104"></script>
<script language="JavaScript" type="text/javascript" src="general.js?ver=104"></script>
<script language="JavaScript" type="text/javascript" src="popup.js?ver=104"></script>
<script type="text/javascript" src="ext/ext-base.js?ver=104"></script>
<script type="text/javascript" src="ext/ext-all.js?ver=104"></script>
<!--<script type="text/javascript" language="JavaScript" src="/help.js?ver=104"></script>-->
<script type="text/javascript" language="JavaScript" src="detect.js?ver=104"></script>
<script type="text/javascript" src="plugin/jquery.iphone-switch.js?ver=104"></script>
<script type="text/javascript" src="Setting.js?ver=104"></script>
<style type="text/css">
a:focus{outline:none;}
</style>
<script>

function initial(){
	show_menu();
	initial_nzb();
	//document.getElementById("select_lang_"+multi_INT).selected = true;
	get_Refresh_time(generalnntp_data_tmp);
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

function checkPassword(){
	updateStatusCounter=0;
	var x = true;
	//if(document.getElementById('Password').readOnly == false){
		var a = document.getElementById('Password').value;
		var b = document.getElementById('Confirm_Password').value;
		var c = document.getElementById('Server1Host').value;
		var d = document.getElementById('User_name').value;
		var e = document.getElementById('serverPort').value;
		var f = document.getElementById('Number_of_connections').value;
		var g = document.getElementById('downloadSpeed_limit').value;
		if(c == ""){
			x = false;
			Ext.MessageBox.alert(multiLanguage_setting_array[multi_INT][39],multiLanguage_setting_array[multi_INT][40]);
			}
		else if(e == ""){
			x = false;
			Ext.MessageBox.alert(multiLanguage_setting_array[multi_INT][39],multiLanguage_setting_array[multi_INT][41]);
			}
		else if(f == ""){
			x = false;
			Ext.MessageBox.alert(multiLanguage_setting_array[multi_INT][39],multiLanguage_setting_array[multi_INT][42]);
			}
		else if(d == ""){
			x = false;
			Ext.MessageBox.alert(multiLanguage_setting_array[multi_INT][39],multiLanguage_setting_array[multi_INT][43]);
			}
		else if(a=="" || b ==""){
			x = false;
			Ext.MessageBox.alert(multiLanguage_setting_array[multi_INT][39],multiLanguage_setting_array[multi_INT][44]);
			}
		else if(a!=b){
			x = false
			Ext.MessageBox.alert(multiLanguage_setting_array[multi_INT][39],multiLanguage_setting_array[multi_INT][45]);
			}
		if (f<1||f>10){
			x = false
			Ext.MessageBox.alert(multiLanguage_setting_array[multi_INT][39],multiLanguage_setting_array[multi_INT][84]);
			}
		if (g == ""){
			x = false
			Ext.MessageBox.alert(multiLanguage_setting_array[multi_INT][39],multiLanguage_setting_array[multi_INT][47]);
			}
		//}
		if(x)
		NZBapplyRule();
	}

function NZBMaximum_download_speed(){
	updateStatusCounter=0;
	if($j("#NZBDown_limit_checkbox").attr("value") == 0)
	{
		$j("#NZBDown_limit_checkbox").attr("value",1);
		//$j("#NZBDown_limit_checkbox").attr("checked","checked");
		$j("#downloadSpeed_limit").attr("value",0);
		document.getElementById("NZBDown_limit_checkbox").checked = true;
		$j("#downloadSpeed_limit").hide();
		$j("#NZBdownload_speed_span").hide();
	}
	else{
		$j("#NZBDown_limit_checkbox").attr("value",0);
		//$j("#NZBDown_limit_checkbox").attr("checked","");
		document.getElementById("NZBDown_limit_checkbox").checked = false;
		$j("#downloadSpeed_limit").show();
		$j("#NZBdownload_speed_span").show();
	}
}

function judgeDigit(){
	var digit = /^\d*$/
	var port = document.getElementById("serverPort").value;
	if(!digit.test(port)){
		Ext.MessageBox.alert(multiLanguage_setting_array[multi_INT][39],multiLanguage_setting_array[multi_INT][96]);
		document.getElementById("serverPort").value = "";
	}
	}
</script>
</head>

<body onload="initial();" onunLoad="return unload_body();">
<div id="TopBanner"></div>

<div id="Loading" class="popup_bg"></div>

<iframe name="hidden_frame" id="hidden_frame" src="" width="0" height="0" frameborder="0"></iframe>
<form method="get" name="form" id="Setting_NZB_form" action="dm_apply.cgi" target="hidden_frame" />
<input type="hidden" name="action_mode" value="DM_APPLY" />
<input type="hidden" name="download_type" value="NZB" />


<table class="content" align="center" cellpadding="0" cellspacing="0">
	<tr>
		<td width="17">&nbsp;</td>
		
		<td valign="top" width="202">				
		<div  id="mainMenu"></div>	
		<div  id="subMenu"></div>		
		</td>				
		
    <td valign="top">
		<!-- delete by alan <div id="tabMenu" class="submenuBlock"></div> -->
		<!--===================================Beginning of Main Content===========================================-->
        
        <div style="margin-top:-150px; padding-left:0px;">
<table width="98%" border="0" align="left" cellpadding="0" cellspacing="0">
	<tr>
		<td align="left" valign="top" >
          
		<table width="760px" border="0" cellpadding="5" cellspacing="0" class="FormTitle" id="FormTitle">
		<tbody>
			<tr>
		  		<td bgcolor="#4D595D">
		  		<div>&nbsp;</div>
		  		<div class="formfonttitle" id="multiSetting_30"></div>
		  		<div style="margin-left:5px;margin-top:10px;margin-bottom:10px"><img src="images/New_ui/export/line_export.png"></div>
		 		  <!-- delete by alan  <div class="formfontdesc"><#LANHostConfig_x_DDNSEnable_sectiondesc#></div>-->

			<table width="100%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3"  class="FormTable">
			<tr><td colspan="2" id="multiSetting_31"></td></tr>
            <tr>
            	<th id="multiSetting_32"></th>
            	<td><input type="text" class="input_25_table" name="Server1.Host" id="Server1Host" /></td>
            </tr>
             <tr>
            	<th id="multiSetting_33"></th>
            	<td><input type="text" maxlength="5" class="input_25_table" value="119" name="Server1.Port" id="serverPort" onkeyup="judgeDigit();" /></td>
            </tr>
            <tr><th id="multiSetting_34"></th>
                <td>
                &nbsp;<input type="checkbox" value="0" checked="checked" id="NZBDown_limit_checkbox" onclick="NZBMaximum_download_speed();" />&nbsp;<span style="color:#FFF;" id="multiSetting_35"></span>
                <input name="DownloadRate" type="text" value="" id="downloadSpeed_limit" maxlength="6" class="input_6_table" />
                <span id="NZBdownload_speed_span"><font color="#FFFFFF" id="multiSetting_36"></font></span>
                </td></tr>
			<tr>
				<th width="200" id="multiSetting_37"></th>
				<td>
					<div class="left" style="width: 94px;" id="radio_ddns_enable_x" title=""></div>
                    <input type="hidden" value="" name="Server1.Encryption" id="Encryption" />
					<div class="clear"></div>
				</td>
			</tr>
            <!--<tr>
				<th width="200">Authentication required</th>
				<td>
					<div class="left" style="width: 94px;" id="Authentication_required"></div>
					<div class="clear"></div><input type="hidden" name="Enable_auth" id="Enable_auth" value="0" />					
					<script type="text/javascript">
						$j('#Authentication_required').iphoneSwitch('0', 
							 function() {
								 document.getElementById('User_name').readOnly= false;
								 document.getElementById('Password').readOnly= false;
								 document.getElementById('Confirm_Password').readOnly= false;
								 document.getElementById('Number_of_connections').readOnly= false;
								 $j("#Enable_auth").attr("value","0");
								//document.form.ddns_enable_x.value = "1";
								//return change_common_radio(document.form.ddns_enable_x, 'LANHostConfig', 'ddns_enable_x', '1')
							 },
							 function() {
								document.getElementById('User_name').value = "";
								document.getElementById('Password').value = "";
								document.getElementById('Confirm_Password').value = "";
								document.getElementById('Number_of_connections').value = 2;
								document.getElementById('User_name').readOnly= true;
								document.getElementById('Password').readOnly= true;
								document.getElementById('Confirm_Password').readOnly= true;
								document.getElementById('Number_of_connections').readOnly= true;
								$j("#Enable_auth").attr("value","1");
								//document.form.ddns_enable_x.value = "0";
								//return change_common_radio(document.form.ddns_enable_x, 'LANHostConfig', 'ddns_enable_x', '1')
							 },
							 {
								//switch_on_container_path: '/plugin/iphone_switch_container_off.png'
							 }
						);
					</script>
				</td>
			</tr>-->
            <tr>
            	<th id="multiSetting_38"></th>
            	<td><input id="User_name" type="text" class="input_25_table" name="Server1.Username" /></td>
            </tr>
            <tr>
            	<th id="multiSetting_39"></th>
            	<td><input id="Password" type="password" class="input_25_table" name="Server1.Password" /></td>
            </tr>
            <tr>
            	<th id="multiSetting_40"></th>
            	<td><input id="Confirm_Password" type="password" class="input_25_table" /></td>
            </tr>
            <tr>
            	<th id="multiSetting_41"></th>
            	<td><input id="Number_of_connections"  maxlength="2" name="Server1.Connections" type="text" class="input_25_table" value="4" /></td>
            </tr>
		</table>
		
				<div class="apply_gen">
					<input id="multiSetting_42" class="button_gen" onclick="checkPassword();" type="button" value="" />
				</div>		
		
			  </td>
              </tr>
            </tbody>

            </table>
		  </td>
</form>


        </tr>
      </table>	
      </div>			
		<!--===================================Ending of Main Content===========================================-->		
	</td>
		
    <td width="10" align="center" valign="top">&nbsp;</td>
	</tr>
</table>

<div id="footer"></div>
<script>
$j("#multiSetting_30").html(multiLanguage_setting_array[multi_INT][30]);
$j("#multiSetting_31").html(multiLanguage_setting_array[multi_INT][31]);
$j("#multiSetting_32").html(multiLanguage_setting_array[multi_INT][32]);
$j("#multiSetting_33").html(multiLanguage_setting_array[multi_INT][33]);
$j("#multiSetting_34").html(multiLanguage_setting_array[multi_INT][18]);
$j("#multiSetting_35").html(multiLanguage_setting_array[multi_INT][19]);
$j("#multiSetting_36").html(multiLanguage_setting_array[multi_INT][20]);
$j("#multiSetting_37").html(multiLanguage_setting_array[multi_INT][34]);
$j("#multiSetting_38").html(multiLanguage_setting_array[multi_INT][35]);
$j("#multiSetting_39").html(multiLanguage_setting_array[multi_INT][36]);
$j("#multiSetting_40").html(multiLanguage_setting_array[multi_INT][37]);
$j("#multiSetting_41").html(multiLanguage_setting_array[multi_INT][38]);
$j("#multiSetting_42").attr("value",multiLanguage_setting_array[multi_INT][2]);
</script>
</body>
</html>
