#include "fscorepch.h"
#include "xrenderobj.h"
#include "xcamera.h"
#include "vertexbatch.h"
#include "xtexture.h"
#include "renderer.h"
#include "data_map.h"
#include "game_config.h"

using wyc::xpointer;
using wyc::xobjevent;
using wyc::xpackev;
using wyc::xrenderer;
using wyc::xvertex_batch;
using wyc::xvertex_buffer;

template<typename T>
class xbufferobj : public wyc::xrefobj
{
	T *m_pbuffer;
	size_t m_size;
public:
	xbufferobj(size_t count) {
		m_size=count;
		if(count)
			m_pbuffer=new T[count];
		else
			m_pbuffer=0;
	}
	virtual void delthis() {
		delete [] m_pbuffer;
	}
	T* buffer() {
		return m_pbuffer;
	}
	size_t size() const {
		return m_size;
	}
};

class xtilemap : public wyc::xrenderobj
{
	USE_EVENT_MAP;
	// terrain rendering
	struct TILE_TEXCOORD {
		GLfloat m_bs, m_bt;
		GLfloat m_ls0, m_lt0;
		GLfloat m_ls1, m_lt1;
		GLfloat m_ls2, m_lt2;
		GLfloat m_ls3, m_lt3;
		GLfloat m_ls4, m_lt4;
		GLfloat m_ls5, m_lt5;
		GLfloat m_ls6, m_lt6;
		GLfloat m_ls7, m_lt7;
	};
	typedef std::vector<std::pair<unsigned,unsigned> > xupdate_record;
	TILE_TEXCOORD *m_pTexCoordBuff;
	xupdate_record m_texUpdate;
	xpointer<wyc::xvertex_batch> m_spMapBatch;
	xpointer<wyc::xvertex_batch> m_spGridBatch;
	xpointer<wyc::xtexture> m_spTexture;
	xpointer<wyc::xvertex_buffer> m_spTexCoord;
	GLuint m_mapShader;
	GLuint m_gridShader;
	xpointer<wyc::xcamera> m_spCamera;
	// map data
	xpointer<xmapdata> m_spMap;
	enum BORDER {
		BORDER_NW=0,
		BORDER_N,
		BORDER_NE,
		BORDER_W,
		BORDER_E,
		BORDER_SW,
		BORDER_S,
		BORDER_SE,
		BORDER_NUM,
	};
	struct TILE {
		wyc::uint8_t m_transition[BORDER_NUM];
		bool m_modified;
		TILE() {
			memset(m_transition,0xFF,sizeof(wyc::uint8_t)*BORDER_NUM);
			m_modified=false;
		}
	};
	TILE *m_tiles;
	wyc::uint8_t m_tileID;
	// others
	unsigned m_gridw, m_gridh;
	unsigned m_prevPaintTile;
	bool m_rebuild;
	bool m_showGrid;
	bool m_updateTiles;
public:
	xtilemap();
	~xtilemap();
	virtual void update(float interval);
	virtual void draw();
private:
	void clear();
	void set_tile(unsigned x, unsigned y, wyc::uint8_t tid);
	void update_tiles();
	void mark_buffer(xupdate_record &uprec, unsigned offset, unsigned size);
	//-----------------------------------------------------------
	// GPU pass
	//-----------------------------------------------------------
	void r_rebuild_map();
	void r_build_grids();
	void r_update_batch();
	void r_render_map();
	void r_render_grid();
	//-----------------------------------------------------------
	// 消息处理函数
	//-----------------------------------------------------------
	/// 设置相机
	void set_camera(xobjevent *pev);
	/** @brief 创建地图
		@param xpackev:dddd
			- unsigned 地图宽度(单位:格子)
			- unsigned 地图长度(单位:格子)
			- unsigned 格子宽度(单位:像素)
			- unsigned 格子长度(单位:像素)
	*/
	void create_map(xobjevent *pev);
	/// callback: 纹理加载成功
	void on_image_ok(xobjevent *pev);
	/// 切换格子显示状态
	void switch_grid_show(xobjevent *pev);
	/** @brief 设置当前的地形
		@param xpackev:d
			- unsigned char 地形ID
	*/
	void edit_set_tile(xobjevent *pev);
	/** @brief 绘制地图
		@param xpackev:ff
			- float 地图坐标x
			- float 地图坐标y
	*/
	void edit_put_tile(xobjevent *pev);
	/** @brief 绘制地图
		@param xpackev:ff
			- float 地图坐标x
			- float 地图坐标y
	*/
	void edit_paint_tiles(xobjevent *pev);

};

