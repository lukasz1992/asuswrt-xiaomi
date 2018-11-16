echo InfName=$1
echo RootAP=$2
echo Security=$3/$4
echo Passphrases=$5

iwpriv ra0 set MACRepeaterEn=1
iwpriv rai0 set MACRepeaterEn=1

ifconfig $1 up
iwpriv $1 set ApCliEnable=0
iwpriv $1 set ApCliSsid=$2
iwpriv $1 set ApCliAuthMode=$3
iwpriv $1 set ApCliEncrypType=$4
iwpriv $1 set ApCliWPAPSK=$5
iwpriv $1 set ApCliSsid=$2
iwpriv $1 set ApCliEnable=1
iwpriv $1 set ApCliAutoConnect=1
#brctl addif br0 apcli0

#brctl delif br0 rai0
#brctl delif br0 apclii0
#
#brctl addbr br1
#ifconfig br1 0.0.0.0 netmask 255.255.255.0 up
#brctl addif br1 rai0
#brctl addif br1 apclii0
#
