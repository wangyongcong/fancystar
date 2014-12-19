/* Exit Games Common - C++ Client Lib
 * Copyright (C) 2004-2012 by Exit Games GmbH. All rights reserved.
 * http://www.exitgames.com
 * mailto:developer@exitgames.com
 */

#ifndef __JVECTOR_BASE_H
#define __JVECTOR_BASE_H

#include "Base.h"
#include "M95_types.h"

#undef min

#ifndef _EG_BREW_PLATFORM
namespace ExitGames
{
#endif

	/* Summary
	   This is a C++ implementation of the <i>Vector</i> Container
	   class from Sun Java.
	   Description
	   This class is based on the Java Vector class and as such
	   contains all the public member functions of its Java
	   equivalent. Unlike Java, typecasts are not necessary since
	   C++ allows template instantiation of types at compile time.
	   In addition to the Java public member functions, some
	   operators were also added in order to take advantage of the
	   \operator overloading feature available in C++.     */
	template<typename Etype>
	class JVectorBase : public Base
	{
	public:
		JVectorBase(unsigned int initialCapacity=40, unsigned int capacityIncrement=10);
		virtual ~JVectorBase(void);

		JVectorBase(const JVectorBase<Etype>& rhv);
		JVectorBase& operator=(const JVectorBase<Etype> &rhv);

		const Etype& operator[](unsigned int index) const;
		Etype& operator[](unsigned int index);

		bool operator==(const JVectorBase<Etype>& toCompare) const;
		bool operator!=(const JVectorBase<Etype>& toCompare) const;

		unsigned int capacity(void) const;
		bool contains(const Etype& elem) const;
		const Etype& firstElement(void) const;
		int indexOf(const Etype& elem) const;
		bool isEmpty(void) const;
		const Etype& lastElement(void) const;
		int lastIndexOf(const Etype& elem) const;
		unsigned int size(void) const;
		void copyInto(Etype* array) const;
		void addElement(const Etype& obj);
		void ensureCapacity(unsigned int minCapacity);
		void removeAllElements(void);
		bool removeElement(const Etype& obj);
		void trimToSize(void);
		Etype& elementAt(unsigned int index) const;
		void insertElementAt(const Etype &obj, unsigned int index);
		void removeElementAt(unsigned int index);
		void setElementAt(const Etype &obj, unsigned int index);
	protected:
		int min(unsigned int left, unsigned int right) const;
		void verifyIndex(unsigned int index) const;
		unsigned int mSize;
		unsigned int mCapacity;
		unsigned int mIncrement;
		Etype** mpData;
	};



	/* Summary
	   Constructor.
	   
	   Creates an empty <link JVectorBase> of elements of the type of
	   the template parameter. Neutron mostly uses <link Object>
	   as type, to wrap the real datatype, which makes it possible,
	   to put different types of data into the same JVectorBase.
	   Parameters
	   isObject :           false, if the template parameter is a
	                        primitive type, true if it is a class
	                        object. The default is true
	   initialCapacity :    the amount of elements, the JVectorBase can
	                        take without need for resize. If you
	                        choose this too small, the JVectorBase needs
	                        expensive resizes later (it's most
	                        likely, that the complete memory has to
	                        be copied to a new location on resize),
	                        if you choose it too big, you will waste
	                        much memory. The default is 40.
	   capacityIncrement :  Every time, you add an element to the
	                        Vector and it has no capacity left
	                        anymore, it's capacity will grow with
	                        this amount of elements on automatic
	                        resize. If you pass a too small value
	                        here, expensive resize will be needed
	                        more often, if you choose a too big one,
	                        possibly memory is wasted. The default is
	                        10.                                         */
	template<typename Etype>
	JVectorBase<Etype>::JVectorBase(unsigned int initialCapacity, unsigned int capacityIncrement) 
	{
		mSize = 0;
		mCapacity = initialCapacity;
		mpData = new Etype*[mCapacity];
		mIncrement = capacityIncrement;
	}

