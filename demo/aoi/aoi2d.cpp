#include "aoi2d.h"
#include <cstdlib>
#include <cstdio>
#include <cassert>
#include <queue>

#define Assert(L, Condition)  assert(Condition)

namespace aoi
{
    inline point origin()
    {
        return point(ORIGIN_POS, ORIGIN_POS);
    }

    point::point(float x, float y)
            : x(x), y(y)
    {}

    point::point(const point &p)
            : x(p.x), y(p.y)
    {}

    point& point::operator=(const point& rhs)
    {
        x = rhs.x;
        y = rhs.y;
        return *this;
    }

    range::range(float from, float to)
            : from(from), to(to)
    {}

    bool range::in(float value)
    {
        return value >= from && value <= to;
    }

    rect::rect(float left, float top , float right, float bottom)
            : left(left), top(top), right(right), bottom(bottom)
    {}

    rect::rect(const point& p1, const point& p2)
            : left(p1.x), top(p1.y), right(p2.x), bottom(p2.y)
    {}

    float rect::height()
    {
        return bottom - top;
    }

    float rect::width()
    {
        return right - left;
    }

//-------------------------------------------------------------------------
    trigger::trigger()
            : _header(0), _prev(0), _next(0), _owner(0), _flag(0), _order(0)
    {
        _pos.x = ORIGIN_POS;
        _pos.y = ORIGIN_POS;
    };

    trigger::trigger(group* owner, const point& pos, int flag , int order)
            : _header(0), _prev(0), _next(0), _owner(owner), _pos(pos), _flag(flag), _order(order)
    {};

    trigger::trigger(const trigger& value)
            : _header(0),  _prev(value._prev), _next(value._next), _owner(value._owner),
            _pos(value._pos), _flag(value._flag), _order(value._order)
    {};

    trigger& trigger::operator=(const trigger& rhs)
    {
        _header = rhs._header;
        _prev   = rhs._prev;
        _next   = rhs._next;
        _owner  = rhs._owner;
        _pos    = rhs._pos;
        _flag   = rhs._flag;
        _order  = rhs._order;
        return *this;
    }

    trigger::~trigger()
    {
        _header = 0;
        _prev   = 0;
        _next   = 0;
        _owner  = 0;
        _flag   = 0;
        _order  = 0;
    }

    void trigger::dump()
    {
        if (this->_header == 0) return;
        trigger* cur = this->_header->_next;
        int i = 0;
        while (cur)
        {
            if ((i != 0) && (cur == this->_header->_next))
            {
                break;
            }
            //printf("[%d] type %d, flag %d (%d,%d), %x <- %x -> %x\n", cur->_owner->_entity, cur->_owner->_type, cur->_flag, cur->_pos.x, cur->_pos.y, (int)cur->_prev, (int)cur, (int)cur->_next);
            cur = cur->_next;
            i++;
        }
    }

    //交换相邻节点。 A节点在前，B节点在后。
    inline void exchange(trigger * A, trigger * B)
    {
        //printf("%p %p %p %p %p %p\n", A->_prev, A->_next, A, B->_prev, B->_next, B);
        assert(A != B && A != A->_header && B != B->_header);
        assert((A->_prev != A->_next) && (A->_prev != A) && (A->_next != A));
        assert((B->_prev != B->_next) && (B->_prev != B) && (B->_next != B));
        B->_prev = A->_prev;
        if (A->_prev)
            A->_prev->_next = B;
        A->_next = B->_next;
        if (B->_next)
            B->_next->_prev = A;

        B->_next = A;
        A->_prev = B;
        assert((A->_prev != A->_next) && (A->_prev != A) && (A->_next != A));
        assert((B->_prev != B->_next) && (B->_prev != B) && (B->_next != B));
    }
    inline void exchange_x(trigger * A, trigger * B)
    {
        assert(((B->_pos.x < A->_pos.x) || ((B->_pos.x == A->_pos.x) && B->_order <= A->_order)));
        exchange(A, B);
    }
    inline void exchange_y(trigger * A, trigger * B)
    {
        assert(((B->_pos.y < A->_pos.y) || ((B->_pos.y == A->_pos.y) && B->_order <= A->_order)));
        exchange(A, B);
    }

