// JavaScript Document

var dm_dir = new Array(); 
var Download_path = 'Download2';
var WH_INT=0,Floder_WH_INT=0,General_WH_INT=0;
var BASE_PATH;
var initial_array = new Array();
var folderlist = new Array();
var DM_port_tmp;
var DM_https_port_tmp;
var PATH_tmp;
var Enable_time_tmp;
var DM_Misc_seeding_x_tmp;
var dm_radio_time2_x_tmp;
var dm_radio_time_x_tmp;
var Day_tmp;
var DM_Misc_http_x_tmp;
var FromObject = "0";
var lastClickedObj = 0;
var path_directory;
var amule_status_tmp;
var httpTag = 'https:' == document.location.protocol ? false : true;
///leo {
function apply_cgi(data){
    var url_l = "dm_apply.cgi";
	var action_mode_l = "initial";
	var type_l = "General";
	url_l += "?action_mode=" + action_mode_l + "&download_type=" +type_l+ "&t=" +Math.random();
	$j.ajax({url: url_l,
			async: false,
			success: function(data){callBackGeneral(data)},
			error: function(XMLHttpRequest, textStatus, errorThrown){
			if(XMLHttpRequest.status==598){
				if(httpTag)
					self.location = "http://"+ location.host +"/Main_Login.asp";
				else
					self.location = "https://"+ location.host +"/Main_Login.asp";
			}}});

}

//leo }
function aMuleConServer(IP,Port){
	var action_mode = "DM_ED2K_CON";
	var url = "dm_apply.cgi?action_mode="+action_mode+"&ED2K_SERVER_IP="+IP+"&ED2K_SERVER_PORT="+Port+"&t="+Math.random();
	$j.get(url);
	}
	
function aMuleDisConServer(IP,Port){
	var action_mode = "DM_ED2K_DISCON";
	var url = "dm_apply.cgi?action_mode="+action_mode+"&ED2K_SERVER_IP="+IP+"&ED2K_SERVER_PORT="+Port+"&t="+Math.random();
	$j.get(url);
}
	
function aMuleAddServer(IP,Port) {
	var action_mode = "DM_ED2K_ADD"
	var url = "dm_apply.cgi?action_mode="+action_mode+"&ED2K_SERVER_IP="+IP+"&ED2K_SERVER_PORT="+Port+"&t="+Math.random();
	$j.get(url);
}

function removeaMuleServer(IP,Port,value) {
	var remove = "";
	if(value == "removed")
	{
		remove = value;
	}
	var action_mode = "DM_ED2K_REM"
	var url = "dm_apply.cgi?action_mode="+action_mode+"&ED2K_SERVER_IP="+IP+"&ED2K_SERVER_PORT="+Port+"&ED2K_SERVER_REMOVED="+ remove +"&t="+Math.random();
	$j.get(url);
}

function getaMuleServerList() {
	var url = "dm_apply.cgi";
	var action_mode = "initial";
	var type = "ED2K_SERVER_LIST";
	url += "?action_mode=" + action_mode + "&download_type=" +type+ "&t=" +Math.random();
	$j.get(url, function(data){showServerList(data);});
}


function get_amule_server_status(){
	var url = "dm_apply.cgi";
	var action_mode = "initial";
	var type = "ED2K_SERVER_STATUS";
	url += "?action_mode=" + action_mode + "&download_type=" +type+ "&t=" +Math.random();
	$j.ajax({url: 	url,
			async: 	false,
			success:function(data){showServerList(data);},
			error: 	function(XMLHttpRequest, textStatus, errorThrown){
					if(XMLHttpRequest.status==598) {
						if(httpTag)
							self.location = "http://"+ location.host +"/Main_Login.asp";
						else
							self.location = "https://"+ location.host +"/Main_Login.asp";
					}
				}
			})
	}

function NZBapplyRule(){
	var digit = /^\d*$/
	var port = $j("#serverPort").attr("value");
	var downSpeed = $j("#downloadSpeed_limit").attr("value");
	var connections = $j("#Number_of_connections").attr("value");
	/*if(!digit.test(port) || port > 65535 || port < 0 || port == 9092 || port == 6789 || port == 3490 || port == 80 || port == 8082)
	Ext.MessageBox.alert(multiLanguage_setting_array[multi_INT][39],multiLanguage_setting_array[multi_INT][46]+"between 0-65535 except:9092,6789,3490 80 and 8082!");*/
	if(!digit.test(port) || port > 65535 || port < 0 )
	Ext.MessageBox.alert(multiLanguage_setting_array[multi_INT][39],multiLanguage_setting_array[multi_INT][46]);
	else if(!digit.test(downSpeed))
	Ext.MessageBox.alert(multiLanguage_setting_array[multi_INT][39],multiLanguage_setting_array[multi_INT][47]);
	else if(!digit.test(connections))
	Ext.MessageBox.alert(multiLanguage_setting_array[multi_INT][39],multiLanguage_setting_array[multi_INT][48]);
	else{
		showLoading(3);
		$j("#Setting_NZB_form").submit();
		//Ext.MessageBox.confirm("message","Are you sure to apply?",callBackNZB);
		}
	
	}


/*
function HTTPapplyRule(){
	$j("#Setting_HTTP_form").submit();
	}



function FTPapplyRule(){
	$j("#Setting_FTP_form").submit();
	}
*/

/*function callBackBT(id){
	if(id == "yes")
	$j("#Setting_BT_form").submit();
	}*/

function BTapplyRule(){
	updateStatusCounter=0;
	digit = /^\d*$/
	var port = $j("#Incoming_port").attr("value");
	var downSpeed = $j("#download_speed_limit").attr("value");
	var upSpeed = $j("#upload_speed_limit").attr("value");
	var max_torrent_peer =  $j("#Max_torrent_peer").attr("value");
	var maxPeer = $j("#Max_peer").attr("value");
	if (port == $j("#bt_port_tmp").attr("value")){
		$j("#peer_port_change").attr("value","0");
	}else{
		$j("#peer_port_change").attr("value","1");
	}

	if((!digit.test(downSpeed) || downSpeed=="")&& document.getElementById('Down_limit_checkbox').checked == true)
		$j("#download_speed_limit").attr("value","100");
	else if((!digit.test(upSpeed) || upSpeed=="")&& document.getElementById('Up_limit_checkbox').checked == true)
		$j("#upload_speed_limit").attr("value","100");

	if(!digit.test(port) || port > 65535 || port < 1024 || port == 9092 || port == 6789 || port == 3490 || port == 80 || port==8082 || port==""  || port == 8200 || port == 3689 || port == 3678 || port == 3567 || port == 3568 || port == 3569)
	Ext.MessageBox.alert(multiLanguage_setting_array[multi_INT][39],multiLanguage_setting_array[multi_INT][79]);
	else if(port == location.host.split(":")[1])
	Ext.MessageBox.alert(multiLanguage_setting_array[multi_INT][39],multiLanguage_setting_array[multi_INT][86]+port);

	else if((!digit.test(downSpeed) || downSpeed=="")&& document.getElementById('Down_limit_checkbox').checked == false)
	Ext.MessageBox.alert(multiLanguage_setting_array[multi_INT][39],multiLanguage_setting_array[multi_INT][47]);
	else if((!digit.test(upSpeed) || upSpeed=="")&& document.getElementById('Up_limit_checkbox').checked == false)
	Ext.MessageBox.alert(multiLanguage_setting_array[multi_INT][39],multiLanguage_setting_array[multi_INT][50]);

	else if(!digit.test(max_torrent_peer) || max_torrent_peer=="")
	Ext.MessageBox.alert(multiLanguage_setting_array[multi_INT][39],multiLanguage_setting_array[multi_INT][51]);
	else if(!digit.test(maxPeer) || maxPeer=="")
	Ext.MessageBox.alert(multiLanguage_setting_array[multi_INT][39],multiLanguage_setting_array[multi_INT][87]);
	else if(eval(max_torrent_peer) > eval(maxPeer))
	Ext.MessageBox.alert(multiLanguage_setting_array[multi_INT][39],multiLanguage_setting_array[multi_INT][146]);
	else{
			showLoading(3);
			$j("#Setting_BT_form").submit();
			$j("#peer_port_change").attr("value",port);
		}
	//Ext.MessageBox.confirm("message","Are you sure to apply?",callBackBT);
	}
	
	
