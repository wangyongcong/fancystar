#include "platform2d.h"

#include <cassert>
#include <fstream>
#include <sstream>
#include <algorithm>

#include "wyc/util/util.h"
#include "wyc/util/strutil.h"

namespace wyc
{

inline bool compare_vec2d_by_xy (const xvec2f_t &left, const xvec2f_t &right)
{
	if(left.x!=right.x)
		return left.x<right.x;
	return left.y<right.y;
}

xplatform2d::path_t* xplatform2d::_new_path()
{
	return new path_t();
}

void xplatform2d::_del_path(path_t* path)
{
	if(path->nodes)
	{
		_del_path_nodes(path->nodes);
	}
	delete path;
}

xplatform2d::path_node_t* xplatform2d::_new_path_nodes(unsigned count)
{
	return new path_node_t[count];
}

void xplatform2d::_del_path_nodes(path_node_t* nodes)
{
	delete [] nodes;
}


xplatform2d::xplatform2d()
{
	m_space_tree=0;
}

xplatform2d::~xplatform2d()
{
	clear();
}

void xplatform2d::clear()
{
	for(size_t i=0, cnt=m_paths.size(); i<cnt; ++i)
		_del_path(m_paths[i]);
	m_paths.clear();
	if(m_space_tree) 
		_destroy_space_tree();
}

bool get_attribute (std::istream &stream, const char *attr_name, std::string &attr)
{
	size_t len = strlen(attr_name);
	std::string token;
	while(stream>>token)
	{
		if(0==token.compare(0,len,attr_name))
		{
			std::string::size_type pos = token.find('"');
			if(pos!=std::string::npos)
				attr=token.substr(pos+1,token.size());
			else 
				attr.clear();
			std::string content;
			std::getline(stream,content,'"');
			attr += content;
			return true;
		}
	}
	return false;
}

bool split_xy (const std::string &coord, float &x, float &y)
{
	std::string::size_type pos = coord.find(',');
	if(std::string::npos==pos)
		return false;
	std::string val = coord.substr(0,pos);
	x = str2float(val);
	val = coord.substr(pos+1);
	y = str2float(val);
	return true;
}

bool xplatform2d::load_svg(const std::string &file_name)
{
	std::fstream fs;
	fs.open(file_name.c_str(),std::ios_base::in);
	if(!fs.is_open()) {
		printf("can't open file: %s\n",file_name.c_str());
		return false;
	}
	fs.seekg(0,std::ios_base::end);
	std::fstream::pos_type length = fs.tellg();
	fs.seekg(0,std::ios_base::beg);
	std::string token, viewbox, points, err;
	unsigned view_x=0, view_y=0, view_w=2048, view_h=2048;
	while(fs.ignore(length,'<'))
	{
		fs>>token;
		if(token=="svg")
		{
			if(!get_attribute(fs,"viewBox",viewbox)) {
				err="SVG error: no viewBox";
				fs.close();
				return false;
			}
		}
		else if(token=="path") 
		{
			if(!get_attribute(fs,"d",points)) {
				err="SVG error: invalid path info";
				fs.close();
				return false;
			}
		}
		fs.ignore(length,'>');
	}
	if(viewbox.empty())
	{
		fs.close();
		return false;
	}
	std::stringstream ss;
	ss.str(viewbox);
	ss.seekg(0,std::ios::beg);
	ss>>view_x>>view_y>>view_w>>view_h;
	std::vector<xvec2f_t> positions;
	// 提取坐标并转换到新的坐标系(如下图)
	//
	//	^ +y
	//  |
	//  |----> +x
	//
	if(!points.empty()) {
		xvec2f_t v;
		ss.str(points);
		ss.seekg(0,std::ios::beg);
		while(ss>>token)
		{
			if(token=="C") // Bezier curve
			{
				ss>>token;
				if(split_xy(token,v.x,v.y)) {
					v.y=view_h-v.y;
					positions.push_back(v);
				}
				for(ss>>token; ss; ss>>token>>token>>token)
				{
					if(split_xy(token,v.x,v.y)) {
						v.y=view_h-v.y;
						positions.push_back(v);
					}
				}
			}
			ss.ignore(points.size(),'\n');
		}
	}
	fs.close();
	if(positions.size()) 
	{
		std::sort(positions.begin(),positions.end(),compare_vec2d_by_xy);
		append_path(positions);
	}
	return true;
}

void xplatform2d::append_path(const std::vector<xvec2f_t> &positions)
{
	if(positions.size()<2)
		return;
	size_t max_node_count = positions.size()-1;
	path_node_t *nodes=_new_path_nodes(max_node_count), *cur;
	cur=nodes;
	std::vector<xvec2f_t>::const_iterator prev=positions.begin(), end=positions.end(), next;
	next = prev+1;
	float offx, offy, min_x=prev->x, min_y=prev->y, max_x=prev->x, max_y=prev->y;
	for(; next!=end; ++prev, ++next)
	{
		offx = next->x - prev->x;
		offy = next->y - prev->y;
		cur->length = sqrt(offx*offx+offy*offy);
		if(fabs(cur->length)<EPSILON_E4)
			// skip close position
			continue;
		cur->beg_x=prev->x;
		cur->beg_y=prev->y;
		cur->end_x=next->x;
		cur->end_y=next->y;
		if(fabs(offx)<EPSILON_E4) 
			cur->tangent = FLT_MAX;
		else 
			cur->tangent = offy/offx;
		cur->cosine = offx/cur->length;
		++cur;
		// update bounding box
		if(next->x<min_x)
			min_x=next->x;
		else if(next->x>max_x)
			max_x=next->x;
		if(next->y<min_y)
			min_y=next->y;
		else if(next->y>max_y)
			max_y=next->y;
	}
	if(cur==nodes) {
		// empty path, do not save it
		_del_path_nodes(nodes);
		return;
	}
	path_t *path = _new_path();
	path->nodes=nodes;
	path->node_count=cur-nodes;
	path->bounding_box.set(min_x,min_y,max_x,max_y);
	m_paths.push_back(path);
	// update bounding box
	if(min_x<m_bounding_box.xmin)
		m_bounding_box.xmin=min_x;
	if(max_x>m_bounding_box.xmax)
		m_bounding_box.xmax=max_x;
	if(min_y<m_bounding_box.ymin)
		m_bounding_box.ymin=min_y;
	if(max_y>m_bounding_box.ymax)
		m_bounding_box.ymax=max_y;
}

void xplatform2d::begin_path()
{
	clear();
	m_bounding_box.set(FLT_MAX,FLT_MAX,FLT_MIN,FLT_MIN);
}

#define Y_PARTITION_LEVEL 3
#define Y_PARTITION_NODE_COUNT 7 // (1<<Y_PARTITION_LEVEL)-1

void xplatform2d::_create_space_tree()
{
	// TODO: 目前使用二分分割，还有更优的分割算法，参考R-Tree之类的空间分割算法
	float map_w = m_bounding_box.width(), map_h = m_bounding_box.height();
	partition_node_t *nodes=new partition_node_t[Y_PARTITION_NODE_COUNT], *iter=nodes, *root;
	typedef std::pair<partition_node_t*,unsigned> stack_node_t;
	std::vector<stack_node_t> stack;
	stack.reserve(Y_PARTITION_LEVEL);
	iter->radius=map_h*0.5f;
	iter->spliter=(m_bounding_box.ymax+m_bounding_box.ymin)*0.5f;
	iter->left=iter->right=0;
	iter->data=0;
	stack.push_back(stack_node_t(iter,1));
	unsigned level;
	while(!stack.empty())
	{
		root=stack.back().first;
		level=stack.back().second+1;		
		stack.pop_back();
		// left child
		++iter;
		root->left=iter;
		iter->radius=root->radius*0.5f;
		iter->spliter=(root->spliter+root->spliter-root->radius)*0.5f;		
		iter->left=iter->right=0;
		if(level<Y_PARTITION_LEVEL) {
			stack.push_back(stack_node_t(iter,level));
			iter->data=0;
		}
		else iter->data=new std::vector<path_t*>();
		// right child
		++iter;
		root->right=iter;
		iter->radius=root->radius*0.5f;
		iter->spliter=(root->spliter+root->spliter+root->radius)*0.5f;
		iter->left=iter->right=0;
		if(level<Y_PARTITION_LEVEL) {
			stack.push_back(stack_node_t(iter,level));
			iter->data=0;
		}
		else iter->data=new std::vector<path_t*>();
	}
	assert(iter-nodes+1==Y_PARTITION_NODE_COUNT);
	m_space_tree=nodes;
}

void xplatform2d::_destroy_space_tree()
{
	for(unsigned i=0; i<Y_PARTITION_NODE_COUNT; ++i)
	{
		if(m_space_tree[i].data) 
			delete (std::vector<path_t*>*)(m_space_tree[i].data);
	}
	delete [] m_space_tree;
	m_space_tree=0;
}

void xplatform2d::_insert_path_by_y(partition_node_t* root, path_t *path)
{
	assert(root);
	if(root->data) {
		root->data->push_back(path);
		return;
	}
	if(root->spliter<path->bounding_box.ymax)
	{
		assert(root->right);
		_insert_path_by_y(root->right,path);
	}
	if(root->spliter>=path->bounding_box.ymin)
	{
		assert(root->left);
		_insert_path_by_y(root->left,path);
	}
}

void xplatform2d::end_path()
{
	if(m_space_tree)
		_destroy_space_tree();
	_create_space_tree();
	for(size_t i=0, cnt=m_paths.size(); i<cnt; ++i)
		_insert_path_by_y(m_space_tree,m_paths[i]);
}

xplatform2d::partition_node_t* xplatform2d::_space_search(partition_node_t *root, float pos)
{
	partition_node_t *parent=0;
	while(root)
	{
		parent=root;
		if(pos<=root->spliter)
			root=root->left;
		else 
			root=root->right;
	}
	return parent;
}

struct find_node_by_x
{
	inline int operator() (const xplatform2d::path_node_t &node, float x)
	{
		if(node.end_x<=x)
			return -1;
		if(node.beg_x>x)
			return 1;
		return 0;
	}
};

/*
bool xplatform2d::stand_on_ground (const agent_t *agent, path_info_t *info)
{
	assert(agent && info);
	if(agent->type!=AGENT_AABB) // currently aabb only !
		return false;
	agent_aabb_t *aabb = (agent_aabb_t*)agent;
	if(m_paths.empty())
		return false;
	float left = aabb->position.x-aabb->size.x*0.5f, right = left+aabb->size.x, left_y, right_y;
	partition_node_t *pn = _space_search(m_space_tree,aabb->position.y);
	assert(pn->data);
	path_t *path;
	path_node_t *node, *node_end;
	int left_node, right_node;
	float max_y=FLT_MIN, actual_y, pole;
	for(size_t i=0, cnt=pn->data->size(); i<cnt; ++i)
	{
		path=(*pn->data)[i];
		if(left>path->bounding_box.xmax || right<path->bounding_box.xmin)
			continue;
		left_y=right_y=FLT_MIN;
		if(wyc::binary_search<path_node_t,float,find_node_by_x>(path->nodes,path->node_count,left,left_node))
		{
			node=path->nodes+left_node;
			for(++node, node_end=path->nodes+path->node_count; node!=node_end; ++node)
			{
			}
			
			if(left>=node->beg_x)
				left_y = (left-node->beg_x)*node->tangent+node->beg_y;
			if(right<node->end_x)
				right_y = (right-node->beg_x)*node->tangent+node->beg_y;

			if(y>=actual_y) {
				info->path_id=i;
				info->node_id=node_idx;
				info->exact_position.set(aabb->position.x,actual_y);
				return true;
			}
		}
		if(right_y==FLT_MIN && wyc::binary_search<path_node_t,float,find_node_by_x>(path->nodes,path->node_count,right,node_idx))
		{
			node=path->nodes+node_idx;
			if(right>=node->beg_x)
				right_y = (right-node->beg_x)*node->tangent+node->beg_y;
		}
		actual_y = std::max(left_y,right_y);
		if(actual_y>max_y)
		{
			max_y=actual_y;
			info->path_id=i;
			info->node_id=node_idx;
		}
	}
	return false;
}*/


}; // namespace wyc



