################################################################################
#
# NFC Linux driver
# Copyright(c) 1999 - 2006 Intel Corporation.
#
# This program is free software; you can redistribute it and/or modify it
# under the terms and conditions of the GNU General Public License,
# version 2, as published by the Free Software Foundation.
#
# This program is distributed in the hope it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
# more details.
#
# You should have received a copy of the GNU General Public License along with
# this program; if not, write to the Free Software Foundation, Inc.,
# 51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.
#
# The full GNU General Public License is included in this distribution in
# the file called "COPYING".
#
# Contact Information:
# 
# 
# 
#
################################################################################

#
# Makefile for the Mediatek NFC driver
#

#obj-$(CONFIG_MT6605_NFC) += nfc_drv.o

#nfc-objs := nfc_drv.o
#include $(srctree)/drivers/misc/mediatek/Makefile.custom
#ccflags-$(CONFIG_NFC_MT6605) +=  -DDEBUG
obj-$(CONFIG_MT6605_NFC)   += mt6605.o
nfc-objs := mt6605.o