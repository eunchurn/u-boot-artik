/*
 * Copyright (C) 2016 Samsung Electronics
 * Heesub Shin <heesub.shin@samsung.com>
 *
 * Configuation settings for the ARTIK-051 (EXYNOS T200) board.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __CONFIG_ARTIK_051_H
#define __CONFIG_ARTIK_051_H

#include <configs/exynos0200-common.h>

#undef CONFIG_CMD_PXE

#define CONFIG_SYS_TEXT_BASE		0x04004020
#define CONFIG_SYS_INIT_SP_ADDR		0x02150000

#define CONFIG_MISC_INIT_R

#ifdef CONFIG_SKIP_LOWLEVEL_INIT
#undef CONFIG_SKIP_LOWLEVEL_INIT
#endif
#define CONFIG_SKIP_LOWLEVEL_INIT_ONLY

#define CONFIG_SYS_DCACHE_OFF

#define CONFIG_MTD_DEVICE
#define CONFIG_SYS_MAX_FLASH_SECT	2048
#define CONFIG_SYS_FLASH_EMPTY_INFO

#define CONFIG_SECURE_BOOT

/* remove CONFIG_ENV_IS_IN_FLASH before production release */
#define CONFIG_ENV_IS_IN_FLASH

#ifndef CONFIG_ENV_IS_IN_FLASH
#define CONFIG_ENV_IS_NOWHERE
#endif

#define CONFIG_ENV_OFFSET		(SZ_1K * 252)
#define CONFIG_ENV_SIZE			(SZ_1K * 4)
#define CONFIG_ENV_SECT_SIZE		0x1000

#define CONFIG_SYS_SRAM_BASE		0x02023800
#define CONFIG_SYS_SRAM_SIZE		(722 * 1024)

#define CONFIG_SYS_SDRAM_BASE		CONFIG_SYS_SRAM_BASE

#define CONFIG_SYS_FLASH_BASE		0x04000000
#define CONFIG_SYS_MAX_FLASH_BANKS	1

#define SDRAM_BANK_SIZE			0

#define CONFIG_SYS_LOAD_ADDR		CONFIG_SYS_SRAM_BASE

#define CONFIG_HW_WATCHDOG
#define CONFIG_HW_WATCHDOG_TIMEOUT_MS	10000

#define CONFIG_BOOTCOMMAND	"\n"					\
	"    if env exists do_rescue; then\n"				\
	"        c=0;\n"						\
	"        r=1;\n"						\
	"        while test ${c} -lt 10 && test ${r} -eq 1; do\n"	\
	"            run do_checkrescue;\n"				\
	"            sleep 0.1;\n"					\
	"            setexpr c ${c} + 1;\n"				\
	"        done\n"						\
	"        if test ${r} -eq 1; then\n"				\
	"            run do_rescue;\n"					\
	"        fi\n"							\
	"    fi\n"							\
	"    run do_boot;"

#define CONFIG_EXTRA_ENV_SETTINGS					\
	"parta=0x04040000\0"						\
	"partb=0x04320000\0"						\
	"bootoffset=0x20\0"						\
	"dateoffset=0x1ffffc\0"						\
	"partsize=0x2e0000\0"						\
	"do_boot=\n"							\
	"    run do_fwupdate;\n"					\
	"    setexpr bootaddr ${parta} + ${bootoffset}\n"		\
	"    echo \"## Verifying image at ${parta} ...\"\n"		\
	"    sss_auth_img ${parta} && go ${bootaddr}\n"			\
	"    echo \"## Trying to recover ...\"\n"			\
	"    if sss_auth_img ${partb}; then\n"				\
	"        erase ${parta} +${partsize}\n"				\
	"        cp.b ${partb} ${parta} ${partsize}\n"			\
	"    fi\n"							\
	"    reset\0"							\
	"do_fwupdate=\n"						\
	"    setexpr parta_date ${parta} + ${dateoffset}\n"		\
	"    setexpr partb_date ${partb} + ${dateoffset}\n"		\
	"    if itest *${parta_date} -lt *${partb_date}; then\n"	\
	"        if sss_auth_img ${partb}; then\n"			\
	"            echo \"## Updating firmware ...\"\n"		\
	"            erase ${parta} +${partsize}\n"			\
	"            cp.b ${partb} ${parta} ${partsize}\n" 		\
	"        fi\n"							\
	"    fi\0"

#endif	/* __CONFIG_ARTIK_051_H */
