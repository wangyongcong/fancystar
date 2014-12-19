#ifndef _AOI_HPP_
#define _AOI_HPP_
#include <list>
#include <map>
#include <string>

namespace aoi
{
// flag
#define FLAG_MAKE_CROSS 	2
#define FLAG_WANT_CROSS 	1
#define FLAG_LOWER_BOUND    4
#define FLAG_UPPER_BOUND    8

// type
#define IS_ENTITY			1
#define IS_AOI				2
#define IS_BAR				4

    const float ORIGIN_POS = -127.0f;

    class group;
	class manager;

    class point
    {
    public:
        point(float x = 0, float y = 0);
        point(const point &p);
        point& operator=(const point& rhs);
    public:
        float		x;
        float		y;
    };

    class range
    {
    public:
        range(float from, float to);
        bool in(float value);
    public:
        float from;
        float to;
    };

    class rect
    {
    public:
        rect(float left = 0, float top = 0, float right = 0, float bottom = 0);
        rect(const point& p1, const point& p2);
        float height();
        float width();
    public:
        float left;
        float top;
        float right;
        float bottom;
    };

    class trigger
    {
        friend class group;
        friend class linklist;
        friend class manager;

    public:
        trigger *_header;
        trigger *_prev;
        trigger *_next;
        group	*_owner;
        point	_pos;
        int 	_flag;
        int     _order;
    public:
        trigger();
        trigger(group* owner, const point& pos, int flag = 0, int order = 0);
        trigger(const trigger& value);
        trigger& operator=(const trigger& rhs);
        ~trigger();
        void check_x();
        void check_y();
        void check();
        void dump();
        void force_shufflex();
        void force_shuffley();
        bool shufflex(const point& oldpos, void* context);
        bool shuffley(const point& oldpos, void* context);
        void on_crossedx(trigger *node, bool positive, const point & oldpos, void* context);
        void on_crossedy(trigger *node, bool positive, const point & oldpos, void* context);
    };

    class group
    {
        friend class trigger;
        friend class linklist;
        friend class manager;
    public:
		unsigned int _id ;
        int     _entity;
        int 	_type;
		int		_self_mask;
		int		_imask;
        point	_center;
        rect    _area;
        point  	_oldcenter;
        rect   	_oldarea;
        trigger _triggers[4]; // 0,1-x; 2,3-y;
        manager * _manager;
    public:
        group(const rect& area, int entity, int type, unsigned int self_mask, unsigned int imask, unsigned int id);
        group& operator=(const group&rhs);
        virtual ~group();
        void _move(int x, int y, void* context);
        void move_to(float x, float y, void* context);
        bool was_in_xrange(float value);
        bool was_in_yrange(float value);
        bool is_in_xrange(float value);
        bool is_in_yrange(float value);
        bool shufflex(float x, float y, void* context);
        bool shuffley(float x, float y, void* context);
        virtual void on_trigger_enter(trigger* node, void* context);
        virtual void on_trigger_leave(trigger* node, void* context);
    };

    class groups
    {
        friend class group;
        friend class manager;
    public:
        typedef std::map<std::string, group*> container;
        container _groups;
    public:
        groups();
        ~groups();
        group* add(const std::string & name, group* value);
        group* remove(const std::string & name);
        void move_to(float x, float y, void* context);
        void _internal_move_to(int x, int y, void* context);
    };

    class linklist
    {
    public:
        trigger *_header;
    public:
        linklist();
        ~linklist();
        void add(trigger& value);
        void remove(trigger& value);
        void arrange();
        void dump();
    };

    class manager
    {
    public:
        linklist _list[2];
    public:
        void add(group* value, float x, float y, void* context);
        void remove(group* value, void* context);
        void add(groups* value, float x, float y, void* context);
        void remove(groups* value, void* context);
    };
};

#ifdef _MYAOI_STAT
struct _myaoi_stat
{
	int manager_new;
	int manager_delete;
	
	int groups_new;
	int groups_delete;

	int group_new;
	int group_delete;

	int trigger_new;
	int trigger_delete;
};
extern struct _myaoi_stat * _myaoi_st;
#endif

#endif
