#ifndef __HEADER_WYC_MOVE
#define __HEADER_WYC_MOVE

#include "collision_detector.h"
#include "recorder.h"

namespace wyc
{

class xmove
{
	xcollision_agent *m_agent;
	xcollision_agent *m_terrain;
	float m_inflections[3];
	unsigned char m_cursec, m_cntsec;
	char m_apex;
	std::vector<float> m_kpos;
public:
	// 地形碰撞组
	static unsigned ms_terrain_filter;
	// 平台碰撞组
	static unsigned ms_platform_filter;
	// 可站立碰撞组
	static unsigned ms_standon_filter;
	// 身体离障碍物的距离,避免因浮点误差导致穿插
	static float ms_drawback;
	// 小于该高度的平台视为可以直接登上的台阶
	static float ms_stair_height;
	// 只有倾斜度低于该值的斜坡才能行走
	// tan(30度)=0.577; tan(45度)=1; tan(60度)=1.732
	static float ms_slope_can_stand;
	// 法线过滤, 默认88度～92度
	// 用于对空中平台的碰撞进行法线过滤，如果碰撞法线在该区间内，则认为发生碰撞
	static xvec2f_t ms_min_dir;
	static xvec2f_t ms_max_dir;
	// 记录运行数据, 用于DEBUG
	static xrecorder ms_rec;

	static void init_move_env(unsigned terrain_filter, unsigned platform_filter);
		
	xmove ();
	inline void set_agent(xcollision_agent *agent) {
		m_agent = agent;
	}
	inline xcollision_agent* get_agent() {
		return m_agent;
	}
	inline bool is_on_ground() const {
		return m_terrain!=0;
	}
	bool stick_on_ground ();
	void set_terrain (xcollision_agent *terrain);
	xcollision_agent* get_terrain() const {
		return m_terrain;
	}
	inline void leave_terrain () {
		m_terrain = 0;
	}
	const std::vector<float>& get_kpos() const {
		return m_kpos;
	}
	void clear_kpos() {
		m_kpos.clear();
	}
	const wyc::xrecorder& get_record() const {
		return ms_rec;
	}
	xcollision_agent* walk (float &t, float spdx, unsigned extra_filter=0, unsigned max_iteration=3);
	xcollision_agent* fall (float &t, xvec2f_t &speed, float accy, unsigned extra_filter=0, unsigned max_iteration=3);
	xcollision_agent* jump (float &t, xvec2f_t &speed, float accy, unsigned extra_filter=0, unsigned max_iteration=3);
	xcollision_agent* fly  (float &t, const xvec2f_t speed, unsigned extra_filter=0);

protected:
	xcollision_agent* _detect_collision (const xvec2f_t& offset, unsigned filter, float &dt, xvec2f_t &normal, unsigned normal_filter=0);
	xcollision_agent* _detect_stand_point (xvec2f_t &pos);
}; // class xmove

}// namespace wyc

#endif // __HEADER_WYC_MOVE