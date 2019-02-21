#!/bin/sh

#Dump P6/7 egress count
switch reg r 1790
switch reg r 1794
switch reg r 1798
switch reg r 179c

switch reg r 1690
switch reg r 1694
switch reg r 1698
switch reg r 169c
# Dump free buffer
switch reg r 1fc0
# Dump CPU port
reg s b0100400
reg d 100
reg d 200
# Dump PDMA FSM
reg s b0100800
reg d 0
reg d 100
reg d 200
# Dump PPE
reg s b0100c00
reg d 0 
reg d 100
reg d 200
reg d 300

echo "sleep 2 secs"
sleep 2
echo "done"

reg d 300
reg s b0100400
reg d 0
reg s b0100500
reg d 0
reg s b0100600
reg d 0
# Dump ESW MIB
reg s b0110000
reg r 3fe0
reg r 2030
reg r 4000
reg r 402c
reg r 4030
reg r 2430
reg r 4400
reg r 442c
reg r 4430
reg r 4600
reg r 462c
reg r 4630
reg r 2630
reg r 4700
reg r 472c
reg r 4730
reg r 2730

#Leon
reg s b0000420
reg w 0 0
reg r 0
reg w 0 1000
reg r 0
reg w 0 2000
reg r 0
reg w 0 3000
reg r 0
reg w 0 4000
reg r 0
reg w 0 5000
reg r 0
reg w 0 6000
reg r 0
reg w 0 7000
reg r 0

cat /proc/mt7620/gmac
cat /proc/mt7620/tx_ring
cat /proc/mt7620/rx_ring
cat /proc/mt7620/esw_cnt
