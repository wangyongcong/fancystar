#include "winpch.h"
#include <fstream>
#include "xutil.h"
#include "xconsole.h"
#include "psd_loader.h"

namespace wyc
{

inline bool read_bytes(std::istream &stream, void *buff, int size)
{
	stream.read((char*)buff,size);
	return stream.gcount()==size;
}

inline uint32_t get4b(std::istream &stream) {
	uint32_t val=stream.get()<<24;
	val|=stream.get()<<16;
	val|=stream.get()<<8;
	val|=stream.get();
	return val;
}

inline uint16_t get2b(std::istream &stream) {
	uint16_t val=stream.get()<<8;
	val|=stream.get();
	return val;
}

bool read_header(std::istream &stream, PSD_HEADER &header)
{
	read_bytes(stream,&header.signature,4);
	header.version=get2b(stream);
	read_bytes(stream,&header.reserved,6);
	header.channel=get2b(stream);
	header.height=get4b(stream);
	header.width=get4b(stream);
	header.depth=get2b(stream);
	header.mode=get2b(stream);
	return stream.good();
}

bool read_layer(std::istream &stream, PSD_LAYER &layer)
{
	layer.top=int32_t(get4b(stream));
	if(layer.top<0)
		layer.top=0;
	layer.left=int32_t(get4b(stream));
	if(layer.left<0)
		layer.left=0;
	layer.bottom=int32_t(get4b(stream));
	if(layer.bottom<0)
		layer.bottom=0;
	layer.right=int32_t(get4b(stream));
	if(layer.right<0)
		layer.right=0;
	layer.channel=get2b(stream);
	// skip channel info
	uint32_t len=layer.channel*6+12;
	stream.seekg(len,std::ios_base::cur);
	uint32_t extra_len=get4b(stream);
	uint32_t extra_start=stream.tellg();
	// layer mask data
	len=get4b(stream);
	if(len>0)
		stream.seekg(len,std::ios_base::cur);
	// layer blending range
	len=get4b(stream);
	if(len>0)
		stream.seekg(len,std::ios_base::cur);
	// layer name
	len=stream.get();
	if(len>MAX_LAYER_NAME) {
		len=MAX_LAYER_NAME;
		wyc_warn("Layer name 截断到%d个字符",len);
	}
	read_bytes(stream,layer.name,len);
	layer.name[len]=0;
	len=((len+3)&(~3))-len-1;
	if(len)	
		stream.seekg(len,std::ios_base::cur);
	// extra data
	/*
	char signature[4];
	read_bytes(stream,signature,4);
	char keycode[4];
	read_bytes(stream,keycode,4);
	len=get4b(stream);
	*/
	stream.seekg(extra_start+extra_len);
	return stream.good();
}

void print_layer(const PSD_LAYER &layer) {
	wyc_print("Layer: %s",layer.name);
	wyc_print("\trect: (%d,%d,%d,%d)",layer.left,layer.top,layer.right,layer.bottom);
	wyc_print("\tsize: (%d,%d)",layer.right-layer.left,layer.bottom-layer.top);
	wyc_print("\tchannel: %d",layer.channel);
}

xpsd_loader::xpsd_loader()
{
	m_layers=0;
	m_layerNum=0;
}

xpsd_loader::~xpsd_loader()
{
	clear();
}

void xpsd_loader::clear()
{
	if(m_layers) {
		delete [] m_layers;
		m_layers=0;
		m_layerNum=0;
	}
}

void extract_filename(std::string &filepath, bool bExt=true)
{
	std::string::size_type beg=filepath.rfind('/');
	if(beg!=std::string::npos)
		beg+=1;
	std::string::size_type end=filepath.rfind('\\');
	if(end!=std::string::npos)
		end+=1;
	if(beg>end)
		beg=end;
	if(!bExt) {
		end=filepath.rfind('.');
		if(end<beg)
			end=std::string::npos;
	}
	filepath=filepath.substr(beg,end-beg);
}

bool xpsd_loader::load(const char *pfile)
{
	clear();
	std::fstream fs(pfile,std::ios_base::in | std::ios_base::binary);
	if(!fs.is_open()) {
		wyc_error("无法读取文件: %s",pfile);
		return false;
	}
	m_name=pfile;
	extract_filename(m_name,false);
	uint32_t len;
	// file header
	if(!read_header(fs,m_header)) {
		wyc_error("文件头错误");
		goto CLOSE_FILE_AND_EXIT;
	}
	if(memcmp(&m_header.signature,"8BPS",4)!=0) {
		wyc_error("文件格式错误");
		goto CLOSE_FILE_AND_EXIT;
	}
	// color mode data
	if(!read_bytes(fs,&len,4)) {
		wyc_error("Color Mode Section Error");
		goto CLOSE_FILE_AND_EXIT;
	}
	if(len>0) 
		fs.seekg(len,std::ios_base::cur);
	// image resource
	len=get4b(fs);
	if(len>0) 
		fs.seekg(len,std::ios_base::cur);
	// layer section
	len=get4b(fs); //layer and mask length
	len=get4b(fs); // layer info length
	int16_t num_layers=int16_t(get2b(fs));
	if(num_layers<0) {
		num_layers=-num_layers;
	}
	if(num_layers) {
		m_layers=new PSD_LAYER[num_layers];
		m_layerNum=num_layers;
		for(uint16_t i=0; i<num_layers; ++i) {
			if(!read_layer(fs,m_layers[i])) {
				wyc_error("Layer[%d] Data Error",i);
				goto CLOSE_FILE_AND_EXIT;
			}
		}
	}
	fs.close();
	return true;
CLOSE_FILE_AND_EXIT:
	fs.close();
	return false;
}

bool xpsd_loader::save_as_imageset(const char *filepath) const
{
	std::fstream fs(filepath,std::ios_base::out | std::ios_base::trunc);
	if(!fs.is_open())
	{
		wyc_error("无法创建文件: %s",filepath);
		return false;
	}
	wyc_print("generating [%s.imageset]...",m_name.c_str());
	fs<<"<?xml version=\"1.0\"?>"<<std::endl;
	fs<<"<Imageset Name=\""<<m_name<<"\" Imagefile=\""<<m_name<<".png\" NativeHorzRes=\"800\" NativeVertRes=\"600\" AutoScaled=\"false\">"<<std::endl;
	PSD_LAYER *layer;
	unsigned w, h;
	for(unsigned i=0; i<m_layerNum; ++i) {
		layer=m_layers+i;
		w=layer->right-layer->left;
		h=layer->bottom-layer->top;
		if(w>0 && h>0) {
			wyc_print("\timage [%s]",layer->name);
			fs<<"\t<Image Name=\""<<layer->name<<"\" XPos=\""<<layer->left<<"\" YPos=\""<<layer->top\
				<<"\" Width=\""<<w<<"\" Height=\""<<h<<"\" />"<<std::endl;
		}
	}
	fs<<"</Imageset>\n";
	fs.close();
	wyc_print("total %d image",m_layerNum);
	wyc_print("save file OK: %s",filepath);
	return true;
}

}; // namespace wyc
