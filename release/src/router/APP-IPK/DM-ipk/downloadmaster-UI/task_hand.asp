<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<head>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
<meta HTTP-EQUIV="Pragma" CONTENT="no-cache">
<meta HTTP-EQUIV="Cache-Control" CONTENT="no-cache, must-revalidate">
<meta HTTP-EQUIV="Expires" CONTENT="-1">
<meta name="viewport" content="width=device-width; initial-scale=1.0; maximum-scale=1.0; user-scalable=0;"/>
<link rel="shortcut icon" href="images/favicon.png">
<link rel="icon" href="images/favicon.png">
<title>Download Master</title>
<link rel="stylesheet" type="text/css" href="index_style_hand.css">
<link rel="stylesheet" type="text/css" href="NM_style.css">
<link rel="stylesheet" type="text/css" href="other.css">
<link rel="stylesheet" type="text/css" href="ext/css/ext-all.css">
<!--<link href="multiLanguageCss/english.css" rel="stylesheet" type="text/css" id="languageCss" />
<link href="multiLanguageCss/english_1.css" rel="stylesheet" type="text/css" id="languageCss_1" />-->
<style type="text/css">
.style1 {color: #00AADD}
.style4 {color: #333333}
.style5 {
	color: #CC0000;
	font-weight: bold;
}
a:focus{outline:none;}
.NM_table{
	/*padding:10px; 
	padding-top:20px;*/
	padding-left:0px;
	position:relative;
	width:98%;
	background-color:#4d595d;
	border-width:0;
	/*margin-top:-150px;
	margin:0;	//want to use css to replace cellspacing , not yet //Viz
	border-collapse:collapse; */
}
#AddTask{
	/*width:38px;*/
	height:54px;
	border:none;
}
#AddTask td{
	font-size: 11px;
	font-style:normal;
	font-weight:normal;
	text-decoration:none;
	border:none;
}

#RemoveTask{
	/*width:45px;*/
	height:54px;
	z-index:20;
}
#RemoveTask td{
	font-size: 11px;
	font-style:normal;
	font-weight:normal;
	text-decoration:none;
	border:none;
}


#PauseTask{
	/*width:38px;*/
	height:54px;
	z-index:21;
}
#PauseTask td{
	font-size: 11px;
	font-style:normal;
	font-weight:normal;
	text-decoration:none;
	border:none;
}

#ResumeTask{
	/*width:38px;*/
	height:54px;
	z-index:22;
}
#ResumeTask td{
	font-size: 11px;
	font-style:normal;
	font-weight:normal;
	text-decoration:none;
	border:none;
}


#PauseAllTask{
	/*width:49px;*/
	height:54px;
	z-index:23;
}
#PauseAllTask td{
	font-size: 11px;
	font-style:normal;
	font-weight:normal;
	text-decoration:none;
	border:none;
}
	
#ResumeAllTask{
	/*width:58px;*/
	height:54px;
	z-index:24;
}
#ResumeAllTask td{
	font-size: 11px;
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
#ClearCompletedTask td{
	font-size: 11px;
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
#Home td{
	font-size: 12px;
	font-style:normal;
	font-weight:normal;
	text-decoration:none;
	border:none;
}

#downloadSpeed img{
	width:11px;
	height:11px;	
}

#uploadSpeed {
	width: 90px;
	}

#uploadSpeed img{
	width:11px;
	height:11px;
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
	width:30px;
	height:30px;
}
.icon a:link, .icon a:visited{
	cursor:pointer;
	display: block;
	width:30px;	
	height:30px;
}

#icon_add{background:url(images/Hand_ui/add_disable.png) no-repeat;}
#icon_add a:link, #icon_add a:visited, #icon_add a:hover{background:url(images/Hand_ui/add.png) no-repeat;}
#icon_add a:active{background:url(images/Hand_ui/add_click.png) no-repeat;}

