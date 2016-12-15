/*
 * Copyright (C) 2012 Samsung Electronics
 * Minkyu Kang <mk7.kang@samsung.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/watchdog.h>

#define PRESCALER_VAL 255

void wdt_stop(void)
{
	struct s5p_watchdog *wdt =
		(struct s5p_watchdog *)samsung_get_base_watchdog();
	unsigned int wtcon;

	wtcon = readl(&wdt->wtcon);
	wtcon &= ~(WTCON_EN | WTCON_INT | WTCON_RESET);

	writel(wtcon, &wdt->wtcon);
}

void wdt_start(unsigned int timeout)
{
	struct s5p_watchdog *wdt =
		(struct s5p_watchdog *)samsung_get_base_watchdog();
	unsigned int wtcon;

	wdt_stop();

	wtcon = readl(&wdt->wtcon);
	wtcon |= (WTCON_EN | WTCON_CLK(WTCON_CLK_128));
	wtcon &= ~WTCON_INT;
	wtcon |= WTCON_RESET;
	wtcon &= ~WTCON_PRESCALER_MASK;
	wtcon |= WTCON_PRESCALER(PRESCALER_VAL);

	writel(timeout, &wdt->wtdat);
	writel(timeout, &wdt->wtcnt);
	writel(wtcon, &wdt->wtcon);
}

static unsigned int wdt_is_started(void)
{
	struct s5p_watchdog *wdt =
		(struct s5p_watchdog *) samsung_get_base_watchdog();

	return !!(readl(&wdt->wtcon) & WTCON_EN);
}

static void wdt_kick(void)
{
	struct s5p_watchdog *wdt =
		(struct s5p_watchdog *) samsung_get_base_watchdog();
	unsigned int freq = CONFIG_SYS_CLK_FREQ / (PRESCALER_VAL + 1)
						/ (16 << WTCON_CLK_128);

	/* kick the dog by reloading wtcnt */
	writel((CONFIG_HW_WATCHDOG_TIMEOUT_MS * freq) / 1000, &wdt->wtdat);
	writel((CONFIG_HW_WATCHDOG_TIMEOUT_MS * freq) / 1000, &wdt->wtcnt);
}

#if defined(CONFIG_HW_WATCHDOG)
void hw_watchdog_reset(void)
{
	if (wdt_is_started())
		wdt_kick();
}

void hw_watchdog_init(void)
{
	wdt_start(CONFIG_HW_WATCHDOG_TIMEOUT_MS);
}
#endif
