#!/bin/sh
SECS=1262278080

if [ ! -d "/opt/etc/asus_lighttpd" ]; then
	mkdir -p /opt/etc/asus_lighttpd
fi
cd /opt/etc/asus_lighttpd

if [ -f "/userfs/bin/tcapi" ];then
	NVCN=`/userfs/bin/tcapi get Aicloud_Entry https_crt_cn`
       if [ "$NVCN" == "no attribute information" ]; then
	   NVCN=`/userfs/bin/tcapi get Lan_Entry IP`
       fi

else
	NVCN=`nvram get https_crt_cn`
       if [ "$NVCN" == "" ]; then
	   NVCN=`nvram get lan_ipaddr`
       fi

fi
cp -f /opt/etc/asus_conf.d/openssl.cnf /opt/etc/asus_lighttpd/openssl.config

I=0
for CN in $NVCN; do
        echo "$I.commonName=CN" >> openssl.config
        echo "$I.commonName_value=$CN" >> openssl.config
        I=$(($I + 1))
done



# create the key and certificate request
OPENSSL_CONF=/opt/etc/asus_lighttpd/openssl.config /opt/bin/openssl req -new -out /tmp/cert.csr -keyout /tmp/privkey.pem -newkey rsa:1024 -passout pass:password
# remove the passphrase from the key
OPENSSL_CONF=/opt/etc/asus_lighttpd/openssl.cnf /opt/bin/openssl rsa -in /tmp/privkey.pem -out key.pem -passin pass:password
# convert the certificate request into a signed certificate
OPENSSL_CONF=/opt/etc/asus_lighttpd/openssl.cnf RANDFILE=/dev/urandom /opt/bin/openssl x509 -in /tmp/cert.csr -out cert.pem -req -signkey key.pem -days 3653

#	openssl x509 -in /etc/cert.pem -text -noout

cat key.pem cert.pem > server.pem

rm -f /tmp/cert.csr /tmp/privkey.pem openssl.config

#openssl req -new -nodes -x509 -keyout server.pem -out server.pem -days 365