	/*
    void trigger::check_x()
    {
        if (this->_header == 0) return;
        assert(this->_header);
        trigger* cur = this->_header->_next;
        while (cur)
        {
            if (cur->_next)
            {
                if (cur->_pos.x > cur->_next->_pos.x);
                {
                    cur->force_shufflex();
                    break;
                }
            }
            cur = cur->_next;
        }
    }
    void trigger::check_y()
    {
        if (this->_header == 0) return;
        assert(this->_header);
        trigger* cur = this->_header->_next;
        while (cur)
        {
            if (cur->_next)
            {
                if (cur->_pos.y > cur->_next->_pos.y)
                {
                    cur->force_shuffley();
                    break;
                }
            }
            cur = cur->_next;
        }
    }
	*/

    void trigger::force_shufflex()
    {
        while (_prev && _prev->_pos.x > _pos.x)
        {
            exchange_x(_prev, this);
        }
        while (_next && _pos.x > _next->_pos.x)
        {
            exchange_x(this, _next);
        }
    }

    void trigger::force_shuffley()
    {
        while (_prev && _prev->_pos.y > _pos.y)
        {
            exchange_y(_prev, this);
        }
        while (_next && _pos.y > _next->_pos.y)
        {
            exchange_y(this, _next);
        }
    }

    bool trigger::shufflex(const point & oldpos, void* context)
    {
        point topos = _pos;
        // shuffle to the left(negative)
        while ((_prev != 0) && (_prev != _header) && ((_pos.x < _prev->_pos.x) || ((_pos.x == _prev->_pos.x) && _order <= _prev->_order)))
        {
            trigger* _saved_prev = _prev;
            trigger* _saved_next = _next;
            if ((this->_flag & FLAG_WANT_CROSS) && (_prev->_flag & FLAG_MAKE_CROSS))
            {
                this->on_crossedx(_prev, true, _prev->_pos, context);
            };

            if ((this->_flag & FLAG_MAKE_CROSS) && (_prev->_flag & FLAG_WANT_CROSS))
            {
                _prev->on_crossedx(this, false, oldpos, context);
            }

            //坐标发生改变
            if ((topos.x != _pos.x) || (topos.y != _pos.y))
            {
                return false;
            }

            if ((_saved_prev != _prev) || (_saved_next != _next))
            {
                Assert(context, (_prev != 0) ? (_prev->_pos.x <= _pos.x) : true);
                Assert(context, (_next != 0) ? (_pos.x <= _next->_pos.x) : true);
                return false;
            }


            exchange_x(_prev, this);
            Assert(context, (_prev != _next) && (_prev != this) && (_next != this));
            Assert(context, (_next != 0) ? (_pos.x <= _next->_pos.x) : true);
        }

        //printf("dump x1.\n");
        //dump();

        // shuffle to the right(positive)
        while ((_next != 0) && ((_pos.x > _next->_pos.x) || ((_pos.x == _next->_pos.x) && _order >= _next->_order)))
        {
            trigger* _saved_prev = _prev;
            trigger* _saved_next = _next;

            if ((this->_flag & FLAG_WANT_CROSS) && (_next->_flag & FLAG_MAKE_CROSS))
            {
                this->on_crossedx(_next, false, _next->_pos, context);
            };

            if ((this->_flag & FLAG_MAKE_CROSS) && (_next->_flag & FLAG_WANT_CROSS))
            {
                _next->on_crossedx(this, true, oldpos, context);
            }

            //坐标发生改变
            if ((topos.x != _pos.x) || (topos.y != _pos.y))
            {
                return false;
            }

            if ((_saved_prev != _prev) || (_saved_next != _next))
            {
                Assert(context, (_prev != 0) ? (_prev->_pos.x <= _pos.x) : true);
                Assert(context, (_next != 0) ? (_pos.x <= _next->_pos.x) : true);
                return false;
            }

            exchange_x(this, _next);
            Assert(context, (_prev != _next) && (_prev != this) && (_next != this));
            Assert(context, (_prev != 0) ? (_prev->_pos.x <= _pos.x) : true);
        }
        //printf("dump x2.\n");
        //dump();
        return true;
    }

