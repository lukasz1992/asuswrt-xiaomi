var Refresh_time = 5000;
var dm_array = new Array();     //all tasks
var dm_array_tmp = new Array();   //the tasks saved in this array 
var cancel_array = new Array();
var ProgressBar_array = new Array();  //all progressBars
var dm = new Array();       	//the selected task
var dm_tmp = new Array();       //saved selected task for remove
var dm_Log = new Array();   	//the selected task's desired logInfo
var dm_num = 0;             	//old task nums
var MAX_NAMELEN = 1024;      	//max url len
var BT_INIT_HASH = false;      // for BT initial or hashing
var ajaxRequest = true;    	//for circle ajaxRequest
var old_dmid = "";          	//old selected task's dmid
var old_dmid_tmp = "";      	//save old selected task's dmid
var isLogShow = false;      	//if taskLog is not show,isLogShow=false else isLogShow=true
var taskLogShowType = 0;    	//"0"is General /"1" is transfer/ "2" is File /"3" is Log
var sorted_by = "All";          //used for show desire tasks 
var old_sorted_by = "All";
var old_sorted_by2 = "All";
var WH_INT = 0;
var httpTag = 'https:' == document.location.protocol ? false : true;

function showSortList() {
	if (sorted_by == "All") {
		dm_array.length = 0;
		dm_array = dm_array_tmp.concat();
		create_task();
		dm_num = dm_array.length;
		old_sorted_by = sorted_by;
		update_task();
	}
	else if (sorted_by == "Finished_Seeding") {
		var array = new Array();
		dm_array.length = 0;
		dm_array = dm_array_tmp.concat();
		for (var i = 0; i < dm_array.length; i++) {
			if (dm_array[i][4] == "Finished" || dm_array[i][4] == "Seeding") {
				array.push(dm_array[i]);
			}
		}
		dm_array.length = 0;
		dm_array = array.concat();
		create_task();
		dm_num = dm_array.length;
		old_sorted_by = sorted_by;
		update_task();
	}
	else {
		var array = new Array();
		dm_array.length = 0;
		dm_array = dm_array_tmp.concat();
		for (var i = 0; i < dm_array.length; i++) {
			if (dm_array[i][4] == sorted_by) {
				array.push(dm_array[i]);
			}
		}
		dm_array.length = 0;
		dm_array = array.concat();
		create_task();
		dm_num = dm_array.length;
		old_sorted_by = sorted_by;
		update_task();
	}
}

function showTask_select(type) {
	switch (type) {
		case 0: if ($j("#All_tasks").attr("class") == "noselected") {
			$j("#taskClass").children("span").removeClass();
			$j("#All_tasks").siblings().addClass("noselected");
			$j("#All_tasks").addClass("selected2");
			sorted_by = "All";
			showSortList();
		}
			break;
		case 1: if ($j("#Downloading_tasks").attr("class") == "noselected") {
			$j("#taskClass").children("span").removeClass();
			$j("#Downloading_tasks").siblings().addClass("noselected");
			$j("#Downloading_tasks").addClass("selected2");
			sorted_by = "Downloading";
			showSortList();
			//Ajax_Get_DM_Status();   here  need to try on
		}
			break;
		case 2: if ($j("#Seeding_tasks").attr("class") == "noselected") {
			$j("#taskClass").children("span").removeClass();
			$j("#Seeding_tasks").siblings().addClass("noselected");
			$j("#Seeding_tasks").addClass("selected2");
			sorted_by = "Seeding";
			showSortList();
		}
			break;
		case 3: if ($j("#Paused_tasks").attr("class") == "noselected") {
			$j("#taskClass").children("span").removeClass();
			$j("#Paused_tasks").siblings().addClass("noselected");
			$j("#Paused_tasks").addClass("selected2");
			sorted_by = "Paused";
			showSortList();
		}
			break;
		case 4: if ($j("#Finished_tasks").attr("class") == "noselected") {
			$j("#taskClass").children("span").removeClass();
			$j("#Finished_tasks").siblings().addClass("noselected");
			$j("#Finished_tasks").addClass("selected2");
			sorted_by = "Finished_Seeding";
			showSortList();
		}
			break;
	}

}

function getWH() {
	var winWidth;
	var winHeight;
	winWidth = document.documentElement.scrollWidth;
	if (document.documentElement.clientHeight > document.documentElement.scrollHeight)
		winHeight = document.documentElement.clientHeight;
	else
		winHeight = document.documentElement.scrollHeight;
	$("DM_mask").style.width = winWidth + "px";
	$("DM_mask").style.height = winHeight + "px";
}

function showPanel() {
	WH_INT = setInterval("getWH();", 1000);
	$j("#DM_mask").fadeIn(1000);
	$j("#panel_add").show(1000);
	var task_code = '<span style="margin-right:15px;margin-left:15px;"><b>' + multiLanguage_array[multi_INT][18] + '</b></span>\n';
	task_code += '<input type="file" id="open_usb_dm_url" value="" name="filename" /><br /><br />\n';
	$("open_usb_dm_url_div").innerHTML = task_code;
	$j("#HTTP_usb_dm_url").attr("value", "");
}

function hidePanel() {
	clearInterval(WH_INT);
	$j("#DM_mask").fadeOut('fast');
	$j("#panel_add").hide('fast');
	$j("#HTTP_usb_dm_url").attr("value", "");
	var task_code = '<div id="open_usb_dm_url_div" style="display:block; float:left;"></div>\n';
	$j(task_code).replaceAll("#open_usb_dm_url_div");
}

//deal with dm_ctrl_status responsed
function response_dm_ctrl(data) {
	//updateCtrlIcon();
	if (data.search(/ACK_SUCESS/) >= 0) {
		Ajax_Get_DM_Status();
		return;
	}
	else if (data.search(/ACK_FAIL/) >= 0) {
		return;
	}
	else if (data.search(/ACK_LIMIT/) >= 0) {
		Ext.MessageBox.alert(multiLanguage_array[multi_INT][3], multiLanguage_array[multi_INT][74]);
		return;
	}

}

function response_dm_clear(data) {
	if (data.search(/ACK_SUCESS/) >= 0) {
		var task_code = '<div style="display:none;"></div>';
		for (var i = 0; i < dm_array.length; i++) {
			if (dm_array[i][4] == "Seeding" || dm_array[i][4] == "Finished") {
				$j(task_code).replaceAll("#" + dm_array[i][0]);
				for (var j = 0; j < ProgressBar_array.length; j++) {
					if (dm_array[i][0] == ProgressBar_array[j].stateId.substring(4))
						ProgressBar_array.splice(j, 1);
				}
			}
		}
	}
}
//设置延时函数
function sleep(numberMillis) {
	var now = new Date();
	var exitTime = now.getTime() + numberMillis;
	while (true) {
		now = new Date();
		if (now.getTime() > exitTime)
			return;
	}
}
function dm_ctrl_status(status) {
	updateStatusCounter = 0;
	var url = "dm_apply.cgi";
	var action_mode = "DM_CTRL";
	var dm_ctrl = "";
	var download_type = "";
	var t;
	switch (status) {
		case "pause_all":
			if (BT_INIT_HASH) {
				//alert("BT_init_hsash=true");
				for (var i = 0; i < dm_array.length; i++) {
					// code to be executed

					//alert("for circle ");
					//alert(dm_array[i][5]);
					if (dm_array[i][5] != "BT") {
						//alert(dm_array[i][5]);
						dm_ctrl = "paused";
						url = "dm_apply.cgi";
						url += "?action_mode=" + action_mode;
						url += "&dm_ctrl=" + dm_ctrl;
						url += "&task_id=" + dm_array[i][0];
						url += "&download_type=" + dm_array[i][5] + "&t=" + Math.random();
						//alert(url);
						$j.get(url, function (data) { response_dm_ctrl(data); });
						sleep(800);

					}
				}
			}
			else {
				dm_ctrl = "pause_all";
				url += "?action_mode=" + action_mode;
				url += "&dm_ctrl=" + dm_ctrl;
				url += "&download_type=ALL&t=" + Math.random();
				$j.get(url, function (data) { response_dm_ctrl(data); });

			}
			break;
		case "start_all":

			if (BT_INIT_HASH) {
				for (var i = 0; i < dm_array.length; i++) {
					if (dm_array[i][5] != "BT") {

						url = "dm_apply.cgi";
						dm_ctrl = "start";
						url += "?action_mode=" + action_mode;
						url += "&dm_ctrl=" + dm_ctrl;
						url += "&task_id=" + dm_array[i][0];
						url += "&download_type=" + dm_array[i][5] + "&t=" + Math.random();

						$j.get(url, function (data) { response_dm_ctrl(data); });
						sleep(800);

					}
				}
			}
			else {
				dm_ctrl = "start_all";
				url += "?action_mode=" + action_mode;
				url += "&dm_ctrl=" + dm_ctrl;
				url += "&download_type=ALL&t=" + Math.random();
				$j.get(url, function (data) { response_dm_ctrl(data); });

			}
			break;
		case "clear":
			dm_ctrl = "clear";
			url += "?action_mode=" + action_mode;
			url += "&dm_ctrl=" + dm_ctrl;
			url += "&task_id=" + dm[0];
			url += "&download_type=" + dm[5] + "&t=" + Math.random();
			$j.get(url, function (data) { response_dm_clear(data); });
			break;
		case "cancel":
			dm_ctrl = "cancel";
			url += "?action_mode=" + action_mode;
			url += "&dm_ctrl=" + dm_ctrl;
			url += "&task_id=" + dm_tmp[0];
			url += "&download_type=" + dm_tmp[5] + "&t=" + Math.random();
			dm.length = 0;
			$j.get(url, function (data) { response_dm_cancel(data); });
			break;
		case "paused":
			dm_ctrl = "paused";
			url += "?action_mode=" + action_mode;
			url += "&dm_ctrl=" + dm_ctrl;
			url += "&task_id=" + dm[0];
			url += "&download_type=" + dm[5] + "&t=" + Math.random();
			$j.get(url, function (data) { response_dm_ctrl(data); });
			break;
		case "start":
			dm_ctrl = "start";
			url += "?action_mode=" + action_mode;
			url += "&dm_ctrl=" + dm_ctrl;
			url += "&task_id=" + dm[0];
			url += "&download_type=" + dm[5] + "&t=" + Math.random();
			$j.get(url, function (data) { response_dm_ctrl(data); });
			break;
	}
}

function DM_Ctrl(action) {

	switch (action) {
		case "pause_all":
			if (bt_initialing("pause_all"))
				return;
			dm_ctrl_status("pause_all");
			break;
		case "start_all":
			if (bt_initialing("start_all"))
				return;
			dm_ctrl_status("start_all");
			break;
		case "clear":
			if (bt_initialing("clear"))
				return;
			dm_ctrl_status("clear");
			break;
		case "cancel":
			//alert("dm_ctrl c");
			if (bt_initialing("cancel"))
				return;
			dm_tmp = dm;
			if (confirm(multiLanguage_array[multi_INT][15]))
				dm_ctrl_status("cancel");
			break;
		case "paused":
			if (bt_initialing("paused"))
				return;
			dm_ctrl_status("paused");
			break;
		case "start":
			if (bt_initialing("start"))
				return;
			dm_ctrl_status("start");
			break;
	}

}

function bt_initialing(status) {
	var BT = "BT";
	var NZB = "NZB";
	var Initialing = "Initialing";
	var Finished = "Finished";
	var Seeding = "Seeding";
	var Hashing = "Hashing";
	var d = true;
	var st = true;
	switch (status) {
		case "start_all":
			for (var i = 0; i < dm_array.length; i++) {     //select all task
				if (dm_array[i][5] != BT) {
					st = false;
					break;
				}
			};
			for (var i = 0; i < dm_array.length; i++) {
				if (st) {
					if ((dm_array[i][5] == BT) && dm_array[i][4] == Initialing || dm_array[i][4] == Hashing) {
						Ext.MessageBox.alert(multiLanguage_array[multi_INT][3], multiLanguage_array[multi_INT][0] + "<br />" + multiLanguage_array[multi_INT][1]);
						return 1;
					}
				}
			}
			return 0;
			break;
		case "pause_all":

			for (var i = 0; i < dm_array.length; i++) {     //select all task

				if (dm_array[i][5] != BT) {

					d = false;
					break;
				}
			};
			for (var i = 0; i < dm_array.length; i++) {
				if (d) {
					if ((dm_array[i][5] == BT) && dm_array[i][4] == Initialing || dm_array[i][4] == Hashing) {
						Ext.MessageBox.alert(multiLanguage_array[multi_INT][3], multiLanguage_array[multi_INT][0] + "<br />" + multiLanguage_array[multi_INT][1]);
						return 1;
					}
				}
				else {
					if ((dm_array[i][5] == BT) && dm_array[i][4] == Initialing || dm_array[i][4] == Hashing) {

						Ext.MessageBox.alert(multiLanguage_array[multi_INT][3], multiLanguage_array[multi_INT][0] + "<br />" + "<br />" + multiLanguage_array[multi_INT][85]);
						BT_INIT_HASH = true;
					}
				}
			}
			return 0;
			break;

		case "cancel":
		case "paused":
		case "start":

			if ((dm[5] == BT) && (dm[4] == Initialing) || (dm[4] == Hashing)) {
				Ext.MessageBox.alert(multiLanguage_array[multi_INT][3], multiLanguage_array[multi_INT][0] + "\n" + multiLanguage_array[multi_INT][1]);
				return 1;
			}
			return 0;
			break;
		case "clear": for (var i = 0; i < dm_array.length; i++) {
			if (dm_array[i][4] == Seeding || dm_array[i][4] == Finished) {
				return 0;
			}
		}
			Ext.MessageBox.alert(multiLanguage_array[multi_INT][3], multiLanguage_array[multi_INT][2]);
			return 1;
			break;
	}

}

