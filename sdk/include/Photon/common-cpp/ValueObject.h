/* Exit Games Common - C++ Client Lib
 * Copyright (C) 2004-2012 by Exit Games GmbH. All rights reserved.
 * http://www.exitgames.com
 * mailto:developer@exitgames.com
 */

#ifndef __VALUE_OBJECT_H
#define __VALUE_OBJECT_H

#include "ConfirmAllowed.h"

#ifndef _EG_BREW_PLATFORM
namespace ExitGames
{
#endif

	/* Summary
	   Container class template for objects to be stored as values
	   in a Hashtable.
	   Description
	   Please refer to <link Hashtable> for more information and an
	   \example.                                                    */
	template<typename Etype>
	class ValueObject : public Object
	{
	public:
		ValueObject(const ValueObject<Etype>& toCopy);
		ValueObject(const Object& obj);
		ValueObject(const Object* const obj);
		ValueObject(typename ConfirmAllowed<Etype>::type data);
		~ValueObject(void);

		ValueObject<Etype>& operator=(const ValueObject<Etype>& toCopy);
		ValueObject<Etype>& operator=(const Object& toCopy);

		Etype getDataCopy(void);
		Etype* getDataAddress(void);

	private:
		typedef Object super;

		void convert(const Object* const obj, nByte type);
	};

	template<typename Etype>
	class ValueObject<Etype*> : public Object
	{
	public:
		ValueObject(const ValueObject<Etype*>& toCopy);
		ValueObject(const Object& obj);
		ValueObject(const Object* const obj);
		ValueObject(typename ConfirmAllowed<Etype*>::type data, short size);
		ValueObject(typename ConfirmAllowed<Etype*>::type data, const short* const sizes);
		~ValueObject(void);

		ValueObject<Etype*>& operator=(const ValueObject<Etype*>& toCopy);
		ValueObject<Etype*>& operator=(const Object& toCopy);

		Etype* getDataCopy(void);
		Etype** getDataAddress(void);

	private:
		typedef Object super;

		void convert(const Object* const obj, nByte type, unsigned int dimensions);
	};



	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// CONSTRUCTORS                                                                                                //
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	/* Summary
	   Copy-Constructor.

	   Creates an object out of a deep copy of its parameter.

	   The parameter has to be of the same template overload as the
	   \object, you want to create.
	   Parameters
	   toCopy :  The object to copy.                                */
	template<typename Etype>
	ValueObject<Etype>::ValueObject(const ValueObject<Etype> &toCopy) : Object(toCopy)
	{
	}



	/* Summary
	   Constructor.

	   Creates an object out of a deep copy of the passed <link ExitGames::Object, Object>&.

	   If the type of the content of the passed object does not
	   match the template overload of the object to create, an empty
	   \object is created instead of a copy of the passed object,
	   which leads to <link ExitGames::KeyObject::getDataCopy, getDataCopy()>
	   and <link ExitGames::KeyObject::getDataAddress, getDataAddress()>
	   returning 0.
	   Parameters
	   obj :  The <link ExitGames::Object, Object>& to copy.                                 */
	template<typename Etype>
	ValueObject<Etype>::ValueObject(const Object& obj)
	{
		convert(&obj, ConfirmAllowed<Etype>::typeName);
	}

	template<typename Etype>
	ValueObject<Etype*>::ValueObject(const Object& obj)
	{
		convert(&obj, ConfirmAllowed<Etype*>::typeName, ConfirmAllowed<Etype*>::dimensions);
	}



	/* Summary
	   Constructor.

	   Creates an object out of a deep copy of the passed <link ExitGames::Object, Object>*.

	   If the type of the content of the passed object does not
	   match the template overload of the object to create, an empty
	   \object is created instead of a copy of the passed object,
	   which leads to <link ExitGames::KeyObject::getDataCopy, getDataCopy()>
	   and <link ExitGames::KeyObject::getDataAddress, getDataAddress()>
	   return 0.
	   Parameters
	   obj :  The <link ExitGames::Object, Object>* to copy.                                 */
	template<typename Etype>
	ValueObject<Etype>::ValueObject(const Object* const obj)
	{
		convert(obj, ConfirmAllowed<Etype>::typeName);
	}