BEGIN_EVENT_MAP(xtilemap,wyc::xrenderobj)
	REG_EVENT(set_camera)
	REG_EVENT(create_map)
	REG_EVENT(on_image_ok)
	REG_EVENT(switch_grid_show)
	REG_EVENT(edit_set_tile)
	REG_EVENT(edit_put_tile)
	REG_EVENT(edit_paint_tiles)
END_EVENT_MAP

xtilemap::xtilemap()
{
	m_gridw=m_gridh=0;
	m_rebuild=false;
	m_showGrid=false;
	m_updateTiles=false;
	m_tileID=0;
	m_pTexCoordBuff=0;
	m_tiles=0;
	m_mapShader=0;
	m_gridShader=0;
}

xtilemap::~xtilemap()
{
	clear();
	m_spCamera=0;
}

void xtilemap::clear()
{
	m_spMap=0;
	m_spTexture=0;
	m_spMapBatch=0;
	m_spGridBatch=0;
	m_spTexCoord=0;
	if(m_pTexCoordBuff) {
		delete [] m_pTexCoordBuff;
		m_pTexCoordBuff=0;
	}
	if(m_tiles) {
		delete [] m_tiles;
		m_tiles=0;
	}
}

void xtilemap::update(float)
{
	if(m_updateTiles) 
		update_tiles();
	xrenderer::get_renderer()->draw(this);
}

void xtilemap::draw()
{
	if(!m_spCamera)
		return;
	if(m_rebuild) {
		m_rebuild=false;
		r_rebuild_map();
	}
	if(m_spMapBatch)  {
		if(!m_texUpdate.empty()) 
			r_update_batch();
		r_render_map();
	}
	if(m_showGrid) {
		if(!m_spGridBatch) 
			r_build_grids();
		r_render_grid();
	}
}

void xtilemap::set_camera(xobjevent *pev)
{
	wyc::xcamera *pcam;
	if(!((xpackev*)pev)->unpack("o",&pcam)) 
		return;
	m_spCamera=pcam;
}

void xtilemap::create_map(xobjevent *pev)
{
	unsigned mw, mh, gw, gh;
	if(!((xpackev*)pev)->unpack("dddd",&mw,&mh,&gw,&gh)) {
		wyc_error("xtilemap::create_map: bad args");
		return;
	}
	unsigned grid_count=mw*mh;
	if(grid_count>=0x4000) {
		wyc_error("xtilemap::create_map: too many vertices");
		return;
	}
	clear();
	m_spMap=wycnew xmapdata;
	m_spMap->create(mw,mh);
	m_tiles=new TILE[grid_count];
	m_gridw=gw, m_gridh=gh;
	m_rebuild=true;
	
	xrenderer::get_renderer()->send_event(xrenderer::EV_LOAD_TEXTURE,xpackev::pack("ods",this,wyc::strhash("on_image_ok"),"tile.png"));
}

void xtilemap::on_image_ok(xobjevent *pev)
{
	wyc::xtexture *ptex;
	if(!((xpackev*)pev)->unpack("o",&ptex))
		return;
	m_spTexture=ptex;
}

void xtilemap::switch_grid_show(xobjevent*)
{
	m_showGrid=!m_showGrid;
}

void xtilemap::edit_set_tile(wyc::xobjevent *pev)
{
	unsigned tileID;
	if(!((xpackev*)pev)->unpack("d",&tileID))
		return;
	m_tileID=(wyc::uint8_t)tileID;
}

