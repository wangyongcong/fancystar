#include "winpch.h"
#include "wyc/obj/object.h"

using namespace wyc;

namespace 
{

static unsigned s_allocCount=0;

xobjevent* test_event_alloc(size_t size) {
	s_allocCount+=1;
	return (xobjevent*) new char[size];
}

void test_event_free(xobjevent *pev) {
	s_allocCount-=1;
	delete [] (char*)pev;
}

} // anonymous namespace

void test_packet() 
{
	wyc_print("Testing data packet...");

	xobjevent::allocator_t old_allocator=xobjevent::set_allocator(test_event_alloc);
	xobjevent::reclaimer_t old_reclaimer=xobjevent::set_reclaimer(test_event_free);

	int ival=rand(), ival2;
	float fval=float(rand())/RAND_MAX, fval2;
	const char *s="Hello, world!", *s2;
	const wchar_t *ws=L"ÃÎ»ÃÖ®ÐÇ", *ws2;

	xpackev *pev=xpackev::pack("dfsu",ival,fval,s,ws);
	pev->unpack("dfsu",&ival2,&fval2,&s2,&ws2);
	assert(ival==ival2);
	assert(fval==fval2);
	assert(strcmp(s,s2)==0);
	assert(wcscmp(ws,ws2)==0);
	pev->delthis();

	pev=xpackev::pack("pp",s,ws);
	pev->unpack("pp",&s2,&ws2);
	assert(s==s2);
	assert(ws==ws2);
	pev->delthis();

	xobject *pobj;
	xobjptr spobj=wycnew xobject, spobj2;
	assert(spobj->refcount()==1);
	pev=xpackev::pack("ox",(xobject*)spobj,spobj);
	assert(spobj->refcount()==3);
	pev->unpack("ox",&pobj,&spobj2);
	assert(spobj==pobj);
	assert(spobj==spobj2);
	assert(spobj->refcount()==4);
	pev->delthis();
	assert(spobj->refcount()==2);
	spobj2=0;
	assert(spobj->refcount()==1);
	spobj=0;

	int pi[4]={1,2,3,4}, *pi2;
	float pf[4]={1.5f,2.5f,3.5f,4.5f}, *pf2;
	xobject *po[4]={0,0,0,0}, **po2;
	xobjptr spo[4], *spo2;
	for(int i=0; i<4; ++i) { 
		po[i]=wycnew xobject;
		spo[i]=po[i];
	}
	pev=xpackev::pack("4d4f4p",pi,pf,po);
	pev->unpack("4d4f4p",&pi2,&pf2,&po2);
	for(int i=0; i<4; ++i) {
		assert(pi[i]==pi2[i]);
	}
	for(int i=0; i<4; ++i) {
		assert(pf[i]==pf2[i]);
	}
	for(int i=0; i<4; ++i) {
		assert(po[i]==po2[i]);
	}
	pev->delthis();

	pev=xpackev::pack("4o4x",po,spo);
	pev->unpack("4o4x",&po2,&spo2);
	for(int i=0; i<4; ++i) {
		assert(po[i]==po2[i]);
		assert(spo[i]==spo2[i]);
		assert(spo[i]->refcount()==3);
	}
	pev->delthis();
	for(int i=0; i<4; ++i) {
		assert(spo[i]->refcount()==1);
		spo[i]=0;
	}

	int lpi[16], *lpi2;
	for(int i=0; i<16; ++i)
		lpi[i]=i;
	pev=xpackev::pack("16d",lpi);
	pev->unpack("16d",&lpi2);
	for(int i=0; i<16; ++i)
		assert(lpi[i]==lpi2[i]);
	pev->delthis();
	assert(0==s_allocCount);

	xobjevent::set_allocator(old_allocator);
	xobjevent::set_reclaimer(old_reclaimer);

	wyc_print("All tests are passed");

}
