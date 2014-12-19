/* Exit Games Common - C Client Lib
 * Copyright (C) 2004-2010 by Exit Games. All rights reserved.
 * http://www.exitgames.com
 * mailto:developer@exitgames.com
 */

#ifndef __GP_OPERATION_H
	#define __GP_OPERATION_H

	#include "data_structures.h"

	#ifdef __cplusplus
		extern "C"
		{
	#endif
		void EG_writeShort(EG_GpOperation* op, short val);
		void EG_writeInt(EG_GpOperation* op, int val);
		void EG_writeByte(EG_GpOperation* op, nByte val);
		void EG_writeBoolean(EG_GpOperation* op, bool val);
		void EG_writeLong(EG_GpOperation* op, int64 val);
		void EG_writeFloat(EG_GpOperation* op, float val);
		void EG_writeDouble(EG_GpOperation* op, double val);

		bool EG_serialize(EG_GpOperation* op, EG_Object* serObject, bool setType, bool protocolOneFive, bool initEncryption);
		void EG_serializeString(EG_GpOperation* op, EG_CHAR** str, bool setType);
		void EG_serializeStringArray(EG_GpOperation* op, EG_CHAR** strArr, short length, bool setType);
		void EG_serializeByteArray(EG_GpOperation* op, nByte* byteArr, short length, bool setType);
		void EG_serializeIntArray(EG_GpOperation* op, int* intArr, short length, bool setType);
		bool EG_serializeHashTable(EG_GpOperation* op, EG_HashTable* hash, bool setType, bool protocolOneFive);
		void EG_serializeVector(EG_GpOperation* op, EG_Vector* vect, bool setType);
		void EG_serializeBoolean(EG_GpOperation* op, bool* bol, bool setType);
		void EG_serializeByte(EG_GpOperation* op, nByte* bt, bool setType);
		void EG_serializeShort(EG_GpOperation* op, short* srt, bool setType);
		void EG_serializeInteger(EG_GpOperation* op, int* in, bool setType);
		void EG_serializeLong(EG_GpOperation* op, int64* lng, bool setType);
		void EG_serializeFloat(EG_GpOperation* op, float* flt, bool setType);
		void EG_serializeDouble(EG_GpOperation* op, double* dbl, bool setType);
		bool EG_serializeArray_15(EG_GpOperation* op, EG_Object* obj);
		bool EG_serializeArray_15Helper(EG_GpOperation* op, void* pData, nByte type, nByte customType, unsigned int dimensions, short* arraySizes, unsigned int recursionDepth);
		void EG_serializeArray_15Type(EG_GpOperation* op, void* pData, nByte type, nByte customType, short i);
		void EG_serializeCustom(EG_GpOperation* op, void* data, nByte customType, bool setType);

		bool EG_readBoolean(EG_GpOperation* op);
		short EG_readShort(EG_GpOperation* op);
		int EG_readInt(EG_GpOperation* op);
		int64 EG_readLong(EG_GpOperation* op);
		nByte EG_readByte(EG_GpOperation* op);
		float EG_readFloat(EG_GpOperation* op);
		double EG_readDouble(EG_GpOperation* op);

		EG_Object* EG_deserialize(EG_GpOperation* op);
		EG_Object* EG_deserializeType(EG_GpOperation* op, nByte param);
		EG_Object* EG_deserializeVector(EG_GpOperation* op);
		EG_Object* EG_deserializeHashTable(EG_GpOperation* op);
		EG_HashTable EG_deserializeHashTableHelper(EG_GpOperation* op);
		EG_Object* EG_deserializeString(EG_GpOperation* op);
		EG_Object* EG_deserializeInteger(EG_GpOperation* op);
		EG_Object* EG_deserializeByte(EG_GpOperation* op);
		EG_Object* EG_deserializeByteArray(EG_GpOperation* op);
		EG_Object* EG_deserializeBoolean(EG_GpOperation* op);
		EG_Object* EG_deserializeShort(EG_GpOperation* op);
		EG_Object* EG_deserializeIntArray(EG_GpOperation* op);
		EG_Object* EG_deserializeStringArray(EG_GpOperation* op);
		EG_CHAR* EG_deserializeStringHelper(EG_GpOperation* op);
		EG_Object* EG_deserializeLong(EG_GpOperation* op);
		EG_Object* EG_deserializeArray_15(EG_GpOperation* op);
		EG_Object* EG_deserializeFloat(EG_GpOperation* op);
		EG_Object* EG_deserializeDouble(EG_GpOperation* op);
		EG_Object* EG_deserializeObjectArray(EG_GpOperation* op);
		EG_Object* EG_deserializeCustom(EG_GpOperation* op, nByte customType);
		void* EG_deserializeCustomHelper(EG_GpOperation* op, nByte customType);

		bool EG_GpserializeData(EG_GpOperation* op, bool protocolOneFive);
		void EG_GpdeserializeData(EG_GpOperation* op);

		void EG_setUrl(EG_GpOperation* op, const char* url);
		void EG_setOpType(EG_GpOperation* op, nByte opType);
		void EG_setDoDeserialize(EG_GpOperation* op, bool doDeserialize);
		void EG_setOperation(EG_GpOperation* op, nByte operation);
		nByte EG_getOperation(EG_GpOperation* op);
		void EG_addParameter(EG_GpOperation* op, nByte param, EG_Object* serObject);
		void EG_releaseGpOperation(EG_GpOperation* gp);

		EG_Object* EG_getRetVal(EG_GpOperation* op, nByte retVal);
		EG_HashTable* EG_getRetVals(EG_GpOperation* op);
		EG_HashTable* EG_getParameters(EG_GpOperation* op);
	#ifdef __cplusplus
		}
	#endif 

#endif