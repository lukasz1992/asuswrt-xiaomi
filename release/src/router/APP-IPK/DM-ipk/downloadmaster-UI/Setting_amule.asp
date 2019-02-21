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
	<title>aMule Setting</title>
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
var generalamule_data_tmp;
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
	generalamule_data_tmp=data;
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
		.contentM_qis {
			position: absolute;
			-webkit-border-radius: 5px;
			-moz-border-radius: 5px;
			border-radius: 5px;
			z-index: 200;
			background-color: #2B373B;
			display: none;
			margin-left: 40%;
			top: 180px;
			width: 400px;
		}
		
		a:focus {
			outline: none;
		}
	</style>
	<script>

function initial(){
	show_menu();
	getaMuleServerList();
	get_Refresh_time(generalamule_data_tmp);
 	//get_amule_server_status();
	setInterval("getaMuleServerList();",5000);
	AUTOLOGOUT_MAX_MINUTE=AUTOLOGOUT_MAX_MINUTE_TMP;
	updateStatus_AJAX();
}
function get_Refresh_time(data){
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
}}

function initial_Refresh_time(data){
	var initial_array = new Array();
	eval("initial_array="+data);
	document.getElementById("helpAddress").href = "http://"+initial_array[10]+":8081/downloadmaster/help.asp";
	}

function enable_add_button(data) {
//<input class="button_gen_long" onclick="add_rule()" type="button" id="add_button">
	var code = ""
	if(data==1) {
		code += '<input class="button_disable" type="button" id="add_button"'
		code += 'value=' + multiLanguage_setting_array[multi_INT][64] + '>';
		$j("#aMule_server_status_string").html(multiLanguage_setting_array[multi_INT][113]);
	} else {
		code += '<input class="button_gen_long" onclick="add_rule()" type="button" id="add_button"'
		code += 'value=' + multiLanguage_setting_array[multi_INT][64] + '>';
	}
	document.getElementById("apply_gen").innerHTML = code;
}

function inet_network(ip_str){
	if(!ip_str)
		return -1;
	
	var re = /^(\d+)\.(\d+)\.(\d+)\.(\d+)$/;
	if(re.test(ip_str)){
		var v1 = parseInt(RegExp.$1);
		var v2 = parseInt(RegExp.$2);
		var v3 = parseInt(RegExp.$3);
		var v4 = parseInt(RegExp.$4);
		
		if(v1 < 256 && v2 < 256 && v3 < 256 && v4 < 256)
			return v1*256*256*256+v2*256*256+v3*256+v4;
	}
	
	return -2;
}

function valid_IP(data){
	// A : 1.0.0.0~126.255.255.255
	// B : 127.0.0.0~127.255.255.255 (forbidden)
	// C : 128.0.0.0~255.255.255.254
	var A_class_start = inet_network("1.0.0.0");
	var A_class_end = inet_network("126.255.255.255");
	var B_class_start = inet_network("127.0.0.0");
	var B_class_end = inet_network("127.255.255.255");
	var C_class_start = inet_network("128.0.0.0");
	var C_class_end = inet_network("255.255.255.255");		
	var ip_num = inet_network(data);

	if(ip_num > A_class_start && ip_num < A_class_end){
			return true;
	}
	else if(ip_num > B_class_start && ip_num < B_class_end){
		return false;
	}
	else if(ip_num > C_class_start && ip_num < C_class_end){
		return true;
	}
	else{
		return false;
	}
}
	
function judgeDigit(){
	var digit = /^\d*$/
	var port = document.getElementById("aMuleServerPort").value;
	if(!digit.test(port)){
		Ext.MessageBox.alert(multiLanguage_setting_array[multi_INT][39],multiLanguage_setting_array[multi_INT][96]);
		document.getElementById("aMuleServerPort").value = "";
	}
}

function add_rule() {
		var x = true;
		var a = document.getElementById('aMuleServerIP').value;
		var b = document.getElementById('aMuleServerPort').value;
		var digit = /^\d*$/
		var IP_test = valid_IP(a);
		if(serverlist_array[2] == 1){
			x = false;
			Ext.MessageBox.alert(multiLanguage_setting_array[multi_INT][39],multiLanguage_setting_array[multi_INT][114]);
		}
		else if(a=="" || b==""){
			x = false;
			Ext.MessageBox.alert(multiLanguage_setting_array[multi_INT][39],multiLanguage_setting_array[multi_INT][93]);
		}
		else if(!digit.test(b) || b > 65535 || b < 1){
			x = false;
			Ext.MessageBox.alert(multiLanguage_setting_array[multi_INT][39],multiLanguage_setting_array[multi_INT][94]);
			}
		else if(!IP_test){
			x = false;
			Ext.MessageBox.alert(multiLanguage_setting_array[multi_INT][39],a+multiLanguage_setting_array[multi_INT][95]);
		}else if(serverlist_array.length > 32) {
			x = false;
			Ext.MessageBox.alert(multiLanguage_setting_array[multi_INT][39],multiLanguage_setting_array[multi_INT][140]);
		} else {
			for(var i=0; i<serverlist_array.length; i=i+3){
				if(serverlist_array[i] == a)
				{
					Ext.MessageBox.alert(multiLanguage_setting_array[multi_INT][39],multiLanguage_setting_array[multi_INT][141]);
					x = false;
					break;
				}
			}
		}
		if(x) {
			aMuleAddServer(a,b);
			showLoading(3);
			getaMuleServerList();
			document.form.aMuleServerIP.value = "";
			document.form.aMuleServerPort.value = "";
		}
}