    bool trigger::shuffley(const point & oldpos, void* context)
    {
        point topos = _pos;
        // shuffle to the left(negative)
        while ((_prev != 0) && (_prev != _header) && ((_pos.y < _prev->_pos.y) || ((_pos.y == _prev->_pos.y) && _order <= _prev->_order)))
        {
            trigger* _saved_prev = _prev;
            trigger* _saved_next = _next;
            if ((this->_flag & FLAG_WANT_CROSS) && (_prev->_flag & FLAG_MAKE_CROSS))
            {
                this->on_crossedy(_prev, true, _prev->_pos, context);
            }

            if ((this->_flag & FLAG_MAKE_CROSS) && (_prev->_flag & FLAG_WANT_CROSS))
            {
                _prev->on_crossedy(this, false, oldpos, context);
            }

            //坐标发生改变
            if ((topos.x != _pos.x) || (topos.y != _pos.y))
            {
                return false;
            }

            if ((_saved_prev != _prev) || (_saved_next != _next))
            {
                Assert(context, (_prev != 0) ? (_prev->_pos.y <= _pos.y) : true);
                Assert(context, (_next != 0) ? (_pos.y <= _next->_pos.y) : true);
                return false;
            }

            exchange_y(_prev, this);
            Assert(context, (_prev != _next) && (_prev != this) && (_next != this));
            Assert(context, (_next != 0) ? (_pos.y <= _next->_pos.y) : true);
        }

        // shuffle to the right(positive)
        while ((_next != 0) && ((_pos.y > _next->_pos.y) || ((_pos.y == _next->_pos.y) && _order >= _next->_order)))
        {
            trigger* _saved_prev = _prev;
            trigger* _saved_next = _next;
            if ((this->_flag & FLAG_WANT_CROSS) && (_next->_flag & FLAG_MAKE_CROSS))
            {
                this->on_crossedy(_next, false, _next->_pos, context);
            };

            if ((this->_flag & FLAG_MAKE_CROSS) && (_next->_flag & FLAG_WANT_CROSS))
            {
                _next->on_crossedy(this, true, oldpos, context);
            }

            //坐标发生改变
            if ((topos.x != _pos.x) || (topos.y != _pos.y))
            {
                return false;
            }

            if ((_saved_prev != _prev) || (_saved_next != _next))
            {
                Assert(context, (_prev != 0) ? (_prev->_pos.y <= _pos.y) : true);
                Assert(context, (_next != 0) ? (_pos.y <= _next->_pos.y) : true);
                return false;
            }

            exchange_y(this, _next);
            Assert(context, (_prev != _next) && (_prev != this) && (_next != this));
            Assert(context, (_prev != 0) ? (_prev->_pos.y <= _pos.y) : true);
        }
        return true;
    }

    void trigger::on_crossedx(trigger *node, bool positive, const point & oldpos, void* context)
    {
        //if (_owner->_entity == node->_owner->_entity) return;
        if (_owner->_id == node->_owner->_id) return;

		// this节点对应的对象在运动中越过了node节点对应的对象,
		// 而因为每个对象都有imask值来定义自己的感兴趣掩码，
		// 所以可以根据imask快速跳过自己不感兴趣的对象，从而达到加速处理
		if ((_owner->_imask & node->_owner->_self_mask) == 0) return;

        /*
        printf("[x]node %d oldpos(%d,%d), pos(%d,%d), self oldcenter(%d,%d), center(%d,%d), oldarea(%d,%d,%d,%d), area(%d,%d,%d,%d)\n",
               node->_owner->_entity,
               oldpos.x, oldpos.y, node->_pos.x, node->_pos.y,
               _owner->_oldcenter.x, _owner->_oldcenter.y,
               _owner->_center.x, _owner->_center.y,
               _owner->_oldarea.left, _owner->_oldarea.top, _owner->_oldarea.right, _owner->_oldarea.bottom,
               _owner->_area.left, _owner->_area.top, _owner->_area.right, _owner->_area.bottom);
        */

        bool is_valid = _owner->was_in_yrange(oldpos.y);
        if (is_valid == false) return;

        bool is_enter = (!!(this->_flag & FLAG_LOWER_BOUND) == positive);
        if (is_enter)
        {
            bool is_in = _owner->is_in_xrange(node->_pos.x) && _owner->is_in_yrange(node->_pos.y);
            if (is_in)
            {
                _owner->on_trigger_enter(node, context);
            }
        }
        else
        {
            bool was_in = _owner->was_in_xrange(oldpos.x);
            //&& _owner->was_in_yrange( oldpos.y ) ;
            if (was_in)
            {
                _owner->on_trigger_leave(node, context);
            }
        }
    }

