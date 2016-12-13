#include <common.h>
#include <asm/arch/cpu.h>
#include <asm/arch/pinmux.h>
#include <dm/pinctrl.h>

DECLARE_GLOBAL_DATA_PTR;

#ifdef CONFIG_BOARD_EARLY_INIT_F
int board_early_init_f(void)
{
	/* GPIO for debug UART */
	if (exynos_pinmux_config(PERIPH_ID_UART4, PINMUX_FLAG_NONE))
		debug("UART4 not configured.\n");

	/* SRAM size */
	gd->ram_size = CONFIG_SYS_SRAM_SIZE;

	return 0;
}
#endif /* CONFIG_BOARD_EARLY_INIT_F */

int dram_init(void)
{
	return 0;
}

int board_init(void)
{
	return 0;
}

#ifdef CONFIG_MISC_INIT_R
int misc_init_r(void)
{
	return 0;
}
#endif /* CONFIG_MISC_INIT_R */

void relocate_vectors(void)
{
}
