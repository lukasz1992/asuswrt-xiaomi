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
	<title>Help</title>
	<link href="sliderplugin/slider_css.css" rel="stylesheet" type="text/css" />
	<link rel="stylesheet" type="text/css" href="index_style.css">
	<link rel="stylesheet" type="text/css" href="NM_style.css">
	<link rel="stylesheet" type="text/css" href="other.css">
	<link rel="stylesheet" type="text/css" href="form_style.css">
	<link rel="stylesheet" type="text/css" href="routercss.css">
	<link rel="stylesheet" type="text/css" href="ext/css/ext-all.css">
	<link rel="stylesheet" href="sliderplugin/jquery.tabs.css" type="text/css" media="print,projection,screen" />
	<!--<link href="english.css" rel="stylesheet" type="text/css" id="languageCss" />
<link href="english_1.css" rel="stylesheet" type="text/css" id="languageCss_1" />-->
	<script type="text/javascript" src="jquery.js"></script>
	<script>
var httpTag = 'https:' == document.location.protocol ? false : true;
var utility_ver="";
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
			if(XMLHttpRequest.status==598) {
				if(httpTag)
					self.location = "http://"+ location.host +"/Main_Login.asp";
				else
					self.location = "https://"+ location.host +"/Main_Login.asp";
			}}});
function initial_multi_INT_status(data){
	var array = new Array();
	eval("array="+data);
	var lang = array[14];
	utility_ver = array[28];
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
	<script type="text/javascript" src="multiLanguage_help.js"></script>
	<script type="text/javascript" src="state.js"></script>
	<script type="text/javascript" src="general.js"></script>
	<script type="text/javascript" src="popup.js"></script>
	<!--<script type="text/javascript" src="/help.js"></script>-->
	<script type="text/javascript" src="detect.js"></script>
	<script type="text/javascript" src="ext/ext-base.js"></script>
	<script type="text/javascript" src="ext/ext-all.js"></script>
	<script type="text/javascript" src="sliderplugin/jquery-easing.1.2.pack.js"></script>
	<script type="text/javascript" src="sliderplugin/jquery-easing-compatibility.1.2.pack.js"></script>
	<script type="text/javascript" src="sliderplugin/coda-slider.1.1.1.pack.js"></script>
	<style type="text/css">
		.mask_bg {
			position: absolute;
			margin-top: -160px;
			width: 100%;
			height: 100%;
			z-index: 100;
			/*background-color: #FFF;*/
			background: url(images/popup_bg2.gif);
			background-repeat: repeat;
			filter: progid: DXImageTransform.Microsoft.Alpha(opacity=60);
			-moz-opacity: 0.6;
			display: none;
			/*visibility:hidden;*/
			overflow: hidden;
		}
		
		.icon {
			width: 40px;
			height: 40px;
			margin-left: 2px;
			float: left;
			text-decoration: none;
		}
		
		.icon a:link,
		.icon a:visited {
			cursor: pointer;
			display: block;
			width: 40px;
			height: 40px;
		}
		
		#icon_back {
			background: url(images/icon/back.png)
		}
		
		#icon_back a:active {
			background: url(images/icon/back_click.png);
		}
		
		div.wrapper {
			margin: 0 auto;
			width: 730px;
		}
		
		td.sidenav {
			width: 200px;
		}
		
		body {
			font-family: Verdana, Tohoma, Arial, Helvetica, sans-serif;
			padding: 0;
			margin: 0;
		}
		
		.wrapperDesc {
			margin: 0 auto;
			width: 570px;
		}
	</style>

	<script>
jQuery(window).bind("load", function() {
jQuery("div#slider1").codaSlider()
jQuery("div#slider2").codaSlider()
});
	
function initial(){
	show_menu();
	if(httpTag) {	
		document.getElementById("helpAddress").href = "http://"+ location.host +"/downloadmaster/help.asp";
		document.getElementById("backAddress").href = "http://"+ location.host +"/downloadmaster/index.asp";
	} else {
		document.getElementById("helpAddress").href = "https://"+ location.host +"/downloadmaster/help.asp";
		document.getElementById("backAddress").href = "https://"+ location.host +"/downloadmaster/index.asp";
	}
}

