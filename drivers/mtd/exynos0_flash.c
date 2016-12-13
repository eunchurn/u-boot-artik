/*
 * Samsung Exynos0200 Serial Flash diver
 * Copyright (C) 2016 SAMSUNG ELECTRONICS
 * Heesub Shin <heesub.shin@samsung.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#include <common.h>
#include <console.h>
#include <dm.h>
#include <errno.h>
#include <fdt_support.h>
#include <flash.h>
#include <mtd.h>
#include <asm/io.h>

struct exynos0_sflash_regs {
	u32	reserved1[1];
	u32	sf_con;			/* 0x04 */
	u32	reserved2[2];
	u32	erase_address;		/* 0x10 */
	u32	reserved3[1];
	u32	user_command;		/* 0x18 */
	u32	command1;		/* 0x1c */
	u32	command2;		/* 0x20 */
	u32	command3;		/* 0x24 */
	u32	command4;		/* 0x28 */
	u32	command5;		/* 0x2c */
	u8	reserved4[41];
	u8	user_instruction;	/* 0x59 */
	u8	reserved5[4];
	u8	se;			/* 0x5e */
	u8	reserved6[20];
	u32	flash_io_mode;		/* 0x74 */
	u32	flash_perf_mode;	/* 0x78 */
	u8	reserved7[48];
	u32	rdid;			/* 0xac */
	u8	reserved8[14];
	u8	be;			/* 0xbe */
	u8	reserved9[15];
	u8	ce;			/* 0xce */
	u8	reserved10[13];
	u8	rdsr;			/* 0xdc */
	u8	wrdi;			/* 0xdd */
	u16	wrsr;			/* 0xde */
	u8	reserved11[14];
	u8	wren;			/* 0xee */
	u8	reserved12[1];
};

struct exynos0_sflash_platdata {
	struct exynos0_sflash_regs *regs;
	void *base;
};

flash_info_t flash_info[CONFIG_SYS_MAX_FLASH_BANKS];

struct flash_device {
	char *name;
	u8 erase_cmd;
	u32 device_id;
	u32 pagesize;
	unsigned long sectorsize;
	unsigned long size_in_bytes;
};

#define FLASH_ID(n, es, id, psize, ssize, size) \
{						\
	.name = n,				\
	.erase_cmd = es,			\
	.device_id = id,			\
	.pagesize = psize,			\
	.sectorsize = ssize,			\
	.size_in_bytes = size,			\
}

/* Flash devices supported */
static const struct flash_device flash_devices[] = {
	FLASH_ID("mac 25l6433l", 0xd8, 0x001720c2, 0x100, 0x1000, 0x800000),
};

static unsigned int sflash_read_id(struct exynos0_sflash_regs *regs)
{
	return readl(&regs->rdid) & 0xFFFFFF;
}

static unsigned int sflash_read_status(struct exynos0_sflash_regs *regs)
{
	return readb(&regs->rdsr);
}

static void sflash_write_enable(struct exynos0_sflash_regs *regs)
{
	u32 sf_con = readl(&regs->sf_con);

	writel(sf_con & ~(1 << 31), &regs->sf_con);
	writel(sf_con | (1 << 31), &regs->sf_con);
}

static void sflash_write_disable(struct exynos0_sflash_regs *regs)
{
	u32 sf_con = readl(&regs->sf_con);

	writel(sf_con & ~(1 << 31), &regs->sf_con);
}

static void sflash_erase_sector(struct exynos0_sflash_regs *regs, u32 sect)
{
	writel(sect, &regs->erase_address);
	writeb(1, &regs->se);
}