    void trigger::on_crossedy(trigger *node, bool positive, const point & oldpos, void* context)
    {
        //if (_owner->_entity == node->_owner->_entity) return;
        if (_owner->_id == node->_owner->_id) return;

		// this节点对应的对象在运动中越过了node节点对应的对象,
		// 而因为每个对象都有imask值来定义自己的感兴趣掩码，
		// 所以可以根据imask快速跳过自己不感兴趣的对象，从而达到加速处理
		if ((_owner->_imask & node->_owner->_self_mask) == 0) return;

        /*
        printf("[y]node %d oldpos(%d,%d), pos(%d,%d), self oldcenter(%d,%d), center(%d,%d), oldarea(%d,%d,%d,%d), area(%d,%d,%d,%d)\n",
               node->_owner->_entity,
               oldpos.x, oldpos.y, node->_pos.x, node->_pos.y,
               _owner->_oldcenter.x, _owner->_oldcenter.y,
               _owner->_center.x, _owner->_center.y,
               _owner->_oldarea.left, _owner->_oldarea.top, _owner->_oldarea.right, _owner->_oldarea.bottom,
               _owner->_area.left, _owner->_area.top, _owner->_area.right, _owner->_area.bottom);
        */

        bool is_valid = _owner->is_in_xrange(node->_pos.x);
        if (is_valid == false) return;

        bool is_enter = (!!(this->_flag & FLAG_LOWER_BOUND) == positive);
        if (is_enter)
        {
            bool is_in = // _owner->is_in_xrange( node->_pos.x ) &&
                _owner->is_in_yrange(node->_pos.y);
            if (is_in)
            {
                _owner->on_trigger_enter(node, context);
            }
        }
        else
        {
            bool was_in = _owner->was_in_xrange(oldpos.x) && _owner->was_in_yrange(oldpos.y) ;
            if (was_in)
            {
                _owner->on_trigger_leave(node, context);
            }
        }
    }

    void swap(float &a, float &b)
    {
        float tmp = a;
        a = b;
        b = tmp;
    }

    //-------------------------------------------------------------------------
    group& group::operator =(const group &rhs)
    {
		_id		= rhs._id ;
        _entity = rhs._entity;
        _type   = rhs._type;
		_imask  = rhs._imask;
        _center = rhs._center;
        _area   = rhs._area;
        _oldcenter = rhs._oldcenter;
        _oldarea = rhs._oldarea;
        for (unsigned int i = 0 ; i < 4; i++)
        {
            _triggers[i] = rhs._triggers[i];
        }
        return *this;
    }

    group::group(const rect& area, int entity, int type, unsigned int self_mask, unsigned int imask, unsigned int id)
    {
		this->_id 		= id ;
        this->_type 	= type;
		this->_self_mask= self_mask;
		this->_imask	= imask;
        this->_area 	= area;
        this->_center = origin();
        this->_entity = entity;

        if (this->_area.top > this->_area.bottom)
        {
            swap(this->_area.top, this->_area.bottom);
        }

        if (this->_area.left > this->_area.right)
        {
            swap(this->_area.left, this->_area.right);
        }

        if (type & IS_AOI)
        {
            if (this->_area.top == this->_area.bottom)
            {
                this->_area.top = this->_area.bottom - 1;
            }

            if (this->_area.left == this->_area.right)
            {
                this->_area.left = this->_area.right - 1;
            }
        }
        //printf(" %d, %d, {%d, %d, %d, %d} \n", this->_entity, this->_type, _area.left, _area.top, _area.right, _area.bottom);
        this->_oldarea = _area;
        this->_oldcenter = origin();


        int flag = 0;

        if (type & IS_ENTITY)
        {
            flag |= FLAG_MAKE_CROSS;
        }

        if (type & IS_AOI)
        {
            flag |= FLAG_WANT_CROSS;
        }

        //if ( type & IS_BAR )
        //	flag |= FLAG_WANT_CROSS | FLAG_MAKE_CROSS;

        if (type & IS_ENTITY)
        {
            _triggers[0] = trigger(this, origin(), flag, 0);
            _triggers[2] = trigger(this, origin(), flag, 0);
        }
        else
        {
            // x axis
            _triggers[0] = trigger(this, origin(), flag | FLAG_LOWER_BOUND, -2);
            _triggers[1] = trigger(this, origin(), flag | FLAG_UPPER_BOUND, 2);
            // y axis
            _triggers[2] = trigger(this, origin(), flag | FLAG_LOWER_BOUND, -2);
            _triggers[3] = trigger(this, origin(), flag | FLAG_UPPER_BOUND, 2);
        }
    }

