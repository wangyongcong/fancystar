#ifndef __HEADER_WYC_PROTOCOL
#define __HEADER_WYC_PROTOCOL

namespace wyc
{

enum OperationCodes
{
	OP_ECHO = 1,
	OP_LOGIN,
	OP_LOGOUT,
	OP_UPLOAD_PACKET,
	OP_DOWNLOAD_PACKET,
};

enum EventCodes
{
};

} // namespace wyc

#endif __HEADER_WYC_PROTOCOL