	template<typename Etype>
	ValueObject<Etype*>::ValueObject(const Object* const obj)
	{
		convert(obj, ConfirmAllowed<Etype*>::typeName, ConfirmAllowed<Etype*>::dimensions);
	}



	/* Summary
	   Constructor.
	   
	   Creates an object out of a deep copy of the passed scalar Etype.
	   
	   Parameters
	   data : The value to copy. Has to be of a supported type.                         */
	template<typename Etype>
	ValueObject<Etype>::ValueObject(typename ConfirmAllowed<Etype>::type data)
	{
		COMPILE_TIME_ASSERT_TRUE_MSG(!ConfirmAllowed<Etype>::dimensions, ERROR_THIS_OVERLOAD_IS_ONLY_FOR_SCALAR_TYPES);
		set(&data, ConfirmAllowed<Etype>::typeName, ConfirmAllowed<Etype>::customTypeName, true);
	}

	/* Summary
	   Constructor.
	   
	   Creates an object out of a deep copy of the passed single-dimensional Etype-array.
	   
	   Parameters
	   data : The array to copy.
	   size : The element count of data.                               */
	template<typename Etype>
	ValueObject<Etype*>::ValueObject(typename ConfirmAllowed<Etype*>::type data, short size)
	{
		COMPILE_TIME_ASSERT_TRUE_MSG(ConfirmAllowed<Etype*>::dimensions==1, ERROR_THIS_OVERLOAD_IS_ONLY_FOR_1D_ARRAYS);
		set(data, ConfirmAllowed<Etype*>::typeName, ConfirmAllowed<Etype*>::customTypeName, size, true);
	}

	/* Summary
	   Constructor.
	   
	   Creates an object out of a deep copy of the passed multi-dimensional Etype-array.
	   
	   Parameters
	   data  : The array to copy.
	   sizes : The array of element counts for the different dimensions of data.                               */
	template<typename Etype>
	ValueObject<Etype*>::ValueObject(typename ConfirmAllowed<Etype*>::type data, const short* const sizes)
	{
		COMPILE_TIME_ASSERT_TRUE_MSG((bool)ConfirmAllowed<Etype>::dimensions, ERROR_THIS_OVERLOAD_IS_ONLY_FOR_FOR_ARRAYS);
		set(data, ConfirmAllowed<Etype*>::typeName, ConfirmAllowed<Etype*>::customTypeName, ConfirmAllowed<Etype*>::dimensions, sizes, true);
	}



	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// DESTRUCTOR                                                                                                  //
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	/* Summary
	   Destructor.*/
	template<typename Etype>
	ValueObject<Etype>::~ValueObject(void)
	{
	}

	template<typename Etype>
	ValueObject<Etype*>::~ValueObject(void)
	{
	}



	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// OPERATORS                                                                                                   //
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	/* Summary
	   Operator = : Makes a deep copy of its right operand into its
	   left operand. This overwrites old data in the left operand.  */
	template<typename Etype>
	ValueObject<Etype>& ValueObject<Etype>::operator=(const ValueObject<Etype>& toCopy)
	{
		return super::operator=(toCopy);
	}

	template<typename Etype>
	ValueObject<Etype*>& ValueObject<Etype*>::operator=(const ValueObject<Etype*>& toCopy)
	{
		return super::operator=(toCopy);
	}

	/* Summary
	   Operator = : Makes a deep copy of its right operand into its
	   left operand. This overwrites old data in the left operand.

	   If the type of the content of the right operand does not
	   match the template overload of the left operand, then the left
	   operand stays unchanged.  */
	template<typename Etype>
	ValueObject<Etype>& ValueObject<Etype>::operator=(const Object& toCopy)
	{
		if(ConfirmAllowed<Etype>::typeName == toCopy.getType())
			*((Object*)this) = toCopy;
		return *this;
	}

