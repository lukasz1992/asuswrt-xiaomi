<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml" xmlns:v="">
<!--<html xmlns:v>-->
<head>
<meta http-equiv="X-UA-Compatible" content="IE=EmulateIE7" />
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
<meta HTTP-EQUIV="Pragma" CONTENT="no-cache">
<meta HTTP-EQUIV="Cache-Control" CONTENT="no-cache">
<meta HTTP-EQUIV="Expires" CONTENT="-1">
<link rel="shortcut icon" href="images/favicon.png">
<link rel="icon" href="images/favicon.png">
<title>Download Master</title>
<link rel="stylesheet" type="text/css" href="index_style.css">
<link rel="stylesheet" type="text/css" href="form_style.css">
<link rel="stylesheet" type="text/css" href="NM_style.css">
<link rel="stylesheet" type="text/css" href="other.css">
<link rel="stylesheet" type="text/css" href="ext/css/ext-all.css">
<!--<link href="multiLanguageCss/english.css" rel="stylesheet" type="text/css" id="languageCss" />
<link href="multiLanguageCss/english_1.css" rel="stylesheet" type="text/css" id="languageCss_1" />-->
<style type="text/css">
.back-to-top-fixed{
	position:fixed;
	bottom:50px;
	margin-left:5%;
	_position:absolute;
	font-family:verdana;
	font-size:12px;
	padding:0.2em 0.4em;
	border:1px solid #999;
	text-decoration:none;
}

.style1 {color: #00AADD}
.style4 {color: #333333}
.style5 {
	color: #CC0000;
	font-weight: bold;
}
a:focus{outline:none;}

#AddTask{
	width:38px;
	height:54px;
	border:none;
}
#AddTask img{
	width:32px;
	height:32px;
	border:none;
}
#AddTask td{
	font-size: 12px;
	font-style:normal;
	font-weight:normal;
	text-decoration:none;
	border:none;
}

#RemoveTask{
	width:45px;
	height:54px;
	z-index:20;
}
/*#RemoveTask img{
	width:32px;
	height:32px;
	border:1px solid #000;
}*/
#RemoveTask td{
	font-size: 12px;
	font-style:normal;
	font-weight:normal;
	text-decoration:none;
	border:none;
}


#PauseTask{
	width:55px;
	height:54px;
	z-index:21;
}
#PauseTask img{
	width:32px;
	height:32px;
	border:none;
}
#PauseTask td{
	font-size: 12px;
	font-style:normal;
	font-weight:normal;
	text-decoration:none;
	border:none;
}

#ResumeTask{
	width:43px;
	height:54px;
	z-index:22;
}
#ResumeTask img{
	width:32px;
	height:32px;
	border:none;
}
#ResumeTask td{
	font-size: 12px;
	font-style:normal;
	font-weight:normal;
	text-decoration:none;
	border:none;
}


#PauseAllTask{
	width:65px;
	height:54px;
	z-index:23;
}
#PauseAllTask img{
	width:32px;
	height:32px;
	border:none;
}
#PauseAllTask td{
	font-size: 12px;
	font-style:normal;
	font-weight:normal;
	text-decoration:none;
	border:none;
}
	
#ResumeAllTask{
	width:75px;
	height:54px;
	z-index:24;
}
#ResumeAllTask img{
	width:32px;
	height:32px;
	border:none;
}
#ResumeAllTask td{
	font-size: 12px;
	font-style:normal;
	font-weight:normal;
	text-decoration:none;
	border:none;
}


#ClearCompletedTask{
	width:105px;
	height:54px;
	z-index:85;
}
#ClearCompletedTask img{
	width:32px;
	height:32px;
	border:none;
}
#ClearCompletedTask td{
	font-size: 12px;
	font-style:normal;
	font-weight:normal;
	text-decoration:none;
	border:none;
}


#Home{
	width:72px;
	height:54px;
	z-index:24;
}
#Home img{
	width:32px;
	height:32px;
	border:none;
}
#Home td{
	font-size: 12px;
	font-style:normal;
	font-weight:normal;
	text-decoration:none;
	border:none;
}
#Help{
	width:72px;
	height:54px;
	z-index:24;
}
#Help img{
	width:32px;
	height:32px;
	border:none;
}
#Help td{
	font-size: 12px;
	font-style:normal;
	font-weight:normal;
	text-decoration:none;
	border:none;
}