void xtilemap::edit_put_tile(xobjevent *pev)
{
	if(!m_spMapBatch)
		return;
	float x, y;
	if(!((xpackev*)pev)->unpack("ff",&x,&y))
		return;
	if(x<0 || y<0)
		return;
	unsigned gx, gy;
	gx=unsigned(x/m_gridw);
	gy=unsigned(y/m_gridh);
	if(gx>=m_spMap->width() || gy>=m_spMap->height())
		return;
//	wyc_print("put tile at: (%d,%d)",gx,gy);
	set_tile(gx,gy,m_tileID);
	m_prevPaintTile=m_spMap->width()*gy+gx;
}

void xtilemap::edit_paint_tiles(xobjevent *pev)
{
	if(!m_spMapBatch)
		return;
	float x, y;
	if(!((xpackev*)pev)->unpack("ff",&x,&y))
		return;
	if(x<0 || y<0)
		return;
	unsigned gx, gy;
	gx=unsigned(x/m_gridw);
	gy=unsigned(y/m_gridh);
	if(gx>=m_spMap->width() || gy>=m_spMap->height())
		return;
	unsigned gidx=m_spMap->width()*gy+gx;
	if(gidx==m_prevPaintTile)
		return;
	set_tile(gx,gy,m_tileID);
	m_prevPaintTile=gidx;
}

//---------------------------------------------------------------------------

