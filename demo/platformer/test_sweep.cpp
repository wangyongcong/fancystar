#include <vector>
#include "wyc/util/util.h"
#include "collision_detector.h"

using namespace wyc;

void test_sweep ()
{
	wyc_print("Sweep test");
}
/*
void random_test_sweep_intersect()
{
	wyc_print("Random test sweep interset");
	
	// initialize
	int map_size=512, grid_size=32;
	xquad_tree map;
	if(!map.initialize(grid_size,grid_size,map_size/grid_size,map_size/grid_size))
	{
		wyc_error("Failed to create collision map");
		return;
	}
	map.set_translate( xvec2f_t(float(-map_size),float(-map_size)) );

	xcollision_agent *agent;
	xcollision_agent all_agents[32];
	float rx=random()*32+16, ry=rx, x, y, r, rad;
	float min_dist = std::sqrt(rx*rx+ry*ry)*2,
		max_dist=std::sqrt(float(map_size*map_size+map_size*map_size));
	for(int i=0; i<32; ++i)
	{
		agent = all_agents+i;
		agent->set_mask(1<<i);
		agent->set_radius(rx,ry);
		rad = float(wyc::random()*XMATH_2PI);
		r = min_dist + random()*(max_dist-min_dist);
		x = std::cos(rad)*r;
		y = std::sin(rad)*r;
		agent->set_position(x,y);
		agent->update_aabb();
		map.add_entity(agent);
	}

	xcollision_agent me;
	me.set_radius(rx,ry);
	me.set_position(0,0);
	me.update_aabb();

	xvec2f_t dsts[360];
	for(int i=0; i<360; ++i)
	{
		rad = DEG_TO_RAD(i);
		x = max_dist*std::cos(rad);
		y = max_dist*std::sin(rad);
		dsts[i].set(x,y);
	}

	wyc_print("\tmap size: %dx%d",map_size,map_size);
	wyc_print("\tgrid size: %dx%d",grid_size,grid_size);
	wyc_print("\tLOD: %d",map.lod_count());	
	wyc_print("\tobj size: %.2fx%.2f",rx,ry);
	wyc_print("\tmin dist: %f",min_dist);
	wyc_print("\tmax dist: %f",max_dist);

	// sweep intersect
	printf("Testing");
	const char *reason = "no error";
	unsigned calls=0, collisions=0;
	float prev_dist;
	for(int i=0; i<360; ++i) {
		xsweep_intersect_detector si_detector(&me,dsts[i]);
		map.find_neighbors(me.get_position(),dsts[i],me.get_radius(),0xFFFFFFFF,si_detector);
		calls += 1;
		prev_dist = 0;
		for(unsigned j=0; j<si_detector.count(); ++j)
		{
			printf(".");
			agent = si_detector[j];
			xsweep_detector sweep_detector(&me,dsts[i]);
			map.find_neighbors(me.get_position(),dsts[i],me.get_radius(),agent->get_mask(),sweep_detector);
			xcollision_agent *tmp = sweep_detector.get_object();
			if(tmp!=agent) {
				reason = "sweep detector find wrong agents";
				goto TEST_FAIL;
			}
			if(sweep_detector.get_distance()<prev_dist)
			{
				reason = "results are not ordered";
				goto TEST_FAIL;
			}
			prev_dist = sweep_detector.get_distance();
			collisions+=1;
			agent->set_data((void*)1);
		}
		for(int k=0; k<32; ++k)
		{
			agent = all_agents+k;
			if(agent->get_data()==(void*)1) {
				agent->set_data(0);
				continue;
			}
			xsweep_detector sweep_detector(&me,dsts[i]);
			map.find_neighbors(me.get_position(),dsts[i],me.get_radius(),agent->get_mask(),sweep_detector);
			if(sweep_detector.get_object()!=0) {
				reason = "miss some agents";
				goto TEST_FAIL;
			}
		}
		continue;
TEST_FAIL:
		printf("\n");
		printf("reason: %s\n",reason);
		printf("radius: %f\n",agent->get_radius().x);
		printf("position: %f, %f\n",agent->get_position().x,agent->get_position().y);
		printf("move to[%d]: %f, %f\n",i,dsts[i].x,dsts[i].y);
		rx = agent->get_position().x-me.get_position().x;
		ry = agent->get_position().y-me.get_position().y;
		r  = std::sqrt(rx*rx+ry*ry);
		printf("distance: %f\n", r);
		assert(0);
	}
	printf("\n");
	printf("[%d] sweep intersect calls\n",calls);
	printf("[%d] collisions are detected\n",collisions);
}*/