function removeServer(IP,Port, value) {
	if(IP == "176.103.48.36" && Port == "4184") {
		Ext.MessageBox.alert(multiLanguage_setting_array[multi_INT][39],multiLanguage_setting_array[multi_INT][143]);
		return;
	}
	removeaMuleServer(IP,Port,value);
	showLoading(3);
	setTimeout("getaMuleServerList();",2);
}

function conServer(IP,Port,value) {
	if(value == "connect")
		aMuleConServer(IP,Port);
	else if(value == "disconnect")
		aMuleDisConServer(IP,Port);
	showLoading(3);
	getaMuleServerList();
}

var serverlist_array = new Array();
function showServerList(data) {

	eval("serverlist_array="+data);
	var code = "";
	code +='<table style="margin-bottom:30px;" width="100%" border="1" align="center" cellpadding="4" cellspacing="0" class="list_table" id="aMule_serverlist_table">';
	if(serverlist_array.length <4)
		code +='<tr><td style="color:#FC0;" colspan="6">' +multiLanguage_setting_array[multi_INT][142]+' </td></tr>';
	else{
		for(var i=3; i<serverlist_array.length; i=i+3){
			code +='<tr id="row'+(i/3)+'">';
			code += '<td width="10%">'+ (i/3) +'</td>';//Number
			var server_state = "0";

			
			code += '<td width="35%">'+ serverlist_array[i] +'</td>';//ip
			code += '<td width="10%">'+ serverlist_array[i+1] +'</td>';//port

			if(serverlist_array[i+2] == 1) {
				 //amule disable
				code += '<td width="15%">'+ multiLanguage_setting_array[multi_INT][134]  +'</td>';//status
				code += '<td width="20%"><input class="button_gen" type="button" disabled="disabled"' 
				code += ' value='+multiLanguage_setting_array[multi_INT][130]  +' style="padding:0 0.3em 0 0.3em;" >';
				code += '<td width="10%"><input class="remove_btn" type="button" disabled="disabled" value=""/></td>';
			} else if(serverlist_array[i+2] == 2) {
				//display connect really status is disconnected
				code += '<td width="15%">'+ multiLanguage_setting_array[multi_INT][139] +'</td>';//status
				code += '<td width="20%"><input class="button_gen" type="button" onclick="conServer(\'';
				code += serverlist_array[i] +'\',\'' + serverlist_array[i+1] +'\',\'connect\')" id="disconnect_btn"';
				code += ' value='+multiLanguage_setting_array[multi_INT][131]  +' style="padding:0 0.3em 0 0.3em;" >';

				code += '<td width="10%"><input class="remove_btn" type="button" onclick="removeServer(\'';
				code += serverlist_array[i] +'\',\'' + serverlist_array[i+1] + '\');" value=""/></td>';
			} else if(serverlist_array[i+2] == 3) {
				//display disconnected really status is connected
				code += '<td width="15%">'+ multiLanguage_setting_array[multi_INT][138] +'</td>';//status
				code += '<td width="20%"><input class="button_gen" type="button" onclick="conServer(\'';
				code += serverlist_array[i] +'\',\'' + serverlist_array[i+1] +'\',\'disconnect\')" id="disconnect_btn"';
				code += ' value='+multiLanguage_setting_array[multi_INT][132]  +' style="padding:0 0.3em 0 0.3em;" >';

				code += '<td width="10%"><input class="remove_btn" type="button" disabled="disabled" value=""/></td>';
			} else if(serverlist_array[i+2] == 4) {
				//connectting
				code += '<td width="15%">'+ multiLanguage_setting_array[multi_INT][136] +'</td>';//status
				code += '<td width="20%"><input class="button_disable" type="button" disabled="disabled"';
				code += ' value='+multiLanguage_setting_array[multi_INT][136]  +' style="padding:0 0.3em 0 0.3em;" >';
				code += '<td width="10%"><input class="remove_btn" type="button" disabled="disabled" value=""/></td>';
			} else if(serverlist_array[i+2] == 5) {
				code += '<td width="15%">'+ multiLanguage_setting_array[multi_INT][137] +'</td>';//status
				code += '<td width="20%"><input class="button_gen" type="button" onclick="conServer(\'';
				code += serverlist_array[i] +'\',\'' + serverlist_array[i+1] +'\',\'connect\')" id="disconnect_btn"';
				code += 'value='+multiLanguage_setting_array[multi_INT][131]  +' style="padding:0 0.3em 0 0.3em;" >';

				code += '<td width="10%"><input class="remove_btn" type="button" onclick="removeServer(\'';
				code += serverlist_array[i] +'\',\'' + serverlist_array[i+1] + '\');" value=""/></td>';
			} else if(serverlist_array[i+2] == 6) {
				code += '<td width="15%">'+ multiLanguage_setting_array[multi_INT][135] +'</td>';//status
				code += '<td width="20%"><input class="button_disable" type="button" disabled="disabled"';
				code += 'value='+multiLanguage_setting_array[multi_INT][133]  +' style="padding:0 0.3em 0 0.3em;" >';

				code += '<td width="10%"><input class="remove_btn" type="button" onclick="removeServer(\'';
				code += serverlist_array[i] +'\',\'' + serverlist_array[i+1] + '\', \'removed\');" value=""/></td>';
			}
			//the delete button in the web

			
		}
	}
	code +='</table>';
	document.getElementById("for_server").innerHTML = code;
	enable_add_button(serverlist_array[2]);
}
</script>
</head>

