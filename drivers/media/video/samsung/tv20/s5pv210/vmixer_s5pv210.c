/* linux/drivers/media/video/samsung/tv20/s5pv210/vmixer_s5pv210.c
 *
 * Mixer raw ftn  file for Samsung TVOut driver
 *
 * Copyright (c) 2010 Samsung Electronics
 *	         http://www.samsungsemi.com/
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/string.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/io.h>
#include <linux/slab.h>

#include <plat/clock.h>
#include <plat/regs-vmx.h>

#include "tv_out_s5pv210.h"

#ifdef CONFIG_TVOUT_RAW_DBG
#define S5P_MXR_DEBUG 1
#endif

#ifdef S5P_MXR_DEBUG
#define VMPRINTK(fmt, args...)	\
	printk(KERN_INFO "\t\t[VM] %s: " fmt, __func__ , ## args)
#else
#define VMPRINTK(fmt, args...)
#endif

static struct resource	*g_mixer_mem;
static void __iomem	*g_mixer_base;

/*
 *set  - set functions are only called under running vmixer
*/
enum s5p_tv_vmx_err s5p_vm_set_layer_show(
	enum s5p_tv_vmx_layer layer, bool show)
{
	u32 mxr_config;

	VMPRINTK("%d, %d\n", layer, show);

	switch (layer) {
	case VM_VIDEO_LAYER:
		mxr_config = (show) ?
			(readl(g_mixer_base + S5P_MXR_CFG) |
				S5P_MXR_VIDEO_LAYER_SHOW) :
			(readl(g_mixer_base + S5P_MXR_CFG) &
				~S5P_MXR_VIDEO_LAYER_SHOW);
		break;
	case VM_GPR0_LAYER:
		mxr_config = (show) ?
			(readl(g_mixer_base + S5P_MXR_CFG) |
				S5P_MXR_GRAPHIC0_LAYER_SHOW) :
			(readl(g_mixer_base + S5P_MXR_CFG) &
				~S5P_MXR_GRAPHIC0_LAYER_SHOW);
		break;
	case VM_GPR1_LAYER:
		mxr_config = (show) ?
			(readl(g_mixer_base + S5P_MXR_CFG) |
				S5P_MXR_GRAPHIC1_LAYER_SHOW) :
			(readl(g_mixer_base + S5P_MXR_CFG) &
				~S5P_MXR_GRAPHIC1_LAYER_SHOW);
		break;
	default:
		pr_err(" invalid layer parameter = %d\n", layer);
		return S5P_TV_VMX_ERR_INVALID_PARAM;
		break;
	}

	writel(mxr_config, g_mixer_base + S5P_MXR_CFG);

	VMPRINTK("layer(%d), S5P_MXR_CFG(0x%08x)\n",
		layer, readl(g_mixer_base + S5P_MXR_CFG));

	return VMIXER_NO_ERROR;
}

enum s5p_tv_vmx_err s5p_vm_set_layer_priority(enum s5p_tv_vmx_layer layer,
	u32 priority)
{
	u32 layer_cfg;

	VMPRINTK("%d, %d\n", layer, priority);

	switch (layer) {
	case VM_VIDEO_LAYER:
		layer_cfg = S5P_MXR_VP_LAYER_PRIORITY_CLEAR(
			readl(g_mixer_base + S5P_MXR_LAYER_CFG)) |
			S5P_MXR_VP_LAYER_PRIORITY(priority);
		break;
	case VM_GPR0_LAYER:
		layer_cfg = S5P_MXR_GRP0_LAYER_PRIORITY_CLEAR(
			readl(g_mixer_base + S5P_MXR_LAYER_CFG)) |
			S5P_MXR_GRP0_LAYER_PRIORITY(priority);
		break;
	case VM_GPR1_LAYER:
		layer_cfg = S5P_MXR_GRP1_LAYER_PRIORITY_CLEAR(
			readl(g_mixer_base + S5P_MXR_LAYER_CFG)) |
			S5P_MXR_GRP1_LAYER_PRIORITY(priority);
		break;
	default:
		pr_err(" invalid layer parameter = %d\n", layer);
		return S5P_TV_VMX_ERR_INVALID_PARAM;
		break;
	}

	writel(layer_cfg, g_mixer_base + S5P_MXR_LAYER_CFG);

	return VMIXER_NO_ERROR;
}

enum s5p_tv_vmx_err s5p_vm_set_win_blend(enum s5p_tv_vmx_layer layer,
	bool enable)
{
	u32 temp_reg;
	VMPRINTK("%d, %d\n", layer, enable);

	switch (layer) {
	case VM_VIDEO_LAYER:
		temp_reg = readl(g_mixer_base + S5P_MXR_VIDEO_CFG)
			   & (~S5P_MXR_VP_BLEND_ENABLE) ;

		if (enable)
			temp_reg |= S5P_MXR_VP_BLEND_ENABLE;
		else
			temp_reg |= S5P_MXR_VP_BLEND_DISABLE;

		writel(temp_reg, g_mixer_base + S5P_MXR_VIDEO_CFG);
		break;
	case VM_GPR0_LAYER:
		temp_reg = readl(g_mixer_base + S5P_MXR_GRAPHIC0_CFG)
			   & (~S5P_MXR_WIN_BLEND_ENABLE) ;

		if (enable)
			temp_reg |= S5P_MXR_WIN_BLEND_ENABLE;
		else
			temp_reg |= S5P_MXR_WIN_BLEND_DISABLE;

		writel(temp_reg, g_mixer_base + S5P_MXR_GRAPHIC0_CFG);
		break;
	case VM_GPR1_LAYER:
		temp_reg = readl(g_mixer_base + S5P_MXR_GRAPHIC1_CFG)
			   & (~S5P_MXR_WIN_BLEND_ENABLE) ;

		if (enable)
			temp_reg |= S5P_MXR_WIN_BLEND_ENABLE;
		else
			temp_reg |= S5P_MXR_WIN_BLEND_DISABLE;

		writel(temp_reg, g_mixer_base + S5P_MXR_GRAPHIC1_CFG);
		break;
	default:
		pr_err(" invalid layer parameter = %d\n", layer);
		return S5P_TV_VMX_ERR_INVALID_PARAM;
		break;
	}

	VMPRINTK("layer(%d), enable(%d), \
		S5P_MXR_VIDEO_CFG(0x08%x), \
		S5P_MXR_GRAPHIC0_CFG(0x08%x), \
		S5P_MXR_GRAPHIC1_CFG(0x08%x)\n",
		layer, enable,
		readl(g_mixer_base + S5P_MXR_VIDEO_CFG),
		readl(g_mixer_base + S5P_MXR_GRAPHIC0_CFG),
		readl(g_mixer_base + S5P_MXR_GRAPHIC1_CFG));

	return VMIXER_NO_ERROR;
}

