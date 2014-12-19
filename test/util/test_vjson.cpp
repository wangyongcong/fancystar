#include "wyc/util/util.h"
#include "wyc/util/time.h"
#include "wyc/util/fjson.h"

using namespace vjson;

#define IDENT(n) for (int i = 0; i < n; ++i) printf("    ")

void json_print(const json_value *value, int ident = 0)
{
	IDENT(ident);
	if (value->name) printf("\"%s\" = ", value->name);
	switch(value->type)
	{
	case JSON_NULL:
		printf("null\n");
		break;
	case JSON_OBJECT:
	case JSON_ARRAY:
		printf(value->type == JSON_OBJECT ? "{\n" : "[\n");
		for (const json_value *it = value->first_child; it; it = it->next_sibling)
		{
			json_print(it, ident + 1);
		}
		IDENT(ident);
		printf(value->type == JSON_OBJECT ? "}\n" : "]\n");
		break;
	case JSON_STRING:
		printf("\"%s\"\n", value->string_value);
		break;
	case JSON_INT:
		printf("%d\n", value->int_value);
		break;
	case JSON_FLOAT:
		printf("%f\n", value->float_value);
		break;
	case JSON_BOOL:
		printf(value->int_value ? "true\n" : "false\n");
		break;
	}
}

void test_json_loader()
{
	wyc::xjson json;
	if(!json.load_file("../res/DefaultLook.json"))
		return;
	const json_value *root = json.get_root();
	if(root) json_print(root);
}

#include <fstream>

void test_json_writer()
{
	std::fstream fs;
	fs.open("../../../tmp/dummy.json",std::ios::out);
	if(!fs.is_open()) {
		printf("could not open file\n");
		return;
	}
	printf("writing json file...\n");
	wyc::xjson_writer json(fs);
	json.begin_object();
		json.add_entry("string1","foo bar");
		json.add_entry("int1",-1234);
		json.add_entry("uint1",1234);
		json.add_entry("unicode1",L"测试数据");
		json.begin_object("object1");
			json.add_entry("float1",1.0f);
		json.end_object();
		json.begin_array("array1");
			json.begin_object("object2");
				json.add_entry("float2",3.1415926f);
				json.add_entry("float3",2.71828182845904523536);
			json.end_object();
		json.end_array();
		json.begin_array("array2");
			json.add_entry(3.1415926f);
			json.add_entry(2.71828182845904523536);
		json.end_array();
		json.begin_array("array3");
			json.add_entry("hello world");
			json.add_entry(L"测试数据2");
		json.end_array();
	json.end_object();
	fs.close();
	printf("file is saved.\n");
}