void xtilemap::r_rebuild_map()
{
	unsigned mapw=m_spMap->width(), maph=m_spMap->height();
	unsigned grid_count=mapw*maph;
	// vertex buffer
	unsigned vert_count=grid_count*8;
	GLfloat *pVertbuff=new GLfloat[vert_count];
	GLfloat *pbuff=pVertbuff;
	GLfloat x0=0, y0=0, x1=GLfloat(m_gridw), y1=GLfloat(m_gridh);
	for(unsigned i=0; i<maph; ++i) {
		for(unsigned j=0; j<mapw; ++j) {
			/*------------------
				3---2
				|   |
				0---1
			------------------*/
			*pbuff++=x0;
			*pbuff++=y0;
			*pbuff++=x1;
			*pbuff++=y0;
			*pbuff++=x1;
			*pbuff++=y1;
			*pbuff++=x0;
			*pbuff++=y1;
			x0=x1;
			x1+=m_gridw;
		}
		x0=0;
		x1=GLfloat(m_gridw);
		y0=y1;
		y1+=m_gridh;
	}
	assert(pbuff==pVertbuff+vert_count);

	// texture coordinate
	unsigned texcoord_count=grid_count*4;
	TILE_TEXCOORD *pTexcoord=new TILE_TEXCOORD[texcoord_count];
	memset(pTexcoord,0,sizeof(TILE_TEXCOORD)*texcoord_count);
	TILE_TEXCOORD *ptc=pTexcoord;
	for(unsigned i=0; i<maph; ++i) {
		for(unsigned j=0; j<mapw; ++j) {
			ptc->m_bs=DEF_TEX_L;
			ptc->m_bt=DEF_TEX_B;
			++ptc;
			ptc->m_bs=DEF_TEX_R;
			ptc->m_bt=DEF_TEX_B;
			++ptc;
			ptc->m_bs=DEF_TEX_R;
			ptc->m_bt=DEF_TEX_U;
			++ptc;
			ptc->m_bs=DEF_TEX_L;
			ptc->m_bt=DEF_TEX_U;
			++ptc;
		}
	}
	assert(ptc==pTexcoord+texcoord_count);

	// index buffer
	unsigned index_count=grid_count*6;
	GLushort *pIndexbuff=new GLushort[index_count];
	GLushort *pidx=pIndexbuff;
	GLushort idx=0;
	for(unsigned i=0; i<maph; ++i) {
		for(unsigned j=0; j<mapw; ++j) {
			*pidx++=idx;
			*pidx++=idx+1;
			*pidx++=idx+3;
			*pidx++=idx+1;
			*pidx++=idx+2;
			*pidx++=idx+3;
			idx+=4;
		}
	}
	assert(pidx==pIndexbuff+index_count);
	assert(idx==vert_count/2);

	GLuint vbo[3];
	glGenBuffers(3,vbo);
	
	glBindBuffer(GL_ARRAY_BUFFER,vbo[0]);
	glBufferData(GL_ARRAY_BUFFER,sizeof(GLfloat)*vert_count,pVertbuff,GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,vbo[1]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER,sizeof(GLushort)*index_count,pIndexbuff,GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER,vbo[2]);
	glBufferData(GL_ARRAY_BUFFER,sizeof(TILE_TEXCOORD)*texcoord_count,pTexcoord,GL_STREAM_DRAW);

	delete [] pVertbuff;
	delete [] pIndexbuff;

	// TODO: 如果不是编辑模式,则无需保留
	m_pTexCoordBuff=pTexcoord;

	wyc::xvertex_batch *pbatch;
	wyc::xvertex_buffer *pvb;
	if(GLEW_ARB_vertex_array_object) {
		// enable vertex array objects
		GLuint vao;
		glGenVertexArrays(1,&vao);
		glBindVertexArray(vao);
		// vertex position
		glBindBuffer(GL_ARRAY_BUFFER,vbo[0]);
		glVertexAttribPointer(wyc::USAGE_POSITION,2,GL_FLOAT,GL_FALSE,0,(void*)0);
		glEnableVertexAttribArray(wyc::USAGE_POSITION);
		// vertex index
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,vbo[1]);
		// texture coordinate
		glBindBuffer(GL_ARRAY_BUFFER,vbo[2]);
		glVertexAttribPointer(wyc::USAGE_COLOR,2,GL_FLOAT,GL_FALSE,sizeof(TILE_TEXCOORD),(void*)0);
		glEnableVertexAttribArray(wyc::USAGE_COLOR);
		for(unsigned i=wyc::USAGE_TEXTURE0, offset=sizeof(GLfloat)<<1; i<=wyc::USAGE_TEXTURE3; ++i, offset+=sizeof(GLfloat)<<2) {
			glVertexAttribPointer(i,4,GL_FLOAT,GL_FALSE,sizeof(TILE_TEXCOORD),(void*)offset);
			glEnableVertexAttribArray(i);
		}
		// end of vao
		glBindVertexArray(0);

		pbatch=(wyc::xvertex_batch*)xrenderer::get_renderer()->create_batch("tile map",(void*)vao);
		pvb=wycnew wyc::xvertex_buffer;
		pvb->create(vbo[0]);
		pbatch->add_buffer(pvb);
		pvb=wycnew wyc::xvertex_buffer;
		pvb->create(vbo[1]);
		pbatch->set_index(pvb,GL_UNSIGNED_SHORT);
		pvb=wycnew wyc::xvertex_buffer;
		pvb->create(vbo[2]);
		pbatch->add_buffer(pvb);
		m_spTexCoord=pvb;
	}
	else {
		pbatch=(wyc::xvertex_batch*)xrenderer::get_renderer()->create_batch("tile map",0);
		
		pvb=wycnew wyc::xvertex_buffer;
		pvb->create(vbo[0]);
		pbatch->add_buffer(pvb,wyc::USAGE_POSITION,2,GL_FLOAT,false,0,0);

		pvb=wycnew wyc::xvertex_buffer;
		pvb->create(vbo[1]);
		pbatch->set_index(pvb,GL_UNSIGNED_SHORT);

		pvb=wycnew wyc::xvertex_buffer;
		pvb->create(vbo[2]);
		pbatch->add_buffer(pvb,wyc::USAGE_COLOR,2,GL_FLOAT,false,sizeof(TILE_TEXCOORD),0);
		for(unsigned i=wyc::USAGE_TEXTURE0, offset=sizeof(GLfloat)<<1; i<=wyc::USAGE_TEXTURE3; ++i, offset+=sizeof(GLfloat)<<2) 
			pbatch->add_buffer(pvb,i,4,GL_FLOAT,false,sizeof(TILE_TEXCOORD),offset);
		m_spTexCoord=pvb;
	}
	pbatch->set_mode(GL_TRIANGLES,index_count);
	m_spMapBatch=pbatch;

	GLenum glerr=glGetError();
	if(glerr!=GL_NO_ERROR) {
		wyc_warn("[OpenGL] xtilemap::r_rebuild_map: some error occurred");
	}
}