#icon_del{background:url(images/Hand_ui/delete_disable.png) no-repeat;}
#icon_del a:link, #icon_del a:visited, #icon_del a:hover{ background:url(images/Hand_ui/delete.png) no-repeat;}
#icon_del a:active{background:url(images/Hand_ui/delete_click.png) no-repeat;}

#icon_pause{background:url(images/Hand_ui/pause_disable.png) no-repeat;}
#icon_pause a:link, #icon_pause a:visited, #icon_pause a:hover{background:url(images/Hand_ui/pause.png) no-repeat;}
#icon_pause a:active{background:url(images/Hand_ui/pause_click.png) no-repeat;}

#icon_resume{background:url(images/Hand_ui/resume_disable.png) no-repeat;}
#icon_resume a:link, #icon_resume a:visited, #icon_resume a:hover{background:url(images/Hand_ui/resume.png) no-repeat;}
#icon_resume a:active{background:url(images/Hand_ui/resume_click.png) no-repeat;}

#icon_pause_all{background:url(images/Hand_ui/pause_all_disable.png) no-repeat;}
#icon_pause_all a:link, #icon_pause_all a:visited, #icon_pause_all a:hover{background:url(images/Hand_ui/pause_all.png) no-repeat;}
#icon_pause_all a:active{background:url(images/Hand_ui/pause_all_click.png) no-repeat;}

#icon_resume_all{background:url(images/Hand_ui/resume_all_disable.png) no-repeat;}
#icon_resume_all a:link, #icon_resume_all a:visited, #icon_resume_all a:hover{background:url(images/Hand_ui/resume_all.png) no-repeat;}
#icon_resume_all a:active{background:url(images/Hand_ui/resume_all_click.png) no-repeat;}

#icon_clear{background:url(images/Hand_ui/clear_disable.png) no-repeat;}
#icon_clear a:link, #icon_clear a:visited, #icon_clear a:hover{background:url(images/Hand_ui/clear.png) no-repeat;}
#icon_clear a:active{background:url(images/Hand_ui/clear_click.png) no-repeat;}

#icon_home{background:url(images/Hand_ui/home_disable.png) no-repeat;}
#icon_home a:link, #icon_home a:visited, #icon_home a:hover{background:url(images/Hand_ui/home.png) no-repeat;}
#icon_home a:active{background:url(images/Hand_ui/home_click.png) no-repeat;}

#icon_help{background:url(images/icon/question.png) no-repeat;}

.panel{
	background-color:#999;	
	border:2px outset #CCC;
	font-size:10px;
	font-family:Verdana, Geneva, sans-serif;
	color:#333333;
	width:240px;
	position:fixed;
	margin-top:1%;
	margin-left:15%;
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

.taskLongWidth{width:100%;}
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
.selected2 {font-weight:bold; cursor:default; color:#F90;}
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
div.task_top {
	left: 2%;
	width: 96%;
	height: 30px;
	background: url(images/Hand_ui/hand_title.jpg) repeat-x;
	margin: 0;
	position: relative;
}
div.task_button {
	left: 2%;
	width: 96%;
	height: 60px;
	margin: 0;
	background: url(images/Hand_ui/hand_button.jpg) repeat-x;
	position: relative;
	}
div#handHidden {
	left: 2%;
	width: 96%;
	height: 39px;
	margin: 0;
	background: url(images/Hand_ui/hand_hidden.jpg) repeat-x;
	position: relative;
	}
div#footer {
	left: 2%;
	width: 96%;
	height: 31px;
	background:url(images/Hand_ui/hand_footer.jpg) repeat-x;
	margin: 0;
	position: relative;
	bottom: 0;
	z-index: 80;
	}
div.task_main_div {
	position: relative;
	top: 0 !important;
	left: 2%;
	padding: 0;
	margin: 0;
	min-height: 300px;
	width: 96%;
	background-color: #4d595d;
	}

td.button_width {
	width: 40px;
	}
