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

#include <configs/exynos0200-common.h>

#undef CONFIG_CMD_PXE
#define CONFIG_CMD_UNZIP

#define SDRAM_BANK_SIZE			0

#define CONFIG_SYS_TEXT_BASE		0x04010020
#define CONFIG_SYS_INIT_SP_ADDR		0x02150000
#define CONFIG_SYS_SRAM_BASE		0x02020000
#define CONFIG_SYS_SRAM_SIZE		(1280 * 1024)
#define CONFIG_SYS_LOAD_ADDR		CONFIG_SYS_SRAM_BASE
#define CONFIG_SYS_SDRAM_BASE		CONFIG_SYS_SRAM_BASE
#define CONFIG_SYS_FLASH_BASE		0x04000000
#define CONFIG_SYS_FLASH_EMPTY_INFO
#define CONFIG_SYS_MAX_FLASH_BANKS	1
#define CONFIG_SYS_MAX_FLASH_SECT	2048
#define CONFIG_SYS_DCACHE_OFF

#define CONFIG_ENV_IS_IN_FLASH		/* remove CONFIG_ENV_IS_IN_FLASH before production release */
#ifndef CONFIG_ENV_IS_IN_FLASH
#define CONFIG_ENV_IS_NOWHERE
#endif
#define CONFIG_ENV_OFFSET		(SZ_1K * 252)
#define CONFIG_ENV_SIZE			(SZ_1K * 4)
#define CONFIG_ENV_SECT_SIZE		0x1000

#ifdef CONFIG_SKIP_LOWLEVEL_INIT
#undef CONFIG_SKIP_LOWLEVEL_INIT
#endif
#define CONFIG_SKIP_LOWLEVEL_INIT_ONLY

#define CONFIG_HW_WATCHDOG
#define CONFIG_HW_WATCHDOG_TIMEOUT_MS	10000

#define CONFIG_MISC_INIT_R

#define CONFIG_MTD_DEVICE

#define CONFIG_SECURE_BOOT

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
	"do_boot=\n"							\
	"    run do_checkupdate\n"					\
	"    setexpr entrypoint ${bootpart} + 0x20\n"			\
	"    go ${entrypoint}\0"					\
	"do_rescue=\n"							\
	"    echo Factory reset.\n"					\
	"    run do_eraseboot\n"					\
	"    echo Flashing factory image...\n"				\
	"    unzip ${rescuepart} ${bootpart}\n"				\
	"    reset\0"							\
	"do_checkupdate=\n"						\
	"    if itest *${otapart} -ne 0xffffffff; then\n"		\
	"        setexpr.l startmagicaddr ${otapart}\n"			\
	"        setexpr.l startmagic *${startmagicaddr}\n"		\
	"        if itest ${startmagic} -ne ${otastartmagic}; then\n"	\
	"            echo Bad START MAGIC image!\n"			\
	"            run do_eraseota\n"					\
	"            reset\n"						\
	"        fi\n"							\
	"        setexpr.l sizeaddr ${otapart} + 0x4\n"			\
	"        setexpr.l size *${sizeaddr}\n"				\
	"        setexpr.l sizemax ${otasize} - 0x1000\n"		\
	"        if itest ${size} -gt ${sizemax}; then\n"		\
	"            echo update image is too big!\n"			\
	"            run do_eraseota\n"					\
	"            reset\n"						\
	"        fi\n"							\
	"        setexpr.l otagz ${otapart} + 0x1000\n"			\
	"        setexpr.l endmagicaddr ${otagz} + ${size}\n"		\
	"        setexpr.l endmagic *${endmagicaddr}\n"			\
	"        if itest ${endmagic} -ne ${otaendmagic}; then\n"	\
	"            echo Bad END MAGIC image!\n"			\
	"            run do_eraseota\n"					\
	"            reset\n"						\
	"        fi\n"							\
	"        setexpr.l crcaddr ${otapart} + 0x8\n"			\
	"        setexpr.l crc *${crcaddr}\n"				\
	"        crc32 ${otagz} ${size} 0x02023800\n"			\
	"        if itest.l *0x02023800 -eq ${crc}; then\n"		\
	"            echo Found an update image downloaded.\n"		\
	"            run do_eraseboot\n"				\
	"            echo Updating boot partition...\n"			\
	"            unzip ${otagz} ${bootpart}\n"			\
	"            run do_eraseota\n"					\
	"            echo Done\n"					\
	"        else\n"						\
	"            echo Bad CRC image!\n"				\
	"            run do_eraseota\n"					\
	"        fi\n"							\
	"        reset\n"						\
	"    fi\0"							\
	"do_eraseboot=\n"						\
	"    echo Eraseing boot partitions...\n"			\
	"    erase ${bootpart} +${bootsize}\0"				\
	"do_eraseota=\n"						\
	"    echo Erasing ota partitions...\n"				\
	"    erase ${otapart} +${otasize}\0"				\
	"bootlimit=3\0"							\
	"altbootcmd=\n"							\
	"    run do_eraseota\n"						\
	"    run do_rescue\n"						\
	"    reset\0"							\
	"bootpart=0x040c8000\0"						\
	"bootsize=0x258000\0"						\
	"otapart=0x044a0000\0"						\
	"otasize=0x180000\0"						\
	"otastartmagic=0x455A4954\0"					\
	"otaendmagic=0x4154524E\0"					\
	"rescuepart=0x04320000\0"					\
	"res_gpio=gpg16\0"

#define CONFIG_BOOTCOUNT_LIMIT
#define CONFIG_SYS_BOOTCOUNT_SINGLEWORD
#define CONFIG_SYS_BOOTCOUNT_ADDR	0x80090810

#endif	/* __CONFIG_ARTIK05X_H__ */