void xtilemap::r_build_grids()
{
	unsigned mapw=m_spMap->width(), maph=m_spMap->height();
	size_t vertex_count=(mapw+maph+2)*2;
	size_t size=vertex_count<<1;
	GLfloat *pVertbuff=new GLfloat[size];
	GLfloat *pbuff=pVertbuff;
	GLfloat x=0, y=0;
	GLfloat w=GLfloat(mapw*m_gridw), h=GLfloat(maph*m_gridh);
	for(unsigned i=0; i<=mapw; ++i) {
		*pbuff++=x;
		*pbuff++=0;
		*pbuff++=x;
		*pbuff++=h;
		x+=m_gridw;
	}
	for(unsigned i=0; i<=maph; ++i) {
		*pbuff++=0;
		*pbuff++=y;
		*pbuff++=w;
		*pbuff++=y;
		y+=m_gridh;
	}
	assert(pbuff==pVertbuff+size);

	GLuint vbo;
	glGenBuffers(1,&vbo);
	glBindBuffer(GL_ARRAY_BUFFER,vbo);
	glBufferData(GL_ARRAY_BUFFER,sizeof(GL_FLOAT)*size,pVertbuff,GL_STATIC_DRAW);
	delete [] pVertbuff;

	wyc::xvertex_batch *pbatch;
	wyc::xvertex_buffer *pvb;
	if(GLEW_ARB_vertex_array_object) {
		GLuint vao;
		glGenVertexArrays(1,&vao);
		glBindVertexArray(vao);
		glVertexAttribPointer(wyc::USAGE_POSITION,2,GL_FLOAT,GL_FALSE,0,(void*)0);
		glEnableVertexAttribArray(wyc::USAGE_POSITION);
		glBindVertexArray(0);

		pbatch=(wyc::xvertex_batch*)xrenderer::get_renderer()->create_batch("map grids",(void*)vao);
		pvb=wycnew wyc::xvertex_buffer;
		pvb->create(vbo);
		pbatch->add_buffer(pvb);
	}
	else {
		pbatch=(wyc::xvertex_batch*)xrenderer::get_renderer()->create_batch("map grids",0);
		pvb=wycnew wyc::xvertex_buffer;
		pvb->create(vbo);
		pbatch->add_buffer(pvb,wyc::USAGE_POSITION,2,GL_FLOAT,false,0,0);
	}
	pbatch->set_mode(GL_LINES,vertex_count);
	m_spGridBatch=pbatch;

	GLenum glerr=glGetError();
	if(glerr!=GL_NO_ERROR) {
		wyc_warn("[OpenGL] xtilemap::r_build_grids: some error occurred");
	}

}

void xtilemap::r_update_batch()
{
	assert(m_spMapBatch);
	xupdate_record::iterator iter, end;
	if(m_spTexCoord) {
		iter=m_texUpdate.begin();
		end=m_texUpdate.end();
		glBindBuffer(GL_ARRAY_BUFFER,m_spTexCoord->handle());	
		for(; iter!=end; ++iter) 
			glBufferSubData(GL_ARRAY_BUFFER,iter->first,iter->second,((GLubyte*)m_pTexCoordBuff)+iter->first);
		m_texUpdate.clear();
	}
	GLenum glerr=glGetError();
	if(glerr!=GL_NO_ERROR) {
		wyc_warn("[OpenGL] xtilemap::r_update_batch: some error occurred");
	}
}

void xtilemap::r_render_map()
{
	if(-1==m_mapShader)
		return;
	xrenderer *pRenderer=xrenderer::get_renderer();
	if(0==m_mapShader) {
		m_mapShader=pRenderer->get_shader("tilemap");
		if(0==m_mapShader) {
			m_mapShader=-1;
			return;
		}
	}
	if(!m_spTexture)
		return;
	if(pRenderer->use_shader(m_mapShader)) {
		glBindTexture(GL_TEXTURE_2D,m_spTexture->handle());
		GLint uf=glGetUniformLocation(m_mapShader,"mvp");
		if(uf!=-1)
			glUniformMatrix4fv(uf,1,GL_FALSE,m_spCamera->get_mvpmatrix().data());
		uf=glGetUniformLocation(m_mapShader,"position");
		if(uf!=-1)
			glUniform3f(uf,0,0,0);
		m_spMapBatch->render();
	}
}

