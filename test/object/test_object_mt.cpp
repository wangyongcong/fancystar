#include <windows.h>
#include <process.h>
#include "wyc/util/util.h"
#include "wyc/util/time.h"
#include "wyc/obj/object.h"
#include "wyc/mem/memtracer.h"

using namespace wyc;

//--------------------------------------------------------------------------------

struct TEST_CONTEXT
{
	enum {
		OBJECT_CONTEXT_COUNT=4,
		GROUP_PLAYERS=0,
		GROUP_ITEMS,
		GROUP_COUNT,
	};
	wyc::xobject_group* m_groups[GROUP_COUNT];
	unsigned m_frameCount;
	xatom_int m_playerCount, m_itemCount;
	xatom_int m_playerDelete, m_itemDelete;
	xatom_int m_workComplete;
	xatom_int m_eventError;
	HANDLE m_completeEvent;
	wyc::xobject::context *m_objContext;
	void initialize();
	void terminate();
	inline bool exit() const {
		return m_playerDelete==m_playerCount;
	}

};
TEST_CONTEXT g_context;

static xobject::context* select_context(xobject *)
{
	unsigned idx=0;
	int mincnt=0x7FFFFFFF, cnt;
	for(unsigned i=0; i<TEST_CONTEXT::OBJECT_CONTEXT_COUNT; ++i) {
		cnt=g_context.m_objContext[i].object_count();
		if(cnt<mincnt) {
			mincnt=cnt;
			idx=i;
		}
	}
	return g_context.m_objContext+idx;
}

//--------------------------------------------------------------------------------

class xtest_object : public xobject
{
	USE_EVENT_MAP;
public:
	xtest_object() {
	}
	enum EVENT_ID 
	{
		ON_JOIN_GROUOP=0xBDA8003,
		DELSELF=0xBC615552,
		DUMMY_WORK=0xC4596126,
	};
protected:
	void dummy_work(double used_time)
	{
		double work=0, begin=wyc::xtime_source().singleton().get_time();
		do {
			for (int i = 1; i < 1000; ++i)
				work += 1.0 / i;
		} while (wyc::xtime_source().singleton().get_time()-begin<used_time);
	}
private:
	void delself(wyc::xobjevent*)
	{
		this->delthis();
	}
	void on_join_group(wyc::xobjevent *pev) 
	{
		int ret;
		if(!((xpackev*)pev)->unpack("d",&ret)) {
			++g_context.m_eventError;
			return;
		}
		if(ret==0) 
			printf("(%p) join group ok\n",this);
		else
			printf("(%p) join group err: code=[%d]\n",this,ret);
	}
	void dummy_work(wyc::xobjevent *pev) 
	{
		float t=((wyc::xev_param32*)pev)->m_fval;
		dummy_work(t);
	}
};
BEGIN_EVENT_MAP(xtest_object,xobject)
	REG_EVENT(delself)
	REG_EVENT(on_join_group)
	REG_EVENT(dummy_work)
END_EVENT_MAP

//--------------------------------------------------------------------------------

class xtest_item : public xtest_object
{
	USE_EVENT_MAP;
public:
	xtest_item() {
		set_id(unsigned(g_context.m_itemCount++));
		printf("[item] create (context[%p]) %p\n",this,get_context());
	}
	virtual ~xtest_item() {
		g_context.m_itemDelete+=1;
		printf("[item] destroy %p\n",this);
	}
private:
	void on_join_group(wyc::xobjevent *pev) 
	{
		int ret;
		if(!((xpackev*)pev)->unpack("d",&ret)) {
			++g_context.m_eventError;
			return;
		}
		if(ret==0) 
			printf("[item] (%p) join group ok\n",this);
		else
			printf("[item] (%p) join group err: code=[%d], id=[%d]\n",this,ret,id());
	}
};
BEGIN_EVENT_MAP(xtest_item,xtest_object)
	REG_EVENT(on_join_group)
END_EVENT_MAP

//--------------------------------------------------------------------------------

