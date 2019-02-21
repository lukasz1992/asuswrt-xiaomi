<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<head>
<meta http-equiv="X-UA-Compatible" content="IE=EmulateIE7"/>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
<meta HTTP-EQUIV="Pragma" CONTENT="no-cache">
<meta HTTP-EQUIV="Cache-Control" CONTENT="no-cache">
<meta HTTP-EQUIV="Expires" CONTENT="-1">
<link rel="shortcut icon" href="images/favicon.png">
<link rel="icon" href="images/favicon.png">
<title>General Setting</title>
<link rel="stylesheet" type="text/css" href="index_style.css"> 
<link rel="stylesheet" type="text/css" href="form_style.css">
<!--<link rel="stylesheet" type="text/css" href="NM_style.css">-->
<link rel="stylesheet" type="text/css" href="other.css">
<link rel="stylesheet" type="text/css" href="routercss.css">
<link rel="stylesheet" type="text/css" href="ext/css/ext-all.css">
<!--<link href="multiLanguageCss/english.css" rel="stylesheet" type="text/css" id="languageCss" />
<link href="multiLanguageCss/english_1.css" rel="stylesheet" type="text/css" id="languageCss_1" />-->
<script type="text/javascript" src="jquery.js?ver=104"></script>
<script>
var httpTag = 'https:' == document.location.protocol ? false : true;
var $j = jQuery.noConflict();
var multi_INT = 0;
var fw_enable = 1;
var generalSetting_data_tmp;
var router_uptimeStr="";
var router_timezone;
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
	generalSetting_data_tmp = data;
	AUTOLOGOUT_MAX_MINUTE_TMP=array[30];
	router_uptimeStr = array[32];
	router_timezone = router_uptimeStr.substring(26,31);
	if( array[23]== "0")
	{
	   fw_enable = 0;	
	}
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
<script type="text/javascript" src="multiLanguage_setting.js?ver=104"></script>
<script type="text/javascript" src="multiLanguage_task.js?ver=104"></script>
<script type="text/javascript" src="multiLanguage_all.js?ver=104"></script>
<script type="text/javascript" src="state.js?ver=104"></script>
<script type="text/javascript" src="general.js?ver=104"></script>
<script type="text/javascript" src="popup.js?ver=104"></script>
<!--<script type="text/javascript" src="/help.js?ver=104"></script>-->
<script type="text/javascript" src="detect.js?ver=104"></script>
<script type="text/javascript" src="ext/ext-base.js?ver=104"></script>
<script type="text/javascript" src="ext/ext-all.js?ver=104"></script>
<script type="text/javascript" src="plugin/jquery.iphone-switch.js?ver=104"></script>
<script type="text/javascript" src="Setting.js?ver=104"></script>
<style type="text/css">
a:focus{outline:none;}
.color_red{
	color:#F00;
	}
.color_white{
	color:#FFF;
	}
.mask_bg{
	position:absolute;	
	margin:auto;
	top:0;
	left:0;
	width:100%;
	height:100%;
	z-index:100;
	/*background-color: #FFF;*/
	background:url(images/popup_bg2.gif);
	background-repeat: repeat;
	filter:progid:DXImageTransform.Microsoft.Alpha(opacity=60);
	-moz-opacity: 0.6;
	display:none;
	/*visibility:hidden;*/
	overflow:hidden;
}
.switch_disable_on{
	z-index:100;
	background:url(plugin/iphone_switch.png);
	overflow:hidden;
	background-position:left;
	}
.switch_disable_off{
	z-index:100;
	background:url(plugin/iphone_switch.png);
	overflow:hidden;
	background-position:right;
	}