enum s5p_tv_vmx_err s5p_vm_set_layer_alpha(enum s5p_tv_vmx_layer layer,
	u32 alpha)
{
	u32 temp_reg;
	VMPRINTK("%d, %d\n", layer, alpha);

	switch (layer) {
	case VM_VIDEO_LAYER:
		temp_reg = readl(g_mixer_base + S5P_MXR_VIDEO_CFG)
			   & (~S5P_MXR_ALPHA) ;
		temp_reg |= S5P_MXR_VP_ALPHA_VALUE(alpha);
		writel(temp_reg, g_mixer_base + S5P_MXR_VIDEO_CFG);
		break;
	case VM_GPR0_LAYER:
		temp_reg = readl(g_mixer_base + S5P_MXR_GRAPHIC0_CFG)
			   & (~S5P_MXR_ALPHA) ;
		temp_reg |= S5P_MXR_GRP_ALPHA_VALUE(alpha);
		writel(temp_reg, g_mixer_base + S5P_MXR_GRAPHIC0_CFG);
		break;
	case VM_GPR1_LAYER:
		temp_reg = readl(g_mixer_base + S5P_MXR_GRAPHIC1_CFG)
			   & (~S5P_MXR_ALPHA) ;
		temp_reg |= S5P_MXR_GRP_ALPHA_VALUE(alpha);
		writel(temp_reg, g_mixer_base + S5P_MXR_GRAPHIC1_CFG);
		break;
	default:
		pr_err(" invalid layer parameter = %d\n", layer);
		return S5P_TV_VMX_ERR_INVALID_PARAM;
		break;
	}

	VMPRINTK("layer(%d), alpha(%d), \
		S5P_MXR_VIDEO_CFG(0x08%x), \
		S5P_MXR_GRAPHIC0_CFG(0x08%x), \
		S5P_MXR_GRAPHIC1_CFG(0x08%x)\n",
		layer, alpha,
		readl(g_mixer_base + S5P_MXR_VIDEO_CFG),
		readl(g_mixer_base + S5P_MXR_GRAPHIC0_CFG),
		readl(g_mixer_base + S5P_MXR_GRAPHIC1_CFG));

	return VMIXER_NO_ERROR;
}

enum s5p_tv_vmx_err s5p_vm_set_grp_base_address(enum s5p_tv_vmx_layer layer,
	u32 base_addr)
{
	VMPRINTK("%d, 0x%08x\n", layer, base_addr);

	if (S5P_MXR_GRP_ADDR_ILLEGAL(base_addr)) {
		pr_err(" address is not word align = %d\n", base_addr);
		return S5P_TV_VMX_ERR_BASE_ADDRESS_MUST_WORD_ALIGN;
	}

	switch (layer) {
	case VM_GPR0_LAYER:
		writel(S5P_MXR_GPR_BASE(base_addr),
			g_mixer_base + S5P_MXR_GRAPHIC0_BASE);
		VMPRINTK("S5P_MXR_GRAPHIC0_BASE(0x%x)\n",
			readl(g_mixer_base + S5P_MXR_GRAPHIC0_BASE));
		break;
	case VM_GPR1_LAYER:
		writel(S5P_MXR_GPR_BASE(base_addr),
			g_mixer_base + S5P_MXR_GRAPHIC1_BASE);
		VMPRINTK("S5P_MXR_GRAPHIC1_BASE(0x%x)\n",
			readl(g_mixer_base + S5P_MXR_GRAPHIC1_BASE));
		break;
	default:
		pr_err(" invalid layer parameter = %d\n", layer);
		return S5P_TV_VMX_ERR_INVALID_PARAM;
		break;
	}

	return VMIXER_NO_ERROR;
}

enum s5p_tv_vmx_err s5p_vm_set_grp_layer_position(enum s5p_tv_vmx_layer layer,
	u32 dst_offs_x, u32 dst_offs_y)
{
	VMPRINTK("%d, %d, %d)\n", layer, dst_offs_x, dst_offs_y);

	switch (layer) {
	case VM_GPR0_LAYER:
		writel(S5P_MXR_GRP_DESTX(dst_offs_x) |
			S5P_MXR_GRP_DESTY(dst_offs_y),
			g_mixer_base + S5P_MXR_GRAPHIC0_DXY);

		VMPRINTK("S5P_MXR_GRAPHIC0_DXY(0x%08x)\n",
			readl(g_mixer_base + S5P_MXR_GRAPHIC0_DXY));
		break;
	case VM_GPR1_LAYER:
		writel(S5P_MXR_GRP_DESTX(dst_offs_x) |
			S5P_MXR_GRP_DESTY(dst_offs_y),
			g_mixer_base + S5P_MXR_GRAPHIC1_DXY);

		VMPRINTK("S5P_MXR_GRAPHIC1_DXY(0x%08x)\n",
			readl(g_mixer_base + S5P_MXR_GRAPHIC1_DXY));
		break;
	default:
		pr_err("invalid layer parameter = %d\n", layer);
		return S5P_TV_VMX_ERR_INVALID_PARAM;
		break;
	}

	VMPRINTK("layer(%d), dst_offs_x(%d), dst_offs_y(%d)\n",
		layer, dst_offs_x, dst_offs_y);

	return VMIXER_NO_ERROR;
}