function callBackGeneral(data){
	if(document.getElementById("DM_port").value){
	var url = "dm_apply.cgi";
	var action_mode = "DM_APPLY";
	var type = "General";
	var data_cgi =new Array();
	eval("data_cgi="+data);
	var Lan_ip = document.getElementById("Lan_ip").value;
	var Base_path = document.getElementById("Base_path").value;
	var Download_dir = encodeURI(document.getElementById("PATH").value);
	var Refresh_rate = document.getElementById("Refresh_rate").value;
	var Misc_http_x = document.getElementById("Misc_http_x").value;
	var DM_port =document.getElementById("DM_port").value;
	var DM_https_port =document.getElementById("DM_https_port").value;
	var lang_tmp = document.getElementById("DM_language").value;
	var MISCR_HTTP_X = document.getElementById("MISCR_HTTP_X").value;
	var MISCR_HTTPPORT_X = document.getElementById("MISCR_HTTPPORT_X").value;
	var MISCR_HTTPSPORT_X = document.getElementById("MISCR_HTTPSPORT_X").value;
	var MISCR_HTTPPORT_C = data_cgi[11];
	var MISCR_HTTPSPORT_C = data_cgi[31];

	var Day = document.getElementById("dm_radio_date_x").value;
	var Productid = document.getElementById("Productid").value;
	var APPS_DEV = document.getElementById("APPS_DEV").value;
	var WAN_IP = $j("#WAN_IP").attr("value");
	var DDNS_ENABLE_X = $j("#DDNS_ENABLE_X").attr("value");
	var DDNS_HOSTNAME_X = $j("#DDNS_HOSTNAME_X").attr("value");
	var MAX_ON_HEAVY = $j("#MAX_ON_HEAVY").attr("value");
	var MAX_QUEUES = $j("#MAX_QUEUES").attr("value");
	var MAX_ON_ED2K = $j("#MAX_ON_ED2K").attr("value");
	var RFW_ENABLE_X = $j("#RFW_ENABLE_X").attr("value");
	var DEVICE_TYPE = $j("#DEVICE_TYPE").attr("value");
	var dm_radio_date_x = $j("#dm_radio_date_x").attr("value");
	var dm_radio_time_x = $j("#dm_radio_time_x").attr("value");
	var dm_radio_time2_x = $j("#dm_radio_time2_x").attr("value");
	var misc_seeding_x = $j("#Misc_seeding_x").attr("value");
	var DM_port_renew;
	var DM_https_port_renew;
	var Download_dir_renew;
	var Enable_time_renew;
	var misc_seeding_x_renew;
	var enableTime;
	if(misc_seeding_x == DM_Misc_seeding_x_tmp){
		misc_seeding_x_renew = 0;
		}
	else{
		misc_seeding_x_renew = 1;
		}
	if (document.form.Enable_time0.checked == true){
		enableTime=0;
	}
	else if (document.form.Enable_time1.checked == true){
		enableTime=1;
	}
	if(enableTime == 0 && Enable_time_tmp == 0){
		Enable_time_renew = 0;
		}
	else if(enableTime == 1 && Enable_time_tmp == 1){
		if(Day == Day_tmp && dm_radio_time_x_tmp == dm_radio_time_x && dm_radio_time2_x_tmp == dm_radio_time2_x){
			Enable_time_renew = 0;
			}
		else{
			Enable_time_renew = 1;
			}
		}
	else if(enableTime != Enable_time_tmp){
		Enable_time_renew = 1;
		}
	if(DM_port == DM_port_tmp){
		DM_port_renew = 0;
		}
	else{
		DM_port_renew = 1;
		}
	if(DM_https_port == DM_https_port_tmp){
		DM_https_port_renew = 0;
		}
	else{

		DM_https_port_renew = 1;
		}
	if(Download_dir == PATH_tmp){
		Download_dir_renew = 0;
		}
	else{
		Download_dir_renew = 1;
		}
var task_code = multiLanguage_setting_array[multi_INT][54]+ 'http:' +DM_port+'https:'+DM_https_port + multiLanguage_setting_array[multi_INT][55]+'<br /><br />';
	$("drword").innerHTML = task_code;
	url += "?action_mode=" + action_mode + "&download_type=" +type+  "&Base_path=" +Base_path+  "&Download_dir=" +Download_dir+  "&DM_dir_renew="+Download_dir_renew+"&Refresh_rate=" +Refresh_rate+  "&Misc_http_x=" +Misc_http_x+  "&Lan_ip=" +Lan_ip+ "&DM_port="+ DM_port +"&DM_port_renew="+DM_port_renew+"&DM_https_port="+ DM_https_port +"&DM_https_port_renew="+DM_https_port_renew+"&Start_hour="+"00"+"&End_hour="+"23"+ "&Start_minute="+"00"+"&End_minute="+"59"+"&Enable_time="+enableTime+"&DM_language="+lang_tmp+"&Miscr_http_x="+MISCR_HTTP_X+"&Miscr_httpport_x="+MISCR_HTTPPORT_X+"&Day="+Day+"&Enable_time_renew="+Enable_time_renew+ "&Productid="+Productid+"&APPS_DEV="+APPS_DEV+"&WAN_IP="+WAN_IP+"&DDNS_ENABLE_X="+DDNS_ENABLE_X+"&DDNS_HOSTNAME_X="+DDNS_HOSTNAME_X+"&MAX_ON_HEAVY="+MAX_ON_HEAVY+"&MAX_QUEUES="+MAX_QUEUES+"&MAX_ON_ED2K="+MAX_ON_ED2K+"&RFW_ENABLE_X="+RFW_ENABLE_X+"&DEVICE_TYPE="+DEVICE_TYPE+"&dm_radio_date_x="+dm_radio_date_x+"&dm_radio_time_x="+dm_radio_time_x+"&dm_radio_time2_x="+dm_radio_time2_x+"&misc_seeding_x="+misc_seeding_x+"&misc_seeding_x_renew="+misc_seeding_x_renew+"&t=" +Math.random();
	if(MISCR_HTTPPORT_C == DM_port || MISCR_HTTPSPORT_C == DM_port || MISCR_HTTPPORT_C == DM_https_port || MISCR_HTTPSPORT_C == DM_https_port){
		Ext.MessageBox.alert(multiLanguage_setting_array[multi_INT][39],multiLanguage_setting_array[multi_INT][52]);
		}
	else if(DM_port>65535 || DM_port<1024 || DM_port == 9092 || DM_port == 6789 || DM_port == 3490 || DM_port == 80 || DM_port == 8082 || DM_port == "" || DM_port == 8200 || DM_port == 3689 || DM_port == 3678 || DM_port == 3567 || DM_port == 3568 || DM_port == 3569 || DM_https_port>65535 || DM_https_port<1024 || DM_https_port == 9092 || DM_https_port == 6789 || DM_https_port == 3490 || DM_https_port == 80 || DM_https_port == 8082 || DM_https_port == "" || DM_https_port == 8200 || DM_https_port == 3689 || DM_https_port == 3678 || DM_https_port == 3567 || DM_https_port == 3568 || DM_https_port == 3569 || DM_port == 6666){
		Ext.MessageBox.alert(multiLanguage_setting_array[multi_INT][39],multiLanguage_setting_array[multi_INT][85]);
		} else if(DM_port == DM_https_port) {
		Ext.MessageBox.alert(multiLanguage_setting_array[multi_INT][39],multiLanguage_setting_array[multi_INT][144]);
		}
	else{
		Enable_time_tmp = enableTime;
		DM_Misc_seeding_x_tmp = misc_seeding_x;
		Day_tmp = Day;
		dm_radio_time_x_tmp = dm_radio_time_x;
		dm_radio_time2_x_tmp = dm_radio_time2_x;
		PATH_tmp = Download_dir;
		$j.get(url,function(data){});
		var tmpTag = 'https:' == document.location.protocol ? false : true;
		if(tmpTag)
		{
			if(document.getElementById("DM_port").value != DM_port_tmp){
				showLoading(10,'waiting');
				if (data_cgi[15] == "CM-32_AC2600") {
				setTimeout('show_port_panel()',24500);
				    setTimeout('redirect()',25000);
				}
				else {
					setTimeout('show_port_panel()',5500);
				    setTimeout('redirect()',10000);
				}
			}
			else if(document.getElementById("Misc_http_x").value != DM_Misc_http_x_tmp){
				showLoading(10);
				setTimeout('redirect()',4300);
			}
			else{
				showLoading(10);
			}
		}
		else
		{
			if(document.getElementById("DM_https_port").value != DM_https_port_tmp){
				showLoading(10,'waiting');
				if (data_cgi[15] == "CM-32_AC2600") {
					setTimeout('show_port_panel()',24500);
				    setTimeout('redirect()',25000);
				}
				else {
					setTimeout('show_port_panel()',5500);
				    setTimeout('redirect()',10000);
				}
			}
			else if(document.getElementById("Misc_http_x").value != DM_Misc_http_x_tmp){
				showLoading(10);
				setTimeout('redirect2()',4300);
			}
			else{
				showLoading(10);
			}
		}
	}
	}
	}