    group::~group()
    {
        /*
              for (int i = 0 ; i < 4 ; i++)
              {
                  assert((_triggers[i]._prev == 0) && (_triggers[i]._next == 0));
                  assert((_triggers[0]._header == 0));
              }
        */
    }

    void group::move_to(float x, float y, void* context)
    {
        float dx = x - this->_center.x;
        float dy = y - this->_center.y;

        if ((dx == 0) && (dy == 0)) return ;


        this->_center.x 	= x;
        this->_center.y 	= y;

        if (this->shufflex(x, y, context))
            this->shuffley(x, y, context);
        this->_oldcenter 	= this->_center;
    }

    bool group::was_in_xrange(float value)
    {
        return ((value >= _oldcenter.x + _oldarea.left) && (value <= _oldcenter.x + _oldarea.right));
    }

    bool group::was_in_yrange(float value)
    {
        return ((value >= _oldcenter.y + _oldarea.top) && (value <= _oldcenter.y + _oldarea.bottom));
    }

    bool group::is_in_xrange(float value)
    {
        return ((value >= _center.x + _area.left) && (value <= _center.x + _area.right));
    };

    bool group::is_in_yrange(float value)
    {
        return ((value >= _center.y + _area.top) && (value <= _center.y + _area.bottom));
    };

    void group::on_trigger_enter(trigger* node, void* context)
    {}

    void group::on_trigger_leave(trigger* node, void* context)
    {}

    bool group::shufflex(float x, float y, void* context)
    {
        int to = (_type & IS_ENTITY) ? 0 : 1;
        for (int i = 0; i <= to ; i++)
        {
            point oldpos = _triggers[i]._pos;

            //_triggers[i].check_x();
            if (this->_type & IS_ENTITY)
            {
                _triggers[i]._pos = point(x, y);
            }
            else
            {
                if (i == 0)
                    _triggers[i]._pos.x = _center.x + _area.left;
                else if (i == 1)
                    _triggers[i]._pos.x = _center.x + _area.right;
            };
            if (!_triggers[i].shufflex(oldpos, context))
            {
                //_triggers[i].check_x();
                return false;
            }
            //_triggers[i].check_x();
            assert((_triggers[i]._prev != 0) ? (_triggers[i]._prev->_pos.x <= _triggers[i]._pos.x) : true);
            assert((_triggers[i]._next != 0) ? (_triggers[i]._pos.x <= _triggers[i]._next->_pos.x) : true);
        }
        return true;
    };

    bool group::shuffley(float x, float y, void* context)
    {
        int to = (_type & IS_ENTITY) ? 2 : 3;
        for (int i = 2; i <= to ; i++)
        {
            point oldpos = _triggers[i]._pos;

            //_triggers[i].check_y();
            if (this->_type & IS_ENTITY)
            {
                _triggers[i]._pos = point(x, y);
            }
            else
            {
                if (i == 2)
                    _triggers[i]._pos.y = _center.y + _area.top;
                else if (i == 3)
                    _triggers[i]._pos.y = _center.y + _area.bottom;
            };

            if (!_triggers[i].shuffley(oldpos, context))
            {
                //_triggers[i].check_y();
                return false;
            }
            //_triggers[i].check_y();
            assert((_triggers[i]._prev != 0) ? (_triggers[i]._prev->_pos.y <= _triggers[i]._pos.y) : true);
            assert((_triggers[i]._next != 0) ? (_triggers[i]._pos.y <= _triggers[i]._next->_pos.y) : true);
        }
        return true;
    }

//-------------------------------------------------------------------------
    groups::groups()
    {}

    groups::~groups()
    {
        container::iterator where;
        for (where = _groups.begin();  where != _groups.end(); ++where)
        {
            group *p = where->second;
            delete p;
			#ifdef _MYAOI_STAT
			_myaoi_st->group_delete += 1;
			#endif
        }
        _groups.clear();
    }

    void groups::move_to(float x, float y, void* context)
    {
        container::iterator where;
        for (where = _groups.begin();  where != _groups.end(); ++where)
        {
            group *p = where->second;
            p->move_to(x, y, context);
        }
    }