.mask_floder_bg{
	position:absolute;	
	margin:auto;
	top:0;
	left:0;
	width:100%;
	height:100%;
	z-index:300;
	/*background-color: #FFF;*/
	background:url(images/popup_bg2.gif);
	background-repeat: repeat;
	filter:progid:DXImageTransform.Microsoft.Alpha(opacity=60);
	-moz-opacity: 0.6;
	display:none;
	/*visibility:hidden;*/
	overflow:hidden;
}
.panel{
	width:450px;
	position:absolute;
	margin-top:-8%;
	margin-left:35%;
	z-index:200;
	display:none;
}
.floder_panel{
	background-color:#999;	
	border:2px outset #CCC;
	font-size:15px;
	font-family:Verdana, Geneva, sans-serif;
	color:#333333;
	width:450px;
	position:absolute;
	margin-top:-8%;
	margin-left:35%;
	z-index:400;
	display:none;
}
.panel_folder{
	font-family:Courier ;
	width:500px;
	position:absolute;
	z-index:2000;
	display:none;
	background-image:url(images/Tree/bg_01.png);
	background-repeat:no-repeat;
}
.folder_tree{
	font-size:10pt;
	margin:0px 0px 1px 30px;
	height:339px;
	overflow:auto;
	width:455px;
}
.vert_line{
	max-width:25px;
	width:19px;
	line-height:25px;
	vertical-align:top;
	/*margin-left:2px;
	padding-left:23px;	*/
}
.folderClicked{
	color:#569AC7;
	/*font-weight:bolder;*/
	font-size:14px;
	cursor:text;
}
.lastfolderClicked{
	color:#FFFFFF;
	cursor:pointer;
}
</style>

<script>

function loadDateTime(){
	document.form.dm_radio_date_x_Sun.checked = getDateCheck(document.form.dm_radio_date_x.value, 0);
	document.form.dm_radio_date_x_Mon.checked = getDateCheck(document.form.dm_radio_date_x.value, 1);
	document.form.dm_radio_date_x_Tue.checked = getDateCheck(document.form.dm_radio_date_x.value, 2);
	document.form.dm_radio_date_x_Wed.checked = getDateCheck(document.form.dm_radio_date_x.value, 3);
	document.form.dm_radio_date_x_Thu.checked = getDateCheck(document.form.dm_radio_date_x.value, 4);
	document.form.dm_radio_date_x_Fri.checked = getDateCheck(document.form.dm_radio_date_x.value, 5);
	document.form.dm_radio_date_x_Sat.checked = getDateCheck(document.form.dm_radio_date_x.value, 6);
	document.form.dm_radio_time_x_starthour.value = getTimeRange(document.form.dm_radio_time_x.value, 0);
	document.form.dm_radio_time_x_startmin.value = getTimeRange(document.form.dm_radio_time_x.value, 1);
	document.form.dm_radio_time_x_endhour.value = getTimeRange(document.form.dm_radio_time_x.value, 2);
	document.form.dm_radio_time_x_endmin.value = getTimeRange(document.form.dm_radio_time_x.value, 3);
	document.form.dm_radio_time2_x_starthour.value = getTimeRange(document.form.dm_radio_time2_x.value, 0);
	document.form.dm_radio_time2_x_startmin.value = getTimeRange(document.form.dm_radio_time2_x.value, 1);
	document.form.dm_radio_time2_x_endhour.value = getTimeRange(document.form.dm_radio_time2_x.value, 2);
	document.form.dm_radio_time2_x_endmin.value = getTimeRange(document.form.dm_radio_time2_x.value, 3);
}

function Download_Schedule(a){
	updateStatusCounter=0;
	if(a==1){
			$j("#Time_Schedule").show();
		}
		else{

			  $j("#Time_Schedule").hide();
			}
	}

function corrected_timezone(){
	var today = new Date();
	var StrIndex;	
	if(today.toString().lastIndexOf("-") > 0)
		StrIndex = today.toString().lastIndexOf("-");
	else if(today.toString().lastIndexOf("+") > 0)
		StrIndex = today.toString().lastIndexOf("+");
	if(StrIndex > 0){
		//alert('dstoffset='+dstoffset+', 設定時區='+timezone+' , 當地時區='+today.toString().substring(StrIndex, StrIndex+5))
		if(router_timezone != today.toString().substring(StrIndex, StrIndex+5)){
			$("timezone_hint").style.display = "block";
			$("timezone_hint").innerHTML = multiLanguage_setting_array[multi_INT][121];
		}
		else
			return;			
	}
	else
		return;	
}
	