enum s5p_tv_vmx_err s5p_vm_set_grp_layer_size(enum s5p_tv_vmx_layer layer,
					u32 span,
					u32 width,
					u32 height,
					u32 src_offs_x,
					u32 src_offs_y)
{
	VMPRINTK("%d, %d, %d, %d, %d, %d)\n", layer, span, width, height,
		 src_offs_x, src_offs_y);

	switch (layer) {
	case VM_GPR0_LAYER:
		writel(S5P_MXR_GRP_SPAN(span),
			g_mixer_base + S5P_MXR_GRAPHIC0_SPAN);
		writel(S5P_MXR_GRP_WIDTH(width) | S5P_MXR_GRP_HEIGHT(height),
		       g_mixer_base + S5P_MXR_GRAPHIC0_WH);
		writel(S5P_MXR_GRP_STARTX(src_offs_x) |
			S5P_MXR_GRP_STARTY(src_offs_y),
		       g_mixer_base + S5P_MXR_GRAPHIC0_SXY);

		VMPRINTK("S5P_MXR_GRAPHIC0_SPAN(0x%x), \
			S5P_MXR_GRAPHIC0_WH(0x%x), \
			S5P_MXR_GRAPHIC0_SXY(0x%x)\n",
			readl(g_mixer_base + S5P_MXR_GRAPHIC0_SPAN),
			readl(g_mixer_base + S5P_MXR_GRAPHIC0_WH),
			readl(g_mixer_base + S5P_MXR_GRAPHIC0_SXY));
		break;
	case VM_GPR1_LAYER:
		writel(S5P_MXR_GRP_SPAN(span),
			g_mixer_base + S5P_MXR_GRAPHIC1_SPAN);
		writel(S5P_MXR_GRP_WIDTH(width) | S5P_MXR_GRP_HEIGHT(height),
		       g_mixer_base + S5P_MXR_GRAPHIC1_WH);
		writel(S5P_MXR_GRP_STARTX(src_offs_x) |
			S5P_MXR_GRP_STARTY(src_offs_y),
		       g_mixer_base + S5P_MXR_GRAPHIC1_SXY);

		VMPRINTK("S5P_MXR_GRAPHIC1_SPAN(0x%x), \
			S5P_MXR_GRAPHIC1_WH(0x%x), \
			S5P_MXR_GRAPHIC1_SXY(0x%x)\n",
			readl(g_mixer_base + S5P_MXR_GRAPHIC1_SPAN),
			readl(g_mixer_base + S5P_MXR_GRAPHIC1_WH),
			readl(g_mixer_base + S5P_MXR_GRAPHIC1_SXY));
		break;

	default:
		pr_err("invalid layer parameter = %d\n", layer);
		return S5P_TV_VMX_ERR_INVALID_PARAM;
		break;
	}

	return VMIXER_NO_ERROR;
}

enum s5p_tv_vmx_err s5p_vm_set_bg_color(enum s5p_tv_vmx_bg_color_num colornum,
				u32 color_y,
				u32 color_cb,
				u32 color_cr)
{
	u32 reg_value;
	VMPRINTK("colornum(%d) color_y(%d), color_cb(%d), color_cr(%d)\n",
		colornum, color_y, color_cb, color_cr);

	reg_value = S5P_MXR_BG_COLOR_Y(color_y) |
			S5P_MXR_BG_COLOR_CB(color_cb) |
			S5P_MXR_BG_COLOR_CR(color_cr);

	switch (colornum) {
	case VMIXER_BG_COLOR_0:
		writel(reg_value, g_mixer_base + S5P_MXR_BG_COLOR0);
		VMPRINTK("S5P_MXR_BG_COLOR0(0x%x)\n",
			readl(g_mixer_base + S5P_MXR_BG_COLOR0));
		break;
	case VMIXER_BG_COLOR_1:
		writel(reg_value, g_mixer_base + S5P_MXR_BG_COLOR1);
		VMPRINTK("S5P_MXR_BG_COLOR1(0x%x)\n",
			readl(g_mixer_base + S5P_MXR_BG_COLOR1));
		break;
	case VMIXER_BG_COLOR_2:
		writel(reg_value, g_mixer_base + S5P_MXR_BG_COLOR2);
		VMPRINTK("S5P_MXR_BG_COLOR2(0x%x)\n",
			readl(g_mixer_base + S5P_MXR_BG_COLOR2));
		break;
	default:
		pr_err(" invalid uiColorNum parameter = %d\n", colornum);
		return S5P_TV_VMX_ERR_INVALID_PARAM;
		break;
	}

	return VMIXER_NO_ERROR;
}

/*
* initialization  - initization functions are only called under stopping vmixer
*/
enum s5p_tv_vmx_err s5p_vm_init_status_reg(enum s5p_vmx_burst_mode burst,
	enum s5p_endian_type endian)
{
	u32 temp_reg;
	VMPRINTK("(%d, %d)\n", burst, endian);

	temp_reg = S5P_MXR_MIXER_RESERVED | S5P_MXR_CMU_CANNOT_STOP_CLOCK;

	switch (burst) {
	case VM_BURST_8:
		temp_reg |= S5P_MXR_BURST8_MODE;
		break;
	case VM_BURST_16:
		temp_reg |= S5P_MXR_BURST16_MODE;
		break;
	default:
		pr_err("[ERR] : invalid burst parameter = %d\n", burst);
		return S5P_TV_VMX_ERR_INVALID_PARAM;
		break;
	}

	switch (endian) {
	case TVOUT_BIG_ENDIAN_MODE:
		temp_reg |= S5P_MXR_BIG_ENDIAN_SOURCE_FORMAT;
		break;
	case TVOUT_LITTLE_ENDIAN_MODE:
		temp_reg |= S5P_MXR_LITTLE_ENDIAN_SOURCE_FORMAT;
		break;
	default:
		pr_err("[ERR] : invalid endian parameter = %d\n", endian);
		return S5P_TV_VMX_ERR_INVALID_PARAM;
		break;
	}

	writel(temp_reg, g_mixer_base + S5P_MXR_STATUS);

	VMPRINTK("burst(%d), endian(%d) S5P_MXR_STATUS(0x%08x)\n",
		burst, endian, readl(g_mixer_base + S5P_MXR_STATUS));

	return VMIXER_NO_ERROR;
}

enum s5p_tv_vmx_err s5p_vm_init_display_mode(enum s5p_tv_disp_mode mode,
	enum s5p_tv_o_mode output_mode)
{
	u32 temp_reg = readl(g_mixer_base + S5P_MXR_CFG);
	VMPRINTK("(%d, %d)\n", mode, output_mode);

