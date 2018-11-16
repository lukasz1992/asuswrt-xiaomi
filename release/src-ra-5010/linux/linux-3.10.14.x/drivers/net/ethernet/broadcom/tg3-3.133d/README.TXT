                           Installation Notes
                         Broadcom tg3 Linux Driver
                              Version 3.133d
                                08/07/2013

                          Broadcom Corporation
                         5300 California Avenue
                        Irvine, California  92617

               Copyright (c) 2004 - 2013 Broadcom Corporation
                           All rights reserved


Table of Contents
=================

  Introduction
  Limitations
  Packaging
  Driver Build Options
  Installing Source RPM Package
  Building Driver From TAR File
  Driver Settings
  Driver Defaults
  Sensor Support
  Unloading and Removing Driver
  Driver Messages
  

Introduction
============

This file describes the tg3 Linux driver for the Broadcom NetXtreme
10/100/1000 Mbps PCI/PCI-X/PCI Express Ethernet Network Controllers.
The latest driver is in the latest 3.x Linux kernel. It can also be
downloaded from http://www.broadcom.com as a source package, but is
generally not necessary to do so if you are using the latest 3.x
upstream kernel from http://www.kernel.org or one of the latest
vendor kernels from Red Hat, SuSE, or others.

The tg3 driver from the Broadcom package is almost identical to the
tg3 driver in the latest 3.x upstream Linux kernel. It includes some
additional kernel compatible code to allow it to compile on older 2.6
and some 2.4 kernels. The version number is also similar but generally
has a one letter suffix at the end, (e.g. 3.55b) to distinguish it from
the in-kernel tg3 driver.

The next few sections on packaging, compiling, and installation apply
mostly to the Broadcom driver package only.


Limitations
===========

The current version of the driver has been tested on 2.4.x kernels starting
from 2.4.24 and all 2.6.x/3.x kernels. The driver may not compile on kernels
older than 2.4.24. Testing is concentrated on i386 and x86_64 architectures.
Only limited testing has been done on some other architectures such as
powerpc and sparc64.

Minor changes to some source files and Makefile may be needed on some
kernels.

Forcing 1 Gigabit speed in a copper environment is not supported and may
result in connection issues with certain switches.


SLES 11 Considerations
======================

Starting with SLES 11, all modules not compiled by Novell will refuse to load
through modprobe by default.  Externally compiled modules can be loaded if the
"allow_unsupported_modules" flag is toggled in
/etc/modprobe.d/unsupported-modules, however this will render your kernel and
whole system unsupportable through Novell.  Please refer to the SLES 11 release
notes for more details.


Packaging
=========

To replace an older previously installed or in-kernel tg3 driver, follow
the instructions below.

The driver package from http://www.broadcom.com is released in two packaging
formats: source RPM and compressed tar formats. The file names for the two
packages are tg3-<version>.src.rpm and tg3-<version>.tar.gz respectively.
Identical source files to build the driver are included in both packages.


Driver Build Options
====================

This version of the tg3 driver contains support for Energy Efficient Ethernet.
The feature is on by default but can be disabled at compile time by adding
"TG3_EXTRA_DEFS=TG3_NO_EEE" to the make command line.


Installing Source RPM Package
=============================

The following are general guidelines for installing the driver.

1. Install the source RPM package:

   rpm -ivh tg3-<version>.src.rpm

2. CD to the RPM path and build the binary driver for your kernel:

   cd /usr/src/{redhat,OpenLinux,turbo,packages,rpm ..}

   rpm -bb SPECS/tg3.spec

or

   rpmbuild -bb SPECS/tg3.spec (for RPM version 4.x.x)

Note that the RPM path is different for different Linux distributions.

The driver will be compiled for the running kernel by default. To build
the driver for a kernel different than the running one, specify the
kernel by defining it in KVER:

   rpmbuild -bb SPECS/tg3.spec --define "KVER <kernel version>"

where <kernel version> in the form of 2.x.y-z is the version of another
kernel that is installed on the system.

3. Install the newly built package (driver and man page):

   rpm -ivh RPMS/<arch>/tg3-<version>.<arch>.rpm

