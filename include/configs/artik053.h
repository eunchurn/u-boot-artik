/*
 * Copyright (C) 2017 Samsung Electronics
 * Heesub Shin <heesub.shin@samsung.com>
 *
 * Configuation settings for the ARTIK-053 Starter Kit.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __CONFIG_ARTIK053_H__
#define __CONFIG_ARTIK053_H__

#include <configs/artik051.h>

#ifdef CONFIG_BOOTDELAY
#undef CONFIG_BOOTDELAY
#endif
#define CONFIG_BOOTDELAY	0

#ifdef CONFIG_BOOTCOMMAND
#undef CONFIG_BOOTCOMMAND
#endif

#define CONFIG_BOOTCOMMAND "\n"						\
	"    if env exists do_rescue; then\n"				\
	"        c=0;\n"						\
	"        mw.l 0x800400a8 0x13111111\n"				\
	"        gpio input ${res_gpio}\n"				\
	"        while itest $? == 0 && itest ${c} < 50; do\n"		\
	"            sleep 0.1;\n"					\
	"            setexpr c ${c} + 1;\n"				\
	"            gpio input ${res_gpio}\n"				\
	"        done\n"						\
	"        if itest ${c} >= 50; then\n"				\
	"            run do_rescue;\n"					\
	"        fi\n"							\
	"        mw.l 0x800400a8 0x11111111\n"				\
	"    fi\n"							\
	"    run do_boot;"

#ifdef CONFIG_EXTRA_ENV_SETTINGS
#undef CONFIG_EXTRA_ENV_SETTINGS
#endif
#define CONFIG_EXTRA_ENV_SETTINGS					\
	"bootlimit=3\0"							\
	"do_boot=\n"							\
	"    setexpr entrypoint ${bootimg} + ${offset}\n"		\
	"    go ${entrypoint}\0"					\
	"do_rescue=\n"							\
	"    echo restoring to factory image\n"				\
	"    unzip ${factoryimg} ${bootimg}\n"				\
	"do_checkupdate=\n"						\
	"    if itest *${otaimg} -ne 0xffffffff; then\n"		\
	"        setexpr.l size *0x04320000\n"				\
	"        setexpr.l crc  *0x04320004\n"				\
	"        crc32 0x04321000 ${size} 0x02022000\n"			\
	"        if itest.l *0x02022000 -eq ${crc}; then\n"		\
	"            echo Updating boot image...\n"			\
	"            unzip ${otaimg} ${bootimg}\n"			\
	"            erase ${otahead} +${otasize}\n"			\
	"        fi\n"							\
	"    fi\0"							\
	"bootimg=0x04040000\0"						\
	"offset=0x00088020\0"						\
	"otahead=0x04320000\0"						\
	"otaimg=0x04321000\0"						\
	"otasize=0x170000\0"						\
	"factoryimg=0x04491000\0"					\
	"res_gpio=gpg16\0"

#define CONFIG_BOOTCOUNT_LIMIT
#define CONFIG_SYS_BOOTCOUNT_SINGLEWORD
#define CONFIG_SYS_BOOTCOUNT_ADDR	0x80090810

#endif	/* __CONFIG_ARTIK053_H__ */
