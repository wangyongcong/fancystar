/* Exit Games Photon - C++ Client Lib
* Copyright (C) 2004-2011 by Exit Games GmbH. All rights reserved.
* http://www.exitgames.com
* mailto:developer@exitgames.com
*/

#ifndef __OPERATION_REQUEST_H
#define __OPERATION_REQUEST_H

#include "COperationRequest.h"
#include "Utils.h"

#ifndef _EG_BREW_PLATFORM
namespace ExitGames
{
#endif
	class OperationRequest
	{
	public:
		OperationRequest(nByte operationCode, const Hashtable& parameters=Hashtable());
		~OperationRequest(void);

		OperationRequest(const OperationRequest& toCopy);
		OperationRequest& operator=(const OperationRequest& toCopy);

		const Object& operator[](unsigned int index) const;

		JString toString(bool withParameters=false, bool withParameterTypes=false) const;
		const Object& getParameterForCode(nByte parameterCode) const;

		nByte getOperationCode(void) const;
		const Hashtable& getParameters(void) const;
		Hashtable& getParameters(void);
		void setParameters(const Hashtable& parameters);
	private:
		friend class PhotonPeer;

		COperationRequest* toC(COperationRequest* pCOperationRequest) const;

		nByte mOperationCode;
		Hashtable mParameters;
	};
#ifndef _EG_BREW_PLATFORM
}
#endif

#endif