	switch (mode) {
	case TVOUT_NTSC_M:
	case TVOUT_NTSC_443:
		temp_reg &= ~S5P_MXR_HD;
		temp_reg &= ~S5P_MXR_PAL;
		temp_reg &= S5P_MXR_INTERLACE_MODE;
		break;
	case TVOUT_PAL_BDGHI:
	case TVOUT_PAL_M:
	case TVOUT_PAL_N:
	case TVOUT_PAL_NC:
	case TVOUT_PAL_60:
		temp_reg &= ~S5P_MXR_HD;
		temp_reg |= S5P_MXR_PAL;
		temp_reg &= S5P_MXR_INTERLACE_MODE;
		break;
	case TVOUT_480P_60_16_9:
	case TVOUT_480P_60_4_3:
	case TVOUT_480P_59:
		temp_reg &= ~S5P_MXR_HD;
		temp_reg &= ~S5P_MXR_PAL;
		temp_reg |= S5P_MXR_PROGRESSVE_MODE;
		temp_reg |= RGB601_16_235<<9;
		break;
	case TVOUT_576P_50_16_9:
	case TVOUT_576P_50_4_3:
		temp_reg &= ~S5P_MXR_HD;
		temp_reg |= S5P_MXR_PAL;
		temp_reg |= S5P_MXR_PROGRESSVE_MODE;
		temp_reg |= RGB601_16_235<<9;
		break;
	case TVOUT_720P_50:
	case TVOUT_720P_59:
	case TVOUT_720P_60:
		temp_reg |= S5P_MXR_HD;
		temp_reg &= ~S5P_MXR_HD_1080I_MODE;
		temp_reg |= S5P_MXR_PROGRESSVE_MODE;
		temp_reg |= RGB709_16_235<<9;
		break;
	case TVOUT_1080I_50:
	case TVOUT_1080I_59:
	case TVOUT_1080I_60:
		temp_reg |= S5P_MXR_HD;
		temp_reg |= S5P_MXR_HD_1080I_MODE;
		temp_reg &= S5P_MXR_INTERLACE_MODE;
		temp_reg |= RGB709_16_235<<9;
		break;

	case TVOUT_1080P_50:
	case TVOUT_1080P_59:
	case TVOUT_1080P_60:
	case TVOUT_1080P_30:
		temp_reg |= S5P_MXR_HD;
		temp_reg |= S5P_MXR_HD_1080P_MODE;
		temp_reg |= S5P_MXR_PROGRESSVE_MODE;
		temp_reg |= RGB709_16_235<<9;
		break;
	default:
		pr_err(" invalid mode parameter = %d\n", mode);
		return S5P_TV_VMX_ERR_INVALID_PARAM;
		break;
	}

	switch (output_mode) {
	case TVOUT_OUTPUT_COMPOSITE:
	case TVOUT_OUTPUT_SVIDEO:
	case TVOUT_OUTPUT_COMPONENT_YPBPR_INERLACED:
	case TVOUT_OUTPUT_COMPONENT_YPBPR_PROGRESSIVE:
	case TVOUT_OUTPUT_COMPONENT_RGB_PROGRESSIVE:
		temp_reg &= S5P_MXR_DST_SEL_ANALOG;
		break;
	case TVOUT_OUTPUT_HDMI_RGB:
	case TVOUT_OUTPUT_DVI:
		temp_reg |= S5P_MXR_DST_SEL_HDMI;
		temp_reg &= ~(0x1<<8);
		temp_reg |= MX_RGB888<<8;
		break;
	case TVOUT_OUTPUT_HDMI:
		temp_reg |= S5P_MXR_DST_SEL_HDMI;
		temp_reg &= ~(0x1<<8);
		temp_reg |= MX_YUV444<<8;
		break;
	default:
		pr_err(" invalid mode parameter = %d\n", mode);
		return S5P_TV_VMX_ERR_INVALID_PARAM;
		break;
	}

	writel(temp_reg, g_mixer_base + S5P_MXR_CFG);

	VMPRINTK("mode(%d), output_mode(%d), S5P_MXR_CFG(0x%08x)\n",
		mode, output_mode, readl(g_mixer_base + S5P_MXR_CFG));

	return VMIXER_NO_ERROR;
}

u32 grp_scaling_factor(u32 src, u32 dst, u32 h_v)
{
	u32 factor; /* for scaling factor */

	/* check scale or not */
	if (src == dst)
		factor = 0;

	if (dst % src) {
		factor = 0;

		VMPRINTK(" can't %s scaling src(%d) into dst(%d)\n"
			, h_v ? "horizontal" : "vertical"
			, src, dst);
		VMPRINTK(" scaling vector must be 2/4/8x\n");
	}

	factor = dst / src;

	switch (factor) {
	case 2:
		factor = 1;
		break;
	case 4:
		factor = 2;
		break;
	case 8:
		factor = 3;
		break;
	default:
		pr_err(" scaling vector must be 2/4/8x\n");
		factor = 0;
		break;
	}

	return factor;
}

void s5p_vm_set_ctrl(enum s5p_tv_vmx_layer layer,
			bool premul,
			bool pixel_blending,
			bool blank_change,
			bool win_blending,
			enum s5p_tv_vmx_color_fmt color,
			u32 alpha, u32 blank_color)
{
	u32 reg = readl(g_mixer_base + S5P_MXR_GRAPHIC0_CFG);

	if (blank_change)
		reg &= ~S5P_MXR_BLANK_CHANGE_NEW_PIXEL;
	else
		reg |= S5P_MXR_BLANK_CHANGE_NEW_PIXEL;

	if (premul)
		reg |= S5P_MXR_PRE_MUL_MODE;
	else
		reg &= ~S5P_MXR_PRE_MUL_MODE;

	if (win_blending)
		reg |= S5P_MXR_WIN_BLEND_ENABLE;
	else
		reg &= ~S5P_MXR_WIN_BLEND_ENABLE;

	reg &= ~S5P_MXR_EG_COLOR_FORMAT(0xf);
	reg |= S5P_MXR_EG_COLOR_FORMAT(color);
	reg |= S5P_MXR_GRP_ALPHA_VALUE(alpha);

	writel(reg, g_mixer_base + S5P_MXR_GRAPHIC0_CFG);
	writel(S5P_MXR_GPR_BLANK_COLOR(blank_color),
		g_mixer_base + S5P_MXR_GRAPHIC0_BLANK);
}

