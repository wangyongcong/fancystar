#include <iostream>
#include "wyc/util/util.h"
#include "wyc/util/time.h"
#include "wyc/util/base64.h"

using namespace wyc;

extern void test_singlelist();
extern void test_xargs();
extern void test_priority_queue();
extern void test_circular_queue();
extern void test_sparse_array();
extern void test_task_scheduler();
extern void test_dict();
extern void test_set();
extern void test_wsbucket();
extern void test_file_path();
extern void test_config_file();
extern void test_strutil();
extern void test_log2();
extern void test_bubble_sort(int);
extern void test_quick_sort(int);
extern void test_binary_search();
extern void test_json_loader();
extern void test_json_writer();
extern void test_arrange();
extern void test_string_dict();

extern void strcpy_profile();

int main(int, char **)
{
	setlocale(LC_ALL,"chs");
	xdate dt;
	dt.get_date();
	srand(dt.hour()*10000+dt.minute()*100+dt.second());
#ifdef _DEBUG
//	test_singlelist();
//	test_xargs();
//	test_circular_queue();
	test_priority_queue();
//	test_sparse_array();
//	test_task_scheduler();
//	test_dict();
//	test_set();
//	test_wsbucket();
//	test_log2();
//	test_bubble_sort(32);
//	test_quick_sort(256);
//	test_binary_search();
//	test_config_file();
//	test_file_path();
//	test_strutil();
//	test_json_loader();
//	test_json_writer();
//	test_arrange();
//	test_string_dict();
#else
	// benchmark
	strcpy_profile();
#endif
	wyc_print("Press [Enter] to continue");
	getchar();
	return 0;
}