void xtilemap::r_render_grid()
{
	if(-1==m_gridShader)
		return;
	xrenderer *pRenderer=xrenderer::get_renderer();
	if(0==m_gridShader) {
		m_gridShader=pRenderer->get_shader("simple");
		if(0==m_gridShader) {
			m_gridShader=-1;
			return;
		}
	}
	if(pRenderer->use_shader(m_gridShader)) {
		GLint uf=glGetUniformLocation(m_gridShader,"mvp");
		if(uf!=-1)
			glUniformMatrix4fv(uf,1,GL_FALSE,m_spCamera->get_mvpmatrix().data());
		uf=glGetUniformLocation(m_gridShader,"color");
		if(uf!=-1)
			glUniform4f(uf,0,1.0f,0,1.0f);
		m_spGridBatch->render();
	}
}

void xtilemap::set_tile(unsigned gx, unsigned gy, wyc::uint8_t tid)
{
	// 由于是从下(S)到上(N)遍历9宫格
	// 所以ls_border的顺序是从S到N (刚好相反)
	static const int ls_border[]={
		BORDER_SW, BORDER_S,   BORDER_SE,
		BORDER_W,  BORDER_NUM, BORDER_E,
		BORDER_NW, BORDER_N,   BORDER_NE,
	};
	assert(gx<m_spMap->width() && gy<m_spMap->height());
	unsigned gidx=m_spMap->width()*gy+gx;
	wyc::uint8_t pre_tid=m_spMap->get(gidx);
	if(tid==pre_tid)
		return;
	m_spMap->set(gidx,tid);
	unsigned cidx=gidx;
	wyc::uint8_t cmp;
	unsigned col=1, row=1;
	int bid=4;
	if(gy>0) {
		cidx-=m_spMap->width();
		bid-=3;
		row+=1;
	}
	if(gy<m_spMap->height()-1)
		row+=1;
	if(gx>0) {
		cidx-=1;
		bid-=1;
		col+=1;
	}
	if(gx<m_spMap->width()-1)
		col+=1;
	for(unsigned i=0; i<row; ++i) {
		for(unsigned end=cidx+col; cidx<end; ++cidx, ++bid) {
			if(cidx==gidx)
				continue;
			cmp=m_spMap->get(cidx);
			if(cmp!=tid) {
				if(cmp>tid) {
					m_tiles[gidx].m_transition[ls_border[bid]]=cmp;
				}
				else {
					m_tiles[gidx].m_transition[ls_border[bid]]=0xFF;
					m_tiles[cidx].m_transition[ls_border[8-bid]]=tid;
					m_tiles[cidx].m_modified=true;
				}
			}
			else if(cmp!=pre_tid) {
				if(cmp>pre_tid) {
					m_tiles[gidx].m_transition[ls_border[bid]]=0xFF;
				}
				else {
					m_tiles[cidx].m_transition[ls_border[8-bid]]=0xFF;
					m_tiles[cidx].m_modified=true;
				}
			}
		}
		cidx=cidx-col+m_spMap->width();
		bid=bid-col+3;
	}
	m_tiles[gidx].m_modified=true;
	m_updateTiles=true;
}

void get_tile_coord(wyc::uint8_t tileID, wyc::xvec4f_t &tileCoord)
{
	unsigned x=tileID&3, y=tileID>>2;
	x*=3;
	y*=3;
	x+=1;
	y+=1;
	// left, right, bottom, top
	tileCoord.x=x*TILE_COORD_XSCALE;
	tileCoord.y=tileCoord.x+TILE_COORD_XSCALE;
	tileCoord.w=1.0f-y*TILE_COORD_YSCALE;
	tileCoord.z=tileCoord.w-TILE_COORD_YSCALE;
}