.transfers{
	width:110px;
	
}

#downloadSpeed{
	width:90px;
}
#downloadSpeed img{
	width:15px;
	height:15px;	
}

#uploadSpeed{
	width:90px;
}
#uploadSpeed img{
	width:15px;
	height:15px;
}

#taskLog{
	width:246px;
	height:400px;
	border:#000 1px solid;
}

#taskLogFilename{width:246px; border-left:#000 1px solid; border-right:#000 1px solid;}

.taskLogIcon{
	width:248px;
	height:44px;
	border-left:#000 1px solid;
	border-right:#000 1px solid;
	border-top:#000 1px solid;
	border-bottom:#000 1px solid;
	}

.icon{	
	width:40px;
	height:40px;
	+position:relative;
	margin-left:2px;
	float:center;
	text-decoration:none;
}
.iconnewa{	
	width:40px;
	+width:65px;
	
	margin-left:5px;
	float:center;
	text-decoration:none;
}
.iconnew7{	
	width:100px;
	
	margin-left:18px;
	float:center;
	text-decoration:none;
}
.iconnew6{	
	width:100px;
	
	margin-left: -10px;
	float:center;
	text-decoration:none;
}
.iconnew5{	
	width:100px;
	
	margin-left:-10px;
	float:center;
	text-decoration:none;
}
.iconnew4{	
	width:100px;
	[width:80px;]
	margin-left:-10px;
	+margin-left:-5px;
	float:center;
	text-decoration:none;
}
.iconnew3{	
	width:100px;
	
	margin-left:10px;
	float:center;
	text-decoration:none;
}
.iconnew2{	
	width:100px;
	
	margin-left:30px;
	+margin-left:1px;
	float:center;
	text-decoration:none;
}
.iconnew1{	
	width:40px;
	+width:65px;
	margin-right:5px;
	+margin-left:1px;

	float:center;
	text-decoration:none;
}
.iconnew a:link, .iconnew a:visited{
	cursor:pointer;
	display: block;
	width:40px;	
	height:40px;
}
.icon a:link, .icon a:visited{
	cursor:pointer;
	display: block;
	width:40px;	
	height:40px;
}

#icon_add{background:url(images/icon/add_disable.png);}
#icon_add a:link, #icon_add a:visited, #icon_add a:hover{background:url(images/icon/add.png);}
#icon_add a:active{background:url(images/icon/add_click.png);}

#icon_del{background:url(images/icon/delete_disable.png);}
#icon_del a:link, #icon_del a:visited, #icon_del a:hover{ background:url(images/icon/delete.png); }
#icon_del a:active{background:url(images/icon/delete_click.png);}

#icon_pause{background:url(images/icon/pause_disable.png);}
#icon_pause a:link, #icon_pause a:visited, #icon_pause a:hover{background:url(images/icon/pause.png);}
#icon_pause a:active{background:url(images/icon/pause_click.png);}

#icon_resume{background:url(images/icon/resume_disable.png);}
#icon_resume a:link, #icon_resume a:visited, #icon_resume a:hover{background:url(images/icon/resume.png);}
#icon_resume a:active{background:url(images/icon/resume_click.png);}

#icon_pause_all{background:url(images/icon/pause_all_disable.png);}
#icon_pause_all a:link, #icon_pause_all a:visited, #icon_pause_all a:hover{background:url(images/icon/pause_all.png);}
#icon_pause_all a:active{background:url(images/icon/pause_all_click.png);}

#icon_resume_all{background:url(images/icon/resume_all_disable.png);}
#icon_resume_all a:link, #icon_resume_all a:visited, #icon_resume_all a:hover{background:url(images/icon/resume_all.png);}
#icon_resume_all a:active{background:url(images/icon/resume_all_click.png);}

#icon_clear{background:url(images/icon/clear_disable.png);}
#icon_clear a:link, #icon_clear a:visited, #icon_clear a:hover{background:url(images/icon/clear.png);}
#icon_clear a:active{background:url(images/icon/clear_click.png);}

