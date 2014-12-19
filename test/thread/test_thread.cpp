#include "winpch.h"
#include "wyc/util/util.h"
#include "wyc/util/time.h"

#ifdef _DEBUG
 #pragma comment(lib, "fslog_d.lib")
 #pragma comment(lib, "fsutil_d.lib")
 #pragma comment(lib, "fsthread_d.lib")
#else
 #pragma comment(lib, "fslog.lib")
 #pragma comment(lib, "fsutil.lib")
 #pragma comment(lib, "fsthread.lib")
#endif

extern void test_read_write_fence(void);
extern void test_reference_counter(void);
extern void test_critical_section(void);
extern void test_peterson_lock(void);
extern void test_asyncque(void);
extern void test_mpmc_queue(void);
extern void test_async_cache(void); 
extern void test_ring_queue(void);

#include <atomic>

void std_atomic_lock_free_check()
{
	std::atomic_bool bval;
	std::atomic_char cval;
	std::atomic_int  ival;
	std::atomic_long lval;
	std::atomic_int32_t i32val;
	std::atomic_int64_t i64val;
	std::atomic<float>  fval;
	std::atomic<double> dval;

	const char *t = "true";
	const char *f = "false";
	printf("std::atomic_bool is lock-free: %s\n",bval.is_lock_free()?t:f);
	printf("std::atomic_char is lock-free: %s\n",cval.is_lock_free()?t:f);
	printf("std::atomic_int  is lock-free: %s\n",ival.is_lock_free()?t:f);
	printf("std::atomic_long is lock-free: %s\n",lval.is_lock_free()?t:f);
	printf("std::atomic_int32_t is lock-free: %s\n",i32val.is_lock_free()?t:f);
	printf("std::atomic_int64_t is lock-free: %s\n",i64val.is_lock_free()?t:f);
	printf("std::atomic<float>  is lock-free: %s\n",fval.is_lock_free()?t:f);
	printf("std::atomic<double> is lock-free: %s\n",dval.is_lock_free()?t:f);
}

int main(int, char **)
{
//	std_atomic_lock_free_check();
//	test_read_write_fence();
//	test_reference_counter();
//	test_critical_section();
//	test_peterson_lock();
//	test_asyncque();
	test_mpmc_queue();
//	test_async_cache();
//	test_ring_queue();
	wyc_print("Press [Enter] to continue");
	getchar();
	return 0;
}


