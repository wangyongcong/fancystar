#include <cstdlib>
#include <cstdio>
#include <cassert>
#include <vector>

#include "wyc/util/util.h"
#include "wyc/util/hash.h"
#include "aoi3d.h"

using namespace wyc;

namespace TEST_WALK
{

#define PLAYER0D_COUNT 30
#define PLAYER1D_COUNT 30
#define PLAYER2D_COUNT 30
#define PLAYER3D_COUNT 60
#define FRAME_COUNT 10000
#define SCENE_SIZE 300

enum {
	GROUP_PLAYER = 1,
};

class xtest_player : public xaoi_entity
{
	xset m_nearby_players;
	xvec3f_t m_dir;
	float m_speed;
	xvec3f_t m_lower, m_upper;
	unsigned m_enter, m_leave;
public:
	xtest_player(const xvec3f_t &r) : xaoi_entity(r)
	{
		m_dir.x=wyc::random();
		m_dir.y=wyc::random();
		m_dir.z=wyc::random();
		m_dir.normalize();
		m_speed=wyc::random()+0.2f;
		set_mask(GROUP_PLAYER);
		set_filter(GROUP_PLAYER);
		m_enter=m_leave=0;
	}

	virtual void on_enter (xaoi_entity *en)
	{
		if(!m_nearby_players.add(xset::value_t(en)))
		{
			assert(0 && "obj already in my range");
		}
		m_enter+=1;
	}

	virtual void on_leave (xaoi_entity *en)
	{
		if(!m_nearby_players.del(xset::value_t(en)))
		{
			assert(0 && "obj not found");
		}
		m_leave+=1;
	}

	void walk()
	{
		xvec3f_t pos = get_pos() + m_dir*m_speed;
		for(int d=0; d<3; ++d)
		{
			if(pos.elem[d]<=0 || pos.elem[d]>=SCENE_SIZE)
				m_dir.elem[d]=-m_dir.elem[d];
		}
		move_to(pos);
		m_lower = get_pos()-get_radius();
		m_upper = get_pos()+get_radius();
	}

	void verify_visibility(xtest_player *pl)
	{
		bool is_overlap = !(m_lower.x>pl->m_upper.x || m_upper.x<pl->m_lower.x \
		|| m_lower.y>pl->m_upper.y || m_upper.y<pl->m_lower.y \
		|| m_lower.z>pl->m_upper.z || m_upper.z<pl->m_lower.z);

		assert(is_overlap == m_nearby_players.contain(xset::value_t(pl)));
	}

	unsigned enter_count() const {
		return m_enter;
	}

	unsigned leave_count() const {
		return m_leave;
	}
};

}

void test_walk_through ()
{
	using namespace TEST_WALK;

	printf("walk through test");

	const int PLAYER_COUNT = PLAYER0D_COUNT + PLAYER1D_COUNT + PLAYER2D_COUNT + PLAYER3D_COUNT;
	xaoi_manager mgr;
	std::vector<xtest_player*> all_players;
	all_players.reserve(PLAYER_COUNT);
	xtest_player *pl, *pl2;
	xvec3f_t v;
	int zero_idx;
	v.zero();
	for(int i=0; i<PLAYER0D_COUNT ; ++i)
	{
		pl = new xtest_player(v);
		all_players.push_back(pl);
	}
	for(int i=0; i<PLAYER1D_COUNT ; ++i)
	{
		v.x=wyc::random()+1;
		v.y=wyc::random()+1;
		v.z=wyc::random()+1;
		zero_idx = int(wyc::random()*10000)%3;
		v.elem[zero_idx]=0;
		v.elem[(zero_idx+1)%3]=0;
		pl = new xtest_player(v);
		all_players.push_back(pl);
	}
	for(int i=0; i<PLAYER2D_COUNT ; ++i)
	{
		v.x=wyc::random()+1;
		v.y=wyc::random()+1;
		v.z=wyc::random()+1;
		v.elem[int(wyc::random()*10000)%3]=0;
		pl = new xtest_player(v);
		all_players.push_back(pl);
	}
	for(int i=0; i<PLAYER3D_COUNT ; ++i)
	{
		v.x=wyc::random()+1;
		v.y=wyc::random()+1;
		v.z=wyc::random()+1;
		pl = new xtest_player(v);
		all_players.push_back(pl);
	}
	for(int i=0; i<PLAYER_COUNT; ++i)
	{
		v.x=wyc::random()*SCENE_SIZE;
		v.y=wyc::random()*SCENE_SIZE;
		v.z=wyc::random()*SCENE_SIZE;
		mgr.add(all_players[i],v);
	}
	
	int percent = FRAME_COUNT/10;

	for(int f=0; f<FRAME_COUNT; ++f)
	{
		for(int i=0; i<PLAYER_COUNT; ++i)
		{
			all_players[i]->walk();
		}
		for(int i=0; i<PLAYER_COUNT; ++i)
		{
			pl = all_players[i];
			for(int j=0; j<PLAYER_COUNT; ++j)
			{
				pl2=all_players[j];
				if(pl==pl2)
					continue;
				pl->verify_visibility(pl2);
			}
		}
		mgr._verify();

		percent-=1;
		if(percent==0)
		{
			percent=FRAME_COUNT/10;
			printf(".");
		}
	}
	printf("\n");

	unsigned max_enter=0, max_leave=0, total_enter=0, total_leave=0;
	for(int i=0; i<PLAYER_COUNT; ++i)
	{
		pl=all_players[i];
		if(pl->enter_count()>max_enter)
			max_enter=pl->enter_count();
		if(pl->leave_count()>max_leave)
			max_leave=pl->leave_count();
		total_enter += pl->enter_count();
		total_leave += pl->leave_count();
		delete pl;
	}
	all_players.clear();
	printf("    %d points, %d lines, %d planes, %d cubes\n",PLAYER0D_COUNT,PLAYER1D_COUNT,PLAYER2D_COUNT,PLAYER3D_COUNT);
	printf("    enter: %d, avg = %.2f, max = %d, frame = %.2f\n",total_enter,float(total_enter)/PLAYER_COUNT,max_enter,float(total_enter)/FRAME_COUNT);
	printf("    leave: %d, avg = %.2f, max = %d, frame = %.2f\n",total_leave,float(total_leave)/PLAYER_COUNT,max_leave,float(total_leave)/FRAME_COUNT);
}