function GeneralapplyRule(){
	updateStatusCounter=0;
	var digit = /^\d*$/	
	var rate = $j("#Refresh_rate").attr("value");
	if(!digit.test(rate)||rate==""){
		Ext.MessageBox.alert(multiLanguage_array[multi_INT][3],multiLanguage_setting_array[multi_INT][78]);
		return false;
	}
	else if(rate<3){
		Ext.MessageBox.alert(multiLanguage_array[multi_INT][3],"The Refresh rate must be greater than 2");
		return false;
	}
	else{
	if(!validate_timerange(document.form.dm_radio_time_x_starthour, 0) || !validate_timerange(document.form.dm_radio_time2_x_starthour, 0)
			|| !validate_timerange(document.form.dm_radio_time_x_startmin, 1) || !validate_timerange(document.form.dm_radio_time2_x_startmin, 1)
			|| !validate_timerange(document.form.dm_radio_time_x_endhour, 2) || !validate_timerange(document.form.dm_radio_time2_x_endhour, 2)
			|| !validate_timerange(document.form.dm_radio_time_x_endmin, 3) || !validate_timerange(document.form.dm_radio_time2_x_endmin, 3)
			)
		return false;
	
	if(document.form.Enable_time1.checked == true 
			&& document.form.dm_radio_date_x_Sun.checked == false
			&& document.form.dm_radio_date_x_Mon.checked == false
			&& document.form.dm_radio_date_x_Tue.checked == false
			&& document.form.dm_radio_date_x_Wed.checked == false
			&& document.form.dm_radio_date_x_Thu.checked == false
			&& document.form.dm_radio_date_x_Fri.checked == false
			&& document.form.dm_radio_date_x_Sat.checked == false){
				document.form.dm_radio_date_x_Sun.focus();
				$('blank_warn').style.display = "";
				return false;
		}
	}
	var Download_dir = encodeURI(document.getElementById("PATH").value);
	if(Download_dir.indexOf(";")>0 ||Download_dir.indexOf("/")>0 || Download_dir.indexOf("?")>0 ||Download_dir.indexOf(":")>0 ||Download_dir.indexOf("@")>0 ||Download_dir.indexOf("&")>0 ||Download_dir.indexOf("=")>0 ||Download_dir.indexOf("+")>0 ||Download_dir.indexOf("$")>0 ||Download_dir.indexOf(",")>0 ||Download_dir.indexOf("#")>0 ||Download_dir.indexOf("-")>0 ||Download_dir.indexOf(".")>0 ||Download_dir.indexOf("!")>0 ||Download_dir.indexOf("~")>0 ||Download_dir.indexOf("*")>0 ||Download_dir.indexOf("'")>0 ||Download_dir.indexOf("(")>0 ||Download_dir.indexOf(")")>0){
		Ext.MessageBox.alert(multiLanguage_setting_array[multi_INT][3],multiLanguage_setting_array[multi_INT][145]);
        return false;
	}
	if( Download_dir=="/tmp/mnt"){
		Ext.MessageBox.alert(multiLanguage_setting_array[multi_INT][3],multiLanguage_setting_array[multi_INT][88]);
		return false;
	}
	updateDateTime();
	//callBackGeneral();
	apply_cgi();
	return true;
}

function redirect(){
	location.href = 'http://'+ location.host.split(":")[0] + ":" + document.getElementById("DM_port").value + "/downloadmaster/Setting_General.asp";
	}
function redirect2(){
	location.href = 'https://'+ location.host.split(":")[0] + ":" + document.getElementById("DM_https_port").value + "/downloadmaster/Setting_General.asp";
	}
function show_port_panel(){
		$("Loading").style.visibility = "hidden";
		dr_advise();
	}

