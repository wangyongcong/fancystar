#ifndef __HEADER_WYC_XLIST
#define __HEADER_WYC_XLIST

#include <list>
#include <cassert>

namespace wyc 
{

//------------------------------------------------------------------------------
// 指针链表(双向)
//------------------------------------------------------------------------------

template<class T>
class xptrlist : public std::list<void*>
{
public:
	class xptr_iterator : public std::list<void*>::iterator {
	public:
		xptr_iterator() : std::list<void*>::iterator() {}
		xptr_iterator(std::list<void*>::iterator iter) : std::list<void*>::iterator(iter) {}
		inline T& operator*() const {return *ptr();}
		inline T* operator->() const {return ptr();}
		inline T* ptr() const {return (T*)(std::list<void*>::iterator::operator*());}
	//	typedef std::list<void*>::iterator::iterator_category iterator_category;
		typedef T value_type;
	//	typedef std::list<void*>::iterator::difference_type difference_type;
		typedef T* pointer;
		typedef T& reference;
	};
	class xptr_const_iterator : public std::list<void*>::const_iterator {
	public:
		xptr_const_iterator() : std::list<void*>::const_iterator() {}
		xptr_const_iterator(std::list<void*>::const_iterator iter) : std::list<void*>::const_iterator(iter) {}
		inline const T& operator*() const {return *ptr();}
		inline const T* operator->() const {return ptr();}
		inline const T* ptr() const {return (const T*)(std::list<void*>::const_iterator::operator*());}
	//	typedef std::list<void*>::iterator::iterator_category iterator_category;
		typedef T value_type;
	//	typedef std::list<void*>::iterator::difference_type difference_type;
		typedef const T* pointer;
		typedef const T& reference;
	};
	typedef xptr_iterator iterator;
	typedef xptr_const_iterator const_iterator;
};

//------------------------------------------------------------------------------
// 单向链表
//------------------------------------------------------------------------------

template<class T>
class xsingle_list
{
protected:
	struct NODE
	{
		T	m_Data;
		NODE* m_pNext;
	};
	struct ITERATOR : public std::iterator<std::forward_iterator_tag,T>
	{
		NODE *pnode, *prenode;
		ITERATOR(NODE *prev=0, NODE *ptr=0) {
			prenode=prev;
			pnode=ptr;
		}
		T& operator * () const {return pnode->m_Data;}
		T* operator ->() const {return &pnode->m_Data;}
		ITERATOR& operator ++ () {
			prenode=pnode;
			pnode=pnode->m_pNext;
			return *this;
		}
		ITERATOR operator ++ (int) {
			ITERATOR itmp=*this; 
			++(*this);
			return itmp;
		}
		bool operator == (const ITERATOR &iter) const {return pnode==iter.pnode&&prenode==iter.prenode;}
		bool operator != (const ITERATOR &iter) const {return pnode!=iter.pnode||prenode!=iter.prenode;}
	};
	struct CONST_ITERATOR : public std::iterator<std::forward_iterator_tag,const T>
	{
		NODE *pnode, *prenode;
		CONST_ITERATOR(NODE *prev=0, NODE *ptr=0) {
			prenode=prev;
			pnode=ptr;
		}
		CONST_ITERATOR(const ITERATOR &iter) {
			prenode=iter.prenode;
			pnode=iter.pnode;
		}
		CONST_ITERATOR& operator = (const ITERATOR &iter) {
			prenode=iter.prenode;
			pnode=iter.pnode;
			return *this;
		}
		const T& operator * () const {return pnode->m_Data;}
		const T* operator ->() const {return &pnode->m_Data;}
		CONST_ITERATOR& operator ++ () {
			prenode=pnode;
			pnode=pnode->m_pNext;
			return *this;
		}
		CONST_ITERATOR operator ++ (int) {
			CONST_ITERATOR itmp=*this; 
			++(*this);
			return itmp;
		}
		bool operator == (const CONST_ITERATOR &iter) const {return pnode==iter.pnode&&prenode==iter.prenode;}
		bool operator != (const CONST_ITERATOR &iter) const {return pnode!=iter.pnode||prenode!=iter.prenode;}
	};
	int m_nSize;
	NODE *m_pHead, *m_pTail;
#ifndef _LIB
	int m_newnode;
#endif
public:
	typedef ITERATOR iterator;
	typedef CONST_ITERATOR const_iterator;
	xsingle_list()
	{
		m_nSize=0;
		m_pTail=m_pHead=0;
#ifndef _LIB
		m_newnode=0;
#endif
	}
	~xsingle_list() 
	{
		clear();
	}
	int size() const {return m_nSize;}
	bool empty() const {return m_nSize==0;}
#ifndef _LIB
	inline int new_node() const {
		return m_newnode;
	}
#endif
	iterator begin() {
		return iterator(0,m_pHead);
	}
	const_iterator begin() const {
		return const_iterator(0,m_pHead);
	}
	iterator end() {
		return iterator(m_pTail);
	}
	const_iterator end() const {
		return const_iterator(m_pTail);
	}
	void pop_front()
	{
		if(m_pHead==0) return;
		NODE *p=m_pHead;
		m_pHead=p->m_pNext;
		if(m_pHead==0) 
			m_pTail=0;
		m_nSize-=1;
		FREENODE(p);
	}
	void push_back(const T& val)
	{
		NODE* p=NEWNODE();
		p->m_Data=val;
		p->m_pNext=0;
		if(m_pHead) 
			m_pTail->m_pNext=p;
		else m_pHead=p;
		m_pTail=p;
		m_nSize+=1;
	}
	void push_front(const T& val)
	{
		NODE* p=NEWNODE();
		p->m_Data=val;
		p->m_pNext=m_pHead;
		if(m_pHead==0) m_pTail=p;
		m_pHead=p;
		m_nSize+=1;
	}
	iterator insert(iterator iter, const T& val)
	{
		NODE *p=NEWNODE();
		p->m_Data=val;
		p->m_pNext=iter.pnode;
		if(iter.prenode) 
			iter.prenode->m_pNext=p;
		else m_pHead=p;
		if(iter.pnode==0) 
			m_pTail=p;
		iter.pnode=p;
		m_nSize+=1;
		return iter;
	}
	iterator erase(iterator iter)
	{
		NODE *p=iter.pnode;
		assert(p!=0);
		if(iter.prenode)
			iter.prenode->m_pNext=p->m_pNext;
		else m_pHead=p->m_pNext;
		if(p->m_pNext==0) 
			m_pTail=iter.prenode;
		iter.pnode=p->m_pNext;
		FREENODE(p);
		m_nSize-=1;
		return iter;
	}
	void remove(const T &val)
	{
		NODE *pcur=m_pHead, *prev=0;
		while(pcur) {
			if(pcur->m_Data!=val) {
				prev=pcur;
				pcur=pcur->m_pNext;
				continue;
			}
			if(prev) {
				prev->m_pNext=pcur->m_pNext;
			}
			else {
				m_pHead=pcur->m_pNext;
			}
			FREENODE(pcur);
			return;
		}
	}
	void clear()
	{
		NODE *p;
		while(m_pHead) {
			p=m_pHead->m_pNext;
			FREENODE(m_pHead);
			m_pHead=p;
		}
		m_pTail=0;
		m_nSize=0;
	}
protected:
	inline NODE* NEWNODE() {
#ifndef _LIB
		m_newnode+=1; 
#endif
		return new NODE;
	}
	inline void FREENODE(NODE *pnode) {
#ifndef _LIB
		m_newnode-=1; 
#endif
		delete pnode;
	}
};

//------------------------------------------------------------------------------
// 指针链表(单向)
//------------------------------------------------------------------------------

template<class T>
class xsingle_ptrlist : public xsingle_list<void*>
{
protected:
	typedef struct PTR_ITERATOR : public std::iterator<std::forward_iterator_tag,T>
	{
		NODE *pnode, *prenode;
		PTR_ITERATOR(NODE *prev=0, NODE *ptr=0) {
			prenode=prev;
			pnode=ptr;
		}
		T& operator * () const {return pnode->m_Data;}
		T* operator ->() const {return &pnode->m_Data;}
		PTR_ITERATOR& operator ++ () {
			prenode=pnode;
			pnode=pnode->m_pNext;
			return *this;
		}
		PTR_ITERATOR operator ++ (int) {
			PTR_ITERATOR itmp=*this; 
			++(*this);
			return itmp;
		}
		bool operator == (const PTR_ITERATOR &iter) const {return pnode==iter.pnode&&prenode==iter.prenode;}
		bool operator != (const PTR_ITERATOR &iter) const {return pnode!=iter.pnode||prenode!=iter.prenode;}
	}iterator;
	typedef struct CONST_PTR_ITERATOR : public std::iterator<std::forward_iterator_tag,const T>
	{
		NODE *pnode, *prenode;
		CONST_PTR_ITERATOR(NODE *prev=0, NODE *ptr=0) {
			prenode=prev;
			pnode=ptr;
		}
		CONST_PTR_ITERATOR(const PTR_ITERATOR &iter) {
			prenode=iter.prenode;
			pnode=iter.pnode;
		}
		CONST_PTR_ITERATOR& operator = (const PTR_ITERATOR &iter) {
			prenode=iter.prenode;
			pnode=iter.pnode;
			return *this;
		}
		const T& operator * () const {return pnode->m_Data;}
		const T* operator ->() const {return &pnode->m_Data;}
		CONST_PTR_ITERATOR& operator ++ () {
			prenode=pnode;
			pnode=pnode->m_pNext;
			return *this;
		}
		CONST_PTR_ITERATOR operator ++ (int) {
			CONST_PTR_ITERATOR itmp=*this; 
			++(*this);
			return itmp;
		}
		bool operator == (const CONST_PTR_ITERATOR &iter) const {return pnode==iter.pnode&&prenode==iter.prenode;}
		bool operator != (const CONST_PTR_ITERATOR &iter) const {return pnode!=iter.pnode||prenode!=iter.prenode;}
	}const_iterator;
};

} // namespace wyc

#endif // end of __HEADER_WYC_XLIST


