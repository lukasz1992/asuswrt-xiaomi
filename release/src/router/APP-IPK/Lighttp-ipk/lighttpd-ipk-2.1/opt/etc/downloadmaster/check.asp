<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<html xmlns:v>
<head>
<meta http-equiv="X-UA-Compatible" content="IE=EmulateIE8" />
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
<meta http-equiv="Expires" content="-1" />
<meta HTTP-EQUIV="Cache-Control" CONTENT="no-cache">
<meta http-equiv="Pragma" content="no-cache" />
<title>Login Jump</title>
<script type="text/javascript" src="jquery.js"></script>
</head>
<body>
<script>
var httpTag 		= 'https:' == document.location.protocol ? false : true;
var directurl_tmp 	= location.href.split("&directurl=")[1];
var directurl 		= directurl_tmp.split("&login_username=")[0];
var url 			= decodeURIComponent(location.host + directurl);

if(httpTag)
	self.location = "http://"	+ url;
else
	self.location = "https://"	+ url;

</script>
</body>
</html>
