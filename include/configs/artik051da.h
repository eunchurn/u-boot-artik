/*
 * Copyright (C) 2016 Samsung Electronics
 * Heesub Shin <heesub.shin@samsung.com>
 *
 * Configuation settings for the ARTIK-051 (EXYNOS T200) board.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __CONFIG_ARTIK051DA_H
#define __CONFIG_ARTIK051DA_H

#include <configs/artik051.h>

#define CONFIG_BOOTCOMMAND	"\n"					\
	"    c=0;\n"							\
	"    r=1;\n"							\
	"    while test ${c} -lt 10 && test ${r} -eq 1; do\n"		\
	"        run do_checkrescue;\n"					\
	"        sleep 0.1;\n"						\
	"        setexpr c ${c} + 1;\n"					\
	"    done\n"							\
	"    if test ${r} -eq 1; then\n"				\
	"        run do_rescue;\n"					\
	"    fi\n"							\
	"    run do_boot;"

#define CONFIG_EXTRA_ENV_SETTINGS					\
	"do_rescue=echo Restoring to factory image...\0"		\
	"do_boot=run update_bootaddr; go ${bootaddr}\0"			\
	"parta=0x04040020\0"						\
	"partb=0x04320020\0"						\
	"rescue_gpio=0x800400a4\0"					\
	"rescue_bitmask=0x1\0"						\
	"do_checkrescue="						\
	"setexpr r *${rescue_gpio} \\\\& ${rescue_bitmask};\0"		\
	"update_bootaddr=setenv bootaddr ${parta}"

#define CONFIG_BOOTCOUNT_LIMIT
#define CONFIG_SYS_BOOTCOUNT_SINGLEWORD
#define CONFIG_SYS_BOOTCOUNT_ADDR	0x80090810

#endif	/* __CONFIG_ARTIK051DA_H */