static int exynos0_sflash_erase(struct mtd_info *mtd, struct erase_info *instr)
{
	struct udevice *dev = mtd->dev;
	struct exynos0_sflash_platdata *pdata = dev_get_platdata(dev);
	struct exynos0_sflash_regs *regs = pdata->regs;
	size_t addr = instr->addr;
	size_t len = instr->len;
	size_t end = addr + len;
	u32 sect;
	u32 *flash, *last;

	instr->state = MTD_ERASING;
	addr &= ~(mtd->erasesize - 1);
	while (addr < end) {
		if (ctrlc()) {
			instr->fail_addr = MTD_FAIL_ADDR_UNKNOWN;
			instr->state = MTD_ERASE_FAILED;
			mtd_erase_callback(instr);
			return -EIO;
		}

		flash = pdata->base + addr;
		last = pdata->base + addr + mtd->erasesize;

		/* skip erase if sector is blank */
		while (flash < last) {
			if (readl(flash) != 0xffffffff)
				break;
			flash++;
		}

		if (flash < last) {
			sect = addr & ~(mtd->erasesize - 1);

			sflash_write_enable(regs);

			sflash_erase_sector(regs, sect);

			while (sflash_read_status(regs) & 0x01) {
				/* write in progress */
			}

			sflash_write_disable(regs);
		}

		addr += mtd->erasesize;
	}

	instr->state = MTD_ERASE_DONE;
	mtd_erase_callback(instr);

	return 0;
}

static int exynos0_sflash_read(struct mtd_info *mtd, loff_t from, size_t len,
		size_t *retlen, u_char *buf)
{
	struct udevice *dev = mtd->dev;
	struct exynos0_sflash_platdata *pdata = dev_get_platdata(dev);

	memcpy_fromio(buf, pdata->base + from, len);
	*retlen = len;

	return 0;
}

static int exynos0_sflash_write(struct mtd_info *mtd, loff_t to, size_t len,
		size_t *retlen, const u_char *buf)
{
	struct udevice *dev = mtd->dev;
	struct exynos0_sflash_platdata *pdata = dev_get_platdata(dev);
	struct exynos0_sflash_regs *regs = pdata->regs;

	sflash_write_enable(regs);

	memcpy_toio(pdata->base + to, buf, len);

	/* check whether write triggered a illegal write interrupt */
	while (sflash_read_status(regs) & 0x01) {
		/* write in progress */
		if (ctrlc()) {
			puts("<INTERRUPT>\n");
			break;
		}
	}

	sflash_write_disable(regs);

	*retlen = len;

	return 0;
}

static void exynos0_sflash_sync(struct mtd_info *mtd)
{
}

static int exynos0_sflash_lock(struct mtd_info *mtd, loff_t ofs, uint64_t len)
{
	return 0;
}

static int exynos0_sflash_unlock(struct mtd_info *mtd, loff_t ofs, uint64_t len)
{
	return 0;
}

static int exynos0_sflash_probe(struct udevice *dev)
{
	struct exynos0_sflash_platdata *pdata = dev_get_platdata(dev);
	struct exynos0_sflash_regs *regs = pdata->regs;
	struct mtd_info *mtd;
	flash_info_t *flash = &flash_info[0];
	u32 rdid;
	int i, j;

	rdid = sflash_read_id(regs);

	for (i = 0; i < ARRAY_SIZE(flash_devices); i++) {
		if (flash_devices[i].device_id == rdid) {
			flash->flash_id = rdid;
			flash->size = flash_devices[i].size_in_bytes;
			flash->sector_count =
				flash->size / flash_devices[i].sectorsize;
			flash->start[0] = (ulong) pdata->base;

			for (j = 1; j < flash->sector_count; j++)
				flash->start[j] = flash->start[j - 1] + 0x1000;

			break;
		}
	}

	mtd = dev_get_uclass_priv(dev);

	mtd->dev		= dev;
	mtd->name		= "sflash";
	mtd->type		= MTD_NORFLASH;
	mtd->flags		= MTD_CAP_NORFLASH;
	mtd->_erase		= exynos0_sflash_erase;
	mtd->_read		= exynos0_sflash_read;
	mtd->_write		= exynos0_sflash_write;
	mtd->_sync		= exynos0_sflash_sync;
	mtd->_lock		= exynos0_sflash_lock;
	mtd->_unlock		= exynos0_sflash_unlock;
	mtd->writesize		= 1;
	mtd->writebufsize	= mtd->writesize;
	mtd->erasesize		= 0x1000;
	mtd->size		= flash->size;

	if (add_mtd_device(mtd))
		return -ENOMEM;

	flash->mtd = mtd;

	return 0;
}

