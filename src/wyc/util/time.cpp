#include <string>
#include "wyc/util/time.h"
#include "wyc/util/util.h"

namespace wyc 
{

xcode_statist::xcode_statist()
{
	m_pci=0;
}

void xcode_statist::func_begin(const std::string &funcName)
{
	xfuncmap_t::iterator iter=m_funcmap.find(funcName);
	if(iter==m_funcmap.end()) {
		CALLINFO *pci;
		pci=new CALLINFO;
		pci->called=1;
		pci->time_used=0;
		pci->max_used=0;
		std::pair<xfuncmap_t::iterator,bool> ret=m_funcmap.insert(xfuncmap_t::value_type(funcName,pci));
		if(!ret.second) {
			wyc_error("xcode_statist::func_begin failed with name [%s]",funcName.c_str());
			m_pci=0;
			return;
		}
		m_pci=pci;
	}
	else m_pci=iter->second;
	m_tbeg=xtime_source::singleton().get_time();
}

void xcode_statist::func_end()
{
	double t=xtime_source::singleton().get_time()-m_tbeg;
	if(m_pci) {
		m_pci->called+=1;
		m_pci->time_used+=t;
		if(t>m_pci->max_used)
			m_pci->max_used=t;
		m_pci=0;
	}
}

const xcode_statist::CALLINFO* xcode_statist::get_called_info(const std::string &funcName) const
{
	xfuncmap_t::const_iterator iter=m_funcmap.find(funcName);
	if(iter==m_funcmap.end()) 
		return 0;
	return iter->second;
}

void xcode_statist::report() const
{
	xfuncmap_t::const_iterator iter, end=m_funcmap.end();
	wyc_print("name\tcalled\ttotal\taverage\tmax");
	for(iter=m_funcmap.begin(); iter!=end; ++iter)
	{
		CALLINFO *pcf=iter->second;
		wyc_print("%s\t%d\t%.4f\t%.4f\t%.4f",iter->first.c_str(),\
			pcf->called,pcf->time_used,pcf->time_used/pcf->called,pcf->max_used);
	}
}

} // namespace wyc

