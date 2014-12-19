#include "basedef.h"
#include "driver.h"

#if NATIVE_PIXEL_FORMAT==PIXEL_FMT_RGBA8888
// assert(NATIVE_PIXEL_FORMAT==PIXEL_FMT_RGBA8888)

#include "rgba8888.inl"

unsigned g_pixel_size[NUM_PIXEL_FORMAT]={
	PIXEL_SIZE(PIXEL_FMT_GRAY8),
	PIXEL_SIZE(PIXEL_FMT_GRAY16),
	PIXEL_SIZE(PIXEL_FMT_INDEX8),
	PIXEL_SIZE(PIXEL_FMT_INDEX16),
	PIXEL_SIZE(PIXEL_FMT_RGB555),
	PIXEL_SIZE(PIXEL_FMT_RGB565),
	PIXEL_SIZE(PIXEL_FMT_RGB888),
	PIXEL_SIZE(PIXEL_FMT_RGBA8888),
	PIXEL_SIZE(PIXEL_FMT_RGBA16),
	PIXEL_SIZE(PIXEL_FMT_RGBA32),
	PIXEL_SIZE(PIXEL_FMT_RED),
	PIXEL_SIZE(PIXEL_FMT_GREEN),
	PIXEL_SIZE(PIXEL_FMT_BLUE),
	PIXEL_SIZE(PIXEL_FMT_ALPHA),
	PIXEL_SIZE(PIXEL_FMT_RGBA32F),
	PIXEL_SIZE(PIXEL_FMT_RED32F),
	PIXEL_SIZE(PIXEL_FMT_GREEN32F),
	PIXEL_SIZE(PIXEL_FMT_BLUE32F),
	PIXEL_SIZE(PIXEL_FMT_ALPHA32F),
};

xplotter g_translater[NUM_PIXEL_FORMAT]={
	gray8_to_rgba8888,
	gray16_to_rgba8888,
	index8_to_rgba8888,
	index16_to_rgba8888,
	rgb555_to_rgba8888,
	rgb565_to_rgba8888,
	rgb888_to_rgba8888,
	rgba8888_to_rgba8888,
	rgba16_to_rgba8888,
	rgba32_to_rgba8888,
	red_to_rgba8888,
	green_to_rgba8888,
	blue_to_rgba8888,
	alpha_to_rgba8888,
	rgba32f_to_rgba8888,
	red32f_to_rgba8888,
	green32f_to_rgba8888,
	blue32f_to_rgba8888,
	alpha32f_to_rgba8888,
};

xplotter g_reverse_translater[NUM_PIXEL_FORMAT]={
	rgba8888_to_gray8,
	rgba8888_to_gray16,
	rgba8888_to_index8,
	rgba8888_to_index16,
	rgba8888_to_rgb555,
	rgba8888_to_rgb565,
	rgba8888_to_rgb888,
	rgba8888_to_rgba8888,
	rgba8888_to_rgba16,
	rgba8888_to_rgba32,
	rgba8888_to_red,
	rgba8888_to_green,
	rgba8888_to_blue,
	rgba8888_to_alpha,
	rgba8888_to_rgba32f,
	rgba8888_to_red32f,
	rgba8888_to_green32f,
	rgba8888_to_blue32f,
	rgba8888_to_alpha32f,
};

xplotter g_plotmode[NUM_PLOT_MODE]={
	plot_replace_rgba8888,
	plot_red_rgba8888,
	plot_green_rgba8888,
	plot_blue_rgba8888,
	plot_alpha_rgba8888,
	plot_add_rgba8888,
	plot_and_rgba8888,
	plot_inverse_rgba8888,
	plot_not_rgba8888,
	plot_or_rgba8888,
	plot_sub_rgba8888,
	plot_xor_rgba8888,
	plot_blend_rgba8888,
};

#endif