	/* Summary
	   Copy-Constructor.
	   
	   Creates an object out of a deep copy of its parameter.
	   
	   The parameter has to be of the same template overload as the
	   \object, you want to create.
	   Parameters
	   toCopy :  The object to copy.                                */
	template<typename Etype>
	JVectorBase<Etype>::JVectorBase(const JVectorBase<Etype>& rhv)
	{
		mSize = rhv.mSize;
		mCapacity = rhv.mCapacity;
		mpData = new Etype*[mCapacity];
		mIncrement = rhv.mIncrement;

		for(unsigned int i=0; i<mSize; i++)
			mpData[i] = new Etype(*(rhv.mpData[i]));
	}

	/* Summary
	   Operator =.
	   
	   Makes a deep copy of its right operand into its left operand.
	   Both operands have to be of the same template overload.
	   
	   This overwrites old data in the left operand.                 */
	template<typename Etype>
	JVectorBase<Etype>& JVectorBase<Etype>::operator=(const JVectorBase<Etype> &rhv)
	{
		if(mpData)
		{
			removeAllElements();
			delete[] mpData;
		}

		mSize = rhv.mSize;
		mCapacity = rhv.mCapacity;

		mpData = new Etype*[mCapacity];
		mIncrement = rhv.mIncrement;

		for(unsigned int i=0; i<mSize; i++)
			mpData[i] = new Etype(*(rhv.mpData[i]));
		return *this;
	}
	
	/* Summary
	   Destructor. */
	template<typename Etype>
	JVectorBase<Etype>::~JVectorBase()
	{
		removeAllElements();
		delete[] mpData;
	}

	/* Summary
	   \Returns the current capacity of the <link JVectorBase>.
	   Returns
	   the current capacity.                                */
	template<typename Etype>
	unsigned int JVectorBase<Etype>::capacity(void) const
	{
		return mCapacity;
	}

	/* Summary
	   Checks, if the <link JVectorBase> contains the passed data as an
	   element.
	   Parameters
	   elem :  a reference to the data, you want to check. Needs to
	           be either a primitive type or an object of a class
	           with an overloaded == operator.
	   Returns
	   true, if the element was found, false otherwise.             */
	template<typename Etype>
	bool JVectorBase<Etype>::contains(const Etype &elem) const
	{
		for(unsigned int i=0; i<mSize; i++)
		{
			if(*mpData[i] == elem)
				return true;
		}

		return false;
	}

	/* Summary
	   Copies all elements of the <link JVectorBase> into the passed
	   array. The caller has to make sure, the array is big enough,
	   to take all elements of the vector, otherwise calling this
	   method produces a buffer overflow.
	   Parameters
	   array :  an array of variables of the type of the template
	            overload.
	   Returns
	   nothing.                                                     */
	template<typename Etype>
	void JVectorBase<Etype>::copyInto(Etype* array) const
	{
		for(unsigned int i=0; i<mSize; i++)
			array[i] = *mpData[i];
	}

	/* Summary
	   \Returns the element of the <link JVectorBase> at the passed
	   index. This does not check for valid indexes and shows
	   unexpected behavior for invalid indexes!
	   Parameters
	   index :  the index of the element, that should be returned.
	            Must not be bigger than the current size of the
	            vector!
	   Returns
	   the element at the passed index.                            */
	template<typename Etype>
	Etype& JVectorBase<Etype>::elementAt(unsigned int index) const
	{
		verifyIndex(index);
		return *mpData[index];
	}

	/* Summary
	   \Returns the first element of the <link JVectorBase>. Shows
	   unexpected behaviour for empty vectors.
	   Returns
	   the first element.                                      */
	template<typename Etype>
	const Etype& JVectorBase<Etype>::firstElement(void) const
	{
		verifyIndex(0);
		return *mpData[0];
	}

	/* Summary
	   Searches the <link JVectorBase> from the first element in forward
	   direction for the passed element and returns the first index,
	   where it was found.
	   Parameters
	   elem :  the element, to search for.
	   Returns
	   the index of the first found of the passed element or -1, if
	   the element could not be found at all.                        */
	template<typename Etype>
	int JVectorBase<Etype>::indexOf(const Etype &elem) const
	{
		for(unsigned int i=0; i<mSize; i++)
			if(*mpData[i] == elem)
				return i;
		return -1;
	}