function initial(){
	show_menu();
	//document.getElementById("select_lang_"+multi_INT).selected = true;
	initial_general_status(generalSetting_data_tmp);
	corrected_timezone();
	AUTOLOGOUT_MAX_MINUTE=AUTOLOGOUT_MAX_MINUTE_TMP;
	updateStatus_AJAX();
	//get_Refresh_time();
}

function judgeDigit(){
	var digit = /^\d*$/
	var port = document.getElementById("DM_port").value;
	if(!digit.test(port)){
		Ext.MessageBox.alert(multiLanguage_setting_array[multi_INT][39],multiLanguage_setting_array[multi_INT][96]);
		document.getElementById("DM_port").value = "";
	 }
}
function judgeDigit2(){
	var digit = /^\d*$/
	var port = document.getElementById("DM_https_port").value;
	if(!digit.test(port)){
		Ext.MessageBox.alert(multiLanguage_setting_array[multi_INT][39],multiLanguage_setting_array[multi_INT][96]);
		document.getElementById("DM_https_port").value = "";
	}
	}
</script>
</head>

<body onload="initial();" onunLoad="return unload_body();">

<div id="TopBanner"></div>
<div id="DM_mask" class="mask_bg"></div>


<div id="folderTree_panel" class="panel_folder" >
		<table><tr><td>
			<div class="machineName" style="width:200px;font-family:Microsoft JhengHei;font-size:12pt;font-weight:bolder; margin-top:15px;margin-left:30px;"><span id="router_model"></span></div>
		</td>
		<td>
			<div style="width:240px;margin-top:17px;margin-left:190px;">
				<!--<img id="createFolderBtn" src="images/New_ui/advancesetting/FolderAdd.png" hspace="1" title="Addfolder" onclick="">-->
				<table >
					<tr>
						<td><div id="createFolderBtn" class="createFolderBtn" title="Addfolder"></div></td>
					</tr>
				</table>
			</div>
		</td></tr></table>
		<div id="e0" class="folder_tree"></div>
		<div style="background-image:url(images/Tree/bg_02.png);background-repeat:no-repeat;height:90px;">
		<input class="button_gen" id="multiSetting_17" type="button" style="margin-left:27%;margin-top:18px;" onclick="cancel_folderTree();" value="Cancel">
		<input class="button_gen" id="multiSetting_16" type="button"  onclick="confirm_folderTree();" value="Apply">	
	</div>
</div>
<!--start of add floder div-->
<div id="DM_mask_floder" class="mask_floder_bg"></div>
<div id="panel_addFloder" class="floder_panel">
<span style="margin-left:95px;"><b id="multiSetting_0"></b></span><br /><br />
<span style="margin-left:8px;margin-right:8px;"><b id="multiSetting_1"></b></span>
<input type="text" id="newFloder" class="input_15_table" value="" /><br /><br />
<input type="button" name="AddFloder" id="multiSetting_2" value="" style="margin-left:100px;" onclick="AddFloderName();">
&nbsp;&nbsp;
<input type="button" name="Cancel_Floder_add" id="multiSetting_3" value="" onClick="hide_AddFloder();">
</div>
<!--end of add floder div-->

<div id="hiddenMask" class="popup_bg">
	<table cellpadding="5" cellspacing="0" id="dr_sweet_advise" class="dr_sweet_advise" align="center">
		<tr>
		<td>
			<div class="drword" id="drword" style="height:110px;">
	    </div>
		  <div class="drImg"><img src="images/alertImg.gif"></div>
			<div style="height:70px;"></div>
		</td>
		</tr>
	</table>
<!--[if lte IE 6.5]><iframe class="hackiframe"></iframe><![endif]-->
</div>


