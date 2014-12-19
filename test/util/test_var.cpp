#include <cassert>
#include "wyc/util/util.h"
#include "wyc/util/var.h"
#include "wyc/util/time.h"

using namespace wyc;

struct TEST_DATA
{
	unsigned id;
	float speed;
	std::string name;
	bool operator == (const TEST_DATA &td) const {
		return id==td.id && speed==td.speed && name==td.name;
	}
};

void test_xargs()
{
	xargs var, var2;

	// 简单数据
	int ival=-222, ival2;
	unsigned uval=333, uval2;
	float fval=3.14159f, fval2;
	std::string str="programmer", str2;
	std::wstring wstr=L"程序员", wstr2;
	void *ptr=(void*)0xcccc, *ptr2;

	var.resize(10);
	var[0]=ival;
	var[1]=uval;
	var[2]=fval;
	var[3]=str;
	var[4]=wstr;
	var[5]=ptr;
	var[6]=var[0];
	var[7]=var[3];
	var2.resize(2);
	var2[0]=var[0];
	var2[1]=var[3];
	unsigned oldsz=var.size();
	var.extend(var2);
	assert(var.size()==oldsz+var2.size());
//	var2[2]=var;
	var.clear();
	var2.clear();

	var<<ival<<uval<<fval<<str<<wstr<<ptr;
	assert(var.size()==6);
	wyc_print(var.str());
	var.reset();
	var>>ival2>>uval2>>fval2>>str2>>wstr2>>ptr2;
	assert(var);
	assert(ival==ival2);
	assert(uval==uval2);
	assert(fval==fval2);
	assert(str==str2);
	assert(wstr==wstr2);
	assert(ptr==ptr2);

	ival2=var[0];
	uval2=var[1];
	fval2=var[2];
	str2=var[3];
	wstr2=var[4];
	ptr2=var[5];
	assert(ival==ival2);
	assert(uval==uval2);
	assert(fval==fval2);
	assert(str==str2);
	assert(wstr==wstr2);
	assert(ptr==ptr2);
	
	var2=var;
	var2.reset();
	var2>>ival2>>uval2>>fval2>>str2>>wstr2>>ptr2;
	assert(var2);
	assert(ival==ival2);
	assert(uval==uval2);
	assert(fval==fval2);
	assert(str==str2);
	assert(wstr==wstr2);
	assert(ptr==ptr2);

	var.clear();
	var<<"test process"<<L"测试程序";
	var.reset();
	str="test process";
	wstr=L"测试程序";
	var>>str2>>wstr2;
	assert(str==str2);
	assert(wstr==wstr2);
	const char *pstr=var[0];
	const wchar_t *pwstr=var[1];
	assert(pstr);
	assert(pwstr);
	assert(str==pstr);
	assert(wstr==pwstr);
	var2.clear();
	var2.resize(2);
	var2[0]=pstr;
	var2[1]=pwstr;
	var2.reset();
	var2>>str2>>wstr2;
	assert(var2);
	assert(str==str2);
	assert(wstr==wstr2);
	var[0].get(str2);
	var[1].get(wstr2);
	assert(str==str2);
	assert(wstr==wstr2);

	// 结构体
	TEST_DATA td, td2;
	td.id=1234;
	td.speed=0.8f;
	td.name="ycwang";
	var.clear();
	
	var<<td;
	var.reset();
	var>>td2;
	assert(var);
	assert(td==td2);

	td2.id=0;
	var2=var;
	var2.reset();
	var2>>td2;
	assert(var2);
	assert(td==td2);

	const TEST_DATA *cptd=((const xvar*)&var[0])->packet<TEST_DATA>(), 
		*cptd2=((const xvar*)&var2[0])->packet<TEST_DATA>();
	assert(cptd==cptd2);
	TEST_DATA *ptd=var[0].packet<TEST_DATA>(),
		*ptd2=var2[0].packet<TEST_DATA>();
	assert(ptd!=ptd2);
	assert(ptd2==cptd2);
	ptd->id=999;
	ptd->speed=0.2f;
	ptd->name="carmack";
	var[0].get(td);
	str=td.name;
	str2=td2.name;
	assert(str!=str2);
	wyc_print("td[0]=%s",str.c_str());
	wyc_print("td[1]=%s",str2.c_str());

	var.clear();
	var<<(&td);
	var.reset();
	var>>ptd;
	assert(var);
	assert(ptd==&td);
	ptd=var[0].ptr<TEST_DATA>();
	assert(ptd==&td);

	char raw_data[32], raw_data2[32];
	memset(raw_data,2,sizeof(raw_data));
	var[0].set(raw_data,sizeof(raw_data));
	var[0].get(raw_data2,sizeof(raw_data2));
	assert(memcmp(raw_data,raw_data2,sizeof(raw_data))==0);

	var.clear();
	var.printf("dufpsw",ival,uval,fval,ptr,str.c_str(),wstr.c_str());
	ival2=0;
	uval2=0;
	fval2=0.0f;
	ptr2=0;
	str2="";
	wstr2=L"";
	var.scanf("dufpsw",&ival2,&uval2,&fval2,&ptr2,&str2,&wstr2);
	assert(ival==ival2);
	assert(uval==uval2);
	assert(fval==fval2);
	assert(ptr==ptr2);
	assert(str==str2);
	assert(wstr==wstr2);
}