<arch> is the architecture of the machine, e.g. i386:

   rpm -ivh RPMS/i386/tg3-<version>.i386.rpm

Note that the --force option may be needed on some Linux distributions
if conflicts are reported.

The driver will be installed in the following path:

2.4.x kernels:

    /lib/modules/<kernel_version>/kernel/drivers/net/tg3.o

2.6.x/3.x kernels:

    /lib/modules/<kernel_version>/kernel/drivers/net/tg3.ko

4. Load the driver:

   insmod tg3.o
or
   insmod tg3.ko (on 2.6.x/3.x kernels)
or
   modprobe tg3

5. To configure network protocol and address, refer to various Linux
documentations.


Building Driver From TAR File
=============================

The following are general guidelines for installing the driver.

1. Create a directory and extract the files:

   tar xvzf tg3-<version>.tar.gz

2. Build the driver tg3.o (or tg3.ko) as a loadable module for the
running kernel:

   cd src
   make

The driver will be compiled for the running kernel by default. To build
the driver for a kernel different than the running one, specify the
kernel by defining it in KVER:

  make KVER=<kernel version>

where <kernel version> in the form of 2.x.y-z is the version of another
kernel that is installed on the system.

3. Test the driver by loading it: 

   insmod tg3.o
or
   insmod tg3.ko (on 2.6.x/3.x kernels)
or
   insmod tg3

4. Install the driver:

   make install

See RPM instructions above for the location of the installed driver.

5. To configure network protocol and address, refer to various Linux
documentations.


Driver Settings
===============

This version of the tg3 driver supports one module parameter that is
not available in the in-kernel driver.  The parameter is named tg3_disable_eee.
To disable Energy Efficient Ethernet (EEE) support, set this module parameter
to 1.

The rest of the sections below apply to both the in-kernel tg3
driver and the tg3 driver package from Broadcom.

Driver settings can be queried and changed using ethtool. The latest ethtool
can be downloaded from ftp://ftp.kernel.org/pub/software/network/ethtool if
it is not already installed. The following are some common examples on how to
use ethtool. See the ethtool man page for more information. ethtool settings
do not persist across reboot or module reload. The ethtool commands can be put
in a startup script such as /etc/rc.local to preserve the settings across a
reboot. On Red Hat distributions, "ethtool -s" parameters can be specified
in the ifcfg-ethx scripts using the ETHTOOL_OPTS keyword. The specified
ethtool parameters will be set during ifup. Example:
/etc/sysconfig/network-scripts/ifcfg-eth0:

ETHTOOL_OPTS="wol g speed 100 duplex half autoneg off"

Some ethtool examples:

1. Show current speed, duplex, and link status:

   ethtool eth0

2. Change speed, duplex, autoneg:

Example: 100Mbps half duplex, no autonegotiation:

   ethtool -s eth0 speed 100 duplex half autoneg off

Example: Autonegotiation with full advertisement:

   ethtool -s eth0 autoneg on

Example: Autonegotiation with 100Mbps full duplex advertisement only:

   ethtool -s eth0 speed 100 duplex full autoneg on

3. Show flow control settings:

   ethtool -a eth0

4. Change flow control settings:

Example: Turn off flow control

   ethtool -A eth0 autoneg off rx off tx off

Example: Turn flow control autonegotiation on with tx and rx advertisement:

   ethtool -A eth0 autoneg on rx on tx on

   Note that this is only valid if speed is set to autonegotiation.

5. Show offload settings:

   ethtool -k eth0

6. Change offload settings:

Example: Turn off TSO (TCP segmentation offload)

   ethtool -K eth0 tso off

7. Get statistics:

   ethtool -S eth0

8. Perform self-test:

   ethtool -t eth0

   Note that the interface (eth0) must be up to do all tests.

9. Show number of channels (rings):

   ethtool -l eth0

   Note that this is only supported on newer MSI-X capable devices (e.g.
   57765, 57766, 5717, 5719, 5720) and available on 3.x kernels and 3.x
   ethtool only.