td.button_width1 {
	width: 60px;
	}
	
table.button_table {
	width: 100%;
	border-bottom: 1px solid #CFF;
	border-top: 1px solid #CFF;
	}
.zxx_text_overflow_2{width:90%; font-size:16px; white-space:nowrap; text-overflow:ellipsis; -o-text-overflow:ellipsis; overflow:hidden;}
</style>
<script type="text/javascript" src="jquery.js"></script>
<script>
var httpTag = 'https:' == document.location.protocol ? false : true;
var $j = jQuery.noConflict();
var multi_INT = 0;
var refreshTime_data_tmp;
	var IS_FAT32=0;
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
<script type="text/javascript" src="multiLanguage_all.js"></script>
<script type="text/javascript" src="multiLanguage_task.js"></script>
<script type="text/javascript" src="state_hand.js"></script>
<script type="text/javascript" src="popup.js"></script>
<script type="text/javascript" src="detect.js"></script>
<script type="text/javascript" src="alttxt.js"></script>
<script type="text/javascript" src="ext/ext-base.js"></script>
<script type="text/javascript" src="ext/ext-all.js"></script>
<script type="text/javascript" src="dm_function_hand.js"></script>
<script type="text/javascript" src="plugin/jquery.iphone-switch.js"></script>
<script>
// for client_function.js
function initial(){
	show_menu();
	showTask();
	initial_Refresh_time(refreshTime_data_tmp);
	//document.getElementById("select_lang_"+multi_INT).selected = true;
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

<body onunload="return unload_body();"onload="initial();">
<noscript>
	<div class="popup_bg" style="visibility:visible; z-index:999;">
		<div style="margin:200px auto; width:300px; background-color:#006699; color:#FFFFFF; line-height:150%; border:3px solid #FFF; padding:5px;"><#not_support_script#></p></div>
	</div>
</noscript>
<div id="DM_mask" class="mask_bg"></div><!--[if lte IE 6.5]><iframe class="hackiframe"></iframe><![endif]-->
<div id="Loading" class="popup_bg"></div>

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
                	<span style="" id="multi_17">User name</span>
                    <input id="User_name" type="text" width="280px" value="" />
                </td>
              </tr>
              <tr>
              	<td colspan="2">
                	<span style="" id="multi_18">Password</span>
                    <input id="Password" type="password" width="280px" value="" />
                </td>
              </tr>
			</table>
			<br/>
			<table width="100%"  border="0" cellspacing="1" cellpadding="0" class="form_table">			 
			  <tfoot>
			  <tr>
				<td>
				<input type="button" name="AddFile_1" value="Apply" style="" onclick="Ftp_dm_add_status();">&nbsp;&nbsp;
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

<div class="task_top">
<div id="TopBanner"></div>
</div>

<div class="task_button" align="center">
<table cellpadding="0" cellspacing="0" style="width:100%;">
        <tr>
        	<td>
            <table class="button_table">
            <tr>
            <td class="button_width" align="center">
        	<table id="AddTask">
                	<tr>
                    	<td align="center">
                        <div class="icon" id="icon_add" title="Add"><a href="javascript:create_AddTask();"></a></div>
                        </td>
                    </tr>
                    <tr>
                    	<td align="center" valign="middle">
                        <span id="multi_7"></span>
                        </td>
                    </tr>
            </table>
            </td>
            <td class="button_width" align="center">
            <table id="RemoveTask">
                	<tr>
                    	<td align="center">
                        	<div class="icon" id="icon_del" title="Delete"><a></a></div>
                        </td>
                    </tr>
                    <tr>
                    	<td align="center" valign="middle">
                        <span id="multi_8"></span>
                        </td>
                    </tr>
            </table>
            </td>
            <td class="button_width" align="center">
            <table id="PauseTask">
                	<tr>
                    	<td align="center">
                        	<div class="icon" id="icon_pause" title="Pause"><a></a></div>
                        </td>
                    </tr>
                    <tr>
                    	<td align="center" valign="middle">
                        <span id="multi_9"></span>
                        </td>
                    </tr>
            </table>
            </td>
            <td class="button_width" align="center">
            <table id="ResumeTask">
                	<tr>
                    	<td align="center">
                        	<div class="icon" id="icon_resume" title="Resume"><a></a></div>
                        </td>
                    </tr>
                    <tr>
                    	<td align="center" valign="middle">
                        <span id="multi_10"></span>
                        </td>
                    </tr>
        	</table>
            </td>
            <td align="right">
            <table id="PauseAllTask">
                	<tr>
                    	<td align="center">
                        	<div class="icon" id="icon_pause_all" title="Pause All"><a></a></div>
                        </td>
                    </tr>
                    <tr>
                    	<td align="center" valign="middle" colspan="3">
                        <span id="multi_11"></span>
                        </td>
                	</tr>
        	</table>
            </td>
            <td align="right" class="button_width1">
            <table id="ResumeAllTask">
                	<tr>
                    	<td align="center">
                        	<div class="icon" id="icon_resume_all" title="Resume All"><a></a></div>
                        </td>
                    </tr>
                    <tr>
                    	<td align="center" valign="middle" colspan="3">
                        <span id="multi_12">Resume All</span>
                        </td>
                    </tr>
        	</table>
            </td>
            <td align="right" style="display:none;">
            <table id="ClearCompletedTask">
                	<tr>
                    	<td align="center">
                        	<div class="icon" id="icon_clear" title="Clear" style="margin-left:30px;"><a></a></div>
                        </td>
                    </tr>
                    <tr>
                    	<td align="center" valign="middle" colspan="3">
                        <span id="multi_13"></span>
                        </td>
                    </tr>
        	</table>
            </td>
            </tr>
            </table>
          	</td>
            </tr>
            <!--<tr>
            	<td>
                	<div style="margin-left: 5px; margin-top: 10px; margin-bottom: 10px;">
                    	<img src="/images/New_ui/export/line_export.png" style="width:99%;">
                    </div>
                </td>
            </tr>-->
</table>
</div>

<div style="display:inherit;" id="handHidden">
<table style="width:100%; border-bottom: 1px solid #CCFFFF">
            <tr>
            	<td>
                	<table style="width:100%;">
                    	<tr>
                        	<td align="left" class="transfers">&nbsp;<span id="transfers">0</span>
                            	<span id="multi_15">Transfers</span>
                            </td>
                            <td id="downloadSpeed" align="right"><img src="images/icon/DownArrowIcon.png" />
                            <span id="total_downloadSpeed">0</span> KBps</td>
                            <td id="uploadSpeed"><img src="images/icon/UpArrowIcon.png" />
                            <span id="total_uploadSpeed">0</span> KBps</td>
                        </tr>
                    </table>
                </td>
            </tr>
            <!-- <tr>
            	<td>
                	<div style="margin-left: 5px; margin-bottom: 10px;">
                    	<img src="/images/New_ui/export/line_export.png" style="width:99%;">
                    </div>
                </td>
            </tr>-->
            <tr>
            				<td align="center">
                            <div id="taskClass">
                            <span class="selected2" id="All_tasks" onclick="showTask_select(0);"></span> | 
                            <span class="noselected" id="Downloading_tasks" onclick="showTask_select(1)"></span> | 
                            <span class="noselected" id="Seeding_tasks" onclick="showTask_select(2)"></span> |
                            <span class="noselected" id="Paused_tasks" onclick="showTask_select(3)"></span> |
                            <span class="noselected" id="Finished_tasks" onclick="showTask_select(4)"></span>
                            </div>
                            </td>
            </tr>
</table>
</div>

<div align="center" style="word-break:break-all;" class="task_main_div" id="task_main_div">
</div>
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

</body>
</html>
