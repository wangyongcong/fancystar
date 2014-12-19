#include "wyc/util/util.h"
#include "wyc/util/fpath.h"
#include "wyc/util/fconfig.h"

using namespace wyc;

void test_config_file() 
{
	wyc::xconfig cfg;
	if(!cfg.load("res/newhand.scn")) {
		assert(0);
		return;
	}
	if(!cfg.set_cur_section("baseinfo")) {
		assert(0);
		return;
	}
	std::string str;
	cfg.get_key("name",str);
	cfg.get_key("width",str);
	cfg.get_key("height",str);
	cfg.get_key("region",str);
}

void test_file_path()
{
	wyc_print("[test] file path");
	const char *path="e:\\dev\\fancystar\\src\\";
	wyc::xfilepath fs(path);
	wyc_print(fs.get_path());
	fs.set_fname("test.txt");
	wyc_print(fs.get_path());
	assert(0==strcmp(fs.get_path(),"e:\\dev\\fancystar\\src\\test.txt"));
	fs=path;
	fs.set_fname("test.txt");
	wyc_print(fs.get_path());
	assert(0==strcmp(fs.get_path(),"e:\\dev\\fancystar\\src\\test.txt"));

	fs.set_workdir("////\\\\e:\\dev//fancystar\\//src");
	wyc_print(fs.get_path());
	assert(0==strcmp(fs.get_path(),path));
	fs.set_workdir("e:/dev/..//\\..//../././../dev/./fancystar/src");
	wyc_print(fs.get_path());
	assert(0==strcmp(fs.get_path(),path));
	fs.set_workdir("e:/.svn/..dummy/../../../../dev/fancystar/src/");
	wyc_print(fs.get_path());
	assert(0==strcmp(fs.get_path(),path));

	const char *path2="..\\..\\..\\src\\util\\";
	fs.set_workdir(".././.././../src/util");
	wyc_print(fs.get_path());
	assert(0==strcmp(fs.get_path(),path2));
	fs.set_workdir("./.././.././../src/util");
	wyc_print(fs.get_path());
	assert(0==strcmp(fs.get_path(),path2));

	const char *path3="test\\app\\resource\\pic.png";
	const char *path4="test\\app\\resource\\pic.jpg";
	fs.set_fpath("test/app/resource/pic.png");
	wyc_print(fs.get_path());
	assert(0==strcmp(fs.get_path(),path3));
	fs.set_fname("pic.jpg");
	wyc_print(fs.get_path());
	assert(0==strcmp(fs.get_path(),path4));
	assert(0==strcmp(fs.basename(),"pic.jpg"));
	assert(0==strcmp(fs.extname(),"jpg"));

	const char *path5="test\\app\\resource\\..\\..\\picture\\pic.png";
	fs.set_fname("../../picture/pic.png");
	wyc_print(fs.get_path());
	assert(0==strcmp(fs.get_path(),path5));

	const char *path6="test\\picture\\pic.png";
	fs.chg_workdir("../../picture");
	fs.set_fname("pic.png");
	wyc_print(fs.get_path());
	assert(0==strcmp(fs.get_path(),path6));

	std::string str=fs.get_path();
	assert(str.size()==fs.size());
	fs.get_workdir(str);
	unsigned len=fs.get_workdir(0,0);
	char *pstrbuff=new char[len+1];
	fs.get_workdir(pstrbuff,len+1);
	wyc_print(str.c_str());
	wyc_print(pstrbuff);
	assert(str==pstrbuff);
	assert(str.size()==len);
	delete [] pstrbuff;

	const char *path7="test\\picture\\sub01\\sub02\\sub03\\sub04\\doc.txt";
	fs.set_fname(0);
	fs.append_subpath("sub01/");
	fs.append_subpath("sub02/");
	fs.append_subpath("sub03/sub04/doc.txt");
	wyc_print(fs.get_path());
	assert(0==strcmp(fs.get_path(),path7));

	fs.absdir();
	wyc_print(fs.get_path());
	fs.chg_fpath("../doc.txt");
	wyc_print(fs.get_path());

	wyc_print("[test] All tests are passed");
	return;
}