#icon_home{background:url(images/icon/home_disable.png);}
#icon_home a:link, #icon_home a:visited, #icon_home a:hover{background:url(images/icon/home.png);}
#icon_home a:active{background:url(images/icon/home_click.png);}

#icon_help{background:url(images/icon/question.png);}

.panel{
	background-color:#999;	
	border:2px outset #CCC;
	font-size:15px;
	font-family:Verdana, Geneva, sans-serif;
	color:#333333;
	width:450px;
	position:fixed;
	margin-top:-8%;
	margin-left:35%;
	z-index:200;
	display:none;
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

.form_table{
	position:relative;
	display:table;
}
.form_table thead td{
	background-image:url(images/panel_tbg.gif);
	height:25px;
	padding-left:5px;
}

.form_table input{
	font-size:15px;
	font-family:Verdana, Geneva, sans-serif;
}

.form_table span{
	font-size:18px;
	font-family:Verdana, Geneva, sans-serif;
	}

.form_table tfoot td{
	text-align:right;
	height:30px;
	line-height:30px;
	padding-right:5px;
}

.taskLongWidth{width:720px;}
.taskShortWidth{width:480px;}

#inspector_tab_info {
	/*padding-left:18px;*/
	/*margin-left:18px;*/
	/*padding: 3px 7px;
	-webkit-border-top-left-radius: 5px;
	-webkit-border-top-right-radius: 0px;
	-webkit-border-bottom-right-radius: 0px;
	-webkit-border-bottom-left-radius: 5px;
	-moz-border-radius-topleft: 5px;
	-moz-border-radius-topright: 0px;
	-moz-border-radius-bottomright: 0px;
	-moz-border-radius-bottomleft: 5px;
	border-top-left-radius: 5px;
	border-top-right-radius: 0px;
	border-bottom-right-radius: 0px;
	border-bottom-left-radius: 5px;*/
}

.selected {
	background: #333;
}


.selected2 {font-weight:bold; cursor:default; color: #F90}
.noselected {cursor:pointer;}
.inspector_tab {
	height: 17px;
	cursor: default;
	/*margin-left: 18px;*/
}
#inspector_tab_activity {
	float:left;
	/*margin-right:18px;*/
	/*padding: 3px 8px;*/
	/*padding-right:18px;*/
}
#inspector_tab_peers{
	float:left;
	/*padding: 3px 8px;*/
}
/*#inspector_tab_files {
	padding: 3px 8px;
	border: 1px solid #aaa;
	-webkit-border-top-left-radius: 0px;
	-webkit-border-top-right-radius: 5px;
	-webkit-border-bottom-right-radius: 5px;
	-webkit-border-bottom-left-radius: 0px;
	-moz-border-radius-topleft: 0px;
	-moz-border-radius-topright: 5px;
	-moz-border-radius-bottomright: 5px;
	-moz-border-radius-bottomleft: 0px;
	border-top-left-radius: 0px;
	border-top-right-radius: 5px;
	border-bottom-right-radius: 5px;
	border-bottom-left-radius: 0px;
}*/
/*.greyrow{background-color:#4d595d; border-top-color:#000; border-bottom-color:#000;!important}*/

