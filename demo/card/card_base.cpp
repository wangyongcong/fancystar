#include "fscorepch.h"
#include "card_base.h"
#include <fstream>
#include "wyc/util/fjson.h"
#include "wyc/util/strutil.h"

namespace wyc
{

const wchar_t *get_rarity_desc(CARD_RARITY r)
{
	static const wchar_t *ls_desc[RARITY_COUNT] = {
		L"Common",
		L"Uncommon",
		L"Rare",
		L"Mythic Rare",
	};
	return ls_desc[r];
}

xcard_packet::xcard_packet()
{
	m_card_index=0;
	m_cards.reserve(64);
}

xcard_packet::~xcard_packet()
{
	clear();
}

void xcard_packet::clear()
{
	for(xdict::iterator iter=m_cards.begin(), end=m_cards.end();
		iter!=end; ++iter) {
			delete (xcard_data*)iter->second;
	}
	m_cards.clear();
	m_card_index = 0;
	m_version.clear();
}

xcard_data* xcard_packet::get_card(unsigned id)
{
	return (xcard_data*)m_cards.get(id);
}

xcard_data* xcard_packet::new_card()
{
	xcard_data* cd = new xcard_data();
	cd->index = ++m_card_index;
	cd->cost	= 0;
	cd->strength= 0;
	cd->max_hp	= 0;
	cd->rarity	= RARITY_COMMON;
	if(!m_cards.add(cd->index,cd)) {
		unsigned id = cd->index;
		delete cd;
		return (xcard_data*)m_cards.get(id);
	}
	return cd;
}

void xcard_packet::del_card(unsigned id)
{
	xcard_data *cd = (xcard_data*)m_cards.pop(id);
	if(cd) delete cd;
}

bool xcard_packet::load (const char *file_name)
{
	xjson json;
	if(!json.load_file(file_name))
		return false;
	clear();
	const char *s;
	s=json.get<const char*>("version");
	if(s) 
		str2wstr_utf8(m_version,s);
	else
		m_version = L"";
	m_card_index = json.get<int>("card_index");
	const vjson::json_value *cards = json.get_value("cards");
	if(!cards || vjson::JSON_ARRAY!=cards->type)
		return true;
	unsigned count = json.get<int>("card_count");
	if(count) 
		m_cards.reserve(count);
	xcard_data *cd;
	for(const vjson::json_value *iter = cards->first_child; iter; iter=iter->next_sibling)
	{
		cd=new xcard_data();
		cd->index=json.get<int>("index",iter);
		s=json.get<const char*>("name",iter);
		if(s) str2wstr_utf8(cd->name,s);
		s=json.get<const char*>("type",iter);
		if(s) str2wstr_utf8(cd->type,s);
		s=json.get<const char*>("desc",iter);
		if(s) str2wstr_utf8(cd->desc,s);
		s=json.get<const char*>("avatar",iter);
		if(s) str2wstr_utf8(cd->avatar,s);
		cd->cost=(uint8_t)json.get<int>("cost",iter);
		cd->max_hp=(uint8_t)json.get<int>("max_hp",iter);
		cd->strength=(uint8_t)json.get<int>("strength",iter);
		cd->rarity=(uint8_t)json.get<int>("rarity",iter);
		m_cards.add(cd->index,cd);
		if(cd->index>m_card_index)
			m_card_index=cd->index;
	}
	return true;
}

bool xcard_packet::save (const char *file_name)
{
	std::fstream fs(file_name, std::ios_base::out|std::ios_base::trunc);
	if(!fs.is_open()) 
		return false;
	xcard_data *card;
	xjson_writer json(fs);
	json.begin_object();
		json.add_entry("version",m_version);
		json.add_entry("card_index",m_card_index);
		json.add_entry("card_count",m_cards.size());
		// all cards' info
		json.begin_array("cards");
		for(xdict::iterator iter=m_cards.begin(), end=m_cards.end();
			iter!=end; ++iter) {
				card = (xcard_data*)iter->second;
				json.begin_object();
					json.add_entry("index",card->index);
					json.add_entry("name",card->name);
					json.add_entry("type",card->type);
					json.add_entry("cost",card->cost);
					json.add_entry("max_hp",card->max_hp);
					json.add_entry("strength",card->strength);
					json.add_entry("rarity",card->rarity);
					json.add_entry("desc",card->desc);
					json.add_entry("avatar",card->avatar);
				json.end_object();
		}
		json.end_array();
	json.end_object();
	fs.close();
	return true;
}


//-------------------------------------------------------------------

REG_RTTI(xcard,xobject)

}; // namespace wyc

