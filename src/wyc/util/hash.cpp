#include "wyc/util/hash.h"

namespace wyc
{

#include "wyc/util/crc32.cpp"

unsigned int hash_crc32(const char *str, unsigned int len)
{
	register unsigned hash=0xFFFFFFFF;
	register const unsigned char *iter=(const unsigned char*)str;
	register const unsigned char *end=iter+len;
	while(iter<end) {
		hash=(hash>>8) ^ CRC32_LOOKUP_TABLE[(hash ^ *iter) & 0xFF];
		++iter;
	}
	return hash^0xFFFFFFFF;
}

unsigned int strhash(const char *str)
{
	register unsigned hash=0xFFFFFFFF;
	register const unsigned char *iter=(const unsigned char*)str;
	while(*iter!=0) {
		hash=(hash>>8) ^ CRC32_LOOKUP_TABLE[(hash ^ *iter) & 0xFF];
		++iter;
	}
	return hash^0xFFFFFFFF;
}

unsigned int strhash(const wchar_t *str)
{
	register unsigned hash=0xFFFFFFFF;
	register const wchar_t *iter=str;
	while(*iter!=0) {
		hash=(hash>>8) ^ CRC32_LOOKUP_TABLE[(hash ^ *iter) & 0xFF];
		++iter;
	}
	return hash^0xFFFFFFFF;
}

} // namespace wyc