class xtest_player : public xtest_object
{
	USE_EVENT_MAP;
	xobjptr m_spMyItem;
	unsigned m_countDown;
	enum {
		DEUMMY_WORK=0,
		DEL_SELF,
		CREATE_ITEM,
		DEL_ITEM,
		ACTION_COUNT
	};
	static const float ms_ratio[ACTION_COUNT];
public:
	xtest_player() {
		set_id(unsigned(g_context.m_playerCount++));
		printf("[player] create (context[%d]) %p\n",this,get_context());
	}
	virtual ~xtest_player() {
		g_context.m_playerDelete+=1;
		printf("[player] destroy %p\n",this);
	}
	virtual void update(double, double) 
	{
		if(m_spMyItem && m_spMyItem->dead()) {
			--m_countDown;
			printf("[player] (%p) item(%p) count down: %d\n",this,(xobject*)m_spMyItem,m_countDown);
			if(m_countDown<=0) 
				m_spMyItem=0;
		}
		float dice=wyc::random();
		float ratio=0;
		int i;
		for(i=0; i<ACTION_COUNT; ++i) {
			ratio+=ms_ratio[i];
			if(dice<ratio) 
				break;
		}
		switch(i) {
		case DEUMMY_WORK:
			dummy_work(0.0f);
			printf("[player] (%p) dummy work\n",this);
			break;
		case DEL_SELF:
			delthis();
			printf("[player] (%p) delete self [%d]\n",this,refcount());
			break;
		case CREATE_ITEM:
			create_item();
			printf("[player] (%p) create item\n",this);
			break;
		case DEL_ITEM:
			delete_item();
			printf("[player] (%p) try delete item\n",this);
			break;
		}
	}
	enum EVENT_ID {
		ON_FIND_ITEM=0x4783CEBF,
		ON_DELETE_ITEM=0x80956FA3,
	};
private:
	void create_item()
	{
		xtest_item *pitem=wyc_safecast(xtest_item,xobject::create_object("xtest_item"));
		xobjevent *pev=xpackev::pack("od",pitem,xtest_object::ON_JOIN_GROUOP);
		g_context.m_groups[TEST_CONTEXT::GROUP_ITEMS]->send_event(wyc::xobject_group::EV_JOIN,pev);
	}
	void delete_item()
	{
		xobjevent *pev=xpackev::pack("dod",unsigned(wyc::random()*g_context.m_itemCount),this,xtest_player::ON_DELETE_ITEM);
		g_context.m_groups[TEST_CONTEXT::GROUP_ITEMS]->send_event(wyc::xobject_group::EV_FIND,pev);
	}
	void on_find_item(wyc::xobjevent *pev) 
	{
		xobject *pobj=0;
		if(!((xpackev*)pev)->unpack("o",&pobj)) {
			++g_context.m_eventError;
			return;
		}
		if(pobj) {
			m_spMyItem=pobj;
			m_countDown=10;
		}
	}
	void on_delete_item(wyc::xobjevent *pev)
	{
		xobject *pobj=0;
		if(!((xpackev*)pev)->unpack("o",&pobj)) {
			++g_context.m_eventError;
			return;
		}
		if(pobj) {
			pobj->delthis();
			printf("[player] (%p) delete item %p [%d]\n",this,pobj,pobj->refcount());
		}
	}
	void on_join_group(wyc::xobjevent *pev) 
	{
		int ret;
		if(!((xpackev*)pev)->unpack("d",&ret)) {
			++g_context.m_eventError;
			return;
		}
		if(ret==0) 
			printf("[player] (%p) join group ok\n",this);
		else
			printf("[player] (%p) join group err: code=[%d], id=[%d]\n",this,ret,id());
	}
};
BEGIN_EVENT_MAP(xtest_player,xtest_object)
	REG_EVENT(on_join_group)
	REG_EVENT(on_find_item)
	REG_EVENT(on_delete_item)
END_EVENT_MAP

const float xtest_player::ms_ratio[ACTION_COUNT]={
	0.35f,	// DEUMMY_WORK
	0.10f,	// DEL_SELF
	0.20f,	// CREATE_ITEM
	0.35f,	// DEL_ITEM
};

//--------------------------------------------------------------------------------

void TEST_CONTEXT::initialize() 
{
	m_objContext=new wyc::xobject::context[OBJECT_CONTEXT_COUNT];
	wyc::xobject::set_context_selector(&select_context);
	// create groups
	for(int i=0; i<GROUP_COUNT; ++i) {
		m_groups[i]=wycnew wyc::xobject_group;
		m_groups[i]->incref();
	}
	unsigned initItemCount=400, initPlayerCount=200;
	xobjevent *pev;
	// create items
	xtest_item *pitem;
	for(unsigned i=0; i<initItemCount; ++i) 
	{
		pitem=wycnew xtest_item;
		pev=xpackev::pack("od",pitem,xtest_object::ON_JOIN_GROUOP);
		m_groups[GROUP_ITEMS]->send_event(wyc::xobject_group::EV_JOIN,pev);
	}
	// player join
	xtest_player *pplayer;
	for(unsigned i=0; i<initPlayerCount; ++i) 
	{
		pplayer=wycnew xtest_player;
		pev=xpackev::pack("od",pplayer,xtest_object::ON_JOIN_GROUOP);
		m_groups[GROUP_PLAYERS]->send_event(wyc::xobject_group::EV_JOIN,pev);

		pev=xpackev::pack("dod",unsigned(wyc::random()*initItemCount),pplayer,xtest_player::ON_FIND_ITEM);
		m_groups[GROUP_ITEMS]->send_event(wyc::xobject_group::EV_FIND,pev);
	}
	assert(m_itemCount==int(initItemCount));
	assert(m_playerCount==int(initPlayerCount));
	m_completeEvent=CreateEvent(NULL,FALSE,FALSE,NULL);
	m_frameCount=0;
}

