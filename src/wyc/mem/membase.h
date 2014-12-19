#ifndef __HEAER_XMEMORY_MANAGER
#define __HEAER_XMEMORY_MANAGER

namespace wyc
{

void* debug_malloc(size_t sz, const char *file, size_t line, const char *function);
void debug_free(void *ptr);

} // namespace wyc

#endif // end of __HEAER_XMEMORY_MANAGER