</style>
<script type="text/javascript" src="jquery.js?ver=104"></script>
<script type="text/javascript" src="jquery-scrolltotop.min.js?ver=104"></script>
<script>
var $j = jQuery.noConflict();
var multi_INT = 0;
var refreshTime_data_tmp;
var LAN_IP="";
var LOCAL_DOMAIN="";
var AUTOLOGOUT_MAX_MINUTE_TMP=0;
var IS_FAT32=0;
var httpTag = 'https:' == document.location.protocol ? false : true;
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
	refreshTime_data_tmp = data;
	var lang = array[14];
	LAN_IP= array[10];
	LOCAL_DOMAIN= array[29];
	AUTOLOGOUT_MAX_MINUTE_TMP=array[30];
	var disk= array[24];
	if(disk == "tfat" || disk == "vfat") {
		IS_FAT32 = 1;
	}
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
<script type="text/javascript" src="popup.js?ver=104"></script>
<!--<script type="text/javascript" src="disk_functions.js?ver=104"></script>-->
<!--<script type="text/javascript" src="client_function.js?ver=104"></script>-->
<!--<script type="text/javascript" src="help.js?ver=104"></script>-->
<script type="text/javascript" src="detect.js?ver=104"></script>
<script type="text/javascript" src="alttxt.js?ver=104"></script>
<!--<script type="text/javascript" src="aplist.js?ver=104"></script>-->
<script type="text/javascript" src="ext/ext-base.js?ver=104"></script>
<script type="text/javascript" src="ext/ext-all.js?ver=104"></script>
<script type="text/javascript" src="dm_function.js?ver=104"></script>
<script type="text/javascript" src="plugin/jquery.iphone-switch.js?ver=104"></script>
<script>
// for client_function.js?ver=104
function initial(){
	show_menu();
	showTask();
	initial_Refresh_time(refreshTime_data_tmp);
	AUTOLOGOUT_MAX_MINUTE=AUTOLOGOUT_MAX_MINUTE_TMP;
	updateStatus_AJAX();
	//document.getElementById("select_lang_"+multi_INT).selected = true;
	if(location.search)
	UtilityFlag();
}

//function switch_Download(value) {
	//if (value == 0){
	//$j("#HTTP_usb_dm_url").readOnly = true;
	//$j("#open_usb_dm_url").readOnly = false;
	//}
	/*else if(value == 1){
		$j("#open_a_file_panel").hide();
		$j("#HTTP_panel").hide();
		$j("#magnet_panel").hide();
		$j("#enter_url_panel").show();
		}*/
	//else if(value == 2){
		//$j("#open_usb_dm_url").readOnly = true;
		//$j("#open_usb_dm_url").readOnly = false;
		//}
	/*else{
		$j("#open_a_file_panel").hide();
		$j("#enter_url_panel").hide();
		$j("#HTTP_panel").hide();
		$j("#magnet_panel").show();
		}*/
//	}
</script>

</head>

<body onunload="return unload_body();" onload="initial();">
<noscript>
	<div class="popup_bg" style="visibility:visible; z-index:999;">
		<div style="margin:200px auto; width:300px; background-color:#006699; color:#FFFFFF; line-height:150%; border:3px solid #FFF; padding:5px;"></p></div>
	</div>
</noscript>
<div id="TopBanner"></div>
<div id="DM_mask" class="mask_bg"></div><!--[if lte IE 6.5]><iframe class="hackiframe"></iframe><![endif]-->
<div id="Loading" class="popup_bg"></div>
 <!--Start of Add Panel-->
		<div id="panel_add" class="panel" >
        	<span style="margin-left:50px;"><b id="multi_1"></b></span>
            <br /><br />
			<!--<input id="enter_url" type="radio" name="addTask" value="1" onclick="switch_Download(this.value);" /><b>FTP</b>-->
            <!--<input id="magnet" type="radio" name="addTask" value="3" onclick="switch_Download(this.value);" /><b>magnet</b>-->
            <!-- open_a_file_panel start!!-->
            <div id="open_a_file_panel">
            <span style="margin-left:8px;"><b id="multi_2"></b></span>
            <br />
            <form target="hidden_frame" name="open_a_file_form" id="open_a_file_form" method="post" action="dm_uploadbt.cgi" encType="multipart/form-data">
			<table width="100%"  border="0" cellspacing="1" cellpadding="0" class="form_table">
			  <tr>
				<td>		
				  	<div id="open_usb_dm_url_div" style="display:block; float:left;">
                    <span style="margin-right:15px;margin-left:15px;"><b></b></span>
                    <input type="file" id="open_usb_dm_url" value="" name="filename" /><br /><br /></div>
				</td>
			  </tr> 
			</table>
             </form>
            <span style="margin-left:8px;"><b id="multi_4"></b></span>
            <br />
            <form name="enter_url_form" method="post" action="">
			<table width="100%"  border="0" cellspacing="1" cellpadding="0" class="form_table">
			  <tr>
				<td>		
				  	<div style="display:block; float:left;"><textarea style="width:400px;margin-left:30px;" id="HTTP_usb_dm_url" value="" rows="2"></textarea></div>
				</td>
			  </tr>
			</table>
             </form>
			<table width="440px"  border="0" cellspacing="1" cellpadding="0" class="form_table">			 
			  <tfoot>
			  <tr>
				<td>
				<input id="multi_5" type="button" name="AddFile" value="" style="margin-left:250px;" onclick="dm_add_status();">
                &nbsp;&nbsp;
				<input id="multi_6" type="button" name="Cancel_panel_add" value="" onClick="hidePanel();">
				</td>
			  </tr>
			  </tfoot>
			</table>
            </div>
		</div>
		<!--End of Add Panel-->
        
 <!--Start of UnamePsw Panel-->
 <div class="panel" id="UnamePsw_panel">
			<table width="100%"  border="0" cellspacing="1" cellpadding="0" class="form_table">
              <tr>
              	<td colspan="2">
                	<span style="margin-left:10px;" id="multi_16">Please enter the Username and Password</span>
                </td>
              </tr>
              <tr>
              	<td height="40px" colspan="2">
                	<span style="margin-left:30px; margin-right:30px" id="multi_17">User name</span>
                    <input id="User_name" type="text" width="280px" value="" />
                </td>
              </tr>
              <tr>
              	<td colspan="2">
                	<span style="margin-left:30px; margin-right:43px" id="multi_18">Password</span>
                    <input id="Password" type="password" width="280px" value="" />
                </td>
              </tr>
			</table>

			<br/>
			<table width="400px"  border="0" cellspacing="1" cellpadding="0" class="form_table">			 
			  <tfoot>
			  <tr>
				<td>
				<input type="button" name="AddFile_1" value="Apply" style="margin-left:250px;" onclick="Ftp_dm_add_status();">
				<input type="button" name="Cancel_panel_add_1" value="Cancel" onClick="hideUnamePsw();">
				</td>
			  </tr>
			  </tfoot>
			</table>
 </div>
