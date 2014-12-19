#ifndef __INLINE_WYC_XHASH
#define __INLINE_WYC_XHASH

namespace wyc
{

//====================================================================
// xdict inline functions
//====================================================================

inline xdict::xdict(size_t capacity) : m_table(capacity) 
{
}

inline void xdict::reserve(size_t size) {
	m_table.reserve(size);
}

inline void xdict::clear() {
	m_table.clear();
}

inline bool xdict::is_valid(value_t v)
{
	return HT_EMPTY_POINTER!=v && HT_DUMMY_POINTER!=v;
}

inline bool xdict::add(key_t key, value_t data) {
	if(is_valid(data))
		return m_table.add(key,entry_t(key,data));
	return false;
}

inline bool xdict::del(key_t key) {
	return m_table.del(key);
}

inline xdict::value_t xdict::pop(key_t key, value_t def) {
	entry_t item;
	if(m_table.pop(key,item)) {
		return item.second;
	}
	return def;
}

inline xdict::value_t xdict::get(key_t key, value_t def) const {
	entry_t item;
	if(m_table.get(key,item)) {
		return item.second;
	}
	return def;
}

inline void xdict::set(key_t key, value_t data) {
	if(is_valid(data)) {
		entry_t item(key,data);
		if(!m_table.set(key,item))
			m_table.add(key,item);
	}
}

inline bool xdict::contain(key_t key) const {
	return m_table.contain(key);
}

inline size_t xdict::size() const {
	return m_table.size();
}

inline size_t xdict::capacity() const {
	return m_table.capacity();
}

inline xdict::iterator xdict::begin() {
	return m_table.begin();
}

inline xdict::const_iterator xdict::begin() const {
	return m_table.begin();
}

inline xdict::iterator xdict::end() {
	return m_table.end();
}

inline xdict::const_iterator xdict::end() const {
	return m_table.end();
}

inline xdict::proxy_t xdict::operator [] (key_t key)
{
	size_t pos=m_table.find(key);
	return proxy_t(&m_table,key,pos);
}

inline void* xdict::operator [] (key_t key) const
{
	size_t pos=m_table.find(key);
	if(HT_NULL_POS==pos)
		return 0;
	return m_table[pos].second;
}

inline void xdict::set_auto_shrink(bool b) {
	m_table.set_auto_shrink(b);
}

inline bool xdict::is_auto_shrink() const {
	return m_table.is_auto_shrink();
}


//====================================================================
// xset inline functions
//====================================================================

inline xset::xset(size_t capacity) : m_table(capacity) 
{
}

inline void xset::reserve(size_t size) {
	m_table.reserve(size);
}

inline void xset::clear() {
	m_table.clear();
}

inline bool xset::add(value_t data) {
	if(HT_EMPTY_POINTER==data || HT_DUMMY_POINTER==data) 
		return false;
	return m_table.add(data,data);
}

inline bool xset::del(value_t data) {
	if(HT_EMPTY_POINTER==data || HT_DUMMY_POINTER==data) 
		return false;
	return m_table.del(data);
}

inline bool xset::contain(value_t data) const {
	if(HT_EMPTY_POINTER==data || HT_DUMMY_POINTER==data)
		return false;
	return m_table.contain(data);
}

inline size_t xset::size() const {
	return m_table.size();
}

inline size_t xset::capacity() const {
	return m_table.capacity();
}

inline xset::const_iterator xset::begin() const {
	return m_table.begin();
}

inline xset::const_iterator xset::end() const {
	return m_table.end();
}

inline void xset::set_auto_shrink(bool b) {
	m_table.set_auto_shrink(b);
}

inline bool xset::is_auto_shrink() const {
	return m_table.is_auto_shrink();
}

} // namespace wyc

#endif // __INLINE_WYC_XHASH