<div id="Loading" class="popup_bg"></div>


<iframe name="hidden_frame" id="hidden_frame" src="" width="0" height="0" frameborder="0"></iframe>

<form method="post" name="aidiskForm" action="" target="hidden_frame">
<input type="hidden" name="motion" id="motion" value="">
<input type="hidden" name="layer_order" id="layer_order" value="">
</form>

<form method="get" name="form" id="Setting_General_form" action="dm_apply.cgi" target="hidden_frame">
<input type="hidden" name="action_mode" value="DM_APPLY" />
<input type="hidden" name="download_type" value="General" />
<input type="hidden" name="Lan_ip" id="Lan_ip" value="" />
<input type="hidden" name="Base_path" id="Base_path" value=""/>
<input type="hidden" id="Download_dir"  value="Download2" name="Download_dir" />
<input type="hidden" id="DM_port_renew" name="DM_port_renew" value="" />
<input type="hidden" id="DM_https_port_renew" name="DM_https_port_renew" value="" />
<input type="hidden" id="MISCR_HTTP_X" name="MISCR_HTTP_X" value="" />
<input type="hidden" id="DM_language" name="DM_language" value="" />
<input type="hidden" id="MISCR_HTTPPORT_X" name="MISCR_HTTPPORT_X" value="" />
<input type="hidden" id="MISCR_HTTPSPORT_X" name="MISCR_HTTPSPORT_X" value="" />
<input type="hidden" id="Productid" name="Productid" value="" />
<input type="hidden" id="APPS_DEV" name="APPS_DEV" value="" />
<input type="hidden" id="WAN_IP" name="WAN_IP" value="" />
<input type="hidden" id="DDNS_ENABLE_X" name="DDNS_ENABLE_X" value="" />
<input type="hidden" id="DDNS_HOSTNAME_X" name="DDNS_HOSTNAME_X" value="" />
<input type="hidden" id="MAX_ON_HEAVY" name="MAX_ON_HEAVY" value="" />
<input type="hidden" id="MAX_QUEUES" name="MAX_QUEUES" value="" />
<input type="hidden" id="MAX_ON_ED2K" name="MAX_ON_ED2K" value="" />
<input type="hidden" id="RFW_ENABLE_X" name="RFW_ENABLE_X" value="" />
<input type="hidden" id="DEVICE_TYPE" name="DEVICE_TYPE" value="" />
<input type="hidden" id="dm_radio_date_x" name="dm_radio_date_x" value="">
<input type="hidden" id="dm_radio_time_x" name="dm_radio_time_x" value="">
<input type="hidden" id="dm_radio_time2_x" name="dm_radio_time2_x" value="">
<!--<input type="hidden" class="input_12_table" maxlength="12" name="Base_path" id="Base_path" />-->

<table class="content" align="center" cellpadding="0" cellspacing="0">
  <tr>
	<td width="17">&nbsp;</td>
	<!--=====Beginning of Main Menu=====-->
	<td valign="top" width="202">
	  <div id="mainMenu"></div>
	  <div id="subMenu"></div>
	</td>
	
	<td valign="top" align="left">
	  <!--  delete by alan <div id="tabMenu" class="submenuBlock"></div>-->
	  
	  <!--===================================Beginning of Main Content===========================================-->
      
      <div style="margin-top:-150px; padding-left:0px;">   