	template<typename Etype>
	ValueObject<Etype*>& ValueObject<Etype*>::operator=(const Object& toCopy)
	{
		if(ConfirmAllowed<Etype>::typeName == toCopy.getType() && ConfirmAllowed<Etype>::dimensions == toCopy.getDimensions())
			*((Object*)this) = toCopy;
		return *this;
	}



	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// getDataCopy                                                                                                 //
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	/* Summary
	   \Returns a deep copy of the content of the object. If you
	   \only need access to the content, while the object still
	   exists, you can use <link ValueObject::getDataAddress, getDataAddress()>
	   instead to avoid the deep copy. That is especially
	   interesting for large content, of course.

	   If successful, the template overloads for array types of this
	   method allocate the data for the copy, so you have to free
	   (for arrays of primitive types) or delete (for arrays of
	   class objects) it, as soon, as you do not need the array
	   anymore. All non-array copies free there memory
	   automatically, as soon as they leave their scope, same as the
	   single indices of the array, as soon, as the array is freed.

	   In case of an error this function returns 0 for primitive
	   return types and for arrays and empty objects for classes.
	   Returns
	   a deep copy of the content of the object if successful, 0 or
	   an empty object otherwise.                                               */
	template<typename Etype>
	inline Etype ValueObject<Etype>::getDataCopy(void)
	{
		return (mType == EG_CUSTOM && !mDimensions)?**(Etype**)EG_Object_internal_duplicateData(*(Etype**)mpData, EG_CUSTOM, (*(Etype**)mpData)->TypeCode):0;
	}

	template<typename Etype>
	inline Etype* ValueObject<Etype*>::getDataCopy(void)
	{
		return (mType == EG_CUSTOM && mDimensions == 1)?*(Etype**)EG_Object_internal_duplicateDataArray(*(Etype**)mpData, EG_CUSTOM, (*(Etype**)mpData)->TypeCode, *mpSizes):0;
	}

	template <>
	inline nByte ValueObject<nByte>::getDataCopy(void)
	{
		return (mType == EG_BYTE && !mDimensions)?*(nByte*)mpData:0;
	}

	template <>
	inline short ValueObject<short>::getDataCopy(void)
	{
		return (mType == EG_SHORT && !mDimensions)?*(short*)mpData:0;
	}

	template <>
	inline int ValueObject<int>::getDataCopy(void)
	{
		return (mType == EG_INTEGER && !mDimensions)?*(int*)mpData:0;
	}

	template <>
	inline int64 ValueObject<int64>::getDataCopy(void)
	{
		return (mType == EG_LONG && !mDimensions)?*(int64*)mpData:0;
	}

	template <>
	inline float ValueObject<float>::getDataCopy(void)
	{
		return (mType == EG_FLOAT && !mDimensions)?*(float*)mpData:0;
	}

	template <>
	inline double ValueObject<double>::getDataCopy(void)
	{
		return (mType == EG_DOUBLE && !mDimensions)?*(double*)mpData:0;
	}

	template <>
	inline bool ValueObject<bool>::getDataCopy(void)
	{
		return (mType == EG_BOOLEAN && !mDimensions)?*(bool*)mpData:false;
	}

	template <>
	inline JString ValueObject<JString>::getDataCopy(void)
	{
		return (mType == EG_STRING && !mDimensions)?*(JString*)mpData:JString("");
	}

	template <>
	inline JVector<Object> ValueObject<JVector<Object> >::getDataCopy(void)
	{
		return (mType == EG_VECTOR && !mDimensions)?JVector<Object>(*(JVector<Object>*)mpData):JVector<Object>();
	}

	template <>
	inline Hashtable ValueObject<Hashtable>::getDataCopy(void)
	{
		return (mType == EG_HASHTABLE && !mDimensions)?Hashtable(*(Hashtable*)mpData):Hashtable();
	}

	template <>
	inline nByte* ValueObject<nByte*>::getDataCopy(void)
	{
		return (mType == EG_BYTE && mDimensions == 1)?(nByte*)EG_Object_internal_duplicateDataArray(mpData, EG_BYTE, 0, *mpSizes):0;
	}

	template <>
	inline short* ValueObject<short*>::getDataCopy(void)
	{
		return (mType == EG_SHORT && mDimensions == 1)?(short*)EG_Object_internal_duplicateDataArray(mpData, EG_SHORT, 0, *mpSizes):0;
	}

	template <>
	inline int* ValueObject<int*>::getDataCopy(void)
	{
		return (mType == EG_INTEGER && mDimensions == 1)?(int*)EG_Object_internal_duplicateDataArray(mpData, EG_INTEGER, 0, *mpSizes):0;
	}