void flash_print_info(flash_info_t *info)
{
	int i;

	printf("\n  Device ID: %08x\n", (unsigned int) info->flash_id);

	if (info->size >= 0x100000)
		printf("  Size: %ld MB in %d Sectors\n",
			info->size >> 20, info->sector_count);
	else
		printf("  Size: %ld KB in %d Sectors\n",
			info->size >> 10, info->sector_count);

	puts("  Sector Start Addresses:\n  ");

	for (i = 0; i < info->sector_count; i++) {
		if (ctrlc()) {
			puts("<INTERRUPT>\n");
			return;
		}

#ifdef CONFIG_SYS_FLASH_EMPTY_INFO
		int size;
		u32 *flash;
		int erased;

		flash = (u32 *) info->start[i];
		size = info->size / info->sector_count / sizeof(u32);

		erased = 1;
		while (size--) {
			if (*flash++ != 0xffffffff) {
				erased = 0;
				break;
			}
		}

		printf("  %08lX %c %4s", info->start[i],
				erased ? 'E' : ' ',
#else
		printf("  %08lX %4s", info->start[i],
#endif
				info->protect[i] ? "(RO)" : "");

		if ((i + 1) % 4 == 0)
			puts("\n  ");
	}

	putc('\n');
}

int flash_erase(flash_info_t *info, int s_first, int s_last)
{
	struct mtd_info *mtd = info->mtd;
	struct erase_info instr;
	int ret;

	memset(&instr, 0, sizeof(instr));
	instr.mtd = mtd;
	instr.addr = mtd->erasesize * s_first;
	instr.len = mtd->erasesize * (s_last + 1 - s_first);

	ret = mtd_erase(mtd, &instr);
	if (ret)
		return ERR_PROTECTED;

	puts(" done\n");
	return 0;
}

int write_buff(flash_info_t *info, uchar *src, ulong addr, ulong cnt)
{
	struct mtd_info *mtd = info->mtd;
	struct udevice *dev = mtd->dev;
	struct exynos0_sflash_platdata *pdata = dev_get_platdata(dev);
	ulong base = (ulong) pdata->base;
	loff_t to = addr - base;
	size_t retlen;

	if (mtd_write(mtd, to, cnt, &retlen, src))
		return ERR_PROTECTED;

	return 0;
}

unsigned long flash_init(void)
{
	struct udevice *dev;

	for (uclass_first_device(UCLASS_MTD, &dev); dev;
			uclass_next_device(&dev)) {
		/* probe MTD devices */
	}

	return flash_info[0].size;
}

static int exynos0_sflash_ofdata_to_platdata(struct udevice *dev)
{
	struct exynos0_sflash_platdata *pdata = dev_get_platdata(dev);

	/* TODO: decode regs in FDT */
	pdata->regs = (struct exynos0_sflash_regs *) 0x80310000;
	pdata->base = (void *) 0x04000000;

	return 0;
}

static const struct udevice_id exynos0_sflash_ids[] = {
	{ .compatible = "samsung,exynos0200-sflash" },
	{ }
};

U_BOOT_DRIVER(exynos0_sflash) = {
	.name	= "exynos0_sflash",
	.id	= UCLASS_MTD,
	.of_match = exynos0_sflash_ids,
	.ofdata_to_platdata = exynos0_sflash_ofdata_to_platdata,
	.platdata_auto_alloc_size = sizeof(struct exynos0_sflash_platdata),
	.probe	= exynos0_sflash_probe,
};