void TEST_CONTEXT::terminate() 
{
	CloseHandle(m_completeEvent);
	m_completeEvent=NULL;
	for(int i=0; i<GROUP_COUNT; ++i)
		m_groups[i]->decref();
	printf("Frame:        %d\n",m_frameCount);
	printf("Item count:   %d\n",m_itemCount);
	printf("Item del:     %d\n",m_itemDelete);
	printf("Player count: %d\n",m_playerCount);
	printf("Player del:   %d\n",m_playerDelete);
	printf("Event Error:  %d\n",m_eventError);
	getchar();
	for(unsigned i=0; i<OBJECT_CONTEXT_COUNT; ++i)
		m_objContext[i].clear();
	delete [] m_objContext;
	m_objContext=0;
}

//--------------------------------------------------------------------------------

void CALLBACK handle_object_events(PTP_CALLBACK_INSTANCE pInstance, PVOID pContext, PTP_WORK)
{
	wyc::xobject::context *pobjctx=(wyc::xobject::context*)pContext;
	pobjctx->update_context();
	int wc=g_context.m_workComplete--;
	if(1==wc) {
		::SetEventWhenCallbackReturns(pInstance,g_context.m_completeEvent);
	}
	printf("[%d] object event handler\n",wc);
}

void CALLBACK update_group(PTP_CALLBACK_INSTANCE pInstance, PVOID pContext, PTP_WORK)
{
	std::pair<xobject_group*,double> *pGroupContext=(std::pair<xobject_group*,double>*)pContext;
	pGroupContext->first->update(0,pGroupContext->second);
	int wc=g_context.m_workComplete--;
	if(1==wc) {
		::SetEventWhenCallbackReturns(pInstance,g_context.m_completeEvent);
	}
	printf("[%d] group update (interval %f)\n",wc,pGroupContext->second);
}

//--------------------------------------------------------------------------------

void test_object()
{
	wyc::xdate dt;
	dt.get_date();
	wyc::random_seed(dt.hour()*10000+dt.minute()*100+dt.second());

	// create thread pools
	PTP_POOL ptp;
	TP_CALLBACK_ENVIRON cbenv;
	PTP_CALLBACK_ENVIRON pcbenv=&cbenv;
	
	printf("create thread pools\n");
	::InitializeThreadpoolEnvironment(pcbenv);
	ptp=::CreateThreadpool(NULL);
	::SetThreadpoolThreadMinimum(ptp,4);
	::SetThreadpoolThreadMaximum(ptp,8);
	::SetThreadpoolCallbackPool(pcbenv,ptp);

	xobject::process_init();
	g_context.initialize();

	PTP_WORK objEventHandlers[TEST_CONTEXT::OBJECT_CONTEXT_COUNT];
	for(int i=0; i<TEST_CONTEXT::OBJECT_CONTEXT_COUNT; ++i) {
		objEventHandlers[i]=CreateThreadpoolWork(&handle_object_events,g_context.m_objContext+i,pcbenv);
	}
	PTP_WORK groupUpdaters[TEST_CONTEXT::GROUP_COUNT];
	std::pair<xobject_group*,double> groupContext[2];
	for(int i=0; i<TEST_CONTEXT::GROUP_COUNT; ++i) {
		groupContext[i].first=g_context.m_groups[i];
		groupUpdaters[i]=CreateThreadpoolWork(&update_group,groupContext+i,pcbenv);
	}
	
	char splitter[81];
	memset(splitter,'-',sizeof(splitter));
	splitter[79]='\n';
	splitter[80]=0;
	printf(splitter);
	unsigned frameCount=0;
	double lastUpdate=wyc::xtime_source::singleton().get_time(), current, interval;
	while(!g_context.exit())
	{
		printf("Frame: %d\n",++frameCount);
		current=wyc::xtime_source::singleton().get_time();
		interval=current-lastUpdate;
		lastUpdate=current;
		// update context
		g_context.m_workComplete=TEST_CONTEXT::OBJECT_CONTEXT_COUNT;
		for(int i=0; i<TEST_CONTEXT::OBJECT_CONTEXT_COUNT; ++i)
			SubmitThreadpoolWork(objEventHandlers[i]);
		::WaitForSingleObject(g_context.m_completeEvent,INFINITE);
		assert(g_context.m_workComplete==0);
		// update groups
		g_context.m_workComplete=TEST_CONTEXT::GROUP_COUNT;
		for(int i=0; i<TEST_CONTEXT::GROUP_COUNT; ++i) {
			groupContext[i].second=interval;
			SubmitThreadpoolWork(groupUpdaters[i]);
		}
		::WaitForSingleObject(g_context.m_completeEvent,INFINITE);

		assert(g_context.m_workComplete==0);
		printf(splitter);
	}

	for(int i=0; i<TEST_CONTEXT::OBJECT_CONTEXT_COUNT; ++i)
		CloseThreadpoolWork(objEventHandlers[i]);
	for(int i=0; i<TEST_CONTEXT::GROUP_COUNT; ++i)
		CloseThreadpoolWork(groupUpdaters[i]);

	g_context.terminate();
	xobject::process_exit();

	// clean up thread pools
	printf("clean up thread pools\n");
	::CloseThreadpool(ptp);
	::DestroyThreadpoolEnvironment(pcbenv);

	printf(splitter);
	wyc::xmemtracer::singleton().report();
	printf(splitter);

}