void get_tile_coord(wyc::uint8_t tileID, wyc::uint8_t dir, wyc::xvec4f_t &tileCoord)
{
	// 偏移值相对于左上角(NW),且方向正好相反
	// 所以这里N/S对调,W/E对调
	static const int ls_tidOffset[16]={
		 2, 2,	1, 2,	0, 2,
		 2, 1,			0, 1,
		 2, 0,	1, 0,	0, 0,
	};
	unsigned x=tileID&3, y=tileID>>2;
	x*=3;
	y*=3;
	assert(dir<8);
	dir=dir<<1;
	x+=ls_tidOffset[dir];
	y+=ls_tidOffset[dir+1];
	// left, right, bottom, top
	tileCoord.x=x*TILE_COORD_XSCALE;
	tileCoord.y=tileCoord.x+TILE_COORD_XSCALE;
	tileCoord.w=1.0f-y*TILE_COORD_YSCALE;
	tileCoord.z=tileCoord.w-TILE_COORD_YSCALE;
}

void xtilemap::update_tiles()
{
	m_updateTiles=false;
	wyc::uint16_t layers[8];
	wyc::int8_t lcnt, baseIdx, i;
	wyc::xvec4f_t texCoord;
	unsigned grid_count=m_spMap->width()*m_spMap->height();
	for(unsigned gidx=0; gidx<grid_count; ++gidx) {
		if(!m_tiles[gidx].m_modified)
			continue;
		m_tiles[gidx].m_modified=false;
		wyc::uint8_t *trans=m_tiles[gidx].m_transition;
		lcnt=0;
		for(i=0; i<8; ++i) {
			if(0xFF==trans[i])
				continue;
			layers[lcnt++]=(trans[i]<<8)+i;
		}
		// 升序排列
		wyc::bubble_sort(layers,lcnt);
		GLfloat *ptc=(GLfloat*)(m_pTexCoordBuff+(gidx<<2));
		// 改写基础纹理
		get_tile_coord(m_spMap->get(gidx),texCoord);
		ptc[0]	= texCoord[0];
		ptc[1]	= texCoord[2];
		ptc[18]	= texCoord[1];
		ptc[19]	= texCoord[2];
		ptc[36]	= texCoord[1];
		ptc[37]	= texCoord[3];
		ptc[54]	= texCoord[0];
		ptc[55]	= texCoord[3];
		// 改写边缘纹理
		for(i=0, baseIdx=2; i<lcnt; ++i, baseIdx+=2) {
			get_tile_coord(layers[i]>>8,layers[i]&0xFF,texCoord);
			ptc[baseIdx]	= texCoord[0];
			ptc[baseIdx+1]	= texCoord[2];
			ptc[baseIdx+18]	= texCoord[1];
			ptc[baseIdx+19]	= texCoord[2];
			ptc[baseIdx+36]	= texCoord[1];
			ptc[baseIdx+37]	= texCoord[3];
			ptc[baseIdx+54]	= texCoord[0];
			ptc[baseIdx+55]	= texCoord[3];
		}
		// 清除剩余的纹理坐标
		while(i<8) {
			ptc[baseIdx]	= 1.0f;
			ptc[baseIdx+1]	= 1.0f;
			ptc[baseIdx+18]	= 1.0f;
			ptc[baseIdx+19]	= 1.0f;
			ptc[baseIdx+36]	= 1.0f;
			ptc[baseIdx+37]	= 1.0f;
			ptc[baseIdx+54]	= 1.0f;
			ptc[baseIdx+55]	= 1.0f;
			baseIdx+=2;
			i+=1;
		}
		mark_buffer(m_texUpdate,(GLubyte*)ptc-(GLubyte*)m_pTexCoordBuff,sizeof(TILE_TEXCOORD)*4);
	}
}

void xtilemap::mark_buffer(xupdate_record &recs, unsigned offset, unsigned size)
{
	unsigned up_end=offset+size, upper_bound;
	xupdate_record::iterator iter, end;
	iter=recs.begin();
	end=recs.end();
	for(; iter!=end; ++iter) {
		upper_bound=iter->first+iter->second;
		if(offset>=iter->first && offset<=upper_bound) {
			if(up_end>upper_bound)
				iter->second+=up_end-upper_bound;
			return;
		}
		else if(up_end==iter->first) {
			iter->first=offset;
			iter->second+=size;
			return;
		}
	}
	recs.push_back(std::pair<unsigned,unsigned>(offset,size));
}