    group* groups::add(const std::string & name, group* value)
    {
        group *retval = 0;
        container::iterator where;
        where = _groups.find(name);
        if (where != _groups.end())
        {
            retval = where->second;
        }

        if (value == 0)
        {
            if (where != _groups.end())
            {
                _groups.erase(where);
            }
        }
        else
        {
            _groups[name] = value;
        }
        return retval;
    }

    group* groups::remove(const std::string & name)
    {
        return add(name, 0);
    }

//-------------------------------------------------------------------------
    linklist::linklist()
    {
        _header = new trigger();
		#ifdef _MYAOI_STAT
		_myaoi_st->trigger_new += 1; 
		#endif
        _header->_header = _header;
        _header->_pos.x = -256;//poison
        _header->_pos.y = -256;//poison
        //printf("%p\n", _header);
    }

    linklist::~linklist()
    {
        delete _header;
		#ifdef _MYAOI_STAT
		_myaoi_st->trigger_delete += 1; 
		#endif
    }

    void linklist::add(trigger& value)
    {
        // 进入某个场景时，触发条件进入另外一个场景，会出现_header, _prev, _next不为0的情况
        if ((value._header != 0) || (value._prev != 0) || (value._next != 0))
        {
            return;
        }
        value._header = _header;
        // link
        value._next = _header->_next;
        value._prev = _header;
        if (_header->_next)
            _header->_next->_prev = &value;
        _header->_next = &value;
    }

    void linklist::remove(trigger& value)
    {
        //link
        if (value._prev)
            value._prev->_next = value._next;

        if (value._next)
            value._next->_prev = value._prev;

        value._prev = 0;
        value._next = 0;
        value._header = 0;
    }

    void linklist::arrange()
    {
        // move _header to the front of list.
        while (_header->_prev)
        {
            if (_header->_next)
                _header->_next->_prev = _header->_prev;

            _header->_prev->_next = _header->_next;

            _header->_next = _header->_prev;
            _header->_prev = _header->_prev->_prev;

            if (_header->_prev)
                _header->_prev->_next = _header;
            _header->_next->_prev = _header;
            assert((_header->_prev != _header->_next) && (_header->_prev != _header) && (_header->_next != _header));
        }
    }

    void linklist::dump()
    {
        trigger *cur = _header->_next;
        while (cur)
        {
            //printf("[%d] type %d, flag %d (%d,%d), %x <- %x -> %x\n", cur->_owner->_entity, cur->_owner->_type, cur->_flag, cur->_pos.x, cur->_pos.y, (int)cur->_prev, (int)cur, (int)cur->_next);
            cur = cur->_next;
        }
    }

//-------------------------------------------------------------------------

    void manager::add(group* value, float x, float y, void* context)
    {
		//printf("manager add group %p\n", value);
        value->_oldcenter = origin();

        _list[0].add(value->_triggers[0]);
        value->_triggers[0].force_shufflex();
        _list[1].add(value->_triggers[2]);
        value->_triggers[2].force_shuffley();

        if (!(value->_type & IS_ENTITY))
        {
            _list[0].add(value->_triggers[1]);
            value->_triggers[1].force_shufflex();
            _list[1].add(value->_triggers[3]);
            value->_triggers[3].force_shuffley();
        }

        /*
        printf("\n\ndump x\n");
        _list[0].dump();
        printf("\n\ndump y\n");
        _list[1].dump();
        */

        value->move_to(x, y, context);

        _list[0].arrange();
        _list[1].arrange();
    }

    void manager::remove(group* value, void* context)
    {
		//printf("manager remove group %p\n", value);
        value->move_to(ORIGIN_POS, ORIGIN_POS, context);

        _list[0].remove(value->_triggers[0]);
        _list[1].remove(value->_triggers[2]);
        if (!(value->_type & IS_ENTITY))
        {
            _list[0].remove(value->_triggers[1]);
            _list[1].remove(value->_triggers[3]);
        }
    }

    void manager::add(groups* value, float x, float y, void* context)
    {
        groups::container::iterator where;
        for (where = value->_groups.begin(); where != value->_groups.end(); ++where)
        {
            group* p = where->second;
            this->add(p, x, y, context);
        }
    }

    void manager::remove(groups* value, void* context)
    {
        groups::container::iterator where;
        for (where = value->_groups.begin(); where != value->_groups.end(); ++where)
        {
            group* p = where->second;
            this->remove(p, context);
        }
    }
};