// check http and ftp url
function check_enter_url(url) {
	var download_type = 0;      //http is 1, ftp is 2, BT is 3 ,NZB is 4 ,false is 0

	if (!url || url.length == 0) {



		Ext.MessageBox.alert(multiLanguage_array[multi_INT][3], multiLanguage_array[multi_INT][4]);
		return -1;
	}
	else if (url.length > MAX_NAMELEN) {
		Ext.MessageBox.alert(multiLanguage_array[multi_INT][3], multiLanguage_array[multi_INT][5]);
		return -1;
	}
	else {
		url = url.toLowerCase();
		var http = /^http(.*)/
		var ftp = /^ftp(.*)/
		var bt = /(.*)\.torrent/
		var magnet = /^magnet:\?(.+)/
		if (http.test(url)) {
			download_type = 1;
		}
		else if (ftp.test(url)) {
			download_type = 2;
		}
		else if (magnet.test(url)) {
			download_type = 3;
		}
		else if (bt.test(url)) {
			download_type = 3;
		}

	}
	switch (download_type) {
		case 1: var httpreg = /http(s)?:\/\/[A-Za-z0-9]+\.[A-Za-z0-9]+[\/=\?%\-&_~`@[\]\':+!]*([^<>\"\"])*$/
			if (httpreg.test(url)) {
				if (bt.test(url)) {
					download_type = 3;
					return download_type;
				}
				else
					return download_type;
			} else
				return 0;
			break;
		case 2:
			return download_type;
		case 3: 
			return download_type;
			break;
		default: return 0; break;

	}
}

// check bt and nzb url
function check_open_url(url) {
	var download_type = 0;  //http is 1, ftp is 2, BT is 3 ,NZB is 4 ,false is 0
	if (!url || url.length == 0) {
		Ext.MessageBox.alert(multiLanguage_array[multi_INT][3], multiLanguage_array[multi_INT][4]);
		return -1;
	}
	else if (url.length > MAX_NAMELEN) {
		Ext.MessageBox.alert(multiLanguage_array[multi_INT][3], multiLanguage_array[multi_INT][5]);
		return -1;
	}
	else {
		url = url.toLowerCase();
		var bt = /(.*)\.torrent/
		var nzb = /(.*)\.nzb/
		if (bt.test(url)) {
			download_type = 3;
		}
		else if (nzb.test(url)) {
			download_type = 4;
		}
	}
	switch (download_type) {
		case 3: var is_torrent_file = url.search(/\.torrent$/);
			is_torrent_file = is_torrent_file >= 0 ? true : false;
			if (is_torrent_file) {
				return download_type;
			} else
				return 0;
			break;
		case 4: var is_nzb_file = url.search(/\.nzb$/);
			is_nzb_file = is_nzb_file >= 0 ? true : false;
			if (is_nzb_file) {
				return download_type;
			} else
				return 0;
			break;
		default: return 0; break;
	}
}

function dm_add_status() {
	updateStatusCounter = 0;
	var action_mode = "";                //action_mode
	var t;
	if (document.getElementById("open_usb_dm_url").value == "" && document.getElementById("HTTP_usb_dm_url").value == "") {
		Ext.MessageBox.alert(multiLanguage_array[multi_INT][3], multiLanguage_array[multi_INT][6]);
	}
	//http and ftp add task
	else if (document.getElementById("open_usb_dm_url").value == "" && document.getElementById("HTTP_usb_dm_url").value != "") {
		var usb_dm_url = ""
		usb_dm_url += $j("#HTTP_usb_dm_url").attr("value");
		usb_dm_url = usb_dm_url.replace(/\n/g, '');
		var DownType = usb_dm_url.substring(0, 4);
		if (DownType == "ed2k" && IS_FAT32 == "1") {
			var ed2kfilesize = usb_dm_url.split("|")[3];
			if (ed2kfilesize > 4 * 1024 * 1024 * 1024) {
				Ext.MessageBox.alert(multiLanguage_array[multi_INT][3], multiLanguage_array[multi_INT][78]);
				$j("#HTTP_usb_dm_url").attr("value", "");
				return;
			}
		}
		var url = "dm_apply.cgi";
		var downloadtype = 5; //http is 1, ftp is 2, BT is 3 ,NZB is 4 ,false is 0
		action_mode = "DM_ADD";
		url += "?action_mode=" + action_mode;
		url += "&download_type=" + downloadtype;
		url += "&again=no";
		url += "&usb_dm_url=" + encodeURIComponent(usb_dm_url) + "&t=" + Math.random();
		$j.get(url, function (data) { response_dm_add(data); });
		hidePanel();
	}

	//bt and nzb add task
	else if (document.getElementById("open_usb_dm_url").value != "") {
		var usb_dm_url = $j("#open_usb_dm_url").attr("value");
		var downloadtype = check_open_url(usb_dm_url); //http is 1, ftp is 2, BT is 3 ,NZB is 4 ,false is 0
		if (downloadtype == 3) {
			$j("#open_a_file_form").submit();    //缺少回调函数   添加任务table时需要
			hidePanel();
			showLoading();
		}
		else if (downloadtype == 4) {
			$j("#open_a_file_form").submit();     //缺少回调函数   添加任务table时需要
			hidePanel();
		}
		else if (downloadtype == 0) {
			Ext.MessageBox.alert(multiLanguage_array[multi_INT][3], multiLanguage_array[multi_INT][7]);
		}
	}

}

function Ftp_dm_add_status() {
	updateStatusCounter = 0;
	var http = /^http:\/\/(.*)/
	var ftp = /^ftp:\/\/(.*)/
	var head = /^(.*):\/\//
	var user = $j("#User_name").attr("value");
	var pass = $j("#Password").attr("value");
	var user_pass = "";
	var foot = "";
	var furl = dm[1];
	var fid = dm[0];
	furl_s = furl.toLowerCase();
	var len = furl.length;
	if (user != "" && pass != "")
		user_pass = user + ":" + pass + "@";
	else if (user != "" && pass == "")
		user_pass = user + "@";
	else
		user_pass = "";
	if (ftp.test(furl_s)) {
		var del_user = furl_s.indexOf("@", 6);
		if (del_user == -1) {
			foot = furl.substr(6, len - 6);
			furl = "ftp://" + user_pass + foot;
		}
		else {
			var kk = furl_s.lastIndexOf("@");
			foot = furl.substring(kk + 1);
			furl = "ftp://" + user_pass + foot;
		}

	}
	else if (http.test(furl_s)) {
		var del_user = furl_s.indexOf("@", 7);
		if (del_user == -1) {
			foot = furl.substr(7, len - 7);
			furl = "http://" + user_pass + foot;
		} else {
			var kk = furl_s.lastIndexOf("@");
			foot = furl.substring(kk + 1);
			furl = "http://" + user_pass + foot;

		}
	}
	else {
		if (head.test(furl_s)) {
			var a = furl_s.indexOf("://", 0);
			var he = furl.substr(0, a + 3);
			foot = furl.substr(a + 3, len - a - 3);
			furl = he + user_pass + foot;
		}

		else
			furl = user_pass + furl;
	}
	var url = "dm_apply.cgi";
	var downloadtype = 5; //http is 1, ftp is 2, BT is 3 ,NZB is 4 ,false is 0
	action_mode = "DM_ADD";
	url += "?action_mode=" + action_mode;
	url += "&download_type=" + downloadtype;
	url += "&again=yes";
	url += "&fid=" + fid;
	url += "&usb_dm_url=" + encodeURIComponent(furl) + "&t=" + Math.random();
	$j.get(url, function (data) { response_dm_add(data); });
	hideUnamePsw();
}

function add_progress(i, progress) {

	var j = dm_array[i][0];
	var ProgressBar = "ProgressBar" + j;
	var pbid = "pbid" + j;
	ProgressBar = new Ext.ProgressBar({
		autoWidth: true,
		text: 'working....',
		renderTo: pbid,
		stateId: pbid
	});
	if (ProgressBar_array.length != dm_num) {
		ProgressBar_array.push(ProgressBar);
	}
	else {
		var ProgressBar_array1 = new Array();
		for (var x = 0; x < ProgressBar_array.length; x++) {
			if (ProgressBar_array[x].stateId != ProgressBar.stateId)
				ProgressBar_array1.push(ProgressBar_array[x]);
			else
				ProgressBar_array1.push(ProgressBar);
		}
		ProgressBar_array = ProgressBar_array1;
	}
	var ProgressBar_array2 = new Array();
	for (var m = 0; m < dm_array.length; m++) {
		for (var n = 0; n < ProgressBar_array.length; n++) {
			if (dm_array[m][0] == ProgressBar_array[n].stateId.substring(4))
				ProgressBar_array2.push(ProgressBar_array[n]);
		}
	}
	ProgressBar_array.length = 0;
	ProgressBar_array = ProgressBar_array2;
	update_progress(ProgressBar, progress);
}

function changeshowicon(iconid) {
	$j("#" + iconid).attr("src", "images/icon/Ino2Icon_click.png");
}
function returnshowicon(iconid) {
	$j("#" + iconid).attr("src", "images/icon/Ino2Icon.png");
}
function changeInfoIcon(icon, iconid) {
	$j("#" + iconid).attr("src", icon);
}
function returnInfoIcon(icon, iconid) {
	$j("#" + iconid).attr("src", icon);
}

function add_task(i) {
	var filename, progress, size, status, type, elased, download, upload, peers, havedone, downloaddir_error, downloaddir;//error;
	var progressTxt;
	filename = dm_array[i][1];
	progress = dm_array[i][2];
	size = dm_array[i][3];
	status = dm_array[i][4];
	type = dm_array[i][5];
	download = dm_array[i][7];
	upload = dm_array[i][8];
	peers = dm_array[i][9];
	havedone = dm_array[i][11];
	downloaddir_error = dm_array[i][12];
	downloaddir = dm_array[i][12].slice(8);
	if (type == "BT") {
		progressTxt = (parseFloat(size) * havedone).toFixed(2) + size.substr(size.length - 3, 3);
	}
	else {
		progressTxt = (parseFloat(size) * progress).toFixed(2) + size.substr(size.length - 3, 3);
	}
	var j = dm_array[i][0];
	var task_code = '';
	var task_code2 = '';
	if (!isLogShow) {
		task_code += '<table id="' + j + '" class="taskLongWidth" style="border-bottom:#000 solid 1px; table-layout:fixed;" onclick="selectedTask(this.id);">\n';
	}
	else {
		task_code += '<table id="' + j + '" class="taskShortWidth" style="border-bottom:#000 solid 1px; table-layout:fixed;" onclick="selectedTask(this.id);">\n';
	}
	task_code += '<tr><td colspan="9" style="word-wrap:break-word;word-break:break-all;overflow:hidden;"><span style="font-weight:bold; font-size:16px;" id="filename' + j + '">' + filename + '</span>\n';
	task_code += '<a href="javascript:Show_Log();"><img id="showicon' + j + '" src="images/icon/Ino2Icon.png" width="15" height="15" onmousedown="changeshowicon(this.id);" onmouseup="returnshowicon(this.id);" title="Detail information" /></a></td></tr>\n';
	task_code += '<tr><td colspan="2"><span style="margin-right:30px;" id="type' + j + '">' + type + '</span></td>\n';
	if (status != "notbegin") {
		task_code += '<td colspan="7" id="progresstd' + j + '"><span id="progress' + j + '">' + progressTxt + '</span> of <span id="size' + j + '">' + size + '</span> downloaded</td></tr>\n';
	}
	task_code += '<tr><td colspan="8" style="position:relative;" id="pbid' + j + '"></td>\n';
	if (status == "Hashing") {
		task_code += '<td style="width:40px;"><img id="status_icon' + j + '" src="images/statusIcon/' + 'Initialing.gif" alt="" />';
		if (navigator.appName.indexOf("Microsoft") >= 0 && (location.host.split(":")[0] == LAN_IP || location.host.split(":")[0] == LOCAL_DOMAIN)) {
			if (status == "Finished" || status == "Seeding") {
				if (downloaddir_error == "not_found") {
					task_code += '<a id="open_dir_link' + j + '"  style="display:none;"><img id="open_icon' + j + '" src="images/default/tree/folder.gif" alt="" style="display:none;" title="Not Exist"/></a>';
				}
				else {
					task_code += '<a id="open_dir_link' + j + '" target="_blank" href="file://' + LAN_IP + downloaddir + '" style="display:none;"><img id="open_icon' + j + '" src="images/default/tree/folder.gif" alt="" style="display:none;" title=""/></a>';
				}
			}
			else {
				task_code += '<a id="open_dir_link' + j + '" target="_blank" href="file://' + LAN_IP + downloaddir + '" style="display:none;" ><img id="open_icon' + j + '" src="images/default/tree/folder.gif" alt="" style="display:none;" title=""/></a>';
			}
		}
		task_code += '</td></tr>\n';
	}
	else {
		task_code += '<td style="width:40px;"><img id="status_icon' + j + '" src="images/statusIcon/' + status + '.gif" alt="" />';
		if (navigator.appName.indexOf("Microsoft") >= 0 && (location.host.split(":")[0] == LAN_IP || location.host.split(":")[0] == LOCAL_DOMAIN)) {
			if (status == "Finished" || status == "Seeding") {
				if (downloaddir_error == "not_found") {
					task_code += '<a id="open_dir_link' + j + '" style="display:none;"><img id="open_icon' + j + '" src="images/default/tree/folder.gif" alt="" style="display:none;" title="Not Exist"/></a>';
				}
				else {
					task_code += '<a id="open_dir_link' + j + '" target="_blank" href="file://' + LAN_IP + downloaddir + '" style="display:none;"><img id="open_icon' + j + '" src="images/default/tree/folder.gif" alt="" style="display:none;" title=""/></a>';
				}
			}
			else {
				task_code += '<a id="open_dir_link' + j + '" target="_blank" href="file://' + LAN_IP + downloaddir + '" style="display:none;"><img id="open_icon' + j + '" src="images/default/tree/folder.gif" alt="" style="display:none;" title=""/></a>';
			}
		}
		task_code += '</td></tr>\n';
	}
	var ftp_name = "";
	if (filename.substring(0, 6) == "FTP://" || filename.substring(0, 6) == "ftp://") {
		var ftp_name_index = filename.substring(6).indexOf("@");
		if (ftp_name_index >= 0) {
			var kk = filename.substring(6).indexOf("/");
			if (kk > 0) {
				var tmp = filename.substring(6).substring(0, kk); //user:pwd@host
				var ii = tmp.lastIndexOf("@");
				var account = tmp.substring(0, ii);              //user:pwd
				var yy = account.indexOf(":")
				if (yy > 0) {
					ftp_name = account.substring(0, yy);
				}
				else {
					ftp_name = account;
				}
			}
			else {
				ftp_name = "asus-none-ftpname" + j;
			}
		} else {
			ftp_name = "asus-none-ftpname" + j;
		}
	} else if (filename.substring(0, 7) == "HTTP://" || filename.substring(0, 7) == "http://") {
		var http_name_index = filename.substring(7).indexOf("@");
		if (http_name_index >= 0) {
			var kk = filename.substring(7).indexOf("/");
			if (kk > 0) {
				var tmp = filename.substring(7).substring(0, kk); //user:pwd@host
				var ii = tmp.lastIndexOf("@");
				var account = tmp.substring(0, ii);              //user:pwd
				var yy = account.indexOf(":")
				if (yy > 0) {
					ftp_name = account.substring(0, yy);
				}
				else {
					ftp_name = account;
				}
			}
			else {
				ftp_name = "asus-none-ftpname" + j;
			}
		} else {
			ftp_name = "asus-none-ftpname" + j;
		}
	} else {
		ftp_name = "asus-none-ftpname" + j;
	}

	task_code += '<tr><td colspan="3"><a id="showUP' + j + '" name=' + ftp_name + ' href="javascript:showUnamePsw(' + j + ');"  style="display:none;"><img src="images/statusIcon/warning.png" alt="" /><span>' + multiLanguage_array[multi_INT][69] + '</span></a>\n';
	if (httpTag) {
		task_code += '<a id="showUP_n1' + j + '" href="http://' + location.host + '/downloadmaster/Setting_NZB.asp" style="display:none;"><img src="images/statusIcon/warning.png" alt="" /><span>' + multiLanguage_array[multi_INT][70] + '</span></a>\n';
		task_code += '<a id="showUP_n2' + j + '" href="http://' + location.host + '/downloadmaster/Setting_NZB.asp" style="display:none;"><img src="images/statusIcon/warning.png" alt="" /><span>' + multiLanguage_array[multi_INT][71] + '</span></a>\n';
		task_code += '<a id="showUP_n3' + j + '" href="http://' + location.host + '/downloadmaster/Setting_NZB.asp" style="display:none;"><img src="images/statusIcon/warning.png" alt="" /><span>' + multiLanguage_array[multi_INT][72] + '</span></a>\n';
	} else {
		task_code += '<a id="showUP_n1' + j + '" href="https://' + location.host + '/downloadmaster/Setting_NZB.asp" style="display:none;"><img src="images/statusIcon/warning.png" alt="" /><span>' + multiLanguage_array[multi_INT][70] + '</span></a>\n';
		task_code += '<a id="showUP_n2' + j + '" href="https://' + location.host + '/downloadmaster/Setting_NZB.asp" style="display:none;"><img src="images/statusIcon/warning.png" alt="" /><span>' + multiLanguage_array[multi_INT][71] + '</span></a>\n';
		task_code += '<a id="showUP_n3' + j + '" href="https://' + location.host + '/downloadmaster/Setting_NZB.asp" style="display:none;"><img src="images/statusIcon/warning.png" alt="" /><span>' + multiLanguage_array[multi_INT][72] + '</span></a>\n';
	}
	task_code += '<a id="showUP_n4' + j + '" style="display:none;"><img src="images/statusIcon/warning.png" alt="" /><span>' + multiLanguage_array[multi_INT][76] + '</span></a>\n';
	task_code += '<a id="showUP_n5' + j + '" href="javascript:show_diskfull_info();" style="display:none;"><img src="images/statusIcon/warning.png" alt="" /><span>' + multiLanguage_array[multi_INT][77] + '</span></a>\n';
	task_code += '<a id="showUP_n6' + j + '" href="javascript:show_vfat_info();" style="display:none;"><img src="images/statusIcon/warning.png" alt="" /><span>' + multiLanguage_array[multi_INT][83] + '</span></a>\n';
	task_code += '<a id="showUP_n7' + j + '" href="javascript:show_Move4GBError_info();" style="display:none;"><img src="images/statusIcon/warning.png" alt="" /><span>' + multiLanguage_array[multi_INT][83] + '</span></a>\n';
	task_code += '<a id="showUP_n8' + j + '" href="javascript:show_MoveDiskFull_info();" style="display:none;"><img src="images/statusIcon/warning.png" alt="" /><span>' + multiLanguage_array[multi_INT][77] + '</span></a>\n';
	task_code += '<a id="showUP_n9' + j + '" href="' + location.protocol + '//' + location.host +'/downloadmaster/Setting_amule.asp' + '" style="display:none;"><img src="images/statusIcon/warning.png" alt="" /><span>' + multiLanguage_array[multi_INT][86] + '</span></a>\n';
	if (status != "notbegin") {
		task_code += '<span id="status' + j + '">' + status + '</span></td>\n';
	} else {
		task_code += '<span id="status' + j + '">' + 'Waiting' + '</span></td>\n';
	}
	if (status != "notbegin") {
		if (type != "HTTP" && type != "FTP" && type != "NZB") {
			task_code += '<td id="peerstd' + j + '" colspan="2"><span id="peers' + j + '">' + peers + '</span> peers</td>\n';
		}
		else {
			task_code += '<td colspan="2"></td>\n';
		}
		task_code += '<td id="downloadtd' + j + '" align="right" colspan="2" id="downloadplace' + j + '"><img width="13" height="13" src="images/icon/DownArrowIcon.png" /><span id="download' + j + '">' + download + '</span></td>\n';
		if (type != "HTTP" && type != "FTP" && type != "NZB") {
			task_code += '<td colspan="2" id="uploadtd' + j + '"><img width="13" height="13" src="images/icon/UpArrowIcon.png" /><span id="upload' + j + '">' + upload + '</span></td>\n';
		}
		else {
			task_code += '<td colspan="2"></td>\n';
		}
	}
	task_code += '</tr>\n';
	task_code += '<tr><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td></tr>\n';
	task_code += '</table>\n';
	task_code2 += '<div id="div_for_addtask" style="display:none;"></div>\n';

	$j(task_code).replaceAll("#div_for_addtask");
	$j("#transfers").html(dm_array.length);
	dm_num = dm_array.length;
	add_progress(i, progress);
	$j("#" + j).after(task_code2);
	updateCtrlIcon();
}


function cancel_task(data) {
	eval("dm_array = [" + data + "]");
	var task_code = '<div style="display:none;"></div>';
	$j(task_code).replaceAll("#" + dm_tmp[0]);
	for (var i = 0; i < ProgressBar_array.length; i++) {
		if (dm_tmp[0] == ProgressBar_array[i].stateId.substring(4))
			ProgressBar_array.splice(i, 1);
	}
	dm_num = dm_array.length;
	old_dmid = "";
	$j("#transfers").html(dm_num);
	if (isLogShow)
		select_taskType();
	if (dm_num == 0) {
		$j("#taskLog_show").hide();
		isLogShow = false;
	}
	updateCtrlIcon();
	ajaxRequest = true;
}

function response_dm_cancel(data) {
	if (data.search(/ACK_SUCESS/) >= 0) {	
		Ajax_Get_DM_Status_inphase();
		return;
	}
	else if (data.search(/ACK_FAIL/) >= 0) {
		Ext.MessageBox.alert(multiLanguage_array[multi_INT][3], multiLanguage_array[multi_INT][8]);
		return;
	}
}


var formnum = 0;
var formarray = new Array();
var hhh = new Array();
var fname = "";
var ffname = "";
function parse_bt(num) {
	var formtest;
	formtest = formarray[num];
	var window = new Ext.Window({
		width: 500,
		height: 300,
		minWidth: 300,
		minHeight: 200,
		layout: 'fit',
		plain: true,
		closable: false,
		bodyStyle: 'padding:5px;',
		buttonAlign: 'center',
		items: formtest,
		buttonAlign: 'center',
		buttons: [{
			text: 'Apply',
			handler: function () {
				var gridnum = formtest.selModel.getSelections();
				var gridlength = gridnum.length;
				var idarray = new Array();
				for (var i = 0; i < gridlength; i++) {
					idarray.push(gridnum[i].get('id'));
				};
				var type = "";
				if (gridlength == hhh.length)
					type = "All";
				else
					type = idarray;
				var url = "dm_uploadbt.cgi";
				url += "?filename=" + encodeURIComponent(ffname) + "&download_type=" + type + "&D_type=3" + "&t=" + Math.random();
				$j.ajax({
					url: url,
					async: false,
					success: function (data) { Ajax_Get_DM_Status(); }
				});
				window.hide();
				$j("#DM_mask").fadeOut('fast');
				//formnum = 0;
				formarray.length = 0;
				hhh.length = 0;
			}
		}, {
			text: 'Cancel',
			handler: function () {
				//formnum = 0;
				formarray.length = 0;
				hhh.length = 0;
				window.hide();
				$j("#DM_mask").fadeOut('fast');
			}
		}]
	});
	window.show();
}

function rendervalue(value) {
	return "<span style='color:black;font-weight:bold;'>" + value + "</span>";
}

function response_dm_add(data) {
	if (data.search(/BT_ACK_SUCESS/) >= 0) {	
		var file_result = data.split('BT_ACK_SUCESS=');
		var datastr = file_result[1].split(', #');
		fname = datastr[0];
		ffname = datastr[1];
		for (var i = 2; i < datastr.length; i++) {
			datastr[i] = datastr[i].split('#');

			datastr_size = datastr[i].length;
			if (datastr_size > 5) {
				var temp_array = new Array(4);
				temp_array[0] = datastr[i][1];
				var digit = /^\d*$/;
				if (digit.test(datastr[i][3])) {
					temp_array[2] = datastr[i][3];
					temp_array[1] = datastr[i][2];
					for (var jj = 4; jj < datastr_size - 1; jj++) {
						if (jj != 4)
							temp_array[3] += '#';
						temp_array[3] += datastr[i][jj];
					}
				}
				else {
					for (var j = 3; j < datastr_size - 2; j++) {
						if (digit.test(datastr[i][j])) {
							temp_array[2] = datastr[i][j];
							for (var jj = 2; jj < j; jj++) {
								if (jj != 2)
									temp_array[1] += '#';
								temp_array[1] += datastr[i][jj];
							}
							for (var jj = j + 1; jj < datastr_size; jj++) {
								if (jj != j + 1)
									temp_array[3] != '#';
								temp_array[3] += datastr[i][jj];
							}
						}
					}
				}
				datastr[i] = temp_array;
			}
			var hh = new Array();
			hh.push(datastr[i][0]);
			if (datastr[i][1] == "none")
				hh.push(datastr[i][3]);
			else
				hh.push(datastr[i][3] + "/" + datastr[i][1]);
			hh.push(datastr[i][2]);
			hhh.push(hh);
		}
		var sm2 = new Ext.grid.CheckboxSelectionModel({ handleMouseDown: Ext.emptyFn });
		var store = new Ext.data.SimpleStore({ data: hhh, fields: ["id", "name", "size"] });
		var colM = new Ext.grid.ColumnModel([sm2, { header: "<span style='color:black'><b>id</b></span>", dataIndex: "id", width: 45, sortable: false, renderer: rendervalue }, { header: "<span style='color:black'><b>File Name</b></span>", dataIndex: "name", width: 360, sortable: false, renderer: rendervalue }, { header: "<span style='color:black'><b>File Size</b></span>", dataIndex: "size", width: 100, sortable: false, renderer: rendervalue }]);
		var grid = new Ext.grid.GridPanel({
			title: "Title：" + fname,
			height: 200,
			width: 600,
			stripeRows: true,
			sm: sm2,
			cm: colM,
			store: store,
			autoExpandColumn: 1,
			viewConfig: {
				forceFit: true,
				enableRowBody: true,
				getRowClass: function (record, rowIndex, p, ds) {
					var cls = 'yellow-row';
					return cls;
				}
			}
		});
		grid.on("render", function () { grid.selModel.selectAll(true); }, this, { delay: 200 });
		$j("#DM_mask").fadeIn(1000);
		formarray.push(grid);
		parse_bt(0);
		return;
	}
	else if (data.search(/ACK_SUCESS/) >= 0) {	
		Ajax_Get_DM_Status();
		return;
	}
	else if (data.search(/ACK_FAIL/) >= 0) {
		Ext.MessageBox.alert(multiLanguage_array[multi_INT][3], multiLanguage_array[multi_INT][9]);
		return;
	}
	else if (data.search(/BT_EXIST/) >= 0) {
		Ext.MessageBox.alert(multiLanguage_array[multi_INT][3], multiLanguage_array[multi_INT][10]);
		return;
	}
	else if (data.search(/LIGHT_FULL/) >= 0) {
		Ext.MessageBox.alert(multiLanguage_array[multi_INT][3], multiLanguage_array[multi_INT][11]);
		return;
	}
	else if (data.search(/HEAVY_FULL/) >= 0) {
		Ext.MessageBox.alert(multiLanguage_array[multi_INT][3], multiLanguage_array[multi_INT][12]);
		return;
	}
	else if (data.search(/NNTP_FULL/) >= 0) {
		Ext.MessageBox.alert(multiLanguage_array[multi_INT][3], multiLanguage_array[multi_INT][13]);
		return;
	}
	else if (data.search(/TOTAL_FULL/) >= 0) {
		Ext.MessageBox.alert(multiLanguage_array[multi_INT][3], multiLanguage_array[multi_INT][14]);
		return;
	}
	else if (data.search(/DISK_FULL/) >= 0) {
		Ext.MessageBox.alert(multiLanguage_array[multi_INT][3], multiLanguage_array[multi_INT][75]);
		return;
	}
}


function Ajax_Get_DM_Status_inphase() {
	ajaxRequest = false;
	var t;
	var url = "dm_print_status.cgi" + "?action_mode=" + sorted_by + "&t=" + Math.random();
	$j.ajax({
		url: url,
		async: false,
		success: function (data) { cancel_task(data) },
		complete: function (XMLHttpRequest, textStatus) { ; }
	});

}

function Ajax_Get_DM_Status() {
	if (ajaxRequest) {
		var t;
		old_sorted_by2 = sorted_by;
		var url = "dm_print_status.cgi" + "?action_mode=All&t=" + Math.random();
		$j.ajax({
			url: url,
			async: false,
			success: function (data) { showDMList(data); },
			error: function (XMLHttpRequest, textStatus, errorThrown) {
				if (XMLHttpRequest.status == 598) {
					if (httpTag)
						self.location = "http://" + location.host + "/Main_Login.asp";
					else
						self.location = "https://" + location.host + "/Main_Login.asp";
				}
			}
		})
	}
}



//update taskLogIcon NZB and BT will have 4 icons
function update_taskLogIcon() {
	if (dm[5] == "BT" || dm[5] == "NZB") {
		$j("#inspector_tab_peers").show();
	}
	else {;
		$j("#inspector_tab_peers").hide();
	}

}

//when dm_array change,or taskid change, or status change,update icon
function updateCtrlIcon() {
	var Finished = "Finished";
	var Seeding = "Seeding";
	var Downloading = "Downloading";
	var Paused = "Paused";
	var Error = "Error";
	var Stop = "Stop";
	var Diskfull = "Diskfull"
	//var com = false;
	if (dm_array.length > 0) {// have tasks
		var a = true;
		var b = true;
		var c = true;
		for (var i = 0; i < dm_array.length; i++) {//have Downloading task
			if (dm_array[i][4] == Downloading || dm_array[i][4] == Seeding) {
				$j("#icon_pause_all").children("a").attr("href", "javascript:DM_Ctrl('pause_all');");
				a = false;
				break;
			}
		}
		for (var i = 0; i < dm_array.length; i++) {//have Paused task
			if (dm_array[i][4] == Paused || dm_array[i][4] == Diskfull) {
				$j("#icon_resume_all").children("a").attr("href", "javascript:DM_Ctrl('start_all');");
				b = false;
				break;
			}
		}
		// have completed tasks
		for (var i = 0; i < dm_array.length; i++) {
			if (dm_array[i][4] == Seeding || dm_array[i][4] == Finished) {
				$j("#icon_clear").children("a").attr("href", "javascript:DM_Ctrl('clear');");
				c = false;
				break;
			}
		}
		if (a)
			$j("#icon_pause_all").children("a").removeAttr("href");
		if (b)
			$j("#icon_resume_all").children("a").removeAttr("href");
		if (c)
			$j("#icon_clear").children("a").removeAttr("href");

		/*else
		$j("#icon_clear").children("a").removeAttr("href");
		*/
		if (dm.length != 0) {
			// have selected task
			$j("#icon_del").children("a").attr("href", "javascript:DM_Ctrl('cancel');");
			if (dm[4] == Downloading || dm[4] == Seeding) {
				// the selected task's status is download
				$j("#icon_pause").children("a").attr("href", "javascript:DM_Ctrl('paused');");
				$j("#icon_resume").children("a").removeAttr("href");
			}
			// the selected task's status is pause || error || stoped
			else if (dm[4] == Paused || dm[4] == Error || dm[4] == Stop || dm[4] == Diskfull) {
				$j("#icon_resume").children("a").attr("href", "javascript:DM_Ctrl('start');");
				$j("#icon_pause").children("a").removeAttr("href");
			}
			else {
				$j("#icon_pause").children("a").removeAttr("href");
				$j("#icon_resume").children("a").removeAttr("href");
			}
		}
		else {
			$j("#icon_del").children("a").removeAttr("href");
			$j("#icon_pause").children("a").removeAttr("href");
			$j("#icon_resume").children("a").removeAttr("href");
		}
	}
	else {
		$j("#icon_clear").children("a").removeAttr("href");
		$j("#icon_del").children("a").removeAttr("href");
		$j("#icon_pause").children("a").removeAttr("href");
		$j("#icon_resume").children("a").removeAttr("href");
		$j("#icon_pause_all").children("a").removeAttr("href");
		$j("#icon_resume_all").children("a").removeAttr("href");
	}

}

function update_BT_general() {
	if (document.getElementById("taskLog_Pieces") && $j("#taskLog_Pieces").html() != dm_Log[1])
		$j("#taskLog_Pieces").html(dm_Log[1]);
	if (document.getElementById("taskLog_Hash") && $j("#taskLog_Hash").html() != dm_Log[2])
		$j("#taskLog_Hash").html(dm_Log[2]);
	if (document.getElementById("taskLog_Secure") && $j("#taskLog_Secure").html() != dm_Log[3])
		$j("#taskLog_Secure").html(dm_Log[3]);
	if (document.getElementById("taskLog_Comment") && $j("#taskLog_Comment").html() != dm_Log[4])
		$j("#taskLog_Comment").html(dm_Log[4]);
	if (document.getElementById("taskLog_Creator") && $j("#taskLog_Creator").html() != dm_Log[5])
		$j("#taskLog_Creator").html(dm_Log[5]);
	if (document.getElementById("taskLog_Date") && $j("#taskLog_Date").html() != dm_Log[6])
		$j("#taskLog_Date").html(dm_Log[6]);
	if (document.getElementById("taskLog_Download_Dir") && $j("#taskLog_Download_Dir").html() != dm_Log[7])

		$j("#taskLog_Download_Dir").html(dm_Log[7]);
}

function update_FHN_general() {
	if (document.getElementById("taskLog_FHN_Destination") && $j("#taskLog_FHN_Destination").html() != dm_Log[0])
		$j("#taskLog_FHN_Destination").html(dm_Log[0]);
	if (document.getElementById("taskLog_FHN_Filesize") && $j("#taskLog_FHN_Filesize").html() != dm_Log[1])
		$j("#taskLog_FHN_Filesize").html(dm_Log[1]);
	if (document.getElementById("taskLog_FHN_Createdtime") && $j("#taskLog_FHN_Createdtime").html() != dm_Log[2])
		$j("#taskLog_FHN_Createdtime").html(dm_Log[2]);
	if (document.getElementById("taskLog_FHN_URL") && $j("#taskLog_FHN_URL").html() != dm_Log[3])
		$j("#taskLog_FHN_URL").html(dm_Log[3]);
}

function update_BT_transfer() {
	if (document.getElementById("taskLog_Progress") && $j("#taskLog_Progress").html() != dm_Log[0])
		$j("#taskLog_Progress").html(dm_Log[0]);
	if (document.getElementById("taskLog_Size") && $j("#taskLog_Size").html() != dm_Log[1])
		$j("#taskLog_Size").html(dm_Log[1]);
	if (document.getElementById("taskLog_Status") && $j("#taskLog_Status").html() != dm_Log[2])
		$j("#taskLog_Status").html(dm_Log[2]);
	if (document.getElementById("taskLog_Availability") && $j("#taskLog_Availability").html() != dm_Log[3])
		$j("#taskLog_Availability").html(dm_Log[3]);
	if (document.getElementById("taskLog_Downloaded") && $j("#taskLog_Downloaded").html() != dm_Log[4])
		$j("#taskLog_Downloaded").html(dm_Log[4]);
	if (document.getElementById("taskLog_Uploaded") && $j("#taskLog_Uploaded").html() != dm_Log[5])
		$j("#taskLog_Uploaded").html(dm_Log[5]);
	if (document.getElementById("taskLog_DL_Speed") && $j("#taskLog_DL_Speed").html() != dm_Log[6])
		$j("#taskLog_DL_Speed").html(dm_Log[6]);
	if (document.getElementById("taskLog_UL_Speed") && $j("#taskLog_UL_Speed").html() != dm_Log[7])
		$j("#taskLog_UL_Speed").html(dm_Log[7]);
	if (document.getElementById("taskLog_Error") && $j("#taskLog_Error").html() != dm_Log[8])
		$j("#taskLog_Error").html(dm_Log[8]);
	if (document.getElementById("taskLog_Ratio") && $j("#taskLog_Ratio").html() != dm_Log[9])
		$j("#taskLog_Ratio").html(dm_Log[9]);
	if (document.getElementById("taskLog_UL_To") && $j("#taskLog_UL_To").html() != dm_Log[10])
		$j("#taskLog_UL_To").html(dm_Log[10]);
	if (document.getElementById("taskLog_DL_From") && $j("#taskLog_DL_From").html() != dm_Log[11])
		$j("#taskLog_DL_From").html(dm_Log[11]);

}

function update_FHN_transfer() {
	if (document.getElementById("taskLog_FHN_Status") && $j("#taskLog_FHN_Status").html() != dm_Log[0])
		$j("#taskLog_FHN_Status").html(dm_Log[0]);
	if (document.getElementById("taskLog_FHN_Transferred") && $j("#taskLog_FHN_Transferred").html() != dm_Log[1])
		$j("#taskLog_FHN_Transferred").html(dm_Log[1]);
	if (document.getElementById("taskLog_FHN_Progress") && $j("#taskLog_FHN_Progress").html() != dm_Log[2])
		$j("#taskLog_FHN_Progress").html(dm_Log[2]);
	if (document.getElementById("taskLog_FHN_Speed") && $j("#taskLog_FHN_Speed").html() != dm_Log[3])
		$j("#taskLog_FHN_Speed").html(dm_Log[3]);
	if (document.getElementById("taskLog_FHN_StartTime") && $j("#taskLog_FHN_StartTime").html() != dm_Log[4])
		$j("#taskLog_FHN_StartTime").html(dm_Log[4]);
	if (document.getElementById("taskLog_FHN_TimeLeft") && $j("#taskLog_FHN_TimeLeft").html() != dm_Log[5])
		$j("#taskLog_FHN_TimeLeft").html(dm_Log[5]);
}

function update_BT_links() {
	//假设links的数目不会变化
	for (var i = 0; i < dm_Log.length; i++) {
		if (!document.getElementById("taskLog_linkName" + i))
			create_linksInfo();
		if (document.getElementById("taskLog_linkName" + i) && $j("#taskLog_linkName" + i).html() != dm_Log[i][0])
			$j("#taskLog_linkName" + i).html(dm_Log[i][0]);
		if (document.getElementById("taskLog_lastAnnounce" + i) && $j("#taskLog_lastAnnounce" + i).html() != dm_Log[i][1])
			$j("#taskLog_lastAnnounce" + i).html(dm_Log[i][1]);
		if (document.getElementById("taskLog_Seeders" + i) && $j("#taskLog_Seeders" + i).html() != dm_Log[i][2])
			$j("#taskLog_Seeders" + i).html(dm_Log[i][2]);
		if (document.getElementById("taskLog_Leechers" + i) && $j("#taskLog_Leechers" + i).html() != dm_Log[i][3])
			$j("#taskLog_Leechers" + i).html(dm_Log[i][3]);
		if (document.getElementById("taskLog_Downloads" + i) && $j("#taskLog_Downloads" + i).html() != dm_Log[i][4])
			$j("#taskLog_Downloads" + i).html(dm_Log[i][4]);
		if (document.getElementById("taskLog_lastScrape" + i) && $j("#taskLog_lastScrape" + i).html() != dm_Log[i][5])
			$j("#taskLog_lastScrape" + i).html(dm_Log[i][5]);
	}
}

function update_BT_included() {
	for (var i = 0; i < dm_Log.length; i++) {
		if (!document.getElementById("BTincludedFile" + i))
			create_includedInfo();
		if (document.getElementById("BTincludedFile" + i) && $j("#BTincludedFile" + i).html() != dm_Log[i][0])
			$j("#BTincludedFile" + i).html(dm_Log[i][0]);
		if (document.getElementById("BTincludedFile_progress" + i) && $j("#BTincludedFile_progress" + i).html() != dm_Log[i][1])
			$j("#BTincludedFile_progress" + i).html(dm_Log[i][1]);
		if (document.getElementById("BTincludedFile_downloaded" + i) && $j("#BTincludedFile_downloaded" + i).html() != dm_Log[i][2])
			$j("#BTincludedFile_downloaded" + i).html(dm_Log[i][2]);
		if (document.getElementById("BTincludedFile_size" + i) && $j("#BTincludedFile_size" + i).html() != dm_Log[i][3])
			$j("#BTincludedFile_size" + i).html(dm_Log[i][3]);
	}
}

function update_NZB_Log() {
	for (var i = 0; i < dm_Log.length; i++) {
		if (!document.getElementById("NZBLogName" + i))
			create_includedInfo();
		if (document.getElementById("NZBLogName" + i) && $j("#NZBLogName" + i).html() != dm_Log[i][0])
			$j("#NZBLogName" + i).html(dm_Log[i][0]);
		if (document.getElementById("NZBLogSize" + i) && $j("#NZBLogSize" + i).html() != dm_Log[i][2])
			$j("#NZBLogSize" + i).html(dm_Log[i][2]);
		if (document.getElementById("NZBLogProgress" + i) && $j("#NZBLogProgress" + i).html() != dm_Log[i][1])
			$j("#NZBLogProgress" + i).html(dm_Log[i][1]);
	}
}

function show_dm_Log(data) {
	dm_Log.length = 0;
	if (data != null && data != "") {
		if (taskLogShowType == 0 || taskLogShowType == 1)
			eval("dm_Log=" + data);
		else if (taskLogShowType == 2 || taskLogShowType == 3)
			eval("dm_Log=[" + data + "]");
		if (taskLogShowType == 0 && (dm[5] == "BT" || dm[5] == "ED2K")) {
			update_BT_general();
		}
		else if (taskLogShowType == 0 && (dm[5] == "HTTP" || dm[5] == "NZB" || dm[5] == "FTP")) {
			update_FHN_general();
		}
		else if (taskLogShowType == 1 && (dm[5] == "BT" || dm[5] == "ED2K")) {
			update_BT_transfer();
		}
		else if (taskLogShowType == 1 && (dm[5] == "HTTP" || dm[5] == "NZB" || dm[5] == "FTP")) {
			update_FHN_transfer();
		}
		else if (taskLogShowType == 2 && dm[5] == "BT") {
			update_BT_links();
		}
		else if (taskLogShowType == 3 && dm[5] == "BT") {
			update_BT_included();
		}
		else if (taskLogShowType == 3 && dm[5] == "NZB") {
			update_NZB_Log();
		}
		else if (taskLogShowType == 2 && dm[5] == "NZB") {
			create_NZBFileInfo();
		}
	}
}



//dm_Log is the selected task's desired taskLogInfo
function Get_dm_Log() {
	if (dm.length && isLogShow) {
		dm_Log.length = 0;
		var t;
		var url = "dm_print_status.cgi";
		var action_mode = "show_single_task";
		url += "?action_mode=" + action_mode;
		url += "&task_id=" + dm[0];
		ajaxRequest = false;
		if (taskLogShowType == 0) {
			url += "&logTab=" + taskLogShowType + "&download_type=" + taskLogShowType + "&t=" + Math.random();
			$j.ajax({
				url: url,
				async: true,
				success: function (data) { show_dm_Log(data) },
				complete: function (XMLHttpRequest, textStatus) { ajaxRequest = true; }
			})
		}
		else if (taskLogShowType == 1) {
			url += "&logTab=" + taskLogShowType + "&download_type=" + taskLogShowType + "&t=" + Math.random();
			$j.ajax({
				url: url,
				async: true,
				success: function (data) { show_dm_Log(data) },
				complete: function (XMLHttpRequest, textStatus) { ajaxRequest = true; }
			})
		}
		else if (taskLogShowType == 2) {
			url += "&logTab=" + taskLogShowType + "&download_type=" + taskLogShowType + "&t=" + Math.random();
			$j.ajax({
				url: url,
				async: true,
				success: function (data) { show_dm_Log(data) },
				complete: function (XMLHttpRequest, textStatus) { ajaxRequest = true; }
			})
		}
		else if (taskLogShowType == 3) {
			url += "&logTab=" + taskLogShowType + "&download_type=" + taskLogShowType + "&t=" + Math.random();
			$j.ajax({
				url: url,
				async: true,
				success: function (data) { show_dm_Log(data) },
				complete: function (XMLHttpRequest, textStatus) { ajaxRequest = true; }
			})
		}
	}
}

function Update_dm_Log() {
	if (isLogShow && dm.length && ajaxRequest) {
		dm_Log.length = 0;
		var t;
		var url = "dm_print_status.cgi";
		var action_mode = "show_single_task";
		url += "?action_mode=" + action_mode;
		url += "&task_id=" + dm[0];
		if (taskLogShowType == 0) {
			url += "&logTab=" + taskLogShowType + "&download_type=" + taskLogShowType + "&t=" + Math.random();
			$j.get(url, function (data) { show_dm_Log(data); });
		}
		else if (taskLogShowType == 1) {
			url += "&logTab=" + taskLogShowType + "&download_type=" + taskLogShowType + "&t=" + Math.random();
			$j.get(url, function (data) { show_dm_Log(data); });
		}
		else if (taskLogShowType == 2 && dm[4] == "BT") {
			url += "&logTab=" + taskLogShowType + "&download_type=" + taskLogShowType + "&t=" + Math.random();
			$j.get(url, function (data) { show_dm_Log(data); });
		}
		else if (taskLogShowType == 3) {
			url += "&logTab=" + taskLogShowType + "&download_type=" + taskLogShowType + "&t=" + Math.random();
			$j.get(url, function (data) { show_dm_Log(data); });
		}
	}
}

function select_taskType() {
	if (taskLogShowType == 0) {
		select_taskType_general();
	}
	else if (taskLogShowType == 1) {
		select_taskType_transfer();
	}
	else if (taskLogShowType == 2) {
		select_taskType_links();
	}

}

//dm is the selected task
function selectedTask(id) {
	for (var i = 0; i < dm_array.length; i++) {
		if (id == dm_array[i][0]) {
			dm = dm_array[i];
			break;
		}
	}
	change_bgcolor();
	updateCtrlIcon();
	select_taskType();
	update_taskLogIcon();
	update_taskLogFilename();
}



//create taskLogFilename in the taskLog
function create_taskLogFilename() {

	var taskLog_code = "";

	taskLog_code += "<table style='width:246px;table-layout:fixed;'>\n";
	taskLog_code += "<tr><td id='taskLog_filename' valign='middle' style='font-size:20px;'>No Select File</td></tr></table>\n";

	$("taskLogFilename").innerHTML = taskLog_code;
}
//update taskLogFilename in the taskLog
function update_taskLogFilename() {
	if (document.getElementById("taskLog_filename") && $j("#taskLog_filename").html() != dm[1])
		$j("#taskLog_filename").html(dm[1]);
}


//different task type will have different taskInfo 
function select_taskType_general() {
	taskLogShowType = 0;
	if (dm[5] == "BT" || dm[5] == "ED2K")
		create_fileInfo();
	else if (dm[5] == "NZB")
		create_NZBGeneralInfo();
	else
		create_GeneralInfo();
	Get_dm_Log();
}
//create BT fileInfo in the taskLog table for taskLogIcon 1
function create_fileInfo() {

	var taskLog_code = "";

	taskLog_code += "<table style='width:241px; table-layout:fixed;'>\n";
	taskLog_code += "<tr><td style='font-size:15px; font-weight:bold;' colspan='4'>\n";
	taskLog_code += "<span>" + multiLanguage_array[multi_INT][36] + "</span>\n";
	taskLog_code += "</td></tr>\n";
	if (dm[5] != "ED2K") {
		taskLog_code += "<tr><td colspan='2'><span style='margin-left:5px;'>" + multiLanguage_array[multi_INT][37] + "</span>\n";
		taskLog_code += "</td><td colspan='5'><span id='taskLog_Pieces'>-</span></td></tr>\n";
	}
	taskLog_code += "<tr><td colspan='2' valign='top'><span style='margin-left:5px;'>" + multiLanguage_array[multi_INT][38] + "</span>\n";
	taskLog_code += "</td><td colspan='5' style='word-wrap:break-word;overflow:hidden;'><span id='taskLog_Hash'>-</span></td></tr>\n";
	taskLog_code += "<tr><td colspan='2'><span style='margin-left:5px;'>" + multiLanguage_array[multi_INT][39] + "</span>\n";
	taskLog_code += "</td><td colspan='5'><span id='taskLog_Secure'>-</span></td></tr>\n";
	taskLog_code += "<tr><td colspan='2'><span style='margin-left:5px;'>" + multiLanguage_array[multi_INT][40] + "</span>\n";
	taskLog_code += "</td><td colspan='5'><span  id='taskLog_Comment'>-</span></td></tr>\n";
	if (dm[5] != "ED2K") {
		taskLog_code += "<tr><td style='border-bottom:1px #000 solid;' colspan='4'></td></tr>\n"
		taskLog_code += "<tr><td style='font-size:15px; font-weight:bold;' colspan='4'>\n";
		taskLog_code += "<span>" + multiLanguage_array[multi_INT][41] + "</span>\n";
		taskLog_code += "</td></tr>\n";
		taskLog_code += "<tr><td colspan='2'><span style='margin-left:5px;'>" + multiLanguage_array[multi_INT][42] + "</span>\n";
		taskLog_code += "</td><td colspan='5'><span id='taskLog_Creator'>-</span></td></tr>\n";
		taskLog_code += "<tr><td colspan='2'><span style='margin-left:5px;'>" + multiLanguage_array[multi_INT][43] + "</span>\n";
		taskLog_code += "</td><td colspan='5'><span id='taskLog_Date'>-</span></td></tr>\n";
	}
	taskLog_code += "<tr><td style='border-bottom:1px #000 solid;' colspan='4'></td></tr>\n"
	taskLog_code += "<tr><td style='font-size:15px; font-weight:bold;' colspan='4'>\n";
	taskLog_code += "<span>" + multiLanguage_array[multi_INT][44] + "</span>\n";
	taskLog_code += "</td></tr>\n";
	taskLog_code += "<tr><td colspan='2' valign='top'><span style='margin-left:5px;'>" + multiLanguage_array[multi_INT][45] + "</span>\n";
	taskLog_code += "</td><td colspan='5' style='word-wrap:break-word;overflow:hidden;'><span id='taskLog_Download_Dir'>-</span></td></tr>\n";
	taskLog_code += "<tr><td></td><td></td><td></td><td></td></tr></table>\n";
	$("taskLog").innerHTML = taskLog_code;
}


//create HTTP FTP GeneralInfo in the taskLog table for taskLogIcon 1
function create_GeneralInfo() {
	var taskLog_code = "";

	taskLog_code += "<table style='width:241px; table-layout:fixed;'>\n";
	taskLog_code += "<tr><td colspan='2' valign='top'>\n"
	taskLog_code += "<span style='margin-left:5px;'>" + multiLanguage_array[multi_INT][46] + "</span>\n"
	taskLog_code += "</td><td colspan='5' style='word-break:break-all;' id='taskLog_FHN_Destination'>-</td></tr>\n"
	taskLog_code += "<tr><td valign='top' colspan='2'>\n";
	taskLog_code += "<span style='margin-left:5px;'>" + multiLanguage_array[multi_INT][47] + "</span>\n"
	taskLog_code += "</td><td colspan='5'><span id='taskLog_FHN_Createdtime'>-</span></td></tr>\n"
	taskLog_code += "<tr><td colspan='2'>\n";
	taskLog_code += "<span style='margin-left:5px;'>" + multiLanguage_array[multi_INT][48] + "</span>\n";
	taskLog_code += "</td><td colspan='5' id='taskLog_FHN_Filesize'>-</td></tr>\n"
	taskLog_code += "<tr><td colspan='2'valign='top' style='word-break:break-all;overflow:hidden;'>\n";
	taskLog_code += "<span style='margin-left:5px;'>" + multiLanguage_array[multi_INT][49] + "</span>\n";
	taskLog_code += "</td><td id='taskLog_FHN_URL' colspan='5'>-</td></tr>\n"
	taskLog_code += "<tr><td><td><td><td><td></td></td></td></td></td></tr></table>\n"

	$("taskLog").innerHTML = taskLog_code;
}
//create NZB GeneralInfo in the taskLog table for taskLogIcon 1
function create_NZBGeneralInfo() {
	var taskLog_code = "";

	taskLog_code += "<table style='width:241px; table-layout:fixed;'>\n";
	taskLog_code += "<tr><td colspan='2' valign='top'>\n";
	taskLog_code += "<span style='margin-left:5px;'>" + multiLanguage_array[multi_INT][46] + "</span>\n";
	taskLog_code += "</td><td id='taskLog_FHN_Destination' colspan='3' style='word-wrap:break-word;overflow:hidden;'>-</td></tr>\n"
	taskLog_code += "<tr><td colspan='2'>\n"
	taskLog_code += "<span style='margin-left:5px;'>" + multiLanguage_array[multi_INT][48] + "</span>\n";
	taskLog_code += "</td><td id='taskLog_FHN_Filesize' colspan='3' style='word-wrap:break-word;overflow:hidden;'>-</td></tr>\n"
	taskLog_code += "<tr><td colspan='2' valign='top'>\n";
	taskLog_code += "<span style='margin-left:5px;'>" + multiLanguage_array[multi_INT][47] + "</span>\n";
	taskLog_code += "</td><td colsapn='3'><span id='taskLog_FHN_Createdtime'>-</span></td></tr>\n"
	taskLog_code += "<tr><td><td><td><td><td></td></td></td></td></td></tr></table>\n";

	$("taskLog").innerHTML = taskLog_code;
}

//different task type will have different taskInfo 
function select_taskType_transfer() {
	taskLogShowType = 1;
	if (dm[5] == "BT" || dm[5] == "ED2K")
		create_downloadInfo();
	else
		create_TransferInfo();
	Get_dm_Log();
}
// create BT downloadInfo in the taskLog table for taskLogIcon 2
function create_downloadInfo() {
	var taskLog_code = "";

	taskLog_code += "<table style='width:241px;table-layout:fixed;'>\n";

	taskLog_code += "<tr><td colspan='8' style='font-size:15px; font-weight:bold;'>\n";
	taskLog_code += "<span>" + multiLanguage_array[multi_INT][50] + "</span>\n";
	taskLog_code += "</td></tr>";
	taskLog_code += "<tr><td colspan='3'><span style='margin-left:5px;'>" + multiLanguage_array[multi_INT][51] + "</span></td>\n";
	taskLog_code += "<td colspan='5'><span id='taskLog_Progress'>-</span></td></tr>\n";
	taskLog_code += "<tr><td colspan='3'>\n";
	taskLog_code += "<span style='margin-left:5px;'>" + multiLanguage_array[multi_INT][52] + "</span></td>\n";
	taskLog_code += "<td colspan='5'><span id='taskLog_Size'>-</span></td></tr>\n";
	taskLog_code += "<tr><td colspan='3'>\n";
	taskLog_code += "<span style='margin-left:5px;'>" + multiLanguage_array[multi_INT][53] + "</span></td>\n";
	taskLog_code += "<td colspan='5'><span id='taskLog_Status'>-</span></td></tr>\n";
	// availability no update
	if (dm[5] != "ED2K") {
		taskLog_code += "<tr><td colspan='3'>\n";
		taskLog_code += "<span style='margin-left:5px;'>" + multiLanguage_array[multi_INT][54] + "</span></td>\n";
		taskLog_code += "<td colspan='5'><span id='taskLog_Availability'>-</span></td></tr>\n";
	}
	taskLog_code += "<tr><td colspan='3'>\n";
	taskLog_code += "<span style='margin-left:5px;'>" + multiLanguage_array[multi_INT][55] + "</span></td>\n";
	taskLog_code += "<td colspan='5'><span id='taskLog_Downloaded'>-</span></td></tr>\n";
	if (dm[5] != "ED2K") {
		taskLog_code += "<tr><td colspan='3'>\n";
		taskLog_code += "<span style='margin-left:5px;'>" + multiLanguage_array[multi_INT][56] + "</span></td>\n";
		taskLog_code += "<td colspan='5'><span id='taskLog_Uploaded'>-</span></td></tr>\n";
	}
	taskLog_code += "<tr><td colspan='3'>\n";
	taskLog_code += "<span style='margin-left:5px;'>" + multiLanguage_array[multi_INT][57] + "</span></td>\n";
	taskLog_code += "<td colspan='5'><span id='taskLog_DL_Speed'>-</span></td></tr>\n";
	if (dm[5] != "ED2K") {
		taskLog_code += "<tr><td colspan='3'>\n";
		taskLog_code += "<span style='margin-left:5px;'>" + multiLanguage_array[multi_INT][58] + "</span></td>\n";
		taskLog_code += "<td colspan='5'><span id='taskLog_UL_Speed'>-</span></td></tr>\n";
	}
	taskLog_code += "<tr><td colspan='3'>\n";
	taskLog_code += "<span style='margin-left:5px;'>" + multiLanguage_array[multi_INT][59] + "</span></td>\n";
	taskLog_code += "<td colspan='5'><span id='taskLog_Error'>-</span></td></tr>\n";
	// ratio no update
	if (dm[5] != "ED2K") {
		taskLog_code += "<tr><td colspan='3'>\n";
		taskLog_code += "<span style='margin-left:5px;'>" + multiLanguage_array[multi_INT][60] + "</span></td>\n";
		taskLog_code += "<td colspan='5'><span id='taskLog_Ratio'>-</span></td></tr>\n";
	}
	taskLog_code += "<tr><td colspan='8' style='font-size:15px; font-weight:bold; border-top:#000 solid 1px;'>\n";
	taskLog_code += "<span>" + multiLanguage_array[multi_INT][61] + "</span>\n";
	taskLog_code += "</td></tr>\n";
	//UL To and DL From no update
	if (dm[5] != "ED2K") {
		taskLog_code += "<tr><td colspan='4'>\n";
		taskLog_code += "<span style='margin-left:5px;'>" + multiLanguage_array[multi_INT][62] + "</span></td>\n";
		taskLog_code += "<td colspan='4'><span id='taskLog_UL_To'>-</span></td></tr>\n";
	}
	taskLog_code += "<tr><td colspan='4'>\n";
	taskLog_code += "<span style='margin-left:5px;'>" + multiLanguage_array[multi_INT][63] + "</span></td>\n";
	taskLog_code += "<td colspan='4'><span id='taskLog_DL_From'>-</span></td></tr></table>\n";
	$("taskLog").innerHTML = taskLog_code;
}

//create NZB HTTP FTP TransferInfo in the taskLog table for taskLogIcon 2
function create_TransferInfo() {
	var taskLog_code = "";

	taskLog_code += "<table style='width:241px; table-layout:fixed;'>\n";
	taskLog_code += "<tr><td>\n";
	taskLog_code += "<span style='margin-left:5px;'>" + multiLanguage_array[multi_INT][53] + "</span>\n";
	taskLog_code += "<span id='taskLog_FHN_Status' style='margin-left:8px;'>-</span></td></tr>\n";
	taskLog_code += "<tr><td>\n";
	taskLog_code += "<span style='margin-left:5px;'>" + multiLanguage_array[multi_INT][55] + "</span>\n";
	taskLog_code += "<span id='taskLog_FHN_Transferred' style='margin-left:8px;'>-</span></td></tr>\n";
	taskLog_code += "<tr><td>\n";
	taskLog_code += "<span style='margin-left:5px;'>" + multiLanguage_array[multi_INT][51] + "</span>\n";
	taskLog_code += "<span id='taskLog_FHN_Progress' style='margin-left:8px;'>-</span></td></tr>\n";
	taskLog_code += "<tr><td>\n";
	taskLog_code += "<span style='margin-left:5px;'>" + multiLanguage_array[multi_INT][64] + "</span>\n";
	taskLog_code += "<span id='taskLog_FHN_Speed' style='margin-left:8px;'>-</span></td></tr>\n";
	taskLog_code += "<tr><td>\n";
	taskLog_code += "<span style='margin-left:5px;'>" + multiLanguage_array[multi_INT][65] + "</span>\n";
	taskLog_code += "<span id='taskLog_FHN_StartTime' style='margin-left:8px;'>-</span></td></tr>\n";
	taskLog_code += "<tr><td>\n";
	taskLog_code += "<span style='margin-left:5px;'>" + multiLanguage_array[multi_INT][66] + "</span>\n";
	taskLog_code += "<span id='taskLog_FHN_TimeLeft' style='margin-left:8px;'>-</span></td></tr></table>\n";
	$("taskLog").innerHTML = taskLog_code;
}


//different task type will have different taskInfo 
function select_taskType_links() {
	taskLogShowType = 2;
	Get_dm_Log();
	if (dm[5] == "NZB")
		create_NZBFileInfo();
	else if (dm[5] == "BT")
		create_linksInfo();
	else {
		$("taskLog").innerHTML = "";
	}
}
//create NZB FileInfo in the taskLog table for taskLogIcon 3
function create_NZBFileInfo() {
	var taskLog_code = "";
	taskLog_code += "<table style='width:229px; table-layout:fixed;'>\n";
	taskLog_code += "<tr style='font-weight:bold; font-size:18px;'><th>\n";
	taskLog_code += "<span>" + multiLanguage_array[multi_INT][67] + "</span>\n";
	taskLog_code += "</th></tr>\n"
	for (var i = 0; i < dm_Log.length; i++) {
		taskLog_code += "<tr><td style='word-wrap:break-word;overflow:hidden;'>" + dm_Log[i][0] + "</td></tr>\n";
	}
	taskLog_code += "<tr><td align='right'><input type='button' class='button_gen' value='Refresh' onclick='get_NZBfile_Log();' /></tr></td>\n"
	taskLog_code += "</table>\n";
	$("taskLog").innerHTML = taskLog_code;
}
function get_NZBfile_Log() {
	dm_Log.length = 0;
	var url = "dm_print_status.cgi";
	var action_mode = "show_single_task";
	url += "?action_mode=" + action_mode;
	url += "&task_id=" + dm[0];
	url += "&logTab=" + taskLogShowType + "&download_type=" + taskLogShowType + "&t=" + Math.random();
	$j.get(url, function (data) { show_dm_Log(data); });

}
//create BT linksInfo in the taskLog table for taskLogIcon 3
function create_linksInfo() {

	var taskLog_code = "";
	for (var i = 0; i < dm_Log.length; i++) {
		taskLog_code += "<table style='width:229px; table-layout:fixed;' border='0' cellspacing='0'>\n";
		taskLog_code += "<tr><td style='font-size:15px; font-weight:bold; width:128px;'>\n";
		taskLog_code += "<span>" + multiLanguage_array[multi_INT][68] + "</span>\n";
		taskLog_code += "</span>" + (i + 1) + "<span></td></tr>\n";
		taskLog_code += "<tbody style='background-color:#999;'><tr>\n"
		taskLog_code += "<td style='word-wrap:break-word;overflow:hidden;'><span style='margin-left:7px;' id='taskLog_linkName" + i + " '>" + dm_Log[i][0] + "</span></td></tr>\n";
		taskLog_code += "<tr><td style='word-wrap:break-word;overflow:hidden;'><span style='margin-left:7px;' id='taskLog_lastAnnounce" + i + "'>" + dm_Log[i][1] + "<span></td></tr>\n";
		taskLog_code += "<tr><td style='word-wrap:break-word;overflow:hidden;'><span style='margin-left:7px;' id='taskLog_lastScrape" + i + "'>" + dm_Log[i][5] + "<span></td></tr>\n";
		taskLog_code += "<tr><td style='word-wrap:break-word;overflow:hidden;'><span style='margin-left:7px;' id='taskLog_Seeders" + i + "'>" + dm_Log[i][2] + "</span></td></tr>\n";
		taskLog_code += "<tr><td style='word-wrap:break-word;overflow:hidden;'><span style='margin-left:7px;' id='taskLog_Leechers" + i + "'>" + dm_Log[i][3] + "</span></td></tr>\n";
		taskLog_code += "<tr><td style='word-wrap:break-word;overflow:hidden;'><span style='margin-left:7px;' id='taskLog_Downloads" + i + "'>" + dm_Log[i][4] + "</span></td></tr>\n";
		taskLog_code += "</tbody></table>\n";
	}

	$("taskLog").innerHTML = taskLog_code;
}

//different task type will have different taskInfo 
function select_taskType_included() {
	taskLogShowType = 3;
	Get_dm_Log();
	if (dm[5] == "NZB")
		create_LogInfo();
	else if (dm[5] == "BT")
		create_includedInfo();
	else {
		$("taskLog").innerHTML = "";
	}
}
//create BT includedInfo in the taskLog table 4
function create_includedInfo() {
	var taskLog_code = "";
	taskLog_code += "<table style='width:229px; table-layout:fixed;' border='0' cellspacing='0'>\n";
	for (var i = 0; i < dm_Log.length; i++) {
		taskLog_code += "<tr><td style='width:7px;'></td><td style='word-wrap:break-word;overflow:hidden;'><span id='BTincludedFile" + i + "'>" + dm_Log[i][0] + "</span></td></tr>\n";
		taskLog_code += "<tr><td style='width:7px;'></td><td style='word-wrap:break-word;overflow:hidden;'>progress:<span style='margin-left:10px;' id='BTincludedFile_progress" + i + "'>" + dm_Log[i][1] + "</span></br>\n";
		taskLog_code += "<span id='BTincludedFile_downloaded" + i + "'>" + dm_Log[i][2] + "</span> of\n";
		taskLog_code += "<span id='BTincludedFile_size" + i + "'>" + dm_Log[i][3] + "</span> completed</br></br></td></tr>\n"
	}
	taskLog_code += "</table>\n";
	$("taskLog").innerHTML = taskLog_code;
}
//create NZB LogInfo in the taskLog table 4
function create_LogInfo() {
	var taskLog_code = "";
	taskLog_code += "<table style='width:229px; table-layout:fixed;' border='0' cellspacing='0'>\n";
	for (var i = 0; i < dm_Log.length; i++) {
		taskLog_code += "<tr><td>&nbsp;Name:<span style='margin-left:7px;word-break:break-all;' id='NZBLogName" + i + "'>" + dm_Log[i][0] + "</span><br />\n";
		taskLog_code += "&nbsp;Progress:<span style='margin-left:7px;' id='NZBLogProgress" + i + "'>" + dm_Log[i][1] + "</span><br />\n";
		taskLog_code += "&nbsp;Size:<span style='margin-left:7px;' id='NZBLogSize" + i + "'>" + dm_Log[i][2] + "</span></td></tr>";
	}
	taskLog_code += "</table>\n";
	$("taskLog").innerHTML = taskLog_code;
}


//task that have be selected will have a different bgcolor
function change_bgcolor() {
	var dmid = dm[0];
	if (old_dmid != "") {
		if (dmid != old_dmid) {
			document.getElementById(dmid).bgColor = "#333";
			$j("#" + old_dmid).removeAttr("bgColor");
		}
	}
	else {
		document.getElementById(dmid).bgColor = "#333";
	}
	old_dmid_tmp = old_dmid;
	old_dmid = dmid;
}


function create_progress() {
	ProgressBar_array.length = 0;
	var task_code;
	for (var i = 0; i < dm_array.length; i++) {
		var j = dm_array[i][0];
		task_code = "<td colspan='8' style='position:relative;' id='pbid" + j + "'></td>";
		$j(task_code).replaceAll("#pbid" + j);
	}
	for (var i = 0; i < dm_array.length; i++) {

		var j = dm_array[i][0];
		var ProgressBar = "ProgressBar" + j;
		var pbid = "pbid" + j;
		ProgressBar = new Ext.ProgressBar({
			autoWidth: true,
			text: 'working....',
			//width:400,
			renderTo: pbid,
			stateId: pbid
		});
		ProgressBar_array[i] = ProgressBar;
		update_progress(ProgressBar, dm_array[i][2]);
	}
}

//show and hide the logInfo
function Show_Log() {
	var dmid = dm[0];
	if (!isLogShow) {
		//alert("first"+old_dmid+isLogShow+old_dmid_tmp);
		$j("#" + dmid).siblings().removeClass("taskLongWidth");
		$j("#" + dmid).removeClass("taskLongWidth");
		$j("#" + dmid).siblings().addClass("taskShortWidth");
		$j("#" + dmid).addClass("taskShortWidth");
		$j("#taskLog_show").show();
		isLogShow = true;
		Get_dm_Log();
		create_progress();
	}
	else {
		if (old_dmid == old_dmid_tmp) {
			//alert("second"+old_dmid+isLogShow+old_dmid_tmp);
			$j("#taskLog_show").hide();
			$j("#" + dmid).siblings().removeClass("taskShortWidth");
			$j("#" + dmid).removeClass("taskShortWidth");
			$j("#" + dmid).siblings().addClass("taskLongWidth");
			$j("#" + dmid).addClass("taskLongWidth");
			isLogShow = false;
			create_progress();
		}
		else {
			$j("#" + dmid).siblings().removeClass("taskLongWidth");
			$j("#" + dmid).removeClass("taskLongWidth");
			$j("#" + dmid).siblings().addClass("taskShortWidth");
			$j("#" + dmid).addClass("taskShortWidth");
			$j("#taskLog_show").show();
			isLogShow = true;
			create_progress();
		}
	}
}

function showUnamePsw(j) {
	var ftp_namepasswd = $j("#showUP" + j).attr("name");
	var check_name = "asus-none-ftpname" + j;
	var ftp_passwd_index, ftp_name, ftp_passwd;
	if (ftp_namepasswd != check_name) {
		ftp_passwd_index = ftp_namepasswd.indexOf(":");
		if (ftp_passwd_index == -1) {
			ftp_name = ftp_namepasswd;
			ftp_passwd = "";
		} else {
			ftp_name = ftp_namepasswd.substring(0, ftp_passwd_index);
			ftp_passwd = ftp_namepasswd.substring(ftp_passwd_index + 1);
		}
	}
	else {
		ftp_name = "";
		ftp_passwd = "";
	}
	WH_INT = setInterval("getWH();", 1000);
	$j("#User_name").attr("value", ftp_name);
	$j("#Password").attr("value", ftp_passwd);
	$j("#DM_mask").fadeIn(1000);
	$j("#UnamePsw_panel").show(1000);
}
function hideUnamePsw() {
	clearInterval(WH_INT);
	$j("#DM_mask").fadeOut('fast');
	$j("#UnamePsw_panel").hide('fast');
}

function show_diskfull_info() {
	Ext.MessageBox.alert(multiLanguage_array[multi_INT][3], multiLanguage_array[multi_INT][73]);
}

function show_Move4GBError_info() {
	Ext.MessageBox.alert(multiLanguage_array[multi_INT][3], multiLanguage_array[multi_INT][79]);
}

function show_MoveDiskFull_info() {
	Ext.MessageBox.alert(multiLanguage_array[multi_INT][3], multiLanguage_array[multi_INT][84]);
}

function show_vfat_info() {
	Ext.MessageBox.alert(multiLanguage_array[multi_INT][3], multiLanguage_array[multi_INT][78]);
}


function create_task() {
	//taskid = "";
	$j("#taskLog_show").hide();
	ProgressBar_array.length = 0;
	dm.length = 0;
	old_dmid = "";
	old_dmid_tmp = "";
	//old_taskType = "";
	isLogShow = false;
	var filename, progress, size, status, type, elased, download, upload, peers, havedone, downloaddir_error, downloaddir;//error;
	var progressTxt;

	var task_code = "";
	for (var i = 0; i < dm_array.length; i++) {
		var j = dm_array[i][0];
		filename = dm_array[i][1];
		progress = dm_array[i][2];
		size = dm_array[i][3];
		status = dm_array[i][4];
		type = dm_array[i][5];
		download = dm_array[i][7];
		upload = dm_array[i][8];
		peers = dm_array[i][9];
		havedone = dm_array[i][11];
		downloaddir_error = dm_array[i][12];
		downloaddir = dm_array[i][12].slice(8);
		if (type == "BT") {
			progressTxt = (parseFloat(size) * havedone).toFixed(2) + size.substr(size.length - 3, 3);
		}
		else {
			progressTxt = (parseFloat(size) * progress).toFixed(2) + size.substr(size.length - 3, 3);
		}
		task_code += '<table id="' + j + '" class="taskLongWidth" style="border-bottom:#000 solid 1px; table-layout:fixed;" onclick="selectedTask(this.id);">\n';
		task_code += '<tr><td colspan="9" style="word-wrap:break-word;word-break:break-all;overflow:hidden;"><span style="font-weight:bold; font-size:16px;" id="filename' + j + '">' + filename + '</span>\n';
		task_code += '<a href="javascript:Show_Log();"><img id="showicon' + j + '" src="images/icon/Ino2Icon.png" width="15" height="15" onmousedown="changeshowicon(this.id);" onmouseup="returnshowicon(this.id);" title="Detail information" /></a></td></tr>\n';
		task_code += '<tr><td colspan="2"><span style="margin-right:30px;" id="type' + j + '">' + type + '</span></td>\n';
		if (status != "notbegin") {
			task_code += '<td colspan="7" id="progresstd' + j + '"><span id="progress' + j + '">' + progressTxt + '</span> of <span id="size' + j + '">' + size + '</span> downloaded</td></tr>\n';
		}
		task_code += '<tr><td colspan="8" style="position:relative;" id="pbid' + j + '"></td>\n';
		if (status == "Hashing") {
			task_code += '<td style="width:40px;"><img id="status_icon' + j + '" src="images/statusIcon/' + 'Initialing.gif" alt="" />';
			if (navigator.appName.indexOf("Microsoft") >= 0 && (location.host.split(":")[0] == LAN_IP || location.host.split(":")[0] == LOCAL_DOMAIN)) {
				if (status == "Finished" || status == "Seeding") {
					if (downloaddir_error == "not_found") {
						task_code += '<a id="open_dir_link' + j + '"  style="display:none;"><img id="open_icon' + j + '" src="images/default/tree/folder.gif" alt="" style="display:none;" title="Not Exist"/></a>';
					}
					else {
						task_code += '<a id="open_dir_link' + j + '" target="_blank" href="file://' + LAN_IP + downloaddir + '" style="display:none;"><img id="open_icon' + j + '" src="images/default/tree/folder.gif" alt="" style="display:none;" title=""/></a>';
					}
				}
				else {
					task_code += '<a id="open_dir_link' + j + '" target="_blank" href="file://' + LAN_IP + downloaddir + '" style="display:none;"><img id="open_icon' + j + '" src="images/default/tree/folder.gif" alt="" style="display:none;" title=""/></a>';
				}
			}
			task_code += '</td></tr>\n';
		}
		else {
			task_code += '<td style="width:40px;"><img id="status_icon' + j + '" src="images/statusIcon/' + status + '.gif" alt="" />';
			if (navigator.appName.indexOf("Microsoft") >= 0 && (location.host.split(":")[0] == LAN_IP || location.host.split(":")[0] == LOCAL_DOMAIN)) {
				if (status == "Finished" || status == "Seeding") {
					if (downloaddir_error == "not_found") {
						task_code += '<a id="open_dir_link' + j + '" style="display:none;"><img id="open_icon' + j + '" src="images/default/tree/folder.gif" alt="" style="display:none;" title="Not Exist"/></a>';
					}
					else {
						task_code += '<a id="open_dir_link' + j + '" target="_blank" href="file://' + LAN_IP + downloaddir + '" style="display:none;"><img id="open_icon' + j + '" src="images/default/tree/folder.gif" alt="" style="display:none;" title=""/></a>';
					}
				}
				else {
					task_code += '<a id="open_dir_link' + j + '" target="_blank" href="file://' + LAN_IP + downloaddir + '" style="display:none;"><img id="open_icon' + j + '" src="images/default/tree/folder.gif" alt="" style="display:none;" title=""/></a>';
				}
			}
			task_code += '</td></tr>\n';
		}
		var ftp_name = "";
		if (filename.substring(0, 6) == "FTP://" || filename.substring(0, 6) == "ftp://") {
			var ftp_name_index = filename.substring(6).indexOf("@");
			if (ftp_name_index >= 0) {
				var kk = filename.substring(6).indexOf("/");
				if (kk > 0) {
					var tmp = filename.substring(6).substring(0, kk); //user:pwd@host
					var ii = tmp.lastIndexOf("@");
					var account = tmp.substring(0, ii);              //user:pwd
					var yy = account.indexOf(":")
					if (yy > 0) {
						ftp_name = account.substring(0, yy);
					}
					else {
						ftp_name = account;
					}
				}
				else {
					ftp_name = "asus-none-ftpname" + j;
				}
			} else {
				ftp_name = "asus-none-ftpname" + j;
			}
		} else if (filename.substring(0, 7) == "HTTP://" || filename.substring(0, 7) == "http://") {
			var http_name_index = filename.substring(7).indexOf("@");
			if (http_name_index >= 0) {
				var kk = filename.substring(7).indexOf("/");
				if (kk > 0) {
					var tmp = filename.substring(7).substring(0, kk); //user:pwd@host
					var ii = tmp.lastIndexOf("@");
					var account = tmp.substring(0, ii);              //user:pwd
					var yy = account.indexOf(":")
					if (yy > 0) {
						ftp_name = account.substring(0, yy);
					}
					else {
						ftp_name = account;
					}
				}
				else {
					ftp_name = "asus-none-ftpname" + j;
				}
			} else {
				ftp_name = "asus-none-ftpname" + j;
			}
		} else {
			ftp_name = "asus-none-ftpname" + j;
		}
		task_code += '<tr><td colspan="3"><a id="showUP' + j + '" name=' + ftp_name + ' href="javascript:showUnamePsw(' + j + ');" style="display:none;"><img src="images/statusIcon/warning.png" alt="" /><span>' + multiLanguage_array[multi_INT][69] + '</span></a>\n';
		if (httpTag) {
			task_code += '<a id="showUP_n1' + j + '" href="http://' + location.host + '/downloadmaster/Setting_NZB.asp" style="display:none;"><img src="images/statusIcon/warning.png" alt="" /><span>' + multiLanguage_array[multi_INT][70] + '</span></a>\n';
			task_code += '<a id="showUP_n2' + j + '" href="http://' + location.host + '/downloadmaster/Setting_NZB.asp" style="display:none;"><img src="images/statusIcon/warning.png" alt="" /><span>' + multiLanguage_array[multi_INT][71] + '</span></a>\n';
			task_code += '<a id="showUP_n3' + j + '" href="http://' + location.host + '/downloadmaster/Setting_NZB.asp" style="display:none;"><img src="images/statusIcon/warning.png" alt="" /><span>' + multiLanguage_array[multi_INT][72] + '</span></a>\n';
		} else {
			task_code += '<a id="showUP_n1' + j + '" href="https://' + location.host + '/downloadmaster/Setting_NZB.asp" style="display:none;"><img src="images/statusIcon/warning.png" alt="" /><span>' + multiLanguage_array[multi_INT][70] + '</span></a>\n';
			task_code += '<a id="showUP_n2' + j + '" href="https://' + location.host + '/downloadmaster/Setting_NZB.asp" style="display:none;"><img src="images/statusIcon/warning.png" alt="" /><span>' + multiLanguage_array[multi_INT][71] + '</span></a>\n';
			task_code += '<a id="showUP_n3' + j + '" href="https://' + location.host + '/downloadmaster/Setting_NZB.asp" style="display:none;"><img src="images/statusIcon/warning.png" alt="" /><span>' + multiLanguage_array[multi_INT][72] + '</span></a>\n';
		}
		task_code += '<a id="showUP_n4' + j + '" style="display:none;"><img src="images/statusIcon/warning.png" alt="" /><span>' + multiLanguage_array[multi_INT][76] + '</span></a>\n';
		task_code += '<a id="showUP_n5' + j + '" href="javascript:show_diskfull_info();" style="display:none;"><img src="images/statusIcon/warning.png" alt="" /><span>' + multiLanguage_array[multi_INT][77] + '</span></a>\n';
		task_code += '<a id="showUP_n6' + j + '" href="javascript:show_vfat_info();" style="display:none;"><img src="images/statusIcon/warning.png" alt="" /><span>' + multiLanguage_array[multi_INT][83] + '</span></a>\n';
		task_code += '<a id="showUP_n7' + j + '" href="javascript:show_Move4GBError_info();" style="display:none;"><img src="images/statusIcon/warning.png" alt="" /><span>' + multiLanguage_array[multi_INT][83] + '</span></a>\n';
		task_code += '<a id="showUP_n8' + j + '" href="javascript:show_MoveDiskFull_info();" style="display:none;"><img src="images/statusIcon/warning.png" alt="" /><span>' + multiLanguage_array[multi_INT][77] + '</span></a>\n';
		task_code += '<a id="showUP_n9' + j + '" href="' + location.protocol + '//' + location.host +'/downloadmaster/Setting_amule.asp' + '" style="display:none;"><img src="images/statusIcon/warning.png" alt="" /><span>' + multiLanguage_array[multi_INT][86] + '</span></a>\n';
		if (status != "notbegin") {
			task_code += '<span id="status' + j + '">' + status + '</span></td>\n';
		} else {
			task_code += '<span id="status' + j + '">' + 'Waiting' + '</span></td>\n';
		}
		if (status != "notbegin") {
			if (type != "HTTP" && type != "FTP" && type != "NZB") {
				task_code += '<td colspan="2" id="peerstd' + j + '"><span id="peers' + j + '">' + peers + '</span> peers</td>\n';
			}
			else {
				task_code += '<td colspan="2"></td>\n';
			}
			task_code += '<td id="downloadtd' + j + '" align="right" colspan="2" id="downloadplace' + j + '"><img width="13" height="13" src="images/icon/DownArrowIcon.png" /><span id="download' + j + '">' + download + '</span></td>\n';
			if (type != "HTTP" && type != "FTP" && type != "NZB") {
				task_code += '<td colspan="2" id="uploadtd' + j + '"><img width="13" height="13" src="images/icon/UpArrowIcon.png" /><span id="upload' + j + '">' + upload + '</span></td>\n';
			}
			else {
				task_code += '<td colspan="2"></td>\n';
			}
		}
		task_code += '</tr>\n';
		task_code += '<tr><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td></tr>\n';
		task_code += '</table>\n';
	}
	task_code += '<div id="div_for_addtask" style="display:none;"></div>\n';
	$("TaskMain").innerHTML = task_code;
	$j("#transfers").html(dm_array.length);
	create_progress();

	//clear taskLog everytime wehn task num changed
	create_taskLogFilename();
	//select_taskType_general();
	updateCtrlIcon();

}

function update_progress(ProgressBar, percentage) {
	//var x = percentage*10000;
	//var y = parseFloat(x)/100;
	var x = parseFloat(percentage) * 100;
	var y = x.toFixed(1);
	var progressText = y + "%";
	ProgressBar.updateProgress(percentage, progressText);
}

function updateDownloadplace(downloadplaceId) {

}

function update_task() {
	if (dm_array.length == 0) {
		$j("#total_uploadSpeed").html(0);
		$j("#total_downloadSpeed").html(0);
		return;

	}
	var totaldownloadSpeed = 0;
	var totaluploadSpeed = 0;
	var progressTxt;
	var downloaddir_error, downloaddir;
	for (var i = 0; i < dm_array.length; i++) {
		var j = dm_array[i][0];
		downloaddir_error = dm_array[i][12];
		downloaddir = dm_array[i][12].slice(8);
		if (!document.getElementById(j)) {
			add_task(i);
		}
		if (dm_array[i][5] == "BT") {
			progressTxt = (parseFloat(dm_array[i][3]) * dm_array[i][11]).toFixed(2) + dm_array[i][3].substr(dm_array[i][3].length - 3, 3);
		}
		else {
			progressTxt = (parseFloat(dm_array[i][3]) * dm_array[i][2]).toFixed(2) + dm_array[i][3].substr(dm_array[i][3].length - 3, 3);
		}
		//filename	
		if (document.getElementById("filename" + j) && $j("#filename" + j).html() != dm_array[i][1])
			$j("#filename" + j).html(dm_array[i][1]);
		//status
		if (dm_array[i][4] == "Finished" && document.getElementById("downloadplace" + j))//根据状态如果是Finished，就隐藏掉下载速度
			$j("#downloadplace" + j).hide();
		if (dm_array[i][4] == "warning" && document.getElementById("showUP" + j))//根据状态需要用户名和密码去显示图标
		{
			$j("#showUP" + j).show();
			$j("#showUP_n1" + j).hide();
			$j("#showUP_n2" + j).hide();
			$j("#showUP_n3" + j).hide();
			$j("#showUP_n4" + j).hide();
			$j("#showUP_n5" + j).hide();
			$j("#showUP_n6" + j).hide();
			$j("#showUP_n7" + j).hide();
			$j("#showUP_n8" + j).hide();
			$j("#showUP_n9" + j).hide();
			$j("#status" + j).html("");
		}
		else if ((dm_array[i][4] == "ConnectFail" || dm_array[i][4] == "SoketRecvFail") && document.getElementById("showUP_n1" + j)) {
			$j("#showUP_n1" + j).show();
			$j("#showUP" + j).hide();
			$j("#showUP_n2" + j).hide();
			$j("#showUP_n3" + j).hide();
			$j("#showUP_n4" + j).hide();
			$j("#showUP_n5" + j).hide();
			$j("#showUP_n6" + j).hide();
			$j("#showUP_n7" + j).hide();
			$j("#showUP_n8" + j).hide();
			$j("#showUP_n9" + j).hide();
			$j("#status" + j).html("");
		}
		else if (dm_array[i][4] == "SSLFail" && document.getElementById("showUP_n2" + j)) {
			$j("#showUP_n2" + j).show();
			$j("#showUP" + j).hide();
			$j("#showUP_n1" + j).hide();
			$j("#showUP_n3" + j).hide();
			$j("#showUP_n4" + j).hide();
			$j("#showUP_n5" + j).hide();
			$j("#showUP_n6" + j).hide();
			$j("#showUP_n7" + j).hide();
			$j("#showUP_n8" + j).hide();
			$j("#showUP_n9" + j).hide();
			$j("#status" + j).html("");
		}
		else if (dm_array[i][4] == "AccountFail" && document.getElementById("showUP_n3" + j)) {
			$j("#showUP_n3" + j).show();
			$j("#showUP" + j).hide();
			$j("#showUP_n1" + j).hide();
			$j("#showUP_n2" + j).hide();
			$j("#showUP_n4" + j).hide();
			$j("#showUP_n5" + j).hide();
			$j("#showUP_n6" + j).hide();
			$j("#showUP_n7" + j).hide();
			$j("#showUP_n8" + j).hide();
			$j("#showUP_n9" + j).hide();
			$j("#status" + j).html("");
		}
		else if (dm_array[i][4] == "NotCompleted" && document.getElementById("showUP_n4" + j)) {
			$j("#showUP_n4" + j).show();
			$j("#showUP" + j).hide();
			$j("#showUP_n1" + j).hide();
			$j("#showUP_n2" + j).hide();
			$j("#showUP_n3" + j).hide();
			$j("#showUP_n5" + j).hide();
			$j("#showUP_n6" + j).hide();
			$j("#showUP_n7" + j).hide();
			$j("#showUP_n8" + j).hide();
			$j("#showUP_n9" + j).hide();
			$j("#status" + j).html("");
		}
		else if (dm_array[i][4] == "Diskfull" && document.getElementById("showUP_n5" + j)) {
			$j("#showUP_n6" + j).hide();
			$j("#showUP_n5" + j).show();
			$j("#showUP_n4" + j).hide();
			$j("#showUP" + j).hide();
			$j("#showUP_n1" + j).hide();
			$j("#showUP_n2" + j).hide();
			$j("#showUP_n3" + j).hide();
			$j("#showUP_n7" + j).hide();
			$j("#showUP_n8" + j).hide();
			$j("#showUP_n9" + j).hide();
			$j("#status" + j).html("");
		}
		else if (dm_array[i][4] == "Vfat4G" && document.getElementById("showUP_n6" + j)) {
			$j("#showUP_n6" + j).show();
			$j("#showUP_n5" + j).hide();
			$j("#showUP_n4" + j).hide();
			$j("#showUP" + j).hide();
			$j("#showUP_n1" + j).hide();
			$j("#showUP_n2" + j).hide();
			$j("#showUP_n3" + j).hide();
			$j("#showUP_n7" + j).hide();
			$j("#showUP_n8" + j).hide();
			$j("#showUP_n9" + j).hide();
			$j("#status" + j).html("");
		}
		else if (dm_array[i][4] == "Move4GBError" && document.getElementById("showUP_n7" + j)) {
			$j("#showUP_n6" + j).hide();
			$j("#showUP_n5" + j).hide();
			$j("#showUP_n4" + j).hide();
			$j("#showUP" + j).hide();
			$j("#showUP_n1" + j).hide();
			$j("#showUP_n2" + j).hide();
			$j("#showUP_n3" + j).hide();
			$j("#showUP_n7" + j).show();
			$j("#showUP_n8" + j).hide();
			$j("#showUP_n9" + j).hide();
			$j("#status" + j).html("");
		}
		else if (dm_array[i][4] == "MoveDiskFull" && document.getElementById("showUP_n8" + j)) {
			$j("#showUP_n6" + j).hide();
			$j("#showUP_n5" + j).hide();
			$j("#showUP_n4" + j).hide();
			$j("#showUP" + j).hide();
			$j("#showUP_n1" + j).hide();
			$j("#showUP_n2" + j).hide();
			$j("#showUP_n3" + j).hide();
			$j("#showUP_n7" + j).hide();
			$j("#showUP_n8" + j).show();
			$j("#showUP_n9" + j).hide();
			$j("#status" + j).html("");
		} else if (dm_array[i][4] == "Disconnected" || dm_array[i][4] == "Timeout" && document.getElementById("showUP_n9" + j)) {
			$j("#showUP_n6" + j).hide();
			$j("#showUP_n5" + j).hide();
			$j("#showUP_n4" + j).hide();
			$j("#showUP" + j).hide();
			$j("#showUP_n1" + j).hide();
			$j("#showUP_n2" + j).hide();
			$j("#showUP_n3" + j).hide();
			$j("#showUP_n7" + j).hide();
			$j("#showUP_n8" + j).hide();
			$j("#showUP_n9" + j).show();
			$j("#status" + j).html("");
		}
		else {
			$j("#showUP_n6" + j).hide();
			$j("#showUP" + j).hide();
			$j("#showUP_n1" + j).hide();
			$j("#showUP_n2" + j).hide();
			$j("#showUP_n3" + j).hide();
			$j("#showUP_n4" + j).hide();
			$j("#showUP_n5" + j).hide();
			$j("#showUP_n7" + j).hide();
			$j("#showUP_n8" + j).hide();
			$j("#showUP_n9" + j).hide();
		}
		if (dm_array[i][4] == "Error" && document.getElementById("type" + j)) { //根据状态如果是Error，就隐藏掉一部分信息
			if (document.getElementById("peerstd" + j))
				$j("#peerstd" + j).hide();
			if (document.getElementById("uploadtd" + j))
				$j("#uploadtd" + j).hide();
			if (document.getElementById("downloadtd" + j))
				$j("#downloadtd" + j).hide();
			if (document.getElementById("progresstd" + j))
				$j("#progresstd" + j).hide();
		}
		if (dm_array[i][4] == "warning" && document.getElementById("type" + j)) { //根据状态如果是warning，就隐藏掉一部分信息
			if (document.getElementById("peerstd" + j))
				$j("#peerstd" + j).hide();
			if (document.getElementById("uploadtd" + j))
				$j("#uploadtd" + j).hide();
			if (document.getElementById("downloadtd" + j))
				$j("#downloadtd" + j).hide();
			if (document.getElementById("progresstd" + j))
				$j("#progresstd" + j).hide();
		}
		if (document.getElementById("status" + j) && $j("#status" + j).html() != dm_array[i][4]) {
			if (dm_array[i][4] == "warning" || dm_array[i][4] == "ConnectFail" || dm_array[i][4] == "SoketRecvFail" || dm_array[i][4] == "SSLFail" || dm_array[i][4] == "AccountFail" || dm_array[i][4] == "NotCompleted" || dm_array[i][4] == "Diskfull" || dm_array[i][4] == "Disconnected" || dm_array[i][4] == "Timeout") {
				$j("#status" + j).html("");
				$j("#status_icon" + j).attr("src", "images/statusIcon/" + "warning.gif");  //chang status_icon 
			}
			else if (dm_array[i][4] == "Hashing") {
				$j("#status" + j).html(dm_array[i][4]);
				$j("#status_icon" + j).attr("src", "images/statusIcon/" + "Initialing.gif");  //chang status_icon 
			}
			else if (dm_array[i][4] == "Vfat4G") {
				$j("#status" + j).html("");
				$j("#status_icon" + j).attr("src", "images/statusIcon/" + "Vfat4G.gif");  //chang status_icon 
			}
			else if (dm_array[i][4] == "Move4GBError") {
				$j("#status" + j).html("");
				$j("#status_icon" + j).attr("src", "images/statusIcon/" + "Move4GBError.gif");  //chang status_icon 
			}
			else if (dm_array[i][4] == "MoveDiskFull") {
				$j("#status" + j).html("");
				$j("#status_icon" + j).attr("src", "images/statusIcon/" + "warning.gif");  //chang status_icon 
			}else {
				if (dm_array[i][4] == "notbegin") {
					$j("#status" + j).html("Waiting");
				} else {
					$j("#status" + j).html(dm_array[i][4]);   //change status
				}
				//$j("#status_icon" +j).removeAttr("src");
				$j("#status_icon" + j).attr("src", "images/statusIcon/" + dm_array[i][4] + ".gif");  //chang status_icon 
			}
			updateCtrlIcon();
		}
		if (navigator.appName.indexOf("Microsoft") >= 0 && (location.host.split(":")[0] == LAN_IP || location.host.split(":")[0] == LOCAL_DOMAIN)) {
			if (dm_array[i][4] == "Finished" || dm_array[i][4] == "Seeding") {
				if (downloaddir_error == "not_found") {
					$j("#open_icon" + j).attr("title", "NOT Exist!");
					//$j("#open_dir_link"+j).show();
				}
				else {
					$j("#open_icon" + j).attr("title", "file://" + LAN_IP + downloaddir);
					$j("#open_dir_link" + j).attr("href", "file://" + LAN_IP + downloaddir);
					//$j("#open_dir_link"+j).show();
				}
			}
			else {
				$j("#open_dir_link" + j).hide();
			}
		}
		//type
		if (document.getElementById("type" + j) && $j("#type" + j).html() != dm_array[i][5])
			$j("#type" + j).html(dm_array[i][5]);
		//download
		if (document.getElementById("download" + j) && $j("#download" + j).html() != dm_array[i][7])
			$j("#download" + j).html(dm_array[i][7]);
		//upload
		if (document.getElementById("upload" + j) && $j("#upload" + j).html() != dm_array[i][8])
			$j("#upload" + j).html(dm_array[i][8]);
		//peers
		if (document.getElementById("peers" + j) && $j("#peers" + j).html() != dm_array[i][9])
			$j("#peers" + j).html(dm_array[i][9]);
		//pgrogress
		if (document.getElementById("progress" + j) && $j("#progress" + j).html() != progressTxt)
			$j("#progress" + j).html(progressTxt);
		//size
		if (document.getElementById("size" + j) && $j("#size" + j).html() != dm_array[i][3])
			$j("#size" + j).html(dm_array[i][3]);
		//progressBar
		if (document.getElementById("pbid" + j))   //还需要添加判断条件--看Progress是否有变化
		{
			//var progress = 0;
			for (var x = 0; x < ProgressBar_array.length; x++) {
				if (ProgressBar_array[x].stateId == "pbid" + j)
					update_progress(ProgressBar_array[x], dm_array[i][2]);
			}
		}
		if (dm_array[i][7] != "")
			totaldownloadSpeed += parseFloat(dm_array[i][7]);
		if (dm_array[i][8] != "")
			totaluploadSpeed += parseFloat(dm_array[i][8]);
	}
	totaldownloadSpeed = parseInt(totaldownloadSpeed);
	totaluploadSpeed = parseInt(totaluploadSpeed);
	if (document.getElementById("total_downloadSpeed") && $j("#total_downloadSpeed").html() != totaldownloadSpeed)
		$j("#total_downloadSpeed").html(totaldownloadSpeed);
	if (document.getElementById("total_uploadSpeed") && $j("#total_uploadSpeed").html() != totaluploadSpeed)
		$j("#total_uploadSpeed").html(totaluploadSpeed);

}

function backstage_cancel() {
	cancel_array.length = 0;
	for (var i = 0; i < dm_array_tmp.length; i++) {
		var a = true;
		for (var j = 0; j < dm_array.length; j++) {
			if (dm_array_tmp[i][0] == dm_array[j][0]) {
				a = false;
				break;
			}
		}
		if (a) {
			cancel_array.push(dm_array_tmp[i]);
		}
	}
	if (cancel_array.length) {
		var task_code = '<div style="display:none;"></div>';
		for (var x = 0; x < cancel_array.length; x++) {
			if (document.getElementById(cancel_array[x][0])) {
				$j(task_code).replaceAll("#" + cancel_array[x][0]);
				if (cancel_array[x][0] == dm[0])
					dm.length = 0;
			}
			for (var m = 0; m < ProgressBar_array.length; m++) {
				if (cancel_array[x][0] == ProgressBar_array[m].stateId.substring(4))
					ProgressBar_array.splice(m, 1);
			}
		}
		dm_num = dm_array.length;
		$j("#transfers").html(dm_num);
		if (isLogShow)
			select_taskType();
		if (dm_num == 0) {
			$j("#taskLog_show").hide();
			isLogShow = false;
		}
		updateCtrlIcon();
		ajaxRequest = true;
	}
}

function showDMList(data) {
	eval("dm_array = [" + data + "]");
	if (dm_array_tmp.length) {
		backstage_cancel();
	}
	eval("dm_array_tmp = [" + data + "]")
	if (sorted_by == "All") { }
	else if (sorted_by == "Finished_Seeding") {
		var array = new Array();
		for (var i = 0; i < dm_array.length; i++) {
			if (dm_array[i][4] == "Finished" || dm_array[i][4] == "Seeding") {
				array.push(dm_array[i]);
			}
		}
		dm_array = array.concat();
		if (dm_array_tmp.length) {
			backstage_cancel();
		}
	} else {
		var array = new Array();
		for (var i = 0; i < dm_array.length; i++) {
			if (dm_array[i][4] == sorted_by) {
				array.push(dm_array[i]);
			}
		}
		dm_array = array.concat();
		if (dm_array_tmp.length) {
			backstage_cancel();
		}
	}
	if (dm_num == 0 || old_sorted_by2 != old_sorted_by) {
		create_task();
		dm_num = dm_array.length;
		update_task();
	} else {
		for (var i = 0; i < dm_array.length; i++) {
			if (dm[0] == dm_array[i][0]) {
				dm = dm_array[i];
				break;
			}
		}
		update_task();
	}
}

function initial_Refresh_time(data) {
	var initial_array = new Array();
	eval("initial_array=" + data);
	var httpurl = "http://" + location.hostname;
	var httpsurl= "https://" + location.hostname;
	
	if(httpTag)
		document.getElementById("helpAddress").href = "http://"+ location.host +"/downloadmaster/help.asp";
	else
		document.getElementById("helpAddress").href = "https://"+ location.host +"/downloadmaster/help.asp";
	if (document.getElementById("handToPhone"))
		document.getElementById("handToPhone").href = location.origin + "/downloadmaster/task_hand.asp";

	if(initial_array[12] == 1 && location.hostname != initial_array[10] && location.hostname != LOCAL_DOMAIN) {
		if(initial_array[36] == 2) {//both https and http
			if(location.protocol == "https:") {
				document.getElementById("homeAddress").href = httpsurl + ":" + initial_array[31] + "/APP_Installation.asp";
			} else {
				document.getElementById("homeAddress").href = httpurl + ":" + initial_array[11]+ "/APP_Installation.asp";
			}
		} else if(initial_array[36] == 1) {//only https
			document.getElementById("homeAddress").href = httpsurl + ":" + initial_array[31] + "/APP_Installation.asp";
		} else {//only http
			document.getElementById("homeAddress").href = httpurl + ":" + initial_array[11]+ "/APP_Installation.asp";
		}
	} else {//lan
		if(initial_array[36] == 2) {//both https and http
			if(location.protocol == "https:") {
				document.getElementById("homeAddress").href = httpsurl + ":" + initial_array[35] + "/APP_Installation.asp";
			} else {
				document.getElementById("homeAddress").href = httpurl + "/APP_Installation.asp";
			}
		} else if(initial_array[36] == 1) {//only https
			document.getElementById("homeAddress").href = httpsurl+ ":" + initial_array[35] + "/APP_Installation.asp";
		} else {//only http
			document.getElementById("homeAddress").href = httpurl + "/APP_Installation.asp";
		}
	} 

	if (initial_array.length)
		Refresh_time = parseInt(initial_array[7]) * 1000;
	setInterval("Ajax_Get_DM_Status();Update_dm_Log();", Refresh_time);
}

function UtilityFlag() {
	var flag;
	flag = location.search.split("flag=")[1];
	if (flag == 73)
		Ext.MessageBox.alert(multiLanguage_array[multi_INT][3], multiLanguage_array[multi_INT][75]);
	else
		Ext.MessageBox.alert(multiLanguage_array[multi_INT][3], multiLanguage_array[multi_INT][flag]);
}


function showTask() {
	Ajax_Get_DM_Status();
}
