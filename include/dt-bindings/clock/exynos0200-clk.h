/*
 * Copyright (c) 2016 Samsung Electronics Co., Ltd.
 * Author: Heesub Shin <heesub.shin@samsung.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#ifndef __DT_BINDINGS_CLOCK_EXYNOS0200_H__
#define __DT_BINDINGS_CLOCK_EXYNOS0200_H__

/* OSC */
#define OSCCLK_XXTI		0

/* CMU_MCU */
#define OSCCLK_MCU		0
#define WPLL_DIV3		1
#define WPLL_DIV6		2
#define WPLL_DIV12		3
#define SPL_CLK_BUS_P0		4
#define SPL_CLK_UART		5
#define DFT_OSCCLK		6
#define MUX_CLKCMU_I2SB		7

/* CMU_SSS */
#define SPL_CLK_BUS_D0_SSS	0
#define SPL_CLK_BUS_P0_SSS	1
#define DFT_OSCCLK_SSS		2

#endif /* __DT_BINDINGS_CLOCK_EXYNOS0200_H__ */
