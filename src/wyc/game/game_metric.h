#ifndef __HEADER_WYC_GAME_METRIC
#define __HEADER_WYC_GAME_METRIC

namespace wyc
{

struct xgame_metric
{
	unsigned long m_frame_count;
	unsigned long m_tick_count;
	double m_fps;
	
	unsigned long m_frame_count_last;
	unsigned long m_tick_skipped_last;
	double m_frame_time_last;
	double m_cosumed_time_last;
	double m_render_time_last;
	double m_logic_time_last;
	xgame_metric()
	{
		m_frame_count=0;
		m_tick_count=0;
		m_fps=0;

		m_frame_time_last=0;
		m_frame_count_last=0;
		m_render_time_last=0;
		m_cosumed_time_last=0;
		m_logic_time_last=0;
		m_tick_skipped_last=0;
	}
};

};

#endif // __HEADER_WYC_GAME_METRIC