<!--End of UnamePsw Panel-->

<iframe name="hidden_frame" id="hidden_frame" width="0" height="0" frameborder="0" scrolling="no"></iframe>

<form name="form">
<input type="hidden" name="current_page" value="index.asp" />
</form>


<table class="content" align="center" cellpadding="0" cellspacing="0">
  <tr>
	<td valign="top" width="17">&nbsp;</td>
		
		<!--=====Beginning of Main Menu=====-->
		<td valign="top" width="204">
			<img src="images/up.png" id="back-to-top-fixed" class="back-to-top-fixed">
			<div id="mainMenu"></div>
			<div id="subMenu"></div>
		</td>
		
		<td align="left" valign="top">
		
		<!--=====Beginning of Network Map=====-->
		<div class="NM_table">
       
        <table cellpadding="0" cellspacing="0">
        <tbody>
        <tr>
        	<td><table width="720" cellpadding="0" cellspacing="0">

        	<tr> 
        	<td id="AddTask" align="center" >
            <div  class="icon" id="icon_add" title="Add" style="width: 40px;margin-left: 5px;+margin-left: 15px;"><a href="javascript:showPanel();"></a></div>	
            </td>

            <td id="RemoveTask" align="center" >
                <div class="icon" id="icon_del" title="Delete" style="width: 40px;margin-left:15px;+margin-left:45px;"><a></a></div>
            </td>

            <td id="PauseTask" align="center" >
               <div class="icon" id="icon_pause" title="Pause" style="width: 40px;margin-left:-10px;+margin-left:18px;"><a></a></div>
            </td>

            <td id="ResumeTask" align="center" >
                <div class="icon" id="icon_resume" title="Resume" style="width: 40px;margin-left:-10px;+margin-left:18px;"><a></a></div>
            </td>

            <td id="PauseAllTask" align="center"  >
                <div class="icon" id="icon_pause_all" title="Pause All" style="width: 40px;margin-left:-5px;+margin-left:5px;" ><a></a></div>
            </td>

            <td id="ResumeAllTask" align="center" >
                <div class="icon" id="icon_resume_all" title="Resume All" style="width: 40px;margin-left:10px;+margin-left:38px;"><a></a></div>
            </td>
           
            <td id="ClearCompletedTask" align="center" >
                <div class="icon" id="icon_clear" title="Clear" style="width: 40px;margin-left:30px;+margin-left:5px;"><a></a></div>
            </td>

            <td id="Home" align="center" >
                <div class="icon" id="icon_home" title="Home" style="width: 40px;margin-left:-5px;"><a id="homeAddress"></a></div>
            </td>
            </tr>
            <tr>
            	        <td align="center" valign="top" >
            	        <div class="iconnewa" >
            	        <span id="multi_7"></span></div>
                        </td>
                
                       	<td align="center" valign="top" >
                       	 <div class="iconnew7" >
                        <span id="multi_8"></span></div>
                        </td>

                        <td  align="center" valign="top" >
                         <div class="iconnew6" >
                        <span id="multi_9"></span></div>
                        </td>

                        <td  align="center" valign="top"  >
                        <div class="iconnew5" >
                        <span id="multi_10"></span></div>
                        </td>

                        <td align="center" valign="top" >
                         <div class="iconnew4" >
                        <span id="multi_11"></span></div>
                        </td>

                        <td  align="center" valign="top"  >
                         <div class="iconnew3" >
                        <span id="multi_12"></span></div> 
                        </td>

                        <td  align="center" valign="top" >
                        <div class="iconnew2">
                        <span id="multi_13"></span></div> 
                        </td>

                        <td align="center" valign="top" >
                         <div class="iconnew1" >
                        <span id="multi_14"></span></div>
                        </td>
                       
            

            </tr>
           
            </table></td></tr>
            <tr>
            	<td>
                	<div style="margin-left: 5px; margin-top: 10px; margin-bottom: 10px;">
                    	<img src="images/New_ui/export/line_export.png">
                    </div>
                </td>
            </tr>
            <tr>
            	<td>
                	<table>
                    	<tr>
                        	<td class="transfers">&nbsp;<span id="transfers">0</span>
                            	<span id="multi_15">Transfers</span>
                            </td>
                            <td style="width:400px;">
                            <div id="taskClass">
                            <span class="selected2" id="All_tasks" onclick="showTask_select(0);"></span> | 
                            <span class="noselected" id="Downloading_tasks" onclick="showTask_select(1)"></span> | 
                            <span class="noselected" id="Seeding_tasks" onclick="showTask_select(2)"></span> |
                            <span class="noselected" id="Paused_tasks" onclick="showTask_select(3)"></span> |
                            <span class="noselected" id="Finished_tasks" onclick="showTask_select(4)"></span>
                            </div>
                            </td>
                            <td id="downloadSpeed"><img src="images/icon/DownArrowIcon.png" />
                            <span id="total_downloadSpeed">0</span> KBps</td>
                            <td id="uploadSpeed"><img src="images/icon/UpArrowIcon.png" />
                            <span id="total_uploadSpeed">0</span> KBps</td>
                        </tr>
                    </table>
                </td>
            </tr>
            <tr>
            	<td>
                	<div style="margin-left: 5px; margin-bottom: 10px;">
                    	<img src="images/New_ui/export/line_export.png">
                    </div>
                </td>
            </tr>
            <tr>	
            	<td>
                	<table width="750">
                    	<tr>
                        	<td valign="top">
                            	<div style="word-break:break-all;" id="TaskMain">


                                        	</div>
                                    	</td>
                                        <td valign="top">&nbsp;</td>
                                        <td valign="top" style="display:none;" id="taskLog_show">
                                        	<table class="taskLogIcon">
                                            <tbody>
                                            	<tr>
                                                	<td width="80">
                                                    	<div id="inspector_tab_info" class="inspector_tab" title="Information">
                                                        <a href="javascript:select_taskType_general();">
                                                        	<img id="SeedInformationIcon" src="images/icon/Info_general.png" onmousedown="changeInfoIcon('images/icon/Info_general_click.png',this.id);" style="margin-left:20px;" onmouseup="returnInfoIcon('images/icon/Info_general.png',this.id);" />
                                                        </a>
                                                        </div>
                                                    </td>
                                                    <td>
                                                    	<div id="inspector_tab_activity" class="inspector_tab" title="Download activity">
                                                        <a href="javascript:select_taskType_transfer();">
                                                        	<img id="DownloadActivityIcon" src="images/icon/Info_activity.png" onmousedown="changeInfoIcon('images/icon/Info_activity_click.png',this.id);" style="margin-left:20px;" onmouseup="returnInfoIcon('images/icon/Info_activity.png',this.id);" />
                                                        </a>
                                                        </div>
                                                    </td>
                                                    <td>
                                                    	<div id="inspector_tab_peers" class="inspector_tab" title="Tracker information">
                                                        	<a href="javascript:select_taskType_links();">
                                                            	<img id="TrackerInformationIcon" onmousedown="changeInfoIcon('images/icon/Info_trackers_click.png',this.id);" alt=" " style="margin-left:18px;" onmouseup="returnInfoIcon('images/icon/Info_trackers.png',this.id);" src="images/icon/Info_trackers.png" />
                                                            </a>
                                                        </div>
                                                    </td>
                                                    <!--<td>
                                                    	<div id="inspector_tab_files" style="margin-right:12px;" class="inspector_tab">
                                                    		<a>
                                                            	<img src="images/icon/info_files.png" />
                                                            </a>
                                                    	</div>
                                                    </td>-->
                                                </tr>
                                            </tbody>
                                            </table>
                                            <div id="taskLogFilename" style="word-wrap:break-word;word-break:break-all;"></div>
                            				<div id="taskLog" style="word-wrap:break-word;word-break:break-all; overflow:auto;"></div>
                           			 	</td>
                                    </tr>
                                </table>
                            </td>
                            
                        </tr>
            </tbody>
            </table>
            </div>
        </td>
