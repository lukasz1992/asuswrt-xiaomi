if [ "$1" == "" ]; then
	brctl delif br0 ra1
	brctl addif br0 ra0
	brctl addif br0 apcli0
	ifconfig ra1 up
	brctl addbr br1
	brctl addif br1 ra1
	brctl addif br1 apcli1
	exit 0
elif [ "$8" == "" ]; then
  echo Usage:./rept_dbdc_test "<InfName> <SSID> <Authmode> <Encryp> <PSK> <WirelessMode> <AutoConn> <ReptEn>"
  echo "<InfName> apcli0/apcli1"
  echo "<SSID> MT7621_AP"
  echo "<Authmode> OPEN/WPA2PSK"
  echo "<Encryp> NONE/TKIP/AES"
  echo "<PSK> 12345678"
  echo "<WirelessMode> 1/2/3/4/5/6/7/8/9/10/11/12/13/14"
  echo "<AutoConn> 0/1"
  echo "<ReptEn> 0/1"
  exit 0
fi

echo "Input:"
echo InfName=$1
echo RootAP=$2
echo Security=$3/$4
echo Passphrases=$5
echo Wirelessmode=$6
echo ReptEn=$8
echo Autoconnect=$7
iwpriv ra0 set MACRepeaterEn=$8
ifconfig $1 up
iwpriv $1 set ApCliEnable=0
iwpriv $1 set ApCliSsid=$2
iwpriv $1 set ApCliWirelessMode=$6
iwpriv $1 set ApCliAuthMode=$3
iwpriv $1 set ApCliEncrypType=$4
iwpriv $1 set ApCliWPAPSK=$5
if [ "$7" == "1" ]; then
iwpriv $1 set ApCliAutoConnect=$7
else
iwpriv $1 set ApCliEnable=1
fi