function initial_general_status(data){
	eval("initial_array="+data);
	if(initial_array[0]!=null && initial_array[0]!="")
	{
		if(initial_array[0] == 0){
			Enable_time_tmp = 0;
			$j("#Enable_time0").attr("checked","checked");
			Download_Schedule(0);
			}
		if(initial_array[0] == 1){
			Enable_time_tmp = 1;
			$j("#Enable_time1").attr("checked","checked");
			Download_Schedule(1);
			}
		}
		
	/*if(initial_array[1]!=null && initial_array[1]!=""){
		$j("#from_hour"+initial_array[1]).attr("selected","selected");
		Start_hour_tmp = initial_array[1];
		}
	if(initial_array[2]!=null && initial_array[2]!=""){
		$j("#from_minute"+initial_array[2]).attr("selected","selected");
		Start_minute_tmp = initial_array[2];
		}
	if(initial_array[3]!=null && initial_array[3]!=""){
		$j("#to_hour"+initial_array[3]).attr("selected","selected");
		End_hour_tmp = initial_array[3];
		}
	if(initial_array[4]!=null && initial_array[4]!=""){
		$j("#to_minute"+initial_array[4]).attr("selected","selected");
		End_minute_tmp = initial_array[4];
		}
	if(initial_array[5]!=null && initial_array[5]!=""){
		$j("#on_day"+initial_array[5]).attr("selected","selected");
		Day_tmp = initial_array[5];
		}*/
	if(initial_array[5]!=null && initial_array[5]!=""){
		document.getElementById("dm_radio_date_x").value=initial_array[5];
		Day_tmp = initial_array[5];
		}
	if(initial_array[25]!=null && initial_array[25]!=""){
		document.getElementById("dm_radio_time_x").value=initial_array[25];
		dm_radio_time_x_tmp=initial_array[25];
		}
	if(initial_array[26]!=null && initial_array[26]!=""){
		document.getElementById("dm_radio_time2_x").value=initial_array[26];
		dm_radio_time2_x_tmp=initial_array[26];
		}
	loadDateTime();

	if(initial_array[6]!=null && initial_array[6]!=""){
		$j("#PATH").attr("value",initial_array[6]);
		PATH_tmp = initial_array[6];
		}
	//document.getElementById("helpAddress").href = "http://"+initial_array[10]+":8081/help.asp";
	if(httpTag)
		document.getElementById("helpAddress").href = "http://"+ location.host +"/downloadmaster/help.asp";
	else
		document.getElementById("helpAddress").href = "https://"+ location.host +"/downloadmaster/help.asp";	
	if(initial_array[7]!=null && initial_array[7]!=""){
		$j("#Refresh_rate").attr("value",initial_array[7]);
		}
	if(initial_array[10]!=null && initial_array[10]!=""){
		$j("#Lan_ip").attr("value",initial_array[10]);
		}
	if(initial_array[8]!=null && initial_array[8]!=""){
		$j("#Base_path").attr("value",initial_array[8]);
		}
	BASE_PATH = initial_array[8];
		if(initial_array[9]!=null && initial_array[9]!="" && initial_array[23] == 1){
			$j("#Misc_http_x").attr("value",initial_array[9]);
			DM_Misc_http_x_tmp = initial_array[9];
			$j("#misc_http_x_1").iphoneSwitch($j("#Misc_http_x").attr("value"), 
							 function() {
								$j("#Misc_http_x").attr("value","1");
							 },
							 function() {
								$j("#Misc_http_x").attr("value","0");
							 },
							 {
								//switch_on_container_path: '/plugin/iphone_switch_container_off.png'
							 }
						);
		}
		else if(initial_array[9]!=null && initial_array[9]!="" && initial_array[23] == 0){
			$j("#Misc_http_x").attr("value","");
			DM_Misc_http_x_tmp = initial_array[9];
		if(initial_array[9] == 0)
			$j("#misc_http_x_1").attr("class","switch_disable_off");
		else
			$j("#misc_http_x_1").attr("class","switch_disable_on");
		}
		if(initial_array[27]!=null && initial_array[27]!=""){
			$j("#Misc_seeding_x").attr("value",initial_array[27]);
			DM_Misc_seeding_x_tmp = initial_array[27];
			$j("#misc_seeding_x_1").iphoneSwitch($j("#Misc_seeding_x").attr("value"), 
							 function() {
								$j("#Misc_seeding_x").attr("value","1");
							 },
							 function() {
								$j("#Misc_seeding_x").attr("value","0");
							 },
							 {
								//switch_on_container_path: '/plugin/iphone_switch_container_off.png'
							 }
						);
		}
		else if(initial_array[27]!=null && initial_array[27]!=""){
			$j("#Misc_seeding_x").attr("value","");
			DM_Misc_seeding_x_tmp = initial_array[27];
		if(initial_array[27] == 0)
			$j("#misc_seeding_x_1").attr("class","switch_disable_off");
		else
			$j("#misc_seeding_x_1").attr("class","switch_disable_on");
		}
		if(initial_array[11]!=null && initial_array[11]!=""){
		$j("#MISCR_HTTPPORT_X").attr("value",initial_array[11]);
		}
		if(initial_array[12]!=null && initial_array[12]!=""){
			$j("#MISCR_HTTP_X").attr("value",initial_array[12]);
			}
		if(initial_array[13]!=null && initial_array[13]!=""){
		$j("#DM_port").attr("value",initial_array[13]);
		DM_port_tmp = initial_array[13];
		$("DMport").innerHTML = initial_array[13];
		}
		if(initial_array[14]!=null && initial_array[14]!=""){
			$j("#DM_language").attr("value",initial_array[14]);
			}
		if(initial_array[15]!=null && initial_array[15]!=""){
			$j("#Productid").attr("value",initial_array[15]);
			}
		if(initial_array[16]!=null && initial_array[16]!=""){
			$j("#APPS_DEV").attr("value",initial_array[16]);
			}
		if(initial_array[17]!=null && initial_array[17]!=""){
			$j("#WAN_IP").attr("value",initial_array[17]);
			}
		if(initial_array[18]!=null && initial_array[18]!=""){
			$j("#DDNS_ENABLE_X").attr("value",initial_array[18]);
			}
		if(initial_array[19]!=null && initial_array[19]!=""){
			$j("#DDNS_HOSTNAME_X").attr("value",initial_array[19]);
			}
		if(initial_array[20]!=null && initial_array[20]!=""){
			$j("#MAX_ON_HEAVY").attr("value",initial_array[20]);
			}
		if(initial_array[21]!=null && initial_array[21]!=""){
			$j("#MAX_QUEUES").attr("value",initial_array[21]);
			}


		if(initial_array[22]!=null && initial_array[22]!=""){
			$j("#MAX_ON_ED2K").attr("value",initial_array[22]);
			}
		if(initial_array[24]!=null && initial_array[24]!=""){
			$j("#DEVICE_TYPE").attr("value",initial_array[24]);
			}

		if(initial_array[31]!=null && initial_array[31]!=""){
		$j("#MISCR_HTTPSPORT_X").attr("value",initial_array[11]);
		}
		if(initial_array[33]!=null && initial_array[33]!="1"){
			$j("#Misc_http_x_tr").hide();
		}
		if(initial_array[34]!=null && initial_array[34]!=""){
		$j("#DM_https_port").attr("value",initial_array[34]);
		DM_https_port_tmp = initial_array[34];
		$("DMhttpsport").innerHTML = initial_array[34];
		}
		if (document.getElementById("handToPhone")) {
			document.getElementById("handToPhone").href = location.origin + "/downloadmaster/task_hand.asp";
		}
		var httpurl = "http://" + location.hostname;
		var httpsurl= "https://" + location.hostname;
		if(initial_array[12] == 1 && location.hostname != initial_array[10] && location.hostname != initial_array[29]) {
			if(initial_array[36] == 2) {//both https and http
				if(location.protocol == "https:") {
					document.getElementById("to_DDNS").href = httpsurl + ":" + initial_array[31] + "/Advanced_ASUSDDNS_Content.asp";
				} else {
					document.getElementById("to_DDNS").href = httpurl + ":" + initial_array[11]+ "/Advanced_ASUSDDNS_Content.asp";
				}
			} else if(initial_array[36] == 1) {//only https
				document.getElementById("to_DDNS").href = httpsurl + ":" + initial_array[31] + "/Advanced_ASUSDDNS_Content.asp";
			} else {//only http
				document.getElementById("to_DDNS").href = httpurl + ":" + initial_array[11]+ "/Advanced_ASUSDDNS_Content.asp";
			}
		} else {//lan
			if(initial_array[36] == 2) {//both https and http
				if(location.protocol == "https:") {
					document.getElementById("to_DDNS").href = httpsurl + ":" + initial_array[35] + "/Advanced_ASUSDDNS_Content.asp";
				} else {
					document.getElementById("to_DDNS").href = httpurl + "/Advanced_ASUSDDNS_Content.asp";
				}
			} else if(initial_array[36] == 1) {//only https
				document.getElementById("to_DDNS").href = httpsurl+ ":" + initial_array[35] + "/Advanced_ASUSDDNS_Content.asp";
			} else {//only http
				document.getElementById("to_DDNS").href = httpurl + "/Advanced_ASUSDDNS_Content.asp";
			}
		}
		if(initial_array[18] == 1) {
			$("DDNSname").innerHTML = initial_array[19];
			$("DDNSname1").innerHTML = initial_array[19];
		}
		else {
			$("DDNSname").innerHTML = initial_array[17];
			$("DDNSname1").innerHTML = initial_array[17];
		}
		if(initial_array[18] == 1 && initial_array[19].length != "") {
			document.getElementById("WANforDM").href = "http://" + initial_array[19] +":"+initial_array[13];
			document.getElementById("WANforDM1").href = "https://" + initial_array[19] +":"+initial_array[34];
		}
		else {
			document.getElementById("WANforDM").href = "http://" + initial_array[17] +":"+initial_array[13];
			document.getElementById("WANforDM1").href = "https://" + initial_array[17] +":"+initial_array[34];
		}
		if(initial_array[33]!=null && initial_array[33]=="1"){
			if((initial_array[9] == 1 && initial_array[17] != "0.0.0.0") ||(initial_array[17] != "0.0.0.0" && initial_array[23] ==0)){
				$j("#WAN_explain_1").show();
				if(httpTag)
				{//http
    					$j("#WANforDM").show();
						$j("#WANforDM1").hide();
				} else {//https
						$j("#WANforDM1").show();
						$j("#WANforDM").hide();
				}
				if(initial_array[12] == 1 || location.host.split(":")[0] == initial_array[10])
				{	$j("#WAN_explain_2").show();
				}
				else
					$j("#WAN_explain_2").hide();
				}
			else{
				$j("#WAN_explain_1").hide();
				$j("#WAN_explain_2").hide();
				}
		}
		else{
				$j("#WAN_explain_1").hide();
				$j("#WAN_explain_2").hide();
		}
	}
/*function initial_general(){
	var url = "dm_apply.cgi";
	var action_mode = "initial";
	var type = "General";
	url += "?action_mode=" + action_mode + "&download_type=" +type+ "&t=" +Math.random();
	$j.get(url,function(data){initial_general_status(data);});
	}*/

function initial_bt_status(data){
	var initial_array = new Array();
	eval("initial_array="+data);
	if(initial_array[10]=="0"){
		$j("#Select_port1").attr("checked","checked");
		$j("#Incoming_port").attr("value",initial_array[0]);
		$j("#bt_port_tmp").attr("value",initial_array[0]);
		document.getElementById('Incoming_port').readOnly= true;
	}
	else{
		$j("#Select_port2").attr("checked","checked");
		$j("#Incoming_port").attr("value",initial_array[0]);
		$j("#bt_port_tmp").attr("value",initial_array[0]);
		document.getElementById('Incoming_port').readOnly= false;
	}
	if(initial_array[1]!=null && initial_array[1]!=""){
		$j("#Auth_type"+initial_array[1]).attr("selected","selected");
		}
	if(initial_array[2]!=null && initial_array[2]!=""){
		$j("#Max_torrent_peer").attr("value",initial_array[2]);
		}
	if(initial_array[3]!=null && initial_array[3]!=""){
		$j("#Max_peer").attr("value",initial_array[3]);
		}
	if(initial_array[4]!=null && initial_array[4]!=""){
		$j("#Enable_dht").attr("value",initial_array[4]);
		if(initial_array[4] == 0)
		$j("#radio_wl2_closed").attr("title","off");
		else
		$j("#radio_wl2_closed").attr("title","on");
		$j('#radio_wl2_closed').iphoneSwitch($j("#Enable_dht").attr("value"), 
							 function() {
								$j("#Enable_dht").attr("value","1");
								$j("#radio_wl2_closed").attr("title","on");
							 },
							 function() {
								$j("#Enable_dht").attr("value","0");
								$j("#radio_wl2_closed").attr("title","off");
							 },
							 {
								//switch_on_container_path: '/plugin/iphone_switch_container_off.png'
							 }
						);
		}
	if(initial_array[5]!=null && initial_array[5]!=""){
		$j("#download_speed_limit").attr("value",initial_array[6]);
		if(initial_array[5] == 0){
			$j("#Down_limit").attr("value","0");
			//document.getElementById("Down_limit_checkbox").checked = true;
			document.getElementById('Down_limit_checkbox').checked = true;
			$j("#download_speed_limit").hide();
			$j("#download_speed_span").hide();
			}
		else{
			$j("#Down_limit").attr("value","1");
			//document.getElementById("Down_limit_checkbox").checked = false;
			document.getElementById('Down_limit_checkbox').checked = false;
			$j("#download_speed_limit").show();
			$j("#download_speed_span").show();
			}
		//Maximum_download_speed();
		}
	if(initial_array[7]!=null && initial_array[7]!=""){
		$j("#upload_speed_limit").attr("value",initial_array[8]);
		if(initial_array[7] == 0){
			$j("#Up_limit").attr("value","0");
			//document.getElementById("Up_limit_checkbox").checked = true;
			document.getElementById('Up_limit_checkbox').checked = true;
			$j("#upload_speed_limit").hide();
			$j("#upload_speed_span").hide();
			}
		else{
			$j("#Up_limit").attr("value","1");
			//document.getElementById("Up_limit_checkbox").checked = false;
			document.getElementById('Up_limit_checkbox').checked = false;
			$j("#upload_speed_limit").show();
			$j("#upload_speed_span").show();
			}
		//Maximum_upload_speed();
		}

	if(initial_array[9]!=null && initial_array[9]!=""){
		$j("#Enable_pex").attr("value",initial_array[9]);
		if(initial_array[9] == 0)
		$j("#radio_pex_closed").attr("title","off");
		else
		$j("#radio_pex_closed").attr("title","on");
		$j('#radio_pex_closed').iphoneSwitch($j("#Enable_pex").attr("value"), 
							 function() {
								$j("#Enable_pex").attr("value","1");
								$j("#radio_pex_closed").attr("title","on");
							 },
							 function() {
								$j("#Enable_pex").attr("value","0");
								$j("#radio_pex_closed").attr("title","off");
							 },
							 {
								//switch_on_container_path: '/plugin/iphone_switch_container_off.png'
							 }
						);
		}
	}	
	
