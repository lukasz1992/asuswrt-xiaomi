/**************************************************************************
 *
 *  BRIEF MODULE DESCRIPTION
 *     board setup for Ralink RT2880 solution
 *
 *  Copyright 2007 Ralink Inc. (bruce_chang@ralinktech.com.tw)
 *
 *  This program is free software; you can redistribute  it and/or modify it
 *  under  the terms of  the GNU General  Public License as published by the
 *  Free Software Foundation;  either version 2 of the  License, or (at your
 *  option) any later version.
 *
 *  THIS  SOFTWARE  IS PROVIDED   ``AS  IS'' AND   ANY  EXPRESS OR IMPLIED
 *  WARRANTIES,   INCLUDING, BUT NOT  LIMITED  TO, THE IMPLIED WARRANTIES OF
 *  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN
 *  NO  EVENT  SHALL   THE AUTHOR  BE    LIABLE FOR ANY   DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 *  NOT LIMITED   TO, PROCUREMENT OF  SUBSTITUTE GOODS  OR SERVICES; LOSS OF
 *  USE, DATA,  OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 *  ANY THEORY OF LIABILITY, WHETHER IN  CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 *  THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *  You should have received a copy of the  GNU General Public License along
 *  with this program; if not, write  to the Free Software Foundation, Inc.,
 *  675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *
 **************************************************************************
 * May 2007 Bruce Chang
 *
 * Initial Release
 *
 *
 *
 **************************************************************************
 */


#include <linux/init.h>
#include <linux/sched.h>
#include <linux/mc146818rtc.h>
#include <linux/ioport.h>

#include <asm/cpu.h>
#include <asm/bootinfo.h>
#include <asm/irq.h>
#include <asm/mach-ralink/generic.h>
#include <asm/mach-ralink/prom.h>
#include <asm/mach-ralink/surfboardint.h>
#include <asm/time.h>
#include <asm/traps.h>
#include <asm/gcmpregs.h>  

#if defined(CONFIG_SERIAL_CONSOLE) || defined(CONFIG_PROM_CONSOLE)
extern void console_setup(char *, int *);
char serial_console[20];
#endif

#ifdef CONFIG_KGDB
extern void rs_kgdb_hook(int);
extern void saa9730_kgdb_hook(void);
extern void breakpoint(void);
int remote_debug = 0;
#endif

//extern struct rtc_ops no_rtc_ops;

extern void mips_reboot_setup(void);

const char *get_system_type(void)
{
#if defined (CONFIG_RALINK_RT2880)
	return "RT2880";
#elif defined (CONFIG_RALINK_RT3052)
	return "RT3052";
#elif defined (CONFIG_RALINK_RT3352)
	return "RT3352";
#elif defined (CONFIG_RALINK_RT5350)
	return "RT5350";
#elif defined (CONFIG_RALINK_RT3883)
	return "RT3883";
#elif defined (CONFIG_RALINK_RT6855)
	return "RT6855";
#elif defined (CONFIG_RALINK_MT7620)
	return "MT7620";
#elif defined (CONFIG_RALINK_MT7621)
	return "MT7621";
#elif defined (CONFIG_RALINK_MT7628)
	return "MT7628";
#else
#error	"unknown chipset"
#endif
}

extern void mips_time_init(void);
extern void mips_timer_setup(struct irqaction *irq);

extern int coherentio;    /* no DMA cache coherency (may be set by user) */
static int __init setcoherentio(char *str)
{
        if (coherentio < 0)
                pr_info("Command line checking done before"
                                " plat_setup_iocoherency!!\n");
        if (coherentio == 0)
                pr_info("Command line enabling coherentio"
                                " (this will break...)!!\n");

        coherentio = 1;
        pr_info("Hardware DMA cache coherency (command line)\n");
        return 1;
}
__setup("coherentio", setcoherentio);

static int __init setnocoherentio(char *str)
{
        if (coherentio < 0)
                pr_info("Command line checking done before"
                                " plat_setup_iocoherency!!\n");
        if (coherentio == 1)
                pr_info("Command line disabling coherentio\n");

        coherentio = 0;
        pr_info("Software DMA cache coherency (command line)\n");
        return 1;
}
__setup("nocoherentio", setnocoherentio);

