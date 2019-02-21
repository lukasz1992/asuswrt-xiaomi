#!/bin/sh
SECS=1262278080

if [ ! -d "/etc/asus_lighttpd" ]; then
	mkdir -p /etc/asus_lighttpd
fi
cd /etc/asus_lighttpd

NVCN=`nvram get https_crt_cn`
if [ "$NVCN" == "" ]; then
	NVCN=`nvram get lan_ipaddr`
fi
cp -L /opt/etc/asus_conf.d/openssl.cnf /etc/asus_lighttpd/openssl.config

I=0
for CN in $NVCN; do
        echo "$I.commonName=CN" >> openssl.config
        echo "$I.commonName_value=$CN" >> openssl.config
        I=$(($I + 1))
done


#export LD_LIBRARY_PATH=".:/usr/local/lib:/usr/lib:/opt/lib:/lib:/lib/gstreamer-0.10:/opt/usr/lib"
#export LD_LIBRARY_PATH="/opt/lib:$LD_LIBRARY_PATH"
# create the key and certificate request
OPENSSL_CONF=/etc/asus_lighttpd/openssl.config /opt/bin/openssl req -new -out /tmp/cert.csr -keyout /tmp/privkey.pem -newkey rsa:1024 -passout pass:password
# remove the passphrase from the key
OPENSSL_CONF=/etc/asus_lighttpd/openssl.cnf /opt/bin/openssl rsa -in /tmp/privkey.pem -out key.pem -passin pass:password
# convert the certificate request into a signed certificate
OPENSSL_CONF=/etc/asus_lighttpd/openssl.cnf RANDFILE=/dev/urandom /opt/bin/openssl x509 -in /tmp/cert.csr -out cert.pem -req -signkey key.pem -days 3653

#	openssl x509 -in /etc/cert.pem -text -noout

cat key.pem cert.pem > server.pem

rm -f /tmp/cert.csr /tmp/privkey.pem openssl.config

#openssl req -new -nodes -x509 -keyout server.pem -out server.pem -days 365
#export LD_LIBRARY_PATH=".:/usr/local/lib:/usr/lib:/lib:/lib/gstreamer-0.10:/opt/lib:/opt/usr/lib"