function initial_bt(){
	var url = "dm_apply.cgi";
	var action_mode = "initial";
	var type = "BT";
	url += "?action_mode=" + action_mode + "&download_type=" +type+ "&t=" +Math.random();
	$j.get(url,function(data){initial_bt_status(data);});
	}
function initial_nzb_status(data){
	var initial_array = new Array();
	var SSLvalue = 0;
	eval("initial_array="+data);
	$j("#Server1Host").attr("value",initial_array[0]);
	$j("#serverPort").attr("value",initial_array[1]);
	if(initial_array[2]=="yes"){
		SSLvalue=1;
		$j("#Encryption").attr("value","yes");
		$j("#radio_ddns_enable_x").attr("title","on");
		}
	else if(initial_array[2]=="no"){
		SSLvalue=0;
		$j("#Encryption").attr("value","no");
		$j("#radio_ddns_enable_x").attr("title","off");
		}
	$j('#radio_ddns_enable_x').iphoneSwitch(SSLvalue, 
							 function() {
								 $j("#Encryption").attr("value","yes");
								 $j("#radio_ddns_enable_x").attr("title","on");
								//document.form.ddns_enable_x.value = "1";
								//return change_common_radio(document.form.ddns_enable_x, 'LANHostConfig', 'ddns_enable_x', '1')
							 },
							 function() {
								$j("#Encryption").attr("value","no");
								$j("#radio_ddns_enable_x").attr("title","off");
								//document.form.ddns_enable_x.value = "0";
								//return change_common_radio(document.form.ddns_enable_x, 'LANHostConfig', 'ddns_enable_x', '1')
							 },
							 {
								//switch_on_container_path: '/plugin/iphone_switch_container_off.png'
							 }
						);
	$j("#User_name").attr("value",initial_array[3]);
	$j("#Password").attr("value",initial_array[4]);
	$j("#Confirm_Password").attr("value",initial_array[4]);
	$j("#Number_of_connections").attr("value",initial_array[5]);
	$j("#downloadSpeed_limit").attr("value",initial_array[6]);
	if(initial_array[6] == 0){
		$j("#NZBDown_limit_checkbox").attr("value",1);
		document.getElementById("NZBDown_limit_checkbox").checked = true;
		$j("#downloadSpeed_limit").hide();
		$j("#NZBdownload_speed_span").hide();
	}
	else{
		$j("#NZBDown_limit_checkbox").attr("value",0);
		document.getElementById("NZBDown_limit_checkbox").checked = false;
	}
	}

function initial_nzb(){
	var url = "dm_apply.cgi";
	var action_mode = "initial";
	var type = "NZB";
	url += "?action_mode=" + action_mode + "&download_type=" +type+ "&t=" +Math.random();
	$j.get(url,function(data){initial_nzb_status(data);});
	
	}
	
function create_tree(){
	var rootNode = new Ext.tree.TreeNode({ text:'/tmp/mnt', id:'0'}); /*Download2->/tmp/mnt*/
	var rootNodechild = new Ext.tree.TreeNode({ text:'', id:'0t'});
	rootNode.appendChild(rootNodechild);
	var tree = new Ext.tree.TreePanel({
			tbar:[{text:multiLanguage_setting_array[multi_INT][2],handler:function(){$j("#PATH").attr("value",Download_path);hidePanel();}},
				 {text:multiLanguage_setting_array[multi_INT][63],handler:function(){
					while(rootNode.firstChild)
						rootNode.removeChild(rootNode.firstChild);
					rootNode.appendChild(rootNodechild);
				}},
				{text:multiLanguage_setting_array[multi_INT][64],handler:function(){
					show_AddFloder();
					}},
				'->',{text:'X',handler:function(){hidePanel();}}
			],
			title:multiLanguage_setting_array[multi_INT][62],
			applyTo:'tree',
			root:rootNode,
			height:300,
			autoScroll:true
	});
	tree.on('expandnode',function(node){
		//alert('展开');
		var allParentNodes = getAllParentNodes(node);
		var path='';
		for(var j=0; j<allParentNodes.length; j++){
		path = allParentNodes[j].text + '/' +path;
		}
		//path = BASE_PATH + '/' + path; //20121206 magic
		initial_dir(path,node);
	});
	tree.on('collapsenode',function(node){
		while(node.firstChild){
			node.removeChild(node.firstChild);
		}
		var childNode = new Ext.tree.TreeNode({ text:'', id:'0t'});
		node.appendChild(childNode);
	});
	tree.on('click',function(node){
	//	alert('单击');
		var allParentNodes = getAllParentNodes(node);
		var path='';
		for(var j=0; j<allParentNodes.length; j++){
		path = allParentNodes[j].text + '/' +path;
		}

		
		//Download_path = path;
		Download_path =	path.slice(0,path.length-1); //20121206 magic
		//path = BASE_PATH + '/' + path; //20121206 magic
		var url = "dm_disk_info.cgi";
		var type = "General";
		url += "?action_mode=" +path+ "&t=" +Math.random();
		$j.get(url,function(data){initial_folderlist(data);});
	});
	//tree.on('dblclick',function(node){
	//	//alert('双击');
	//	var allParentNodes = getAllParentNodes(node);
	//	var path='';
	//	for(var j=0; j<allParentNodes.length; j++){
	//	path = allParentNodes[j].text + '/' +path;
	//	}
	//	initial_dir(path,node);
	//});
}
function getAllParentNodes(node) {
	var parentNodes = [];
	parentNodes.push(node);
	while (node.parentNode) {
		parentNodes = parentNodes.concat(node.parentNode);
		node = node.parentNode;
		}
	return parentNodes;
	};

function initial_dir_status(data,node){
	dm_dir.length = 0;
	if(data == "/" || (data != null && data != "")){
		eval("dm_dir=[" + data +"]");	
		while(node.lastChild &&(node.lastChild !=node.firstChild)) {
    			node.removeChild(node.lastChild);
		}
		for(var i=0; i<dm_dir.length; i++){
			var childNodeId = node.id +i;
			if(dm_dir[i]=="asusware"){
				continue;
			}
			var childnode = new Ext.tree.TreeNode({id:childNodeId,text:dm_dir[i]});
			node.appendChild(childnode);
			var childnodeT = new Ext.tree.TreeNode({id:childNodeId+'t',text:''});
			childnode.appendChild(childnodeT);
		}
		node.removeChild(node.firstChild);
	}
	else{
		while(node.firstChild){
			node.removeChild(node.firstChild);
		}
	}
}

function initial_dir(path,node){
	var url = "dm_disk_info.cgi";
	var type = "General";
	url += "?action_mode=" +path+ "&t=" +Math.random();
	$j.get(url,function(data){initial_dir_status(data,node);});
	}
	
function getWH(){
	var winWidth;
	var winHeight;
	winWidth = document.documentElement.scrollWidth;
	if(document.documentElement.clientHeight > document.documentElement.scrollHeight)
	winHeight = document.documentElement.clientHeight;
	else
	winHeight = document.documentElement.scrollHeight;
	$("DM_mask").style.width = winWidth+"px";
	$("DM_mask").style.height = winHeight+"px";
	}	

function showPanel(){
	WH_INT = setInterval("getWH();",1000);
   	$j("#DM_mask").fadeIn(1000);
    $j("#panel_add").show(1000);
	create_tree();
	}
function hidePanel(){
	($j("#tree").children()).remove();
	clearInterval(WH_INT);
	$j("#DM_mask").fadeOut('fast');
	$j("#panel_add").hide('fast');
	}
