#include "fscorepch.h"
#include <fstream>
#include "scene.h"
#include "collision_detector.h"

namespace demo
{

REG_RTTI(xscene,xobject)

xscene::xscene()
{
	m_lod_view=0;
	m_map_vao=0;
	m_map_vbo=0;
	m_show_tree=false;
	m_show_parent_filled=false;
}

void xscene::on_destroy()
{
	m_map.clear();
	for(unsigned i=0, cnt=m_objects.size(); i<cnt; ++i)
		m_objects[i]->decref();
	m_objects.clear();
	if(m_map_vao) {
		glDeleteVertexArrays(1,&m_map_vao);
		m_map_vao=0;
	}
	if(m_map_vbo) {
		glDeleteBuffers(1,&m_map_vbo);
		m_map_vbo=0;
	}
}


void xscene::create(float scn_w, float scn_h)
{
	unsigned map_w = unsigned(std::ceil(scn_w)), map_h = unsigned(std::ceil(scn_h));
	if(map_w<512)
		map_w=512;
	if(map_h<512)
		map_h=512;
	if(!m_map.initialize(64,64,(map_h+63)>>6,(map_w+63)>>6))
	{
		wyc_error("failed to initialize quad tree: map size %dx%d, grid size 64x64",map_w,map_h);
		return;
	}
	m_lod_view=0;
	_create_scene_mesh();
}

void xscene::update(float frame_time)
{
	
}

void xscene::draw(wyc::xrenderer *rd)
{
	if(m_show_tree && m_map_vao!=0) 
	{
		glColor3f(0.2f,0,0);
		_draw_filled_grids();
		glColor3f(1,1,1);
		glBindVertexArray(m_map_vao);
			if(m_lod_view<m_map_draw.size())
			{
				glDrawArrays(GL_LINES,m_map_draw[m_lod_view].first,m_map_draw[m_lod_view].second);
			}
		glBindVertexArray(0);
	}
	for(unsigned i=0, cnt=m_objects.size(); i<cnt; ++i)
	{
		m_objects[i]->draw(rd);
	}
	assert(glGetError()==GL_NO_ERROR);
}

void xscene::add_object (xentity *obj)
{
	obj->incref();
	m_map.add_entity(obj->get_agent());
	m_objects.push_back(obj);
}

void xscene::del_object (xentity *obj)
{
	for(int i=0, cnt=m_objects.size(); i<cnt; ++i)
	{
		if(obj==m_objects[i])
		{
			m_map.del_entity(obj->get_agent());
			m_objects[i]=m_objects.back();
			m_objects.pop_back();
			obj->decref();
			return;
		}
	}
}

xentity* xscene::pick(float x, float y, unsigned filter)
{
	wyc::xpoint_detector handler(x,y);
	m_map.find_neighbors(wyc::xvec2f_t(x,y),filter,handler);
	wyc::xcollision_agent* ob = handler.get_object();
	return ob?(xentity*)(ob->get_data()):0;
}

void xscene::static_test(xentity *en, unsigned filter, std::vector<xentity*> &list)
{
	wyc::xcollision_agent *agent = en->get_agent();
	wyc::xstatic_detector handler(agent);
	m_map.find_neighbors(agent->get_lower(),agent->get_upper(),filter,handler);
	for(int i=0, cnt=handler.count(); i<cnt; ++i)
		list.push_back( (xentity*)(handler[i]->get_data()) );
}

xentity* xscene::sweep_test(xentity *en, unsigned filter, const wyc::xvec2f_t &end, float &t, wyc::xvec2f_t &normal)
{
	wyc::xcollision_agent *agent = en->get_agent();
	wyc::xvec2f_t offset = end-agent->get_position();
	wyc::xvec2f_t lower, upper;
	if(offset.x>=0) {
		lower.x = agent->get_lower().x;
		upper.x = end.x + agent->get_radius().x;
	}
	else {
		lower.x = end.x - agent->get_radius().x;
		upper.x = agent->get_upper().x;
	}
	if(offset.y>=0) {
		lower.y = agent->get_lower().y;
		upper.y = end.y + agent->get_radius().y;
	}
	else {
		lower.y = end.y - agent->get_radius().y;
		upper.y = agent->get_upper().y;
	}
	wyc::xsweep_detector handler(agent,offset);	
	m_map.find_neighbors(lower,upper,filter,handler);
	wyc::xcollision_agent *ob = handler.get_object();
	if(ob) {
		t = handler.get_distance();
		normal = handler.get_normal();
		return (xentity*)(ob->get_data());
	}
	return 0;
}

bool xscene::_create_scene_mesh()
{
	assert(sizeof(wyc::xvec2f_t)==sizeof(GLfloat)*2);
	GLuint vbo, vao;
	glGenVertexArrays(1,&vao);
	if(!vao)
		return false;
	glGenBuffers(1,&vbo);
	if(!vbo)
	{
		glDeleteVertexArrays(1,&vao);
		return false;
	}
	float map_w = (float)m_map.width(), map_h = (float)m_map.height();
	unsigned col, row, grid_w, grid_h;
	unsigned vertex_count = 0;
	for(unsigned i=0, cnt=m_map.lod_count(); i<cnt; ++i)
	{
		m_map.get_lod_info(i,row,col,grid_w,grid_h);
		vertex_count += (col+row+2)<<1;
	}
	wyc::xvec2f_t *buffer;
	unsigned vertex_base=0;
	m_map_draw.clear();
	glBindVertexArray(vao);
		glBindBuffer(GL_ARRAY_BUFFER,vbo);
		glBufferData(GL_ARRAY_BUFFER,sizeof(GLfloat)*2*vertex_count,0,GL_STATIC_DRAW);
		buffer=(wyc::xvec2f_t*)glMapBuffer(GL_ARRAY_BUFFER,GL_WRITE_ONLY);
		if(!buffer)
		{
			glBindVertexArray(0);
			goto FAILED;
		}
		wyc::xvec2f_t *vertex = buffer;
		unsigned offset=0, count;
		for(unsigned i=0, cnt=m_map.lod_count(); i<cnt; ++i)
		{
			m_map.get_lod_info(i,row,col,grid_w,grid_h);
			count = (col+row+2)<<1;
			m_map_draw.push_back(std::pair<unsigned,unsigned>(offset,count));
			offset += count;
			float v=0;
			for(unsigned j=0; j<row; ++j, v+=grid_h)
			{
				vertex++->set(0,v);
				vertex++->set(map_w,v);
			}
			vertex++->set(0,map_h);
			vertex++->set(map_w,map_h);
			v=0;
			for(unsigned j=0; j<col; ++j, v+=grid_w)
			{
				vertex++->set(v,0);
				vertex++->set(v,map_h);
			}
			vertex++->set(map_w,0);
			vertex++->set(map_w,map_h);
		}
		if(GL_FALSE==glUnmapBuffer(GL_ARRAY_BUFFER))
		{
			glBindVertexArray(0);
			goto FAILED;
		}
		glVertexAttribPointer(wyc::USAGE_POSITION,2,GL_FLOAT,GL_FALSE,0,(char*)NULL);
		glEnableVertexAttribArray(wyc::USAGE_POSITION);
	glBindVertexArray(0);

	if(m_map_vao)
		glDeleteVertexArrays(1,&m_map_vao);
	if(m_map_vbo)
		glDeleteBuffers(1,&m_map_vbo);
	m_map_vao=vao;
	m_map_vbo=vbo;
	assert(glGetError()==GL_NO_ERROR);
	return true;

FAILED:
	glDeleteVertexArrays(1,&vao);
	glDeleteBuffers(1,&vbo);
	return false;
}

void xscene::_draw_filled_grids()
{
	unsigned max_lod = m_show_parent_filled?m_map.lod_count():m_lod_view+1;
	unsigned row, col, grid_w, grid_h;
	float x, y;
	std::vector<std::pair<unsigned,unsigned> > filled_grids;
	glBegin(GL_QUADS);
	for(unsigned i=m_lod_view; i<max_lod; ++i) {
		m_map.get_lod_info(i,row,col,grid_w,grid_h);
		m_map.get_filled_grids(i,filled_grids);
		for(unsigned j=0, cnt=filled_grids.size(); j<cnt; ++j)
		{
			x = float(filled_grids[j].second * grid_w);
			y = float(filled_grids[j].first * grid_h);
			glVertex2f(x,y);
			glVertex2f(x+grid_w,y);
			glVertex2f(x+grid_w,y+grid_h);
			glVertex2f(x,y+grid_h);
		}
		filled_grids.clear();
	}
	glEnd();
}

bool xscene::load_map (const char *filename, float scale)
{
	std::fstream fs;
	fs.open(filename,std::ios_base::in|std::ios_base::binary);
	if(!fs.is_open())
		return false;
	char buffer[64];
	fs.read(buffer,4);
	if(fs.gcount()!=4)
	{
		fs.close();
		return false;
	}
	char version = buffer[0];
	if(version!=1)
	{
		fs.close();
		return false;
	}
	fs.read(buffer,24);
	if(fs.gcount()!=24)
	{
		fs.close();
		return false;
	}
	float *vf = (float*)buffer;
	wyc::xvec2f_t scn_lower, scn_upper, min_obj_size, scn_size;
	scn_lower.set(vf[0],vf[1]);
	scn_upper.set(vf[2],vf[3]);
	min_obj_size.set(vf[4],vf[5]);
	unsigned scn_w = wyc::power2(unsigned(std::ceil((scn_upper.x-scn_lower.x)*scale)));
	unsigned scn_h = wyc::power2(unsigned(std::ceil((scn_upper.y-scn_lower.y)*scale)));
	unsigned grid_w = wyc::power2(unsigned(std::ceil(min_obj_size.x*2*scale)));
	unsigned grid_h = wyc::power2(unsigned(std::ceil(min_obj_size.y*2*scale)));
	unsigned col = (scn_w+grid_w-1)/grid_w;
	unsigned row = (scn_h+grid_h-1)/grid_h;
	if(!m_map.initialize(grid_w,grid_h,row,col))
	{
		fs.close();
		return false;
	}
	typedef struct {
		wyc::xvec2f_t lower, upper;
		unsigned type, group;
	} obj_data_t;
	obj_data_t *ocd;
	xentity *obj;
	wyc::xvec2f_t center, half_size;
	while(fs)
	{
		fs.read(buffer,24);
		if(fs.gcount()!=24)
			break;
		ocd = (obj_data_t*)buffer;
		center = (ocd->lower+ocd->upper)*0.5f-scn_lower;
		center.scale(scale);
		half_size = (ocd->upper-ocd->lower)*0.5f;
		half_size.scale(scale);
		ocd->lower = center-half_size;
		ocd->upper = center+half_size;
		obj = wycnew xobstacle();
		obj->on_create(get_next_pid(),ocd->type,ocd->group);
		obj->set_aabb(ocd->lower,ocd->upper);
		obj->update_aabb();
		add_object(obj);
	}
	fs.close();
	
	// create a floor
	obj = wycnew xobstacle();
	obj->on_create(get_next_pid(),wyc::AGENT_AABB,CG_TERRAIN);
	center.x = (scn_upper.x+scn_lower.x)*0.5f-scn_lower.x;
	center.y = -1;
	center.scale(scale);
	half_size.x = (scn_upper.x-scn_lower.x)*0.5f;
	half_size.y = 1;
	half_size.scale(scale);
	obj->set_position(center);
	obj->set_radius(half_size);
	obj->update_aabb();
	add_object(obj);

	m_lod_view=0;
	_create_scene_mesh();
	return true;
}

bool save_map (const char *filename, float scale)
{
	return false;
}


} // namespace demo