enum s5p_tv_vmx_err s5p_vm_init_layer(enum s5p_tv_disp_mode mode,
				enum s5p_tv_vmx_layer layer,
				bool show,
				bool win_blending,
				u32 alpha,
				u32 priority,
				enum s5p_tv_vmx_color_fmt color,
				bool blank_change,
				bool pixel_blending,
				bool premul,
				u32 blank_color,
				u32 base_addr,
				u32 span,
				u32 width,
				u32 height,
				u32 src_offs_x,
				u32 src_offs_y,
				u32 dst_offs_x,
				u32 dst_offs_y,
				u32 dst_width,
				u32 dst_height)
{
	u32 temp_reg = 0;
	u32 h_factor = 0, v_factor = 0;

	VMPRINTK("(%d, %d, %d, %d, %d, %d, %d, %d, %d, 0x%08x,"
		"0x%08x, %d, %d, %d, %d, %d, %d, %d)\n",
		 layer, show, win_blending, alpha, priority,
		 color, blank_change, pixel_blending, premul,
		 blank_color, base_addr, span, width, height,
		 src_offs_x, src_offs_y, dst_offs_x, dst_offs_y);

	switch (layer) {
	case VM_VIDEO_LAYER:
		temp_reg  = (win_blending) ? S5P_MXR_VP_BLEND_ENABLE : S5P_MXR_VP_BLEND_DISABLE;
		temp_reg |= S5P_MXR_VP_ALPHA_VALUE(alpha);
		/* temp yuv pxl limiter setting*/
		temp_reg &= ~(1<<17);
		writel(temp_reg, g_mixer_base + S5P_MXR_VIDEO_CFG);
		break;
	case VM_GPR0_LAYER:
		temp_reg  = (blank_change)   ? S5P_MXR_BLANK_NOT_CHANGE_NEW_PIXEL : S5P_MXR_BLANK_CHANGE_NEW_PIXEL;
		temp_reg |= (premul)         ? S5P_MXR_PRE_MUL_MODE               : S5P_MXR_NORMAL_MODE;
		temp_reg |= (win_blending)   ? S5P_MXR_WIN_BLEND_ENABLE           : S5P_MXR_WIN_BLEND_DISABLE;
		temp_reg |= (pixel_blending) ? S5P_MXR_PIXEL_BLEND_ENABLE         : S5P_MXR_PIXEL_BLEND_DISABLE;
		temp_reg |= S5P_MXR_EG_COLOR_FORMAT(color);
		temp_reg |= S5P_MXR_GRP_ALPHA_VALUE(alpha);

		writel(temp_reg, g_mixer_base + S5P_MXR_GRAPHIC0_CFG);
		writel(S5P_MXR_GPR_BLANK_COLOR(blank_color),
			g_mixer_base + S5P_MXR_GRAPHIC0_BLANK);

		VMPRINTK("S5P_MXR_GRAPHIC0_CFG(0x%08x), S5P_MXR_GRAPHIC0_BLANK(0x%08x)\n",
			readl(g_mixer_base + S5P_MXR_GRAPHIC0_CFG),
			readl(g_mixer_base + S5P_MXR_GRAPHIC0_BLANK));

		s5p_vm_set_grp_layer_size(layer, span, width, height,
					    src_offs_x, src_offs_y);

		s5p_vm_set_grp_base_address(layer, base_addr);
		s5p_vm_set_grp_layer_position(layer, dst_offs_x, dst_offs_y);

		temp_reg = readl(g_mixer_base + S5P_MXR_GRAPHIC0_WH);
		temp_reg &= ~((0x3 << 28) | (0x3 << 12));

		h_factor = grp_scaling_factor(width, dst_width, 1);
		v_factor = grp_scaling_factor(height, dst_height, 0);

		if (v_factor) {
			u32 reg = readl(g_mixer_base + S5P_MXR_CFG);

			/* In interlaced mode, vertical scaling must be
			 * replaced by PROGRESSIVE_MODE - pixel duplication
			 */
			if (mode == TVOUT_1080I_50 ||
			     mode == TVOUT_1080I_59 ||
			     mode == TVOUT_1080I_60) {
				/* scaled up by progressive setting */
				reg |= S5P_MXR_PROGRESSVE_MODE;
				writel(reg, g_mixer_base + S5P_MXR_CFG);
			} else
				/* scaled up by scale factor */
				temp_reg |= v_factor << 12;
		} else {
			u32 reg = readl(g_mixer_base + S5P_MXR_CFG);

			/*
			 * if v_factor is 0, recover the original mode
			 */
			if (mode == TVOUT_1080I_50 ||
			     mode == TVOUT_1080I_59 ||
			     mode == TVOUT_1080I_60) {
				reg &= S5P_MXR_INTERLACE_MODE;
				writel(reg, g_mixer_base + S5P_MXR_CFG);
			}
		}

		temp_reg |= h_factor << 28;

		writel(temp_reg , g_mixer_base + S5P_MXR_GRAPHIC0_WH);

		break;
	case VM_GPR1_LAYER:
		temp_reg  = (blank_change)   ? S5P_MXR_BLANK_NOT_CHANGE_NEW_PIXEL : S5P_MXR_BLANK_CHANGE_NEW_PIXEL;
		temp_reg |= (premul)         ? S5P_MXR_PRE_MUL_MODE               : S5P_MXR_NORMAL_MODE;
		temp_reg |= (win_blending)   ? S5P_MXR_WIN_BLEND_ENABLE           : S5P_MXR_WIN_BLEND_DISABLE;
		temp_reg |= (pixel_blending) ? S5P_MXR_PIXEL_BLEND_ENABLE         : S5P_MXR_PIXEL_BLEND_DISABLE;
		temp_reg |= S5P_MXR_EG_COLOR_FORMAT(color);
		temp_reg |= S5P_MXR_GRP_ALPHA_VALUE(alpha);

		writel(temp_reg, g_mixer_base + S5P_MXR_GRAPHIC1_CFG);
		writel(S5P_MXR_GPR_BLANK_COLOR(blank_color),
			g_mixer_base + S5P_MXR_GRAPHIC1_BLANK);

		VMPRINTK("S5P_MXR_GRAPHIC1_CFG(0x%x), S5P_MXR_GRAPHIC1_BLANK(0x%x)\n",
			readl(g_mixer_base + S5P_MXR_GRAPHIC1_CFG),
			readl(g_mixer_base + S5P_MXR_GRAPHIC1_BLANK));

		s5p_vm_set_grp_layer_size(layer, span, width, height,
					    src_offs_x, src_offs_y);

		s5p_vm_set_grp_base_address(layer, base_addr);
		s5p_vm_set_grp_layer_position(layer, dst_offs_x, dst_offs_y);

		temp_reg = readl(g_mixer_base + S5P_MXR_GRAPHIC1_WH);
		temp_reg &= ~((0x3 << 28) | (0x3 << 12));
		h_factor = grp_scaling_factor(width, dst_width, 1);
		v_factor = grp_scaling_factor(height, dst_height, 0);

		if (v_factor) {
			u32 reg = readl(g_mixer_base + S5P_MXR_CFG);

			/* In interlaced mode, vertical scaling must be
			 * replaced by PROGRESSIVE_MODE - pixel duplication
			 */
			if (mode == TVOUT_1080I_50 ||
			     mode == TVOUT_1080I_59 ||
			     mode == TVOUT_1080I_60) {
				/* scaled up by progressive setting */
				reg |= S5P_MXR_PROGRESSVE_MODE;
				writel(reg, g_mixer_base + S5P_MXR_CFG);
			} else
				/* scaled up by scale factor */
				temp_reg |= v_factor << 12;
		} else {
			u32 reg = readl(g_mixer_base + S5P_MXR_CFG);

			/*
			 * if v_factor is 0, recover the original mode
			 */
			if (mode == TVOUT_1080I_50 ||
			     mode == TVOUT_1080I_59 ||
			     mode == TVOUT_1080I_60) {
				reg &= S5P_MXR_INTERLACE_MODE;
				writel(reg, g_mixer_base + S5P_MXR_CFG);
			}
		}

		temp_reg |= h_factor << 28;

		writel(temp_reg , g_mixer_base + S5P_MXR_GRAPHIC1_WH);
		break;
	default:
		pr_err("invalid layer parameter = %d\n", layer);
		return S5P_TV_VMX_ERR_INVALID_PARAM;
		break;
	}

	s5p_vm_set_layer_priority(layer, priority);

	s5p_vm_set_layer_show(layer, show);

	return VMIXER_NO_ERROR;
}