function getFloderWH(){
	var winWidth;
	var winHeight;
	winWidth = document.documentElement.scrollWidth;
	if(document.documentElement.clientHeight > document.documentElement.scrollHeight)
	winHeight = document.documentElement.clientHeight;
	else
	winHeight = document.documentElement.scrollHeight;
	$("DM_mask_floder").style.width = winWidth+"px";
	$("DM_mask_floder").style.height = winHeight+"px";
	}
function show_AddFloder(){
	Floder_WH_INT = setInterval("getFloderWH();",1000);
	$j("#DM_mask_floder").fadeIn(1000);
	$j("#panel_addFloder").show(1000);
	}
function hide_AddFloder(){
	clearInterval(Floder_WH_INT);
	$j("#DM_mask_floder").fadeOut('fast');
	$j("#panel_addFloder").hide('fast');
	$j("#newFloder").attr("value","");
	}
function AddFloderName(){
	var url;
	var action_mode = "DM_ADD_FLODER";
	var new_floder_name = document.getElementById("newFloder").value;
	//var path = BASE_PATH + '/' + Download_path; //20121206 magic

	var path = Download_path;
	url = "dm_apply.cgi?action_mode=" +action_mode+"&new_floder_name="+ new_floder_name+"&path="+path+"&t=" +Math.random();
	var judgment = validForm();
	if(judgment){
		$j.get(url,function(data){
							($j("#tree").children()).remove();
							create_tree();
							//Download_path = 'Download2'; //20121206 magic
							Download_path = '/tmp/mnt';
							hide_AddFloder();})
		}
	}
function validForm(){
	$("newFloder").value = trim($("newFloder").value);
	
	// share name
	if($("newFloder").value.length == 0){
		alert(multiLanguage_setting_array[multi_INT][56]);
		$("newFloder").focus();
		return false;
	}
	else if($("newFloder").value == "asusware"){
		//alert("The share name already exists in this volume. Please enter a different name!");
		alert(multiLanguage_setting_array[multi_INT][89]+multiLanguage_setting_array[multi_INT][90])
		$("newFloder").focus();
		return false;
	}
	
	var re = new RegExp("[^a-zA-Z0-9 _-]+", "gi");
	if(re.test($("newFloder").value)){
		alert(multiLanguage_setting_array[multi_INT][57]);
		$("newFloder").focus();
		return false;
	}
	
	if(checkDuplicateName($("newFloder").value, folderlist)){
		alert(multiLanguage_setting_array[multi_INT][58]+"\n"+multiLanguage_setting_array[multi_INT][59]);
		$("newFloder").focus();
		return false;
	}
	
	if(trim($("newFloder").value).length > 12)
		if (!(confirm(multiLanguage_setting_array[multi_INT][60]+"\n"+multiLanguage_setting_array[multi_INT][61])))
			return false;
	
	return true;
}
function checkDuplicateName(newname, teststr){
	var existing_string = teststr.join(',');
	existing_string = "," + existing_string + ",";
	var newstr = "," + trim(newname) + ","; 

	var re = new RegExp(newstr,"gi");
	var matchArray =  existing_string.match(re);
	if (matchArray != null)
		return true;
	else
		return false;
}
function initial_folderlist(data){
	eval("folderlist=["+data+"]");
	}


function validate_timerange(o, p)
{
	if (o.value.length==0) 
		o.value = "00";
	else if (o.value.length==1) 
		o.value = "0" + o.value;
		
	if (o.value.charAt(0)<'0' || o.value.charAt(0)>'9') 
		o.value = "00";
	else if (o.value.charAt(1)<'0' || o.value.charAt(1)>'9') 
		o.value = "00";
	else if (p==0 || p==2)
	{
		if(o.value>23){
			//alert('Please enter a value between 00 to 23');
			alert(multiLanguage_setting_array[multi_INT][91]);
			o.value = "00";
			o.focus();
			o.select();
			return false;			
		}	
		return true;
	}
	else
	{
		if(o.value>59){
			//alert('Please enter a value between 00 to 59');
			alert(multiLanguage_setting_array[multi_INT][92]);
			o.value = "00";
			o.focus();
			o.select();
			return false;			
		}	
		return true;
	}
	return true;
}


function getDateCheck(str, pos)
{if (str.charAt(pos) == '1')
return true;
else
return false;
}
function getTimeRange(str, pos)
{if (pos == 0)
return str.substring(0,2);
else if (pos == 1)
return str.substring(2,4);
else if (pos == 2)
return str.substring(4,6);
else if (pos == 3)
return str.substring(6,8);
}

function setDateCheck(d1, d2, d3, d4, d5, d6, d7)
{str = "";
if (d7.checked == true ) str = "1" + str;
else str = "0" + str;
if (d6.checked == true ) str = "1" + str;
else str = "0" + str;
if (d5.checked == true ) str = "1" + str;
else str = "0" + str;
if (d4.checked == true ) str = "1" + str;
else str = "0" + str;
if (d3.checked == true ) str = "1" + str;
else str = "0" + str;
if (d2.checked == true ) str = "1" + str;
else str = "0" + str;
if (d1.checked == true ) str = "1" + str;
else str = "0" + str;
return str;
}

function setTimeRange(sh, sm, eh, em)
{return(sh.value+sm.value+eh.value+em.value);
}

function updateDateTime()
{
		document.form.dm_radio_date_x.value = setDateCheck(
		document.form.dm_radio_date_x_Sun,
		document.form.dm_radio_date_x_Mon,
		document.form.dm_radio_date_x_Tue,
		document.form.dm_radio_date_x_Wed,
		document.form.dm_radio_date_x_Thu,
		document.form.dm_radio_date_x_Fri,
		document.form.dm_radio_date_x_Sat);
		document.form.dm_radio_time_x.value = setTimeRange(
		document.form.dm_radio_time_x_starthour,
		document.form.dm_radio_time_x_startmin,
		document.form.dm_radio_time_x_endhour,
		document.form.dm_radio_time_x_endmin);
		document.form.dm_radio_time2_x.value = setTimeRange(
		document.form.dm_radio_time2_x_starthour,
		document.form.dm_radio_time2_x_startmin,
		document.form.dm_radio_time2_x_endhour,
		document.form.dm_radio_time2_x_endmin);
	}
	
function get_layer(barcode){
	var tmp, layer;
	
	layer = 0;
	while(barcode.indexOf('_') != -1){
		barcode = barcode.substring(barcode.indexOf('_'), barcode.length);
		++layer;
		barcode = barcode.substring(1);
	}
	
	return layer;
}
	