<!--==============Ending of hint content=============-->
  </tr>
</table>
<div id="navtxt" class="navtext" style="position:absolute; top:50px; left:-100px; visibility:hidden; font-family:Arial, Verdana"></div>
<div id="footer"></div>
<script>
$j("#multi_1").html(multiLanguage_array[multi_INT][16]);
$j("#multi_2").html(multiLanguage_array[multi_INT][17]);
$j("#multi_3").html(multiLanguage_array[multi_INT][18]);
$j("#multi_4").html(multiLanguage_array[multi_INT][19]);
$j("#multi_5").attr("value",multiLanguage_array[multi_INT][20]);
$j("#multi_6").attr("value",multiLanguage_array[multi_INT][21]);
$j("#multi_7").html(multiLanguage_array[multi_INT][22]);
$j("#multi_8").html(multiLanguage_array[multi_INT][23]);
$j("#multi_9").html(multiLanguage_array[multi_INT][24]);
$j("#multi_10").html(multiLanguage_array[multi_INT][25]);
$j("#multi_11").html(multiLanguage_array[multi_INT][26]);
$j("#multi_12").html(multiLanguage_array[multi_INT][27]);
$j("#multi_13").html(multiLanguage_array[multi_INT][28]);
$j("#multi_14").html(multiLanguage_array[multi_INT][29]);
$j("#multi_15").html(multiLanguage_array[multi_INT][30]);
$j("#multi_16").html(multiLanguage_array[multi_INT][80]);
$j("#multi_17").html(multiLanguage_array[multi_INT][81]);
$j("#multi_18").html(multiLanguage_array[multi_INT][82]);
$j("#All_tasks").html(multiLanguage_array[multi_INT][31]);
$j("#Downloading_tasks").html(multiLanguage_array[multi_INT][32]);
$j("#Seeding_tasks").html(multiLanguage_array[multi_INT][33]);
$j("#Paused_tasks").html(multiLanguage_array[multi_INT][34]);
$j("#Finished_tasks").html(multiLanguage_array[multi_INT][35]);
	/*$j('#required_switch').iphoneSwitch('1', 
							 function() {
								 document.getElementById('User_name').disabled= false;
								 document.getElementById('Password').disabled= false;
							 },
							 function() {
								document.getElementById('User_name').disabled= true;
								document.getElementById('Password').disabled= true;
							 },
							 {
							 }
						);*/
</script>
<script type="text/javascript">
	$j('#back-to-top-fixed').scrollToTop();
	$j('#back-to-top-fixed').attr({title:'Scroll Back to Top'});
</script>
</body>
</html>
