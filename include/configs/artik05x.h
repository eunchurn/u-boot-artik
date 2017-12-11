/*
 * Copyright (C) 2017 Samsung Electronics
 * Heesub Shin <heesub.shin@samsung.com>
 *
 * Configuation settings for the ARTIK-05x Starter Kit.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __CONFIG_ARTIK05X_H__
#define __CONFIG_ARTIK05X_H__

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
	"    run do_checkupdate\n"					\
	"    setexpr entrypoint ${bootpart} + 0x20\n"			\
	"    if sss_auth_img ${bootpart}; then\n"			\
	"        go ${entrypoint}\n"					\
	"    else\n"							\
	"        reset;\n"						\
	"    fi\0"							\
	"do_rescue=\n"							\
	"    echo Factory reset.\n"					\
	"    echo Erasing boot partitions...\n"				\
	"    erase ${bootpart} +${bootsize}\n"				\
	"    echo Flashing factory image...\n"				\
	"    unzip ${rescuepart} ${bootpart}\n"				\
	"    reset\0"							\
	"do_checkupdate=\n"						\
	"    if itest *${otapart} -ne 0xffffffff; then\n"		\
	"        setexpr.l sizeaddr ${otapart}\n"			\
	"        setexpr.l crcaddr ${otapart} + 0x4\n"			\
	"        setexpr.l size *${sizeaddr}\n"				\
	"        setexpr.l crc  *${crcaddr}\n"				\
	"        setexpr.l otagz ${otapart} + 0x1000\n"			\
	"        crc32 ${otagz} ${size} 0x02022000\n"			\
	"        if itest.l *0x02022000 -eq ${crc}; then\n"		\
	"            echo Found an update image downloaded.\n"		\
	"            erase ${bootpart} +${bootsize}\n"			\
	"            echo Updating boot partition...\n"			\
	"            unzip ${otagz} ${bootpart}\n"			\
	"        fi\n"							\
	"        erase ${otapart} +${otasize}\n"			\
	"        echo Done\n"						\
	"        reset\n"						\
	"    fi\0"							\
	"bootpart=0x040c8000\0"						\
	"bootsize=0x258000\0"						\
	"otapart=0x044a0000\0"						\
	"otasize=0x180000\0"						\
	"rescuepart=0x04320000\0"					\
	"res_gpio=gpg16\0"

#define CONFIG_BOOTCOUNT_LIMIT
#define CONFIG_SYS_BOOTCOUNT_SINGLEWORD
#define CONFIG_SYS_BOOTCOUNT_ADDR	0x80090810

#endif	/* __CONFIG_ARTIK05X_H__ */