function BuildTree(){
	var ItemText, ItemSub, ItemIcon;
	var vertline, isSubTree;
	var layer;
	var short_ItemText = "";
	var shown_ItemText = "";
	var ItemBarCode ="";		
	var TempObject = "";
	for(var i = 0; i < this.Items.length; ++i){	
		this.Items[i] = this.Items[i].split("#");
		var Item_size = 0;
		Item_size = this.Items[i].length;
		if(Item_size > 3){
			var temp_array = new Array(3);	
			
			temp_array[2] = this.Items[i][Item_size-1];
			temp_array[1] = this.Items[i][Item_size-2];			
			temp_array[0] = "";
			for(var j = 0; j < Item_size-2; ++j){
				if(j != 0)
					temp_array[0] += "#";
				temp_array[0] += this.Items[i][j];
			}
			this.Items[i] = temp_array;
		}	
		ItemText = (this.Items[i][0]).replace(/^[\s]+/gi,"").replace(/[\s]+$/gi,"");
		ItemBarCode = this.FromObject+"_"+(this.Items[i][1]).replace(/^[\s]+/gi,"").replace(/[\s]+$/gi,"");
		ItemSub = parseInt((this.Items[i][2]).replace(/^[\s]+/gi,"").replace(/[\s]+$/gi,""));
		layer = get_layer(ItemBarCode.substring(1));
		if(layer >= 3){
			if(ItemText.length > 21)
		 		short_ItemText = ItemText.substring(0,30)+"...";
		 	else
		 		short_ItemText = ItemText;
		}
		else
			short_ItemText = ItemText;
		
		shown_ItemText = showhtmlspace(short_ItemText);
		
		if(layer == 1)
			ItemIcon = 'disk';
		else if(layer == 2)
			ItemIcon = 'part';
		else
			ItemIcon = 'folders';
		
		SubClick = ' onclick="GetFolderItem(this, ';
		if(ItemSub <= 0){
			SubClick += '0);"';
			isSubTree = 'n';
		}
		else{
			SubClick += '1);"';
			isSubTree = 's';
		}
		
		if(i == this.Items.length-1){
			vertline = '';
			isSubTree += '1';
		}
		else{
			vertline = ' background="images/Tree/vert_line.gif"';
			isSubTree += '0';
		}

		if(layer == 2 && isSubTree == 'n1'){	// Uee to rebuild folder tree if disk without folder, Jieming add at 2012/08/29
			//document.aidiskForm.test_flag.value = 1;			
		}
		

		TempObject +='<table class="tree_table" id="bug_test">';
		TempObject +='<tr>';
		// the line in the front.
		TempObject +='<td class="vert_line">';
		TempObject +='<img id="a'+ItemBarCode+'" onclick=\'$("d'+ItemBarCode+'").onclick();\' class="FdRead" src="images/Tree/vert_line_'+isSubTree+'0.gif">';
		TempObject +='</td>';
	
		/*if(layer == 3){
			TempObject +='<td>';		
			TempObject +='<img id="c'+ItemBarCode+'" onclick=\'$("d'+ItemBarCode+'").onclick();\' src="images/New_ui/advancesetting/'+ItemIcon+'.png">';
			TempObject +='</td>';
			TempObject +='<td>';
			TempObject +='<span id="d'+ItemBarCode+'"'+SubClick+' title="'+ItemText+'">'+shown_ItemText+'</span>\n';		
			TempObject +='</td>';
		}*/
		if(layer >= 2){
			TempObject +='<td>';
			TempObject +='<table class="tree_table">';
			TempObject +='<tr>';
			TempObject +='<td class="vert_line">';
			TempObject +='<img id="c'+ItemBarCode+'" onclick=\'$("d'+ItemBarCode+'").onclick();\' src="images/New_ui/advancesetting/'+ItemIcon+'.png">';
			TempObject +='</td>';
			TempObject +='<td class="FdText">';
			TempObject +='<span id="d'+ItemBarCode+'"'+SubClick+' title="'+ItemText+'">'+shown_ItemText+'</span>';
			TempObject +='</td>';
			TempObject +='<td></td>';
			TempObject +='</tr>';
			TempObject +='</table>';			
			TempObject +='</td>';
			TempObject +='</tr>';
			TempObject +='<tr><td></td>';			
			TempObject +='<td colspan=2><div id="e'+ItemBarCode+'" ></div></td>';
		}
		else{
			TempObject +='<td>';
			TempObject +='<table><tr><td>';
			TempObject +='<img id="c'+ItemBarCode+'" onclick=\'$("d'+ItemBarCode+'").onclick();\' src="images/New_ui/advancesetting/'+ItemIcon+'.png">';
			TempObject +='</td><td>';
			TempObject +='<span id="d'+ItemBarCode+'"'+SubClick+' title="'+ItemText+'">'+shown_ItemText+'</span>';
			TempObject +='</td></tr></table>';
			TempObject +='</td>';
			TempObject +='</tr>';
			TempObject +='<tr><td></td>';
			TempObject +='<td><div id="e'+ItemBarCode+'" ></div></td>';
		}
		
		TempObject +='</tr>';
	}
	TempObject +='</table>';
	$("e"+this.FromObject).innerHTML = TempObject;
}
	
function get_tree_items(treeitems){
	this.isLoading = 1;
	this.Items = treeitems;
		if(this.Items && this.Items.length > 0){
			BuildTree();
		}	
}
	
function get_layer_items(layer_order){
	var url = "dm_disk_info.cgi?action_mode=" + layer_order + "&t=" +Math.random();//eric
	$j.get(url,function(data){ 
				var treeitems = new Array();
				eval("treeitems="+data);
				get_tree_items(treeitems);});//eirc 11.9
}
	
function cal_panel_block(){
	var blockmarginLeft;
	if (window.innerWidth)
		winWidth = window.innerWidth;
	else if ((document.body) && (document.body.clientWidth))
		winWidth = document.body.clientWidth;
		
	if (document.documentElement  && document.documentElement.clientHeight && document.documentElement.clientWidth){
		winWidth = document.documentElement.clientWidth;
	}

	if(winWidth >1050){	
		winPadding = (winWidth-1050)/2;	
		winWidth = 1105;
		blockmarginLeft= (winWidth*0.25)+winPadding;
	}
	else if(winWidth <=1050){
		blockmarginLeft= (winWidth)*0.25+document.body.scrollLeft;	
	}

	$("folderTree_panel").style.marginLeft = blockmarginLeft+"px";
}

function get_disk_tree(){
	updateStatusCounter=0;
	cal_panel_block();
	$j("#folderTree_panel").fadeIn(300);
	get_layer_items("get_disk_tree");
}

function cancel_folderTree(){
	updateStatusCounter=0;
	this.FromObject ="0";
	$j("#folderTree_panel").fadeOut(300);
}

function get_folderpath(layer_order)
{
	var path_la;
	var len;
	path_la = "";
	var layer_array = layer_order.split("_");
	var la_temp="";
	for(len=0;len<layer_array.length;len++)
	{
		if(len==0)
		la_temp=la_temp+layer_array[len];
		else
		{
			la_temp=la_temp+"_"+layer_array[len];
			path_la=path_la+$("d"+la_temp).title+"/";
		}
	}
	return path_la;
}

function GetFolderItem(selectedObj, haveSubTree){
	var barcode, layer = 0;
	showClickedObj(selectedObj);
	barcode = selectedObj.id.substring(1);	
	layer = get_layer(barcode);
	path_directory = get_folderpath(barcode);
	
	if(layer == 0)
		alert("Machine: Wrong");
	else if(layer == 1){
		// chose Disk
		setSelectedDiskOrder(selectedObj.id);
		//path_directory = build_array(selectedObj,layer);
		//alert("path_directory="+path_directory);
		//$('createFolderBtn').src = "images/New_ui/advancesetting/FolderAdd.png";
		//$('deleteFolderBtn').src = "images/New_ui/advancesetting/FolderDel.png";
		//$('modifyFolderBtn').src = "images/New_ui/advancesetting/FolderMod.png";
		//$('createFolderBtn').onclick = function(){};
		//$('deleteFolderBtn').onclick = function(){};
		//$('modifyFolderBtn').onclick = function(){};
		$('createFolderBtn').className = "createFolderBtn";
		$('createFolderBtn').onclick = function(){};
	}
	else if(layer == 2){
		// chose Partition
		setSelectedPoolOrder(selectedObj.id);
		//path_directory = build_array(selectedObj,layer);
		//alert("path_directory="+path_directory);
		//$('createFolderBtn').src = "images/New_ui/advancesetting/FolderAdd_0.png";
		//$('deleteFolderBtn').src = "images/New_ui/advancesetting/FolderDel.png";
		//$('modifyFolderBtn').src = "images/New_ui/advancesetting/FolderMod.png";
		//$('createFolderBtn').onclick = function(){popupWindow('OverlayMask','popCreateFolder.asp');};		
		//$('deleteFolderBtn').onclick = function(){};
		//$('modifyFolderBtn').onclick = function(){};
		$('createFolderBtn').className = "createFolderBtn_add";
		$('createFolderBtn').onclick = function(){popupWindow('OverlayMask','popCreateFolder.asp');};
		document.aidiskForm.layer_order.disabled = "disabled";
		document.aidiskForm.layer_order.value = barcode;
		
	}
	else if(layer >= 3){
		// chose Shared-Folder
		setSelectedFolderOrder(selectedObj.id);
		//path_directory = build_array(selectedObj,layer);
		//alert("path_directory="+path_directory);
		//$('createFolderBtn').src = "images/New_ui/advancesetting/FolderAdd_0.png";
		//$('deleteFolderBtn').src = "images/New_ui/advancesetting/FolderDel_0.png";
		//$('modifyFolderBtn').src = "images/New_ui/advancesetting/FolderMod_0.png";
		//$('createFolderBtn').onclick = function(){popupWindow('OverlayMask','popCreateFolder.asp');};		
		//$('deleteFolderBtn').onclick = function(){popupWindow('OverlayMask','popDeleteFolder.asp');};
		//$('modifyFolderBtn').onclick = function(){popupWindow('OverlayMask','popModifyFolder.asp');};
		$('createFolderBtn').className = "createFolderBtn_add";
		$('createFolderBtn').onclick = function(){popupWindow('OverlayMask','popCreateFolder.asp');};
		document.aidiskForm.layer_order.disabled = "disabled";
		document.aidiskForm.layer_order.value = barcode;
	}

	if(haveSubTree)
		GetTree(barcode, 1,path_directory);
}

function showClickedObj(clickedObj){
	if(this.lastClickedObj != 0)
		this.lastClickedObj.className = "lastfolderClicked";  //this className set in AiDisk_style.css
	
	clickedObj.className = "folderClicked";
	this.lastClickedObj = clickedObj;
}

function getDiskBarcode(src_barcode){
	var layer = get_layer(src_barcode);
	var str_len, tmp_str;
	
	if(layer < 1)
		return "";
	else if(layer == 1)
		return src_barcode;
	
	str_len = src_barcode.indexOf('_');
	tmp_str = src_barcode.substring(str_len+1);
	
	str_len += tmp_str.indexOf('_')+1;
	
	return src_barcode.substring(0, str_len);
}