void s5p_vm_init_bg_dither_enable(bool cr_dither_enable,
				  bool cb_dither_enable,
				  bool y_dither_enable)
{
	u32 temp_reg = 0;

	temp_reg = (cr_dither_enable) ? (temp_reg | S5P_MXR_BG_CR_DIHER_EN) : (temp_reg & ~S5P_MXR_BG_CR_DIHER_EN);
	temp_reg = (cb_dither_enable) ? (temp_reg | S5P_MXR_BG_CB_DIHER_EN) : (temp_reg & ~S5P_MXR_BG_CB_DIHER_EN);
	temp_reg = (y_dither_enable)  ? (temp_reg | S5P_MXR_BG_Y_DIHER_EN)  : (temp_reg & ~S5P_MXR_BG_Y_DIHER_EN);

	writel(temp_reg, g_mixer_base + S5P_MXR_BG_CFG);

	VMPRINTK("cr_dither_enable(%d), cb_dither_enable(%d)\n",
		cr_dither_enable, cb_dither_enable, y_dither_enable);
	VMPRINTK("y_dither_enable(%d), S5P_MXR_BG_CFG(0x%08x)\n",
		readl(g_mixer_base + S5P_MXR_BG_CFG));
}


enum s5p_tv_vmx_err s5p_vm_init_bg_color(
	enum s5p_tv_vmx_bg_color_num color_num,
	u32 color_y,
	u32 color_cb,
	u32 color_cr)
{
	return s5p_vm_set_bg_color(color_num, color_y, color_cb, color_cr);
}

enum s5p_tv_vmx_err s5p_vm_init_csc_coef(enum s5p_yuv_fmt_component component,
					    enum s5p_tv_coef_y_mode mode,
					    u32 coeff0,
					    u32 coeff1,
					    u32 coeff2)
{
	u32 mxr_cm;

	VMPRINTK("%d, %d, %d, %d, %d\n", component, mode, coeff0, coeff1,
		coeff2);

	switch (component) {
	case TVOUT_YUV_Y:
		mxr_cm	= (mode == VMIXER_COEF_Y_WIDE) ?
			     S5P_MXR_BG_COLOR_WIDE : S5P_MXR_BG_COLOR_NARROW;
		mxr_cm |= S5P_MXR_BG_COEFF_0(coeff0) |
			     S5P_MXR_BG_COEFF_1(coeff1) |
			     S5P_MXR_BG_COEFF_2(coeff2);
		writel(mxr_cm, g_mixer_base + S5P_MXR_CM_COEFF_Y);
		VMPRINTK("S5P_MXR_CM_COEFF_Y(0x%x)\n",
			readl(g_mixer_base + S5P_MXR_CM_COEFF_Y));
		break;
	case TVOUT_YUV_CB:
		mxr_cm	= S5P_MXR_BG_COEFF_0(coeff0) |
			     S5P_MXR_BG_COEFF_1(coeff1) |
			     S5P_MXR_BG_COEFF_2(coeff2);
		writel(mxr_cm, g_mixer_base + S5P_MXR_CM_COEFF_CB);
		VMPRINTK("S5P_MXR_CM_COEFF_CB(0x%x)\n",
			readl(g_mixer_base + S5P_MXR_CM_COEFF_CB));
		break;
	case TVOUT_YUV_CR:
		mxr_cm	= S5P_MXR_BG_COEFF_0(coeff0) |
			     S5P_MXR_BG_COEFF_1(coeff1) |
			     S5P_MXR_BG_COEFF_2(coeff2);
		writel(mxr_cm, S5P_MXR_CM_COEFF_CR);
		VMPRINTK("S5P_MXR_CM_COEFF_CR(0x%x)\n",
			readl(g_mixer_base + S5P_MXR_CM_COEFF_CR));
		break;
	default:
		pr_err("invalid component parameter = %d\n", component);
		return S5P_TV_VMX_ERR_INVALID_PARAM;
		break;
	}

	return VMIXER_NO_ERROR;
}

void s5p_vm_init_csc_coef_default(enum s5p_tv_vmx_csc_type csc_type)
{
	u32 coeff_y;
	u32 coeff_cb;
	u32 coef_cr;

	coeff_y  = 0;
	coeff_cb = 0;
	coef_cr  = 0;

	VMPRINTK("csc_type: %d\n", csc_type);

	switch (csc_type) {
	case VMIXER_CSC_RGB_TO_YUV601_LR:
		coeff_y  = ((0   << 30) | (153 << 20) | (300 << 10) | (58 << 0));
		coeff_cb = ((936 << 20) | (851 << 10) | (262 << 0));
		coef_cr  = ((262 << 20) | (805 << 10) | (982 << 0));
		break;
	case VMIXER_CSC_RGB_TO_YUV601_FR:
		coeff_y  = ((1   << 30) | (132 << 20) | (258 << 10) | (50 << 0));
		coeff_cb = ((948 << 20) | (875 << 10) | (225 << 0));
		coef_cr  = ((225 << 20) | (836 << 10) | (988 << 0));
		break;
	case VMIXER_CSC_RGB_TO_YUV709_LR:
		coeff_y  = ((0   << 30) | (109 << 20) | (366  << 10) | (36 << 0));
		coeff_cb = ((964 << 20) | (822 << 10) | (216  << 0));
		coef_cr  = ((262 << 20) | (787 << 10) | (1000 << 0));
		break;
	case VMIXER_CSC_RGB_TO_YUV709_FR:
		coeff_y  = ((1   << 30) | (94  << 20) | (314  << 10) | (32 << 0));
		coeff_cb = ((972 << 20) | (851 << 10) | (225  << 0));
		coef_cr  = ((225 << 20) | (820 << 10) | (1004 << 0));
		break;
	default:
		pr_err(" invalid csc_type parameter = %d\n", csc_type);
		break;
	}

	writel(coeff_y,	 g_mixer_base + S5P_MXR_CM_COEFF_Y);
	writel(coeff_cb, g_mixer_base + S5P_MXR_CM_COEFF_CB);
	writel(coef_cr,  g_mixer_base + S5P_MXR_CM_COEFF_CR);

	VMPRINTK("COEFF_Y(0x%08x), COEFF_CB(0x%08x), COEFF_CR(0x%08x)\n",
		readl(g_mixer_base + S5P_MXR_CM_COEFF_Y),
		readl(g_mixer_base + S5P_MXR_CM_COEFF_CB),
		readl(g_mixer_base + S5P_MXR_CM_COEFF_CR));
}

