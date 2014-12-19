#include "wyc/util/var.h"
#include <cstring>
#include <cassert>
#include <cstdarg>

#ifdef _MSC_VER
#pragma warning(disable: 4996)
#endif // _MSC_VER

namespace wyc 
{

#ifdef _DEBUG
	static unsigned s_packet_counter=0;
#endif
	
xpackdata* xpackdata::alloc_packed_data(unsigned size)
{
	if(size<1) size=1;
	xpackdata *pData=(xpackdata*)new uint8_t[(sizeof(unsigned)<<1)+size];
	pData->m_nCounter=1;
	pData->m_nSize=size;
#ifdef _DEBUG
	s_packet_counter+=1;
#endif
	return pData;
}

void xpackdata::free_packed_data(xpackdata *pData)
{
	pData->m_nCounter-=1;
	if(pData->m_nCounter<1) { 
		uint8_t *pMem=(uint8_t*)pData;
		delete [] pMem;
#ifdef _DEBUG
		s_packet_counter-=1;
#endif
	}
}

#ifdef _DEBUG

unsigned xpackdata::packet_num()
{
	return s_packet_counter;
}

#endif


//==================================================================================

xvar& xvar::operator = (const char* str)
{
	clear_packet();
	if(str==0) {
		m_type=0;
		m_pData=0;
		return *this;
	}
	m_type=XVAR_TYPE_STR;
	unsigned size=(unsigned)strlen(str)+1;
	m_pPacket=xpackdata::alloc_packed_data(size);
	char* pstr=(char*)m_pPacket->buffer();
	strcpy(pstr,str);
	return *this;
}

xvar& xvar::operator = (const wchar_t* str)
{
	clear_packet();
	if(str==0) {
		m_type=0;
		m_pData=0;
		return *this;
	}
	m_type=XVAR_TYPE_WSTR;
	unsigned size=(unsigned)wcslen(str)+1;
	m_pPacket=xpackdata::alloc_packed_data(sizeof(wchar_t)*size);
	wchar_t* pstr=(wchar_t*)m_pPacket->buffer();
	wcscpy(pstr,str);
	return *this;
}

void xvar::set(const void *pdata, size_t size)
{
	assert(pdata);
	clear_packet();
	m_type=XVAR_TYPE_PACKED;
	m_pPacket=xpackdata::alloc_packed_data(size);
	memcpy(m_pPacket->buffer(),pdata,size);
}

bool xvar::get(void *pdata, size_t size) const
{
	assert(pdata);
	if(m_type!=XVAR_TYPE_PACKED || m_pPacket->size()!=size)
		return false;
	memcpy(pdata,m_pPacket->buffer(),size);
	return true;
}

//==================================================================================

void xargs::extend(const wyc::xargs &args)
{
	unsigned len=args.size();
	for(unsigned i=0; i<len; ++i) {
		m_array.push_back(args[i]);
	}
}

void xargs::extend(const wyc::xargs &args, unsigned beg, unsigned end)
{
	if(end>args.size())
		end=args.size();
	while(beg<end) {
		m_array.push_back(args[beg++]);
	}
}

bool xargs::check(const char *fmt) const
{
	unsigned pos=0;
	while(fmt[pos]!=0) {
		if(pos>=m_array.size())
			return false;
		if(m_array[pos].name()!=fmt[pos])
			return false;
		pos+=1;
	}
	return true;
}

const char* xargs::str() const
{
	static char *s_tmpstr=0;
	static unsigned s_maxlen=0;
	unsigned len=m_array.size();
	if(s_maxlen<len) {
		if(s_tmpstr) 
			delete [] s_tmpstr;
		s_tmpstr=new char[len];
		assert(s_tmpstr);
		s_maxlen=len;
	}
	char *iter=s_tmpstr;
	for(unsigned i=0; i<len; ++i) {
		*iter++=m_array[i].name();
	}
	*iter=0;
	return s_tmpstr;
}

bool xargs::scanf(const char *fmt,...)
{
	assert(fmt);
	va_list li;
	va_start(li,fmt);
	char c=*fmt++;
	unsigned i=0;
	while(c!=0 && i<m_array.size()) {
		switch(c) {
		case 'd':
			if(!m_array[i++].get(*va_arg(li,int*)))
				goto END;
			break;
		case 'u':
			if(!m_array[i++].get(*va_arg(li,unsigned*)))
				goto END;
			break;
		case 'f':
			if(!m_array[i++].get(*va_arg(li,float*)))
				goto END;
			break;
		case 'p':
			if(!m_array[i++].get(*va_arg(li,void**)))
				goto END;
			break;
		case 's':
			if(!m_array[i++].get(*va_arg(li,std::string*)))
				goto END;
			break;
		case 'w':
			if(!m_array[i++].get(*va_arg(li,std::wstring*)))
				goto END;
			break;
		default:
			goto END;
		}
		c=*fmt++;
	}
END:
	va_end(li);
	return c==0;
}

bool xargs::printf(const char *fmt,...)
{
	assert(fmt);
	unsigned len=strlen(fmt);
	if(len==0)
		return true;
	unsigned i=m_array.size();
	m_array.resize(i+len);
	va_list li;
	va_start(li,fmt);
	char c=*fmt++;
	while(c!=0) {
		switch(c) {
		case 'd':
			m_array[i++]=va_arg(li,int);
			break;
		case 'u':
			m_array[i++]=va_arg(li,unsigned);
			break;
		case 'f':
			// floatÊµ¼ÊÊÇdouble
			m_array[i++]=float(va_arg(li,double));
			break;
		case 'p':
			m_array[i++]=va_arg(li,void*);
			break;
		case 's':
			m_array[i++]=va_arg(li,const char*);
			break;
		case 'w':
			m_array[i++]=va_arg(li,const wchar_t*);
			break;
		default:
			goto END;
		}
		c=*fmt++;
	}
END:
	va_end(li);
	return c==0;
}

} // namespace wyc


