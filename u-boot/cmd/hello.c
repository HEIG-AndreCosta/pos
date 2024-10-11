// SPDX-License-Identifier: GPL-2.0+
/*
 * André Costa 2024
 */

#include <common.h>
#include <stdint.h>
#include <command.h>
#include <timestamp.h>
#include <version.h>
#include <version_string.h>
#include <linux/compiler.h>
#ifdef CONFIG_SYS_COREBOOT
#include <asm/cb_sysinfo.h>
#endif

#define REG_ACCESS(address)	*(volatile uint32_t *)(address)
#define BASE_GIC_DISTR_ADDR	0x08000000
#define ICDDCR			(BASE_GIC_DISTR_ADDR + 0x0)
#define ICDISER_BASE		(BASE_GIC_DISTR_ADDR + 0x100)
#define AFFINITY_ADDR		(BASE_GIC_DISTR_ADDR + 0x850)
#define ICDISER(interrupt)	(ICDISER_BASE + interrupt * sizeof(uint32_t))
#define BASE_CPU_LOCAL_IF_ADDR	0x08010000
#define ICCICR			(BASE_CPU_LOCAL_IF_ADDR + 0)
#define ICCPMR			(BASE_CPU_LOCAL_IF_ADDR + 0x4)
#define ICCIAR			(BASE_CPU_LOCAL_IF_ADDR + 0xC)
#define ICCEOIR			(BASE_CPU_LOCAL_IF_ADDR + 0x10)
#define INTERRUPT_RAW_NUMBER	50

// L’IRQ est de type SPI ; par conséquent, un offset de 32 est rajouté au no. IRQ.
#define INTERRUPT_NUMBER_OFFSET 32
#define INTERRUPT_NUMBER	(INTERRUPT_RAW_NUMBER + INTERRUPT_NUMBER_OFFSET)
#define ICDISER_NO		(INTERRUPT_NUMBER / 32)
#define ICDISER_BIT_MASK	(1 << (INTERRUPT_NUMBER % 32))

#define BASE_VEXT_ADDR		0x20000000
#define VEXT_IRQ_CTRL_REG	(BASE_VEXT_ADDR + 0x18)
#define VEXT_LED_REG		(BASE_VEXT_ADDR + 0x3A)
#define VEXT_TIMER_REG		(BASE_VEXT_ADDR + 0x40)
#define VEXT_TIMER_ENABLE_MASK	(0x1)
#define VEXT_TIMER_CLEAR_MASK	(0x2)
#define VEXT_IRQ_CTRL_BTN_MASK	(0xE)
#define VEXT_IRQ_CTRL_BTN_SHIFT (0x1)

#define VEXT_IRQ_SOURCE_MASK	0x60
#define VEXT_IRQ_SOURCE_SHIFT	5
#define VEXT_IRQ_SOURCE_BTN	0
#define VEXT_IRQ_SOURCE_LBA	1
#define VEXT_IRQ_SOURCE_TIMER	VEXT_IRQ_SOURCE_LBA

uint8_t irq_source(uint8_t reg_state)
{
	return (reg_state & VEXT_IRQ_SOURCE_MASK) >> VEXT_IRQ_SOURCE_SHIFT;
}

const char *hello_str = "Hello My Cutie Pie!";

static int status = 0;
static int curr_led = -1;

void hello_irq_handler(void)
{
	uint8_t irq_id = REG_ACCESS(ICCIAR) & 0xFF;
	uint8_t irq_reg = REG_ACCESS(VEXT_IRQ_CTRL_REG);
	uint8_t irq_src = irq_source(irq_reg);

	printf("Source: %#x\n", irq_src);
	if (irq_src == VEXT_IRQ_SOURCE_TIMER) {
		status = !status;
		REG_ACCESS(VEXT_TIMER_REG) |=
			(VEXT_TIMER_ENABLE_MASK | VEXT_TIMER_CLEAR_MASK);
	} else if (irq_src == VEXT_IRQ_SOURCE_BTN) {
		curr_led = (irq_reg & VEXT_IRQ_CTRL_BTN_MASK) >>
			   VEXT_IRQ_CTRL_BTN_SHIFT;
		status = 1;
		REG_ACCESS(VEXT_IRQ_CTRL_REG) |= 0x1;
	}
	if (curr_led >= 0) {
		if (status) {
			REG_ACCESS(VEXT_LED_REG) = (1 << curr_led);
		} else {
			REG_ACCESS(VEXT_LED_REG) = 0;
		}
	}

	REG_ACCESS(ICCEOIR) = irq_id;
}
static int do_hello(struct cmd_tbl *cmdtp, int flag, int argc,
		    char *const argv[])
{
	// Setup prios
	REG_ACCESS(ICCPMR) |= 0xFF;
	// Activate distributor
	REG_ACCESS(ICDDCR) |= 0x1;
	// Activate cpu interface
	REG_ACCESS(ICCICR) |= 0x1;
	// Activate interruption #50
	REG_ACCESS(ICDISER(ICDISER_NO)) |= ICDISER_BIT_MASK;
	REG_ACCESS(AFFINITY_ADDR) = 0x01010101;

	local_irq_enable();

	REG_ACCESS(VEXT_IRQ_CTRL_REG) |= 0x80;
	REG_ACCESS(VEXT_TIMER_REG) |= VEXT_TIMER_ENABLE_MASK;
	printf("%s\n", hello_str);

	return 0;
}

U_BOOT_CMD(hello, 1, 1, do_hello, "Print a warm and welcome message", "");