10. Set number of channels (rings):

   ethtool -L eth0 rx N tx N

   rx and tx channel numbers can be different.  0 means use default.  The
   driver selects the best number of channels based on the number of CPU
   cores.

   Note that this is only supported on newer MSI-X capable devices (e.g.
   57765, 57766, 5717, 5719, 5720) and available on 3.x kernels and 3.x
   ethtool only.

11. See ethtool man page for more options.


Driver Defaults
===============

Speed :                    Autonegotiation with all speeds advertised

Flow control :             Autonegotiation with rx and tx advertised

MTU :                      1500 (range 46 - 9000)

                           Some chips do not support jumbo MTUs bigger than
                           1500

Rx Ring Size :              200 (range 0 - 511)

                            Some chips are fixed at 64

Rx Jumbo Ring Size :        100 (range 0 - 255)

                            Not all chips support the jumbo ring, and some
                            chips that support jumbo frames do not use the
                            jumbo ring.

Tx Ring Size :              511 (range (MAX_SKB_FRAGS+1) - 511)

                            MAX_SKB_FRAGS varies on different kernels and
                            different architectures. On a 2.6/3.x kernel for
                            x86, MAX_SKB_FRAGS is 18.

Number of RSS channels:    Varies depending on the number of CPUs on MSI-X
                           capable devices.  (Range 1 - 4)

Number of TSS channels:    Varies depending on the number of CPUs on MSI-X
                           capable devices.  (Range 1 - 4)

Coalesce rx usecs :          20 (range 0 - 1023)

Coalesce rx usecs irq :      20 (range 0 - 255)
	
Coalesce rx frames :          5 (range 0 - 1023)

Coalesce rx frames irq :      5 (range 0 - 255)

Coalesce tx usecs :          72 (range 0 - 1023)

Coalesce tx usecs irq :      20 (range 0 - 255)

Coalesce tx frames :         53 (range 0 - 1023)

Coalesce tx frames irq :     5 (range 0 - 255)

Coalesce stats usecs   : 1000000 (aprox. 1 sec.)

                             Some coalescing parameters are not used or have
                             different defaults on some chips

MSI/MSI-X :                Enabled (if supported by the chip and passed
                                    the interrupt test)

TSO :                      Enabled on newer chips that support TCP segmentation
                           offload in hardware

WoL :                      Disabled


Sensor Support
==============

If your device and platform support sensors and you're running a kernel
that supports CONFIG_HWMON, you can read the sensor data from:

/sys/class/hwmon/hwmon#/device/

Select the hwmon# device that matches the tg3 device.  Only temp1 sensor
attributes are supported at this time.


Unloading and Removing Driver
=============================

To unload the driver, use ifconfig to bring down all eth# interfaces opened
by the driver, then do the following:

rmmod tg3

Note that on 2.6/3.x kernels, it is not necessary to bring down the eth#
interfaces before unloading the driver module.


If the driver was installed using rpm, do the following to remove it:

rpm -e tg3


If the driver was installed using make install from the tar file, the driver
tg3.o (or tg3.ko) has to be manually deleted from the system. Refer
to the section "Installing Source RPM Package" for the location of the
installed driver.


Driver Messages
===============

The following are the most common sample messages that may be logged in the file
/var/log/messages. Use dmesg -n <level> to control the level at which messages
will appear on the console. Most systems are set to level 6 by default. To see
all messages, set the level higher.

Driver signon:
-------------

tg3.c:v3.123e (April 30, 2012)


NIC detected:
------------

eth0: Tigon3 [partno(BCM95704A6) rev 2003] (PCIX:100MHz:64-bit) MAC address 00:10:18:04:3f:36
eth0: attached PHY is 5704 (10/100/1000Base-T Ethernet) (WireSpeed[1])
eth0: RXcsums[1] LinkChgREG[0] MIirq[0] ASF[0] TSOcap[1]
eth0: dma_rwctrl[769f4000] dma_mask[64-bit]


Link up and speed indication:
----------------------------

tg3: eth0: Link is up at 1000 Mbps, full duplex.
tg3: eth0: Flow control is on for TX and on for RX.
 

Link down indication:
--------------------

tg3: eth0: Link is down.



