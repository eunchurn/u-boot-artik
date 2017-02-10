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

#ifdef CONFIG_BOOTDELAY
#undef CONFIG_BOOTDELAY
#endif
#define CONFIG_BOOTDELAY	0

#ifdef CONFIG_BOOTCOMMAND
#undef CONFIG_BOOTCOMMAND
#endif
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

#ifdef CONFIG_EXTRA_ENV_SETTINGS
#undef CONFIG_EXTRA_ENV_SETTINGS
#endif
#define CONFIG_EXTRA_ENV_SETTINGS					\
	"altbootcmd=setenv compare -le; run do_boot;\0"			\
	"bootlimit=3\0"							\
	"do_boot=run update_bootaddr; go ${bootaddr}\0"			\
	"compare=-ge\0"							\
	"parta=0x04048020\0"						\
	"partb=0x04328020\0"						\
	"parta_date=0x04273ffc\0"					\
	"partb_date=0x04553ffc\0"					\
	"rescue_gpio=0x800400a4\0"					\
	"rescue_bitmask=0x1\0"						\
	"do_checkrescue="						\
	    "setexpr r *${rescue_gpio} \\\\& ${rescue_bitmask};\0"	\
	"update_bootaddr=\n"						\
	"    if itest *${parta_date} ${compare} *${partb_date}; then\n"	\
	"        setenv bootaddr ${parta};\n"				\
	"    else\n"							\
	"        setenv bootaddr ${partb};\n"				\
	"    fi\n"							\
	"    if itest *${parta_date} -eq ffffffff; then\n"		\
	"        setenv bootaddr ${partb};\n"				\
	"    fi\n"							\
	"    if itest *${partb_date} -eq ffffffff; then\n"		\
	"        setenv bootaddr ${parta};\n"				\
	"    fi\0"

#define CONFIG_BOOTCOUNT_LIMIT
#define CONFIG_SYS_BOOTCOUNT_SINGLEWORD
#define CONFIG_SYS_BOOTCOUNT_ADDR	0x80090810

#endif	/* __CONFIG_ARTIK051DA_H */