	template <>
	inline int64* ValueObject<int64*>::getDataCopy(void)
	{
		return (mType == EG_LONG && mDimensions == 1)?(int64*)EG_Object_internal_duplicateDataArray(mpData, EG_LONG, 0, *mpSizes):0;
	}

	template <>
	inline float* ValueObject<float*>::getDataCopy(void)
	{
		return (mType == EG_FLOAT && mDimensions == 1)?(float*)EG_Object_internal_duplicateDataArray(mpData, EG_FLOAT, 0, *mpSizes):0;
	}

	template <>
	inline double* ValueObject<double*>::getDataCopy(void)
	{
		return (mType == EG_DOUBLE && mDimensions == 1)?(double*)EG_Object_internal_duplicateDataArray(mpData, EG_DOUBLE, 0, *mpSizes):0;
	}

	template <>
	inline bool* ValueObject<bool*>::getDataCopy(void)
	{
		return (mType == EG_BOOLEAN && mDimensions == 1)?(bool*)EG_Object_internal_duplicateDataArray(mpData, EG_BOOLEAN, 0, *mpSizes):0;
	}

	template <>
	inline JString* ValueObject<JString*>::getDataCopy(void)
	{
		if(mType == EG_STRING && mDimensions == 1)
		{
			JString* temp = new JString[*mpSizes];
			for(short i=0; i<*mpSizes; i++)
				temp[i] = (((JString*)(mpData))[i]);
			return temp;
		}
		else
			return 0;
	}

	template <>
	inline Hashtable* ValueObject<Hashtable*>::getDataCopy(void)
	{
		if(mType == EG_HASHTABLE && mDimensions == 1)
		{
			Hashtable* temp = new Hashtable[*mpSizes];
			for(short i=0; i<*mpSizes; i++)
				temp[i] = (((Hashtable*)(mpData))[i]);
			return temp;
		}
		else
			return 0;
	}

	template <>
	inline Object* ValueObject<Object*>::getDataCopy(void)
	{
		if(mType == EG_OBJECT && mDimensions == 1)
		{
			Object* temp = new Object[*mpSizes];
			for(short i=0; i<*mpSizes; i++)
				temp[i] = (((Object*)(mpData))[i]);
			return temp;
		}
		else
			return 0;
	}



	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// getDataAddress                                                                                              //
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	/* Summary
	   \Returns the address of the original content of the object.
	   If you need access to the data above lifetime of the object,
	   call <link KeyObject::getDataCopy, getDataCopy()>.

	   The return type is a pointer to the data, so it is a
	   double-pointer, of course, for template overloads, which data
	   already is a pointer.

	   In case of an error, this method returns NULL.
	   Returns
	   the address of the original content of the object, if
	   successful, NULL otherwise.                                      */
	template<typename Etype>
	Etype* ValueObject<Etype>::getDataAddress(void)
	{
		return (mType == ConfirmAllowed<Etype>::typeName && mDimensions == ConfirmAllowed<Etype>::dimensions)?(Etype*)(mType==EG_CUSTOM?mDimensions?mpData:*(Etype**)mpData:mDimensions?&mpData:mpData):NULL;
	}

	template<typename Etype>
	Etype** ValueObject<Etype*>::getDataAddress(void)
	{
		return (mType == ConfirmAllowed<Etype*>::typeName && mDimensions == ConfirmAllowed<Etype*>::dimensions)?(Etype**)(mType==EG_CUSTOM?mDimensions?mpData:*(Etype**)mpData:mDimensions?&mpData:mpData):NULL;
	}



	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// INTERNALS                                                                                                   //
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	template<typename Etype>
	void ValueObject<Etype>::convert(const Object* const obj, nByte type)
	{
		if(obj && type == obj->getType())
			*((Object*)this) = *obj;
		else
		{
			mType = 0;
			mpSizes = NULL;
			mDimensions = 0;
			mpData = NULL;
		}
	}

	template<typename Etype>
	void ValueObject<Etype*>::convert(const Object* const obj, nByte type, unsigned int dimensions)
	{
		if(obj && type == obj->getType() && dimensions == obj->getDimensions())
			*((Object*)this) = *obj;
		else
		{
			mType = 0;
			mpSizes = NULL;
			mDimensions = 0;
			mpData = NULL;
		}
	}

#ifndef _EG_BREW_PLATFORM
}
#endif

#endif