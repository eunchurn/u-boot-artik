/*
 * Samsung Exynos0200 clock driver.
 * Copyright (C) 2016 SAMSUNG ELECTRONICS
 * Heesub Shin <heesub.shin@samsung.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#include <common.h>
#include <dm.h>
#include <clk-uclass.h>
#include <asm/arch/clock.h>
#include <dt-bindings/clock/exynos0200-clk.h>

/* WPLL_CON0 */
#define WPLL_EN				0x00400000
#define WPLL_FREF_EN			0x00200000

/* WPLL_CON1 */
#define WPLL_ABC_START			0x00100000

/* WPLL_CON5 */
#define WPLL_EN_CLK_960M		0x00400000
#define WPLL_EN_CLK_480M		0x00200000
#define WPLL_EN_CLK_240M		0x00100000
#define WPLL_EN_CLK_ADC			0x00010000

/* WPLL_STAT */
#define WPLL_LOCK			0x10000000

DECLARE_GLOBAL_DATA_PTR;

struct exynos0200_clk_priv {
	struct udevice *dev;
	struct clk oscclk;
	struct exynos0200_clock *cmu;
};

static unsigned long exynos0200_osc_get_clk(struct exynos0200_clk_priv *priv,
		unsigned int clk_id)
{
	return clk_get_rate(&priv->oscclk);
}

static unsigned long exynos0200_wpll_get_clk(struct exynos0200_clk_priv *priv,
		unsigned int clk_id)
{
	unsigned long sclk, div;
	struct exynos0200_clock *cmu = priv->cmu;

	if (readl(&cmu->pll_con0) & 0x10)
		sclk = 960000000;
	else
		sclk = exynos0200_osc_get_clk(priv, clk_id);

	div = 1;

	switch (clk_id) {
	case WPLL_DIV12:
		div = (readl(&cmu->div_wpll_div12) & 0x1) + 1;
		/* fall through */
	case WPLL_DIV6:
		div = div * ((readl(&cmu->div_wpll_div6) & 0x1) + 1);
		/* fall through */
	case WPLL_DIV3:
		div = div * ((readl(&cmu->div_wpll_div3) & 0xf) + 1);
		break;
	}

	return sclk / div;
}

static unsigned long exynos0200_uart_get_clk(struct exynos0200_clk_priv *priv,
		unsigned int clk_id)
{
	struct exynos0200_clock *cmu = priv->cmu;

	if (readl(&cmu->mux_uart) & 0x1)
		return exynos0200_wpll_get_clk(priv, WPLL_DIV12);

	return exynos0200_osc_get_clk(priv, clk_id);
}

static unsigned long exynos0200_clk_get_rate(struct clk *clk)
{
	unsigned long rate = 0;
	struct exynos0200_clk_priv *priv = dev_get_priv(clk->dev);

	switch (clk->id) {
	case OSCCLK_MCU:
	case DFT_OSCCLK:
		rate = exynos0200_osc_get_clk(priv, clk->id);
		break;

	case WPLL_DIV3:
	case WPLL_DIV6:
	case WPLL_DIV12:
		rate = exynos0200_wpll_get_clk(priv, clk->id);
		break;

	case SPL_CLK_BUS_P0:
		rate = exynos0200_wpll_get_clk(priv, WPLL_DIV6);
		break;

	case SPL_CLK_UART:
		rate = exynos0200_uart_get_clk(priv, clk->id);
		break;
	}

	return rate;
}

static unsigned long exynos0200_clk_set_rate(struct clk *clk,
		unsigned long rate)
{
	return 0;
}

static void cmu_mcu_init(struct exynos0200_clock *cmu)
{
	int timeout = 0xfffff;
	u32 regval;

	/* enable wpll */
	regval = readl(&cmu->wpll_con0);
	regval |= WPLL_EN | WPLL_FREF_EN;
	writel(regval, &cmu->wpll_con0);
	udelay(20);

	regval = readl(&cmu->wpll_con1);
	regval |= WPLL_ABC_START;
	writel(regval, &cmu->wpll_con1);
	udelay(60);

	while (timeout--)
		if (readl(&cmu->wpll_stat) & WPLL_LOCK)
			break;

	if (!timeout)
		return;

	regval = readl(&cmu->wpll_con5);
	regval |= WPLL_EN_CLK_ADC | WPLL_EN_CLK_240M |
			WPLL_EN_CLK_480M | WPLL_EN_CLK_960M;
	writel(regval, &cmu->wpll_con5);
}

static int exynos0200_clk_probe(struct udevice *dev)
{
	int ret;
	struct exynos0200_clk_priv *priv = dev_get_priv(dev);

	ret = clk_get_by_index(dev, 0, &priv->oscclk);
	if (ret) {
		error("clk_get_by_name() failed: %d\n", ret);
		return ret;
	}

	priv->dev = dev;
	cmu_mcu_init(priv->cmu);

	return 0;
}

static int exynos0200_clk_ofdata_to_platdata(struct udevice *dev)
{
	struct exynos0200_clk_priv *priv = dev_get_priv(dev);

	priv->cmu = (struct exynos0200_clock *) dev_get_addr(dev);

	return 0;
}

static struct clk_ops exynos0200_clk_ops = {
	.get_rate = exynos0200_clk_get_rate,
	.set_rate = exynos0200_clk_set_rate,
};

static const struct udevice_id exynos0200_clk_compat[] = {
	{ .compatible = "samsung,exynos0200-cmu-mcu" },
	{ }
};

U_BOOT_DRIVER(clk_exynos0200) = {
	.id			= UCLASS_CLK,
	.name			= "exynos0200_cmu_mcu",
	.of_match		= exynos0200_clk_compat,
	.ops			= &exynos0200_clk_ops,
	.probe			= exynos0200_clk_probe,
	.ofdata_to_platdata	= exynos0200_clk_ofdata_to_platdata,
	.priv_auto_alloc_size	= sizeof(struct exynos0200_clk_priv),
};