/*
* etc
*/
enum s5p_tv_vmx_err s5p_vm_get_layer_info(enum s5p_tv_vmx_layer layer,
				       bool *show,
				       u32 *priority)
{
	VMPRINTK("%d\n", layer);

	switch (layer) {
	case VM_VIDEO_LAYER:
		*show = (readl(g_mixer_base + S5P_MXR_LAYER_CFG) &
			S5P_MXR_VIDEO_LAYER_SHOW) ? 1 : 0;
		*priority = S5P_MXR_VP_LAYER_PRIORITY_INFO(
			readl(g_mixer_base + S5P_MXR_LAYER_CFG));
		break;
	case VM_GPR0_LAYER:
		*show = (readl(g_mixer_base + S5P_MXR_LAYER_CFG) &
			S5P_MXR_GRAPHIC0_LAYER_SHOW) ? 1 : 0;
		*priority = S5P_MXR_GRP0_LAYER_PRIORITY_INFO(
			readl(g_mixer_base + S5P_MXR_LAYER_CFG));
		break;
	case VM_GPR1_LAYER:
		*show = (readl(g_mixer_base + S5P_MXR_LAYER_CFG) &
			S5P_MXR_GRAPHIC1_LAYER_SHOW) ? 1 : 0;
		*priority = S5P_MXR_GRP1_LAYER_PRIORITY_INFO(
			readl(g_mixer_base + S5P_MXR_LAYER_CFG));
		break;
	default:
		pr_err("invalid layer parameter = %d\n", layer);
		return S5P_TV_VMX_ERR_INVALID_PARAM;
		break;
	}

	VMPRINTK("layer(%d), *show(%d), *priority(%d)\n",
		layer, *show, *priority);

	return VMIXER_NO_ERROR;
}

/*
* start  - start functions are only called under stopping vmixer
*/
void s5p_vm_start(void)
{
	VMPRINTK("()\n");
	writel((readl(g_mixer_base + S5P_MXR_STATUS) | S5P_MXR_MIXER_START),
		g_mixer_base + S5P_MXR_STATUS);

	VMPRINTK("S5P_MXR_STATUS \t\t 0x%08x\n",
		readl(g_mixer_base + S5P_MXR_STATUS));
	VMPRINTK("S5P_MXR_INT_EN \t\t 0x%08x\n",
		readl(g_mixer_base + S5P_MXR_INT_EN));
	VMPRINTK("S5P_MXR_BG_CFG \t\t 0x%08x\n",
		readl(g_mixer_base + S5P_MXR_BG_CFG));
	VMPRINTK("S5P_MXR_BG_COLOR0 \t\t 0x%08x\n",
		readl(g_mixer_base + S5P_MXR_BG_COLOR0));
	VMPRINTK("S5P_MXR_BG_COLOR1 \t\t 0x%08x\n",
		readl(g_mixer_base + S5P_MXR_BG_COLOR1));
	VMPRINTK("S5P_MXR_BG_COLOR2 \t\t 0x%08x\n",
		readl(g_mixer_base + S5P_MXR_BG_COLOR2));
	VMPRINTK("S5P_MXR_CM_COEFF_Y \t\t 0x%08x\n",
		readl(g_mixer_base + S5P_MXR_CM_COEFF_Y));
	VMPRINTK("S5P_MXR_CM_COEFF_CB \t\t 0x%08x\n",
		readl(g_mixer_base + S5P_MXR_CM_COEFF_CB));
	VMPRINTK("S5P_MXR_CM_COEFF_CR \t\t 0x%08x\n",
		readl(g_mixer_base + S5P_MXR_CM_COEFF_CR));
	VMPRINTK("S5P_MXR_CM_COEFF_Y \t\t 0x%08x\n",
		readl(g_mixer_base + S5P_MXR_CM_COEFF_Y));
	VMPRINTK("S5P_MXR_CM_COEFF_CB \t\t 0x%08x\n",
		readl(g_mixer_base + S5P_MXR_CM_COEFF_CB));
	VMPRINTK("S5P_MXR_CM_COEFF_CR \t\t 0x%08x\n",
		readl(g_mixer_base + S5P_MXR_CM_COEFF_CR));
	VMPRINTK("S5P_MXR_GRAPHIC0_CFG \t 0x%08x\n",
		readl(g_mixer_base + S5P_MXR_GRAPHIC0_CFG));
	VMPRINTK("S5P_MXR_GRAPHIC0_BASE \t 0x%08x\n",
		readl(g_mixer_base + S5P_MXR_GRAPHIC0_BASE));
	VMPRINTK("S5P_MXR_GRAPHIC0_SPAN \t 0x%08x\n",
		readl(g_mixer_base + S5P_MXR_GRAPHIC0_SPAN));
	VMPRINTK("S5P_MXR_GRAPHIC0_WH \t\t 0x%08x\n",
		readl(g_mixer_base + S5P_MXR_GRAPHIC0_WH));
	VMPRINTK("S5P_MXR_GRAPHIC0_SXY \t 0x%08x\n",
		readl(g_mixer_base + S5P_MXR_GRAPHIC0_SXY));
	VMPRINTK("S5P_MXR_GRAPHIC0_DXY \t 0x%08x\n",
		readl(g_mixer_base + S5P_MXR_GRAPHIC0_DXY));
	VMPRINTK("S5P_MXR_GRAPHIC0_BLANK \t 0x%08x\n",
		readl(g_mixer_base + S5P_MXR_GRAPHIC0_BLANK));
	VMPRINTK("S5P_MXR_GRAPHIC1_BASE \t 0x%08x\n",
		readl(g_mixer_base + S5P_MXR_GRAPHIC1_BASE));
	VMPRINTK("S5P_MXR_GRAPHIC1_SPAN \t 0x%08x\n",
		readl(g_mixer_base + S5P_MXR_GRAPHIC1_SPAN));
	VMPRINTK("S5P_MXR_GRAPHIC1_WH \t\t 0x%08x\n",
		readl(g_mixer_base + S5P_MXR_GRAPHIC1_WH));
	VMPRINTK("S5P_MXR_GRAPHIC1_SXY \t 0x%08x\n",
		readl(g_mixer_base + S5P_MXR_GRAPHIC1_SXY));
	VMPRINTK("S5P_MXR_GRAPHIC1_DXY \t 0x%08x\n",
		readl(g_mixer_base + S5P_MXR_GRAPHIC1_DXY));
	VMPRINTK("S5P_MXR_GRAPHIC1_BLANK \t 0x%08x\n",
		readl(g_mixer_base + S5P_MXR_GRAPHIC1_BLANK));
	VMPRINTK("S5P_MXR_CFG \t\t\t 0x%08x\n",
		readl(g_mixer_base + S5P_MXR_CFG));
	VMPRINTK("S5P_MXR_LAYER_CFG \t\t 0x%08x\n",
		readl(g_mixer_base + S5P_MXR_LAYER_CFG));
}

