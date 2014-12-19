#ifndef __HEADER_PSD_LOADER
#define __HEADER_PSD_LOADER

#include <string>
#include "basedef.h"

namespace wyc
{

struct PSD_HEADER
{
	uint8_t signature[4];
	uint16_t version;
	uint8_t reserved[6];
	uint16_t channel;
	uint32_t height;
	uint32_t width;
	uint16_t depth;
	uint16_t mode;
};

#define MAX_LAYER_NAME 31

struct PSD_LAYER
{
	int32_t top;
	int32_t left;
	int32_t bottom;
	int32_t right;
	uint16_t channel;
	char name[MAX_LAYER_NAME+1];
};

class xpsd_loader
{
	std::string m_name;
	PSD_HEADER m_header;
	PSD_LAYER *m_layers;
	unsigned m_layerNum;
public:
	xpsd_loader();
	~xpsd_loader();
	void clear();
	bool load(const char *filepath);
	unsigned width() const;
	unsigned height() const;
	unsigned short depth() const;
	unsigned layer_num() const;
	const PSD_LAYER* get_layer(unsigned idx) const;
	bool save_as_imageset(const char *filepath) const;
};

inline unsigned xpsd_loader::width() const
{
	return m_header.width;
}

inline unsigned xpsd_loader::height() const
{
	return m_header.height;
}

inline unsigned short xpsd_loader::depth() const
{
	return m_header.depth;
}

inline unsigned xpsd_loader::layer_num() const
{
	return m_layerNum;
}

}; // namespace wyc

#endif // end of __HEADER_PSD_LOADER

