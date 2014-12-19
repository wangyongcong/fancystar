#ifndef __HEADER_GAME_CONFIG
#define __HEADER_GAME_CONFIG

// 地图格的尺寸
#define GRID_WIDTH  40
#define GRID_HEIGHT 40

// 纹理坐标缩放系数 
#define TILE_COORD_XSCALE 0.078125f // (GRID_WIDTH/512.0f)
#define TILE_COORD_YSCALE 0.078125f // (GRID_HEIGHT/512.0f)

// 滚屏速度
#define SCROLL_SPEED 0.6f

// 默认地形的纹理坐标
#define DEF_TEX_L 0.078125f	//  40/512
#define DEF_TEX_R 0.15625f	//  80/512
#define DEF_TEX_B 0.84375f	// 432/512
#define DEF_TEX_U 0.921875f	// 472/512

#endif //__HEADER_GAME_CONFIG