</script>
</head>

<body onload="initial();" onunLoad="return unload_body();">
	<div id="TopBanner"></div>
	<!--<div id="DM_mask" class="mask_bg"></div>-->
	<div id="Loading" class="popup_bg"></div>
	<iframe name="hidden_frame" id="hidden_frame" src="" width="0" height="0" frameborder="0"></iframe>
	<table class="content" align="center" cellpadding="0" cellspacing="0">
		<tr>
			<td width="17">&nbsp;</td>
			<!--=====Beginning of Main Menu=====-->
			<td valign="top" width="202">
				<div id="mainMenu"></div>
				<div id="subMenu"></div>
			</td>

			<td valign="top">
				<!--  delete by alan <div id="tabMenu" class="submenuBlock"></div>-->

				<!--===================================Beginning of Main Content===========================================-->
				<div class="app_table" style="margin-top:-150px; position:relative;">
					<table bgcolor="#4D595D" style="width:760px;">
						<tr>
							<td class="">
								<div class="formfonttitle">
									<table>
										<tr>
											<td valign="bottom" id="multiHelp_0"></td>
											<td>
												<table id="BackTask" style="margin-left:550px;">
													<tbody>
														<tr>
															<td align="center">
																<div class="icon" id="icon_back" title="Back">
																	<a id="backAddress" href=""></a>
																</div>
															</td>
															<td>&nbsp;</td>
														</tr>
														<td align="center" valign="middle" id="multiHelp_1"></td>
														<td>&nbsp;</td>
													</tbody>
												</table>
											</td>
										</tr>
									</table>
									<div style="margin-left:5px;margin-top:10px;margin-bottom:10px"><img width="745px" height="2px" src="images/New_ui/export/line_export.png"></div>
								</div>
							</td>
						</tr>

		</tr>
		<tr>
			<td valign="top">
				<div id="partition_div"></div>
				<div id="DMDesc">
					<div id="mainbody">
						<div class="wrapper">
							<div class="shadow-l">
								<div class="shadow-r">
									<table class="" cellspacing="0" cellpadding="0">
										<tbody>
											<tr valign="top">
												<td class="">
													<div class="padding">
														<div class="">
															<ul style="margin-left:10px;">
																<br>
																<li>
																	<a id="faq" href="http://www.asus.com/support/FAQ/1009773/" target="_blank" style="text-decoration:underline;font-size:14px;font-weight:bolder;color:#FFF">Download Master FAQ</a>
																</li>
																<li style="margin-top:10px;">
																	<a id="faq2" href="http://www.asus.com/support/FAQ/1016385/" target="_blank" style="text-decoration:underline;font-size:14px;font-weight:bolder;color:#FFF">Download Master Tool FAQ</a>
																</li>
																<li style="margin-top:10px;">
																	<a id="DMUtilityLink" href="http://dlcdnet.asus.com/pub/ASUS/wireless/ASUSWRT/DM2_2228.zip" style="text-decoration:underline;font-size:14px;font-weight:bolder;color:#FFF">
																	</a>
																</li>
															</ul>
														</div>
														<span class="article_seperator">&nbsp;</span>
													</div>
												</td>
											</tr>
										</tbody>
									</table>
								</div>
							</div>
						</div>
					</div>
			</td>
		</tr>

		</table>
		</div>

		<!--===================================Ending of Main Content===========================================-->

		<td width="10" align="center" valign="top">&nbsp;</td>
		</tr>
	</table>

	<div id="footer"></div>
	<script>
$j("#multiHelp_0").html(multiLanguage_help_array[multi_INT][0]);
$j("#multiHelp_1").html(multiLanguage_help_array[multi_INT][1]);
$j("#DMUtilityLink").html(multiLanguage_help_array[multi_INT][2]);
</script>
</body>

</html>