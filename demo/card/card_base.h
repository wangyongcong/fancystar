#ifndef __HEADER_WYC_CARD_BASE
#define __HEADER_WYC_CARD_BASE

#include "wyc/obj/object.h"

namespace wyc
{

struct xcard_data
{
	unsigned index;
	std::wstring name;
	std::wstring type;
	std::wstring desc;
	std::wstring avatar;
	uint8_t cost;
	uint8_t max_hp;
	uint8_t strength;
	uint8_t rarity;
};

enum CARD_RARITY {
	RARITY_COMMON =0,
	RARITY_UNCOMMON,
	RARITY_RARE,
	RARITY_MYTHIC_RARE,
	
	RARITY_COUNT
};

const wchar_t *get_rarity_desc(CARD_RARITY r);

class xcard_packet
{
	std::wstring m_version;
	unsigned m_card_index;
	xdict m_cards;
public:
	typedef xdict::iterator iterator;
	typedef xdict::const_iterator const_iterator;
	xcard_packet();
	~xcard_packet();
	bool load (const char *file_name);
	bool save (const char *file_name);
	xcard_data* get_card(unsigned id);
	xcard_data* new_card();
	void del_card(unsigned id);
	void clear();
	inline iterator begin() {
		return m_cards.begin();
	}
	inline iterator end() {
		return m_cards.end();
	}
	inline const_iterator begin() const {
		return m_cards.begin();
	}
	inline const_iterator end() const {
		return m_cards.end();
	}
	bool empty() const {
		return m_card_index==0;
	}
	inline void set_version(const std::wstring &ver) {
		m_version = ver;
	}
	inline const std::wstring& version() const {
		return m_version;
	}
	inline unsigned count() const {
		return m_card_index;
	}
};

class xcard_render_context 
{
protected:
	int m_dirty_flag;
public:
	enum REDRAW_FLAG {
		REDRAW_NONE=0,
		REDRAW_ALL=0xF,
	};
	xcard_render_context() {
		m_dirty_flag=0;
	}
	virtual ~xcard_render_context() {}
	virtual void redraw(int flag=REDRAW_ALL) {
		m_dirty_flag|=flag;
	}
	inline bool is_dirty() const {
		return m_dirty_flag!=0;
	}
	inline int dirty_flag() const {
		return m_dirty_flag;
	}
	inline void reset() {
		m_dirty_flag=0;
	}
};


class xcard : public xobject
{
	USE_RTTI;
	const xcard_data *m_basic_data;
	xcard_render_context *m_rc;
	uint8_t m_hp;
	bool m_redraw;
public:
	xcard() 
	{
		m_basic_data=0;
		m_rc=0;
		m_hp=0;
	}
	void initialize (const xcard_data *data)
	{
		m_basic_data = data;
		if(data) {
			m_hp = data->max_hp;
		}
		if(m_rc)
			m_rc->redraw();
	}
	inline const xcard_data* get_card_data() const 
	{
		return m_basic_data;
	}
	inline void redraw(int flag=xcard_render_context::REDRAW_ALL) {
		if(m_rc)
			m_rc->redraw(flag);
	}

//----------------------------------------------------------------------
// Internal interfaces. 
// Don't call these functions unless you know exactly what you're doing.

	inline void set_render_context(xcard_render_context *rc) {
		assert(!m_rc && "Can't be assigned to two renderers");
		m_rc = rc;
	}
};

}; // namespace wyc

#endif // __HEADER_WYC_CARD_BASE

