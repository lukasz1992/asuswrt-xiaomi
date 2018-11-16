	Broadcom SoC models
	===================

		To install the tools:
		    - copy the tools/brcm/ directory to /opt
		    - add /opt/brcm/hndtools-mipsel-linux/bin to your path
		    - add /opt/brcm/hndtools-mipsel-uclibc/bin to your path

	Mediatek/Ralink SoC models
	==========================

		To install the tools:
	    	    - copy the tools/brcm/ directory to /opt
		    - add /opt/brcm/hndtools-mipsel-linux/bin to your path
		    - add /opt/brcm/hndtools-mipsel-uclibc/bin to your path
		    If it is MT7621 or MT7628 chip:
	    	    - extract tools/buildroot-gcc463_32bits.tar.bz2 to /opt
		    - add /opt/buildroot-gcc463/bin to your path
		    otherwise :
	    	    - extract tools/buildroot-gcc342.tar.bz2 to /opt
		    - add /opt/buildroot-gcc342/bin to your path

		For MT7621 Uboot:
	    	    - extract mips-2012.03.tar.bz2 directory to /opt
		    - add /opt/mips-2012.03/bin to your uboot path

	Qualcomm QCA9557/QCA953x/QCA956x/ MIPS SoC models
	===================

		To install the tools:
		    Mesh Router:
		    - extract openwrt-gcc463.mips.mesh.tar.bz2 directory to /opt
		    - add /opt/openwrt-gcc463.mips.mesh/bin to your path
		    - If you want to build small utilities out of asuswrt box,
		      add STAGING_DIR environment variable as below:

		      export STAGING_DIR=/opt/openwrt-gcc463.mips.mesh

		    Others: (For example, RT-AC55U, 4G-AC55U.)
		    - extract openwrt-gcc463.mips.tar.bz2 directory to /opt
		    - add /opt/openwrt-gcc463.mips/bin to your path
		    - If you want to build small utilities out of asuswrt box,
		      add STAGING_DIR environment variable as below:

		      export STAGING_DIR=/opt/openwrt-gcc463.mips

	Qualcomm IPQ806x/IPQ40xx ARM SoC models
	===============================

		For example, BRT-AC828.

		To install the tools:
		    - extract tools/openwrt-gcc463.arm.tar.bz2 directory to /opt
		    - add /opt/openwrt-gcc463.arm/bin to your path
		    - If you want to build small utilities out of asuswrt box,
		      add STAGING_DIR environment variable as below:

		      export STAGING_DIR=/opt/openwrt-gcc463.arm

	Note: Broadcom/Ralink(except 4708 series) platform use the same toolchain for user space program, so please set PATH to the same directory as above