/*
* stop  - stop functions are only called under running vmixer
*/
void s5p_vm_stop(void)
{
	int time_out = HDMI_TIME_OUT;

	u32 reg = readl(g_mixer_base + S5P_MXR_STATUS);

	reg &= ~S5P_MXR_MIXER_START;

	writel(reg, g_mixer_base + S5P_MXR_STATUS);

	do {
		reg = readl(g_mixer_base + S5P_MXR_STATUS);
		time_out--;
	} while (reg & S5P_MXR_MIXER_START && time_out);

	if (time_out <= 0)
		pr_err("readl(S5P_MXR_STATUS) for S5P_MXR_MIXER_START fail\n");
}

/*
* interrupt - for debug
*/
enum s5p_tv_vmx_err s5p_vm_set_underflow_interrupt_enable(
	enum s5p_tv_vmx_layer layer, bool en)
{
	u32 enablemaks;

	VMPRINTK("%d, %d\n", layer, en);

	switch (layer) {
	case VM_VIDEO_LAYER:
		enablemaks = S5P_MXR_VP_INT_ENABLE;
		break;
	case VM_GPR0_LAYER:
		enablemaks = S5P_MXR_GRP0_INT_ENABLE;
		break;
	case VM_GPR1_LAYER:
		enablemaks = S5P_MXR_GRP1_INT_ENABLE;
		break;
	default:
		pr_err("invalid layer parameter = %d\n", layer);
		return S5P_TV_VMX_ERR_INVALID_PARAM;
		break;
	}

	if (en) {
		writel((readl(g_mixer_base + S5P_MXR_INT_EN) | enablemaks),
			g_mixer_base + S5P_MXR_INT_EN);
	} else {
		writel((readl(g_mixer_base + S5P_MXR_INT_EN) & ~enablemaks),
			g_mixer_base + S5P_MXR_INT_EN);
	}

	VMPRINTK("layer(%d), en(%d), S5P_MXR_INT_EN(0x%08x)\n",
		layer, en,
		readl(g_mixer_base + S5P_MXR_INT_EN));

	return VMIXER_NO_ERROR;
}

void s5p_vm_clear_pend_all(void)
{
	writel(S5P_MXR_INT_FIRED |
		S5P_MXR_VP_INT_FIRED |
		S5P_MXR_GRP0_INT_FIRED |
		S5P_MXR_GRP1_INT_FIRED,
		g_mixer_base + S5P_MXR_INT_STATUS);
}

irqreturn_t s5p_vmixer_irq(int irq, void *dev_id)
{
	bool v_i_f;
	bool g0_i_f;
	bool g1_i_f;
	bool mxr_i_f;
	u32 temp_reg = 0;

	v_i_f = (readl(g_mixer_base + S5P_MXR_INT_STATUS)
			& S5P_MXR_VP_INT_FIRED) ? true : false;
	g0_i_f = (readl(g_mixer_base + S5P_MXR_INT_STATUS)
			& S5P_MXR_GRP0_INT_FIRED) ? true : false;
	g1_i_f = (readl(g_mixer_base + S5P_MXR_INT_STATUS)
			& S5P_MXR_GRP1_INT_FIRED) ? true : false;
	mxr_i_f = (readl(g_mixer_base + S5P_MXR_INT_STATUS)
			& S5P_MXR_INT_FIRED) ? true : false;

	if (mxr_i_f) {
		temp_reg |= S5P_MXR_INT_FIRED;

		if (v_i_f) {
			temp_reg |= S5P_MXR_VP_INT_FIRED;
			pr_err("VP fifo under run!!\n") ;
		}

		if (g0_i_f) {
			temp_reg |= S5P_MXR_GRP0_INT_FIRED;
			pr_err("GRP0 fifo under run!!\n");
		}

		if (g1_i_f) {
			temp_reg |= S5P_MXR_GRP1_INT_FIRED;
			pr_err("GRP1 fifo under run!!\n");
		}

		writel(temp_reg, g_mixer_base + S5P_MXR_INT_STATUS);
	}

	return IRQ_HANDLED;
}

int __init s5p_vmixer_probe(struct platform_device *pdev, u32 res_num)
{
	struct resource *res;
	size_t	size;

	res = platform_get_resource(pdev, IORESOURCE_MEM, res_num);
	if (res == NULL) {
		dev_err(&pdev->dev,
			"failed to get memory region resource\n");
		goto error;

	}

	size = (res->end - res->start) + 1;

	g_mixer_mem = request_mem_region(res->start, size, pdev->name);
	if (g_mixer_mem == NULL) {
		dev_err(&pdev->dev,
			"failed to get memory region\n");
		goto error;

	}

	g_mixer_base = ioremap(res->start, size);
	if (g_mixer_base == NULL) {
		dev_err(&pdev->dev,
			"failed to ioremap address region\n");
		goto error_ioremap_fail;
	}

	return 0;

error_ioremap_fail:
	release_resource(g_mixer_mem);
	kfree(g_mixer_mem);
	g_mixer_mem = NULL;
error:
	return -ENOENT;

}

int __init s5p_vmixer_release(struct platform_device *pdev)
{
	if (g_mixer_base) {
		iounmap(g_mixer_base);
		g_mixer_base = NULL;
	}

	if (g_mixer_mem) {
		if (release_resource(g_mixer_mem))
			dev_err(&pdev->dev,
				"Can't remove tvout drv !!\n");

		kfree(g_mixer_mem);
		g_mixer_mem = NULL;
	}

	return 0;
}