static int __init
plat_enable_iocoherency(void)
{
#ifdef CONFIG_MIPS_IOCU
        int supported = 0;
        if (gcmp_niocu() != 0) {
                /* Nothing special needs to be done to enable coherency */
                printk("CMP IOCU detected\n");
                supported = 1;
        }
        hw_coherentio = supported;
        return supported;
#else
	return 0;
#endif
}

static void __init
plat_setup_iocoherency(void)
{
#ifdef CONFIG_DMA_NONCOHERENT
        /*
         * Kernel has been configured with software coherency
         * but we might choose to turn it off
         */
        if (plat_enable_iocoherency()) {
                if (coherentio == 0)
                        pr_info("Hardware DMA cache coherency supported"
                                        " but disabled from command line\n");
                else {
                        coherentio = 1;
                        printk(KERN_INFO "Hardware DMA cache coherency\n");
                }
        } else {
                if (coherentio == 1)
                        pr_info("Hardware DMA cache coherency not supported"
                                " but enabled from command line\n");
                else {
                        coherentio = 0;
                        pr_info("Software DMA cache coherency\n");
                }
        }
#else
        if (!plat_enable_iocoherency())
                panic("Hardware DMA cache coherency not supported");
#endif
}

void __init rt2880_setup(void)
{
#ifdef CONFIG_KGDB
	int rs_putDebugChar(char);
	char rs_getDebugChar(void);
	int saa9730_putDebugChar(char);
	char saa9730_getDebugChar(void);
	extern int (*generic_putDebugChar)(char);
	extern char (*generic_getDebugChar)(void);
#endif
	char *argptr;

	iomem_resource.start = 0;
	iomem_resource.end= ~0;
	ioport_resource.start= 0;
	ioport_resource.end = ~0;
#ifdef CONFIG_SERIAL_CONSOLE
	argptr = prom_getcmdline();
	if ((argptr = strstr(argptr, "console=ttyS")) == NULL) {
		int i = 0;
		char *s = prom_getenv("modetty0");
		while(s[i] >= '0' && s[i] <= '9')
			i++;
		strcpy(serial_console, "ttyS0,");
		strncpy(serial_console + 6, s, i);
		prom_printf("Config serial console: %s\n", serial_console);
		console_setup(serial_console, NULL);
	}
#endif

#ifdef CONFIG_KGDB
	argptr = prom_getcmdline();
	if ((argptr = strstr(argptr, "kgdb=ttyS")) != NULL) {
		int line;
		argptr += strlen("kgdb=ttyS");
		if (*argptr != '0' && *argptr != '1')
			printk("KGDB: Uknown serial line /dev/ttyS%c, "
			       "falling back to /dev/ttyS1\n", *argptr);
		line = *argptr == '0' ? 0 : 1;
		printk("KGDB: Using serial line /dev/ttyS%d for session\n",
		       line ? 1 : 0);

		if(line == 0) {
			rs_kgdb_hook(line);
			generic_putDebugChar = rs_putDebugChar;
			generic_getDebugChar = rs_getDebugChar;
		} else {
			saa9730_kgdb_hook();
			generic_putDebugChar = saa9730_putDebugChar;
			generic_getDebugChar = saa9730_getDebugChar;
		}

		prom_printf("KGDB: Using serial line /dev/ttyS%d for session, "
			    "please connect your debugger\n", line ? 1 : 0);

		remote_debug = 1;
		/* Breakpoints and stuff are in surfboard_irq_setup() */
	}
#endif
	argptr = prom_getcmdline();

	if ((argptr = strstr(argptr, "nofpu")) != NULL)
		cpu_data[0].options &= ~MIPS_CPU_FPU;

	//rtc_ops = &no_rtc_ops;
	//board_time_init = mips_time_init;
	//board_timer_setup = mips_timer_setup;

	plat_setup_iocoherency();
	mips_reboot_setup();
}

void __init plat_mem_setup(void)
{
  rt2880_setup();
}