<table width="98%" border="0" align="left" cellpadding="0" cellspacing="0">
	<tr>
		<td align="left" valign="top">
			<table width="760" border="0" cellspacing="0" class="FormTitle" id="FormTitle">			
			<tbody valign="top">
				<tr>
		  			<td bgcolor="#4D595D">
		  			<div>&nbsp;</div>
		  			<!--<div class="formfonttitle"><#menu5_3#> - <#menu5_3_1#></div>    delete by alan-->
                    <div class="formfonttitle" id="multiSetting_4"></div>   <!-- add by alan -->
		  			<div style="margin-left:5px;margin-top:10px;margin-bottom:10px"><img src="images/New_ui/export/line_export.png"></div>
					</td>
				</tr>
				
				<tr>
					<td bgcolor="#4D595D" id="ip_sect">
						<table  width="100%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3" class="FormTable">
							<thead>
							<tr>
								<!--<td colspan="2"><#IPConnection_ExternalIPAddress_sectionname#></td>  delete by alan-->
                                <td colspan="2" id="Download_Schedule"></td>  <!-- add by alan -->
							</tr>
             <!-- add by alan -->             
                            </thead>
  <tr bgcolor="#4D595D">
                            	<td colspan="2"><input name="Enable_time" type="radio" class="input" value="0" id="Enable_time0" onclick="Download_Schedule(this.value);"/><span id="Enable_time0_desc"></span>
                                <input name="Enable_time" type="radio" class="input" value="1" id="Enable_time1" onclick="Download_Schedule(this.value);"/><span id="Enable_time1_desc"></span><span id="timezone_hint" style="color:#FF9900;display:none;"></span></td>
                            </tr>
                            <tbody style="display:none;" id="Time_Schedule">
                            <tr id="enable_date_week_tr">
			  			<th id="enable_date_week_desc"></th>
			  			<td>
							<input type="checkbox" class="input" id="dm_radio_date_x_Mon" name="dm_radio_date_x_Mon" >Mon
							<input type="checkbox" class="input" id="dm_radio_date_x_Tue" name="dm_radio_date_x_Tue" >Tue
							<input type="checkbox" class="input" id="dm_radio_date_x_Wed" name="dm_radio_date_x_Wed" >Wed
							<input type="checkbox" class="input" id="dm_radio_date_x_Thu" name="dm_radio_date_x_Thu" >Thu
							<input type="checkbox" class="input" id="dm_radio_date_x_Fri" name="dm_radio_date_x_Fri" >Fri	
			<span id="blank_warn" style="display:none; color:#FF9900;">You cannot leave this field blank!</span>						
			  			</td>
					</tr>
					<tr id="enable_time_week_tr">
			  			<th id="enable_time_week_desc">Time of Day to Enable Download</th>
			  			<td>
			  				<input type="text" maxlength="2" class="input_3_table" id="dm_radio_time_x_starthour" name="dm_radio_time_x_starthour" onKeyPress="return is_number(this,event)" onblur="validate_timerange(this, 0);">:
							<input type="text" maxlength="2" class="input_3_table" id="dm_radio_time_x_startmin" name="dm_radio_time_x_startmin" onKeyPress="return is_number(this,event)" onblur="validate_timerange(this, 1);">-
							<input type="text" maxlength="2" class="input_3_table" id="dm_radio_time_x_endhour" name="dm_radio_time_x_endhour" onKeyPress="return is_number(this,event)" onblur="validate_timerange(this, 2);">:
							<input type="text" maxlength="2" class="input_3_table" id="dm_radio_time_x_endmin" name="dm_radio_time_x_endmin" onKeyPress="return is_number(this,event)" onblur="validate_timerange(this, 3);">
						</td>
					</tr>
					<tr id="enable_date_weekend_tr">
			  			<th id="enable_date_weekend_desc">Date to Enable Download (weekend)</th>
			  			<td>
		<input type="checkbox" class="input" id="dm_radio_date_x_Sat" name="dm_radio_date_x_Sat" >Sat
		<input type="checkbox" class="input" id="dm_radio_date_x_Sun" name="dm_radio_date_x_Sun">Sun	
		<span id="blank_warn" style="display:none; color:#FF9900;">You cannot leave this field blank!</span>					
			  			</td>
					</tr>
					<tr id="enable_time2_weekend_tr">
			  			<th id="enable_time2_weekend_desc">Time of Day to Enable Download</th>
			  			<td>
			  				<input type="text" maxlength="2" class="input_3_table" id="dm_radio_time2_x_starthour" name="dm_radio_time2_x_starthour" onKeyPress="return is_number(this,event)" onblur="validate_timerange(this, 0);">:
							<input type="text" maxlength="2" class="input_3_table" id="dm_radio_time2_x_startmin" name="dm_radio_time2_x_startmin" onKeyPress="return is_number(this,event)" onblur="validate_timerange(this, 1);">-
							<input type="text" maxlength="2" class="input_3_table" id="dm_radio_time2_x_endhour" name="dm_radio_time2_x_endhour" onKeyPress="return is_number(this,event)" onblur="validate_timerange(this, 2);">:
							<input type="text" maxlength="2" class="input_3_table" id="dm_radio_time2_x_endmin" name="dm_radio_time2_x_endmin" onKeyPress="return is_number(this,event)" onblur="validate_timerange(this, 3);">
						</td>
					</tr>
                            
						</tbody>
                        </table>
					</td>
	  		</tr>
	  <tr>
	    <td bgcolor="#4D595D" id="isp_sect">
		<table width="100%" border="1" style="border-top:hidden;" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3"  class="FormTable">
			<!--<thead>
				<tr>-->
            <!--<td colspan="2"><#PPPConnection_x_HostNameForISP_sectionname#></td> delete by alan -->
            <!--<td colspan="2">Other</td>
            </tr>
		</thead>-->
        <tr>
          <th id="multiSetting_5"></th>
          <td>
          <input type="text" id="PATH" class="input_25_table" value="" name="Download_dir" readonly="readonly"/>
          <input type="button" class="button_gen" id="multiSetting_6" value="" onclick="get_disk_tree();"/>
          </td>
        </tr>
        <tr>
          <th id="multiSetting_7"></th>
          <td><input type="text" class="input_12_table" maxlength="12" name="Refresh_rate" id="Refresh_rate" />
          <span id="multiSetting_8" style="color:#FFF;"></span>
          </td>
        </tr>
          <tr>
          <th id="multiSetting_11"></th>
          <td><input type="text" class="input_6_table" maxlength="5" name="DM_port" id="DM_port" onkeyup="judgeDigit();" /></td>
        </tr>
		<tr>
          <th id="multiSetting_20"></th>
          <td><input type="text" class="input_6_table" maxlength="5" name="DM_https_port" id="DM_https_port" onkeyup="judgeDigit2();" /></td>
        </tr>
         <tr id="Misc_http_x_tr">
                <th id="multiSetting_9"></th>
                    <td>
                    <div class="left" style="width: 74px;height: 32px;" id="misc_http_x_1" title=""></div>
                    <div style="position:absolute; margin-left:100px; margin-top:-25px;">
                    <span id="multiSetting_10"></span>
                    </div><input type="hidden" name="Misc_http_x" id="Misc_http_x" value="" />					
                    </td>
                </tr>
         <tr>
                <th id="multiSetting_18"></th>
                    <td>
                    <div class="left" style="width: 74px;height: 32px;" id="misc_seeding_x_1" title=""></div>
                    <div style="position:absolute; margin-left:100px; margin-top:-25px;">
                    <span id="multiSetting_19"></span>
                    </div><input type="hidden" name="Misc_seeding_x" id="Misc_seeding_x" value="" />					
                    </td>
                </tr>
      </table>
      <div id="WAN_explain_1" style="display:none;">
      &nbsp;<span id="multiSetting_13"> On your web browser, key in </span>
      <a href="javascript:void(0);" style="color:#FF9900;" id="WANforDM">http://<span id="DDNSname"></span>:<span id="DMport"></span>/downloadmaster/index.asp</a>
	  <a href="javascript:void(0);" style="color:#FF9900;" id="WANforDM1">https://<span id="DDNSname1"></span>:<span id="DMhttpsport"></span>/downloadmaster/index.asp</a>
		<span id="multiSetting_14"> to launch Download Master.</span><br />
      </div>
      <div id="WAN_explain_2" style="display:none;">
      &nbsp;<a href="javascript:void(0);" id="to_DDNS" style="color:#FF9900;"> You can click here to setup the DDNS.</a>
		<span id="multiSetting_15"> DDNS is a service that allow network clients to connect to the wireless router, even with a dynamic public IP address, through its registered domain name.</span>
      </div>
      
		<div class="apply_gen"> 
			<!-- delete by alan  <input class="button_gen" onclick="applyRule();" type="button" value="<#CTL_apply#>"/> delete by alan -->
			<input id="multiSetting_12" class="button_gen" onclick="setTimeout('GeneralapplyRule()',500);" type="button" value=""/>
			<!-- add by alan -->
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
        </td>
        
		<!--===================================Ending of Main Content===========================================-->
	
    <td width="10" align="center" valign="top">&nbsp;</td>
	</tr>