	/* Summary
	   Checks, if the <link JVectorBase> is empty.
	   Returns
	   true, if the JVectorBase is empty, or false, if it contains at
	   least one element.                                         */
	template<typename Etype>
	bool JVectorBase<Etype>::isEmpty(void) const
	{
		return mSize == 0;
	}

	/* Summary
	   \Returns the last element of the <link JVectorBase>. Shows
	   unexpected behaviour for empty vectors.
	   Returns
	   the last element.                                      */
	template<typename Etype>
	const Etype& JVectorBase<Etype>::lastElement() const
	{
		verifyIndex(0);
		return *mpData[mSize - 1];
	}

	/* Summary
	   Searches the <link JVectorBase> from the last element in backward
	   direction for the passed element and returns the first index,
	   where it was found.
	   Parameters
	   elem :  the element, to search for.
	   Returns
	   the index of the first found of the passed element or -1, if
	   the element could not be found at all.                        */
	template<typename Etype>
	int JVectorBase<Etype>::lastIndexOf(const Etype &elem) const
	{
		for(unsigned int i=mSize; i; i--)
			if(*mpData[i] == elem)
				return i;
		return -1;
	}

	/* Summary
	   Return the size of the <link JVectorBase>.
	   Returns
	   the size.                              */
	template<typename Etype>
	unsigned int JVectorBase<Etype>::size(void) const
	{
		return mSize;
	}

	/* Summary
	   Adds an element to the <link JVectorBase>. This automatically
	   resizes the JVectorBase to it's old size + the capacityIncrement,
	   you passed, when creating the vector (if you passed not
	   capacityIncrement, it was set to it's default value (see
	   constructor doc)), if the size of the JVectorBase has already
	   reached it's capacity, which means, that most likely the
	   whole vector has to be copied to new memory. So this can be
	   an expensive operation for huge vectors.
	   Parameters
	   elem :  the element, to add.
	   Returns
	   nothing.                                                      */
	template<typename Etype>
	void JVectorBase<Etype>::addElement(const Etype &obj)
	{
		if(mSize == mCapacity)
			ensureCapacity(mCapacity+mIncrement);
		mpData[mSize++] = new Etype;
		*mpData[mSize-1] = obj;
	}

	/* Summary
	   Resizes the <link JVectorBase> to the passed capacity, if it's
	   \old capacity has been smaller. Most likely the whole JVectorBase
	   has to be copied into new memory, so this is an expensive
	   \operation for huge JVectorBases. Call this method, before you
	   add a lot of elements to the vector, to avoid multiple
	   expensive resizes through adding.
	   Parameters
	   minCapacity :  the new capacity for the JVectorBase.
	   Returns
	   nothing.                                                      */
	template<typename Etype>
	void JVectorBase<Etype>::ensureCapacity(unsigned int minCapacity)
	{
		if(minCapacity > mCapacity)
		{
			unsigned int i;
			mCapacity = minCapacity;
			Etype** temp = new Etype*[mCapacity];

			//copy all the elements over upto newsize
			for(i=0; i<mSize; i++)
				temp[i] = mpData[i];

			delete[] mpData;

			mpData = temp;
		}
	}

	/* Summary
	   Inserts parameter one into the <link JVectorBase> at the index,
	   passed as parameter two. Because all elements above or at the
	   passed index have to be moved one position up, it is
	   expensive, to insert an element at an low index into a huge
	   JVectorBase.
	   Parameters
	   obj :    the element, to insert.
	   index :  the position in the JVectorBase, the element is inserted
	            at.
	   Returns
	   nothing.                                                      */
	template<typename Etype>
	void JVectorBase<Etype>::insertElementAt(const Etype &obj, unsigned int index)
	{
		if(index == mSize)
			addElement(obj);
		else
		{
			verifyIndex(index);

			if(mSize == mCapacity)
				ensureCapacity(mCapacity + mIncrement);

			Etype* newItem = new Etype;
			*newItem=obj;

			Etype* tmp; //temp to hold item to be moved over.
			for(unsigned int i=index; i<=mSize; i++)
			{
				tmp = mpData[i];
				mpData[i] = newItem;

				if(i != mSize)
					newItem = tmp;
				else
					break;
			}
			mSize++;
		}
	}