function getDiskOrder(disk_barcode){
	return parseInt(disk_barcode.substring(disk_barcode.indexOf('_')+1));
}

/*function build_array(obj,layer){
	var path_temp ="/tmp/mnt";
	var layer2_path ="";
	var layer3_path ="";
	if(obj.id.length>6){
		if(layer ==3){
			layer3_path = "/" + $(obj.id).innerHTML;
			while(layer3_path.indexOf("&nbsp;") != -1)
				layer3_path = layer3_path.replace("&nbsp;"," ");
			while(layer3_path.indexOf("&amp;") != -1)
				layer3_path = layer3_path.replace("&amp;","&");
				
			if(obj.id.length >8)
				layer2_path = "/" + $(obj.id.substring(0,obj.id.length-3)).innerHTML;
			else
				layer2_path = "/" + $(obj.id.substring(0,obj.id.length-2)).innerHTML;
			
			while(layer2_path.indexOf("&nbsp;") != -1)
				layer2_path = layer2_path.replace("&nbsp;"," ");
			while(layer3_path.indexOf("&amp;") != -1)
				layer3_path = layer3_path.replace("&amp;","&");
		}
	}
	if(obj.id.length>4 && obj.id.length<=6){
		if(layer ==2){
			layer2_path = "/" + $(obj.id).innerHTML;
			while(layer2_path.indexOf("&nbsp;") != -1)
				layer2_path = layer2_path.replace("&nbsp;"," ");
			while(layer3_path.indexOf("&amp;") != -1)
				layer3_path = layer3_path.replace("&amp;","&");
		}
	}
	path_temp = path_temp + layer2_path +layer3_path;
	return path_temp;
}*/

function GetTree(layer_order, v,layer_path){
	if(layer_order == "0"){
		this.FromObject = layer_order;
		$('d'+layer_order).innerHTML = '<span class="FdWait">. . . . . . . . . .</span>';
		setTimeout('get_layer_items("'+layer_path+'", "gettree")', 1);		
		return;
	}
	
	if($('a'+layer_order).className == "FdRead"){
		$('a'+layer_order).className = "FdOpen";
		$('a'+layer_order).src = "images/Tree/vert_line_s"+v+"1.gif";		
		this.FromObject = layer_order;		
		$('e'+layer_order).innerHTML = '<img src="images/Tree/folder_wait.gif">';
		setTimeout('get_layer_items("'+layer_path+'", "gettree")', 1);
	}
	else if($('a'+layer_order).className == "FdOpen"){
		$('a'+layer_order).className = "FdClose";
		$('a'+layer_order).src = "images/Tree/vert_line_s"+v+"0.gif";		
		$('e'+layer_order).style.position = "absolute";
		$('e'+layer_order).style.visibility = "hidden";
	}
	else if($('a'+layer_order).className == "FdClose"){
		$('a'+layer_order).className = "FdOpen";
		$('a'+layer_order).src = "images/Tree/vert_line_s"+v+"1.gif";		
		$('e'+layer_order).style.position = "";
		$('e'+layer_order).style.visibility = "";
	}
	else
		alert("Error when show the folder-tree!");
}

function setSelectedDiskOrder(selectedId){
	this.selectedDiskBarcode = getDiskBarcode(selectedId.substring(1));
	this.selectedPoolBarcode = "";
	this.selectedFolderBarcode = "";
	
	this.selectedDiskOrder = getDiskOrder(this.selectedDiskBarcode);
	this.selectedPoolOrder = -1;
	this.selectedFolderOrder = -1;
}

function getPoolOrder(pool_barcode){
	return parseInt(pool_barcode.substring(getDiskBarcode(pool_barcode).length+1));
}

function setSelectedPoolOrder(selectedId){
	this.selectedDiskBarcode = getDiskBarcode(selectedId.substring(1));
	this.selectedPoolBarcode = getPoolBarcode(selectedId.substring(1));
	this.selectedFolderBarcode = "";
	
	this.selectedDiskOrder = getDiskOrder(this.selectedDiskBarcode);
	this.selectedPoolOrder = this.selectedDiskOrder+getPoolOrder(this.selectedPoolBarcode);
	this.selectedFolderOrder = -1;
}

function getPoolBarcode(src_barcode){
	var layer = get_layer(src_barcode);
	var str_len, tmp_str;
	
	if(layer < 2)
		return "";
	else if(layer == 2)
		return src_barcode;
	
	str_len = getDiskBarcode(src_barcode).length;
	tmp_str = src_barcode.substring(str_len+1);
	
	str_len += tmp_str.indexOf('_')+1;
	
	return src_barcode.substring(0, str_len);
}

function popupWindow(w,u){

	disableCheckChangedStatus();
	
	winW_H();
	
	$(w).style.width = winW+"px";
	$(w).style.height = winH+"px";
	$(w).style.visibility = "visible";
	
	$('popupframe').src = u;
}

function get_sharedfolder_in_pool(poolname){
	var url = "dm_disk_info.cgi?action_mode=" + poolname + "&t=" +Math.random();//eric
	var lists = new Array();
	$j.ajax({url: url,
		async: false,
		success: function(data){eval("lists="+data);}
		});
	return lists;	
}

function hidePop(flag){
	if(flag != "apply")
		enableCheckChangedStatus();

	$('popupframe').src = "";
	$('OverlayMask').style.visibility = "hidden";
}

function creatfolder(path){
	var url = "dm_apply.cgi?action_mode=DM_ADD_FOLDER&path=" + path + "&t=" +Math.random();
	var get_data;
	$j.ajax({url: url,
		async: false,
		success: function(data){get_data = data;}
		});
	return get_data;
}

function setSelectedFolderOrder(selectedId){
	this.selectedDiskBarcode = getDiskBarcode(selectedId.substring(1));
	this.selectedPoolBarcode = getPoolBarcode(selectedId.substring(1));
	this.selectedFolderBarcode = getFolderBarcode(selectedId.substring(1));
	
	this.selectedDiskOrder = getDiskOrder(this.selectedDiskBarcode);
	this.selectedPoolOrder = this.selectedDiskOrder+getPoolOrder(this.selectedPoolBarcode);
	this.selectedFolderOrder = 1+getFolderOrder(this.selectedFolderBarcode);
}

function getFolderBarcode(src_barcode){
	var layer = get_layer(src_barcode);
	var str_len, tmp_str;
	
	if(layer < 3)
		return "";
	else if(layer >= 3)
		return src_barcode;
	
	str_len = getPoolBarcode(src_barcode).length;
	tmp_str = src_barcode.substring(str_len+1);
	
	str_len += tmp_str.indexOf('_')+1;
	
	return src_barcode.substring(0, str_len);
}

function getFolderOrder(folder_barcode){
	return parseInt(folder_barcode.substring(getPoolBarcode(folder_barcode).length+1));
}

/*function get_layer_items_test(layer_order_t){
	var url = "dm_disk_info.cgi?action_mode=layer_order=" + layer_order_t + "&t=" +Math.random();
	var flag;
	$j.ajax({url: url,
		async: false,
		success: function(data){var treeitems = new Array();eval("treeitems="+data);flag=treeitems.length;}
		});
	return flag;
}*/

/*function deletefolder(path){
	//alert("path="+path);
	path = path.replace(/\&/g,"spechar7spechar");
	path = path.replace(/\#/g,"spechar3spechar");
	path = path.replace(/\+/g,"spechar12spechar");
	path = path.replace(/\;/g,"spechar11spechar");
	//alert("encode="+encodeURI(path));
	var url = "dm_disk_info.cgi?action_mode=deletefolder=" + encodeURI(path) + "&t=" +Math.random();
	var get_data;
	$j.ajax({url: url,
		async: false,
		success: function(data){get_data = data;}
		});
	return get_data;
}

function modifyfolder(path){
	path = path.replace(/\&/g,"spechar7spechar");
	path = path.replace(/\#/g,"spechar3spechar");
	path = path.replace(/\+/g,"spechar12spechar");
	path = path.replace(/\;/g,"spechar11spechar");
	var url = "dm_disk_info.cgi?action_mode=modifyfolder=" + encodeURI(path) + "&t=" +Math.random();
	var get_data;
	$j.ajax({url: url,
		async: false,
		success: function(data){get_data = data;}
		});
	return get_data;
}*/

function confirm_folderTree(){
	updateStatusCounter=0;
	var path_value;
	path_value = path_directory.substring(0,path_directory.lastIndexOf("/"));
	while(path_value.indexOf("&nbsp;") != -1)
		path_value = path_value.replace("&nbsp;"," ");
	$('PATH').value = path_value;
	this.FromObject ="0";
	$j("#folderTree_panel").fadeOut(300);
}
