#ifndef __HEADER_WYC_PLATFORM_2D
#define __HEADER_WYC_PLATFORM_2D

#include <string>
#include <vector>

#include "wyc/math/vecmath.h"
#include "wyc/util/rect.h"

namespace wyc
{

class xplatform2d
{
public:
	struct path_node_t 
	{
		float beg_x, beg_y;
		float end_x, end_y;
		float length;
		float tangent;
		float cosine;
	};

	struct path_t
	{
		path_node_t *nodes;
		unsigned node_count;
		xrectf_t bounding_box;
	};

//----------------------------------------------------------------------

	xplatform2d();
	~xplatform2d();
	void clear();
	// terrain 
	void begin_path();
	void end_path();
	bool load_svg(const std::string &file_name);
	void append_path(const std::vector<xvec2f_t> &positions);
	const path_t* get_path(unsigned path_id) const;
	unsigned path_count() const;

//----------------------------------------------------------------------

private:
	std::vector<path_t*> m_paths;
	xrectf_t m_bounding_box;
	
	// memory management
	path_t* _new_path();
	void _del_path(path_t*);
	path_node_t* _new_path_nodes(unsigned count);
	void _del_path_nodes(path_node_t*);

	struct partition_node_t
	{
		partition_node_t *left, *right;
		float spliter, radius;
		std::vector<path_t*> *data;
	};
	partition_node_t *m_space_tree;
	void _create_space_tree();
	void _destroy_space_tree();
	void _insert_path_by_y(partition_node_t *root, path_t *path);
	partition_node_t* _space_search(partition_node_t *root, float pos);


};

inline const xplatform2d::path_t* xplatform2d::get_path(unsigned path_id) const
{
	assert(path_id<m_paths.size());
	return m_paths[path_id];
}

inline unsigned xplatform2d::path_count() const
{
	return m_paths.size();
}

}; // namespace wyc

#endif // __HEADER_WYC_PLATFORM_2D