</table>

<div id="footer"></div>
<div id="OverlayMask" class="popup_bg">
	<div align="center">
	<iframe src="" frameborder="0" scrolling="no" id="popupframe" width="400" height="400" allowtransparency="true" style="margin-top:150px;"></iframe>
	</div>
<script>
$j("#multiSetting_0").html(multiLanguage_setting_array[multi_INT][0]);
$j("#multiSetting_1").html(multiLanguage_setting_array[multi_INT][1]);
$j("#multiSetting_2").attr("value",multiLanguage_setting_array[multi_INT][2]);
$j("#multiSetting_3").attr("value",multiLanguage_setting_array[multi_INT][3]);
$j("#multiSetting_16").attr("value",multiLanguage_setting_array[multi_INT][2]);
$j("#multiSetting_17").attr("value",multiLanguage_setting_array[multi_INT][3]);
$j("#multiSetting_4").html(multiLanguage_setting_array[multi_INT][4]);
$j("#multiSetting_5").html(multiLanguage_setting_array[multi_INT][5]);
$j("#multiSetting_6").attr("value",multiLanguage_setting_array[multi_INT][6]);
$j("#multiSetting_7").html(multiLanguage_setting_array[multi_INT][7]);
$j("#multiSetting_8").html(multiLanguage_setting_array[multi_INT][8]);
$j("#multiSetting_9").html(multiLanguage_setting_array[multi_INT][9]);
if(fw_enable==0){
$j("#multiSetting_10").html(multiLanguage_setting_array[multi_INT][102]);
$j("#multiSetting_10").attr("class","color_red");
}else{
$j("#multiSetting_10").html(multiLanguage_setting_array[multi_INT][10]);
$j("#multiSetting_10").attr("class","color_white");
}
$j("#multiSetting_11").html(multiLanguage_setting_array[multi_INT][11]);
$j("#multiSetting_12").attr("value",multiLanguage_setting_array[multi_INT][2]);
$j("#multiSetting_13").html(multiLanguage_setting_array[multi_INT][98]);
$j("#multiSetting_14").html(multiLanguage_setting_array[multi_INT][99]);
$j("#to_DDNS").html(multiLanguage_setting_array[multi_INT][100]);
$j("#multiSetting_15").html(multiLanguage_setting_array[multi_INT][101]);
$j("#multiSetting_18").html(multiLanguage_setting_array[multi_INT][112]);
$j("#multiSetting_20").html(multiLanguage_setting_array[multi_INT][122]);
$j("#Download_Schedule").html(multiLanguage_setting_array[multi_INT][75]);
$j("#Enable_time0_desc").html(multiLanguage_setting_array[multi_INT][76]);
$j("#Enable_time1_desc").html(multiLanguage_setting_array[multi_INT][77]);
$j("#enable_date_week_desc").html(multiLanguage_setting_array[multi_INT][80]);
$j("#enable_time_week_desc").html(multiLanguage_setting_array[multi_INT][81]);
$j("#enable_date_weekend_desc").html(multiLanguage_setting_array[multi_INT][82]);
$j("#enable_time2_weekend_desc").html(multiLanguage_setting_array[multi_INT][83]);
</script>
</body>
</html>