	/* Summary
	   Clears the <link JVectorBase>.
	   Returns
	   nothing.                   */
	template<typename Etype>
	void JVectorBase<Etype>::removeAllElements()
	{
		for(unsigned int i=0; i<mSize; i++)
			delete mpData[i];

		mSize = 0;
	}

	/* Summary
	   Removes the passed element from the <link JVectorBase>.
	   Parameters
	   obj :  the element, to remove.
	   Returns
	   true, if the element has been removed, false, if it could not
	   be found.                                                     */
	template<typename Etype>
	bool JVectorBase<Etype>::removeElement(const Etype &obj)
	{
		for(unsigned int i=0; i<mSize; i++)
		{
			if(*mpData[i] == obj)
			{
				removeElementAt(i);
				return true;
			}
		}
		return false;
	}

	/* Summary
	   Removes the element at the passed index from the <link JVectorBase>.
	   Shows unexpected behaviour for invalid indexes.
	   Parameters
	   index :  the index of the element to remove.
	   Returns
	   nothing.                                                         */
	template<typename Etype>
	void JVectorBase<Etype>::removeElementAt(unsigned int index)
	{
		verifyIndex(index);

		delete mpData[index];

		for(unsigned int i=index+1; i<mSize; i++)
			mpData[i-1] = mpData[i];

		mSize--;
	}

	/* Summary
	   Sets the element at the passed index of the <link JVectorBase> to
	   the passed new value. Shows unexpected behaviour for invalid
	   indexes.
	   Parameters
	   obj :    the new value.
	   index :  the index of the element, which is set to the new
	            value.
	   Returns
	   nothing.                                                      */
	template<typename Etype>
	void JVectorBase<Etype>::setElementAt(const Etype &obj, unsigned int index)
	{
		verifyIndex(index);
		
		*mpData[index] = obj;
	}

	/* Summary
	   Trims the capacity of the <link JVectorBase> to the size, it
	   currently uses. Call this method for a JVectorBase with huge
	   unused capacity, if you do not want to add further elements
	   to it and if you are short of memory. This method copies the
	   whole vector to new memory, so it is expensive for huge
	   vectors. If you only add one element to the JVectorBase later,
	   it's copied again.                                           */
	template<typename Etype>
	void JVectorBase<Etype>::trimToSize(void)
	{
		if(mSize != mCapacity)
		{
			Etype** temp = new Etype*[mSize];
			unsigned int i;

			for(i=0; i<mSize; i++)
				temp[i] = mpData[i];

			delete[] mpData;

			mpData = temp;
			mCapacity = mSize;
		}
	}

	template<typename Etype>
	int JVectorBase<Etype>::min(unsigned int left, unsigned int right) const
	{
		return left<right?left:right;
	}
	  
	template<typename Etype>
	void JVectorBase<Etype>::verifyIndex(unsigned int index) const
	{
		if(index >= mSize)
			debugReturn("JVectorBase: Index Out Of Bounds");
	}

	/* Summary
	   \operator[]. Wraps the method <link JVectorBase::elementAt@unsigned int@const, elementAt>(),
	   so you have same syntax like for arrays.                                                 */
	template<typename Etype>
	const Etype& JVectorBase<Etype>::operator[](unsigned int index) const
	{
		return elementAt(index);
	}

	/* Summary
	   \operator[]. Wraps the method <link JVectorBase::elementAt@unsigned int@const, elementAt>(),
	   so you have same syntax like for arrays.                                                 */
	template<typename Etype>
	Etype& JVectorBase<Etype>::operator[](unsigned int index)
	{
		verifyIndex(index);
		return *mpData[index];
	}

	template<typename Etype>
	bool JVectorBase<Etype>::operator==(const JVectorBase<Etype>& toCompare) const
	{
		if(size() != toCompare.size())
			return false;
		for(unsigned int i=0; i<size(); ++i)
			if((*this)[i] != toCompare[i])
				return false;
		return true;
	}

	template<typename Etype>
	bool JVectorBase<Etype>::operator!=(const JVectorBase<Etype>& toCompare) const
	{
		return !(*this == toCompare);
	}

#ifndef _EG_BREW_PLATFORM
}
#endif

#endif