<body onload="initial();" onunLoad="return unload_body();">
	<div id="TopBanner"></div>

	<div id="Loading" class="popup_bg"></div>

	<iframe name="hidden_frame" id="hidden_frame" src="" width="0" height="0" frameborder="0"></iframe>
	<form method="get" name="form" id="Setting_NZB_form" action="dm_apply.cgi" target="hidden_frame" />
	<input type="hidden" name="action_mode" value="DM_APPLY" />
	<input type="hidden" name="download_type" value="amule" />
	<table class="content" align="center" cellpadding="0" cellspacing="0">
		<tr>
			<td width="17">&nbsp;</td>

			<td valign="top" width="202">
				<div id="mainMenu"></div>
				<div id="subMenu"></div>
			</td>

			<td valign="top">
				<!-- delete by alan <div id="tabMenu" class="submenuBlock"></div> -->
				<!--===================================Beginning of Main Content===========================================-->

				<div style="margin-top:-150px; padding-left:0px;">

					<table width="100%" border="0" align="left" cellpadding="0" cellspacing="0">
						<tr>
							<td align="left" valign="top" bgcolor="#4D595D">

								<table width="100%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3" class="FormTable">
									<tr>
										<div>&nbsp;</div>
										<div class="formfonttitle" id="multiSetting_30"></div>
										<div style="margin-left:5px;margin-top:10px;margin-bottom:10px"><img src="images/New_ui/export/line_export.png"></div>
									</tr>
									<tr>
										<td height="34px" colspan="2" id="multiSetting_31"></td>
									</tr>
									<tr>
										<th id="multiSetting_32"></th>
										<td><input type="text" class="input_15_table" maxlength="15" name="aMuleServerIP" id="aMuleServerIP" />&nbsp;&nbsp;&nbsp;
											<span name="aMule_server_status_string" id="aMule_server_status_string" style="FONT-SIZE:10pt;color: #ff0000"></span>
										</td>
									</tr>
									<tr>
										<th id="multiSetting_33"></th>
										<td><input type="text" maxlength="5" class="input_15_table" value="" name="aMuleServerPort" id="aMuleServerPort" onkeyup="judgeDigit();"/>
										</td>
									</tr>
								</table>
								<div id="apply_gen" class="apply_gen">
									
								</div>
								<table width="100%" border="0" cellpadding="5" cellspacing="0" class="FormTitle" id="FormTitle">
									<tbody>
										<tr>
											<td bgcolor="#4D595D">

												<table width="100%" border="1" align="center" cellpadding="4" cellspacing="0" class="FormTable_table">
													<thead>
														<tr>
															<td height="30px" colspan="6" id="multiSetting_123"></td>
														</tr>
													</thead>
													<tr>

														<th width="10%" >
															<div id="multiSetting_124"></div>
														</th>
														<th width="35%">
															<div id="multiSetting_125"></div>
														</th>
														<th width="10%">
															<div id="multiSetting_126"></div>
														</th>
														<th width="15%">
															<div id="multiSetting_127"></div>
														</th>
														<th width="20%">
															<div id="multiSetting_128"></div>
														</th>
														<th width="10%">
															<div id="multiSetting_129"></div>
														</th>

													</tr>
												</table>

												<div id="for_server"></div>

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
$j("#multiSetting_30").html(multiLanguage_setting_array[multi_INT][65]);
$j("#multiSetting_31").html(multiLanguage_setting_array[multi_INT][66]);
$j("#multiSetting_32").html(multiLanguage_setting_array[multi_INT][67]);
$j("#multiSetting_33").html(multiLanguage_setting_array[multi_INT][68]);
$j("#multiSetting_123").html(multiLanguage_setting_array[multi_INT][123]);
$j("#multiSetting_124").html(multiLanguage_setting_array[multi_INT][124]);
$j("#multiSetting_125").html(multiLanguage_setting_array[multi_INT][125]);
$j("#multiSetting_126").html(multiLanguage_setting_array[multi_INT][126]);
$j("#multiSetting_127").html(multiLanguage_setting_array[multi_INT][127]);
$j("#multiSetting_128").html(multiLanguage_setting_array[multi_INT][128]);
$j("#multiSetting_129").html(multiLanguage_setting_array[multi_INT][129]);
</script>
</body>

</html>
