/*
 * Copyright (C) 2016-2018 Daniel Rossier <daniel.rossier@soo.tech>
 * Copyright (C) 2019 Baptiste Delporte <bonel@bonel.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */
#include <linux/types.h>
#include <linux/printk.h>
#include <linux/string.h>

#include <avz.h>

extern int vsnprintf(char *buf, size_t size, const char *fmt, va_list args);

#ifdef CONFIG_LINUXVIRT
void (*__printch)(char c);
#else
void __printch(char c) {
	pr_cont("%c", c);
}
#endif

void __lprintk(const char *format, va_list va) {
	char buf[128];
	char *__start;
	int i;

	vsnprintf(buf, 128, format, va);

	__start = buf;

	/* Skip printk prefix issued by the standard Linux printk if any */
	if ((*__start != 0) && (*__start < 10))
		__start += 2;

	for (i = 0; i < strlen(__start); i++)
            
#ifdef CONFIG_LINUXVIRT
		if (likely(__printch))
			__printch(__start[i]);
#else
#ifdef CONFIG_ARM64
		avz_hypercall(__HYPERVISOR_console_io, __start[i], 0, 0, 0);
#else
		/* Other cases */
		__printch(__start[i]);
#endif /* CONFIG_ARM64 */
#endif /* !CONFIG_LINUXVIRT */

}

void lprintch(char c) {
#ifdef CONFIG_LINUXVIRT
	if (likely(__printch))
#endif
		__printch(c);
}

void lprintk(char *format, ...) {

	va_list va;
	va_start(va, format);

	__lprintk(format, va);

	va_end(va);
}
 
/**
 * Manage the character received from the UART console.
 * According to the focus state, the character is forwarded to
 * Linux *or* to the AVZ hypervisor.
 */
/*
 * avz_switch_console() - Allow the user to give input focus to various input sources (agency, MEs, avz)
 *
 * Warning !! We are in interrupt context top half when the function is called and a lock is pending
 * on the UART. Use of printk() is forbidden and we need to use lprintk() to avoid deadlock.
 *
 */
int avz_switch_console(char ch)
{
#ifdef CONFIG_LINUXVIRT

	static int switch_code_count = 0;

	static char *input_str[2] = { "Agency domain", "Agency AVZ Hypervisor" };
	static bool focus_on_avz = false;

	/* Check for Ctrl/a */
	if (ch == 1) {

		/* We eat CTRL-<switch_char> in groups of 2 to switch console input. */
		if (++switch_code_count == 1) {

			switch_code_count = 0;
			focus_on_avz = !focus_on_avz;

			lprintk("*** Serial input -> %s (type 'CTRL-%c' twice to switch input to %s).\n", input_str[(focus_on_avz ? 1 : 0)], 'a', input_str[(focus_on_avz ? 0 : 1)]);

			return 1;
		}
	}

	if (focus_on_avz) {

		hypercall_trampoline(__HYPERVISOR_console_io, CONSOLEIO_process_char, 1, (long) &ch, 0);

		return 1;

	} else

#endif /* CONFIG_LINUXVIRT */

		return 0;
}

