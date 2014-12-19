# encoding utf-8

import os, os.path, shutil

def rmtree_exception_handler(func, path, excinfo):
	print("[失败]",func,":",path)

def clean_directory(path, keep_files, rm_files, exclude_subdirs, just_print):
	keep_file_types=[]
	tmpli=[]
	for filename in keep_files:
		basename, ext=os.path.splitext(filename)
		if basename=="*":
			keep_file_types.append(ext)
		else: tmpli.append(filename)
	keep_files=tmpli

	rm_file_types=[]
	tmpli=[]
	for filename in rm_files:
		basename, ext=os.path.splitext(filename)
		if basename=="*":
			rm_file_types.append(ext)
		else: tmpli.append(filename)
	rm_files=tmpli
	
	counter=0
	for curdir, subdirs, files in os.walk(path):
		for filename in files:
			if filename in keep_files:
				continue
			basename, ext=os.path.splitext(filename)
			if ext in keep_file_types:
				continue
			if filename in rm_files or ext in rm_file_types:
				try:
					filename=curdir+"\\"+filename
					print("删除文件:",os.path.exists(filename),filename)
					if not just_print:
						os.remove(filename)
					counter+=1
				except:
					print("[失败] 无法删除")
		for dirname in exclude_subdirs:
			if dirname in subdirs:
				subdirs.remove(dirname)
	print(counter,"files was deleted")


'''
清理工程目录，移除所有中间文件和目标文件
'''
def rm_tmp_files(just_print=True):
	## 要保留的文件
	keep_files=("FreeImage.dll","FreeImage.lib","glew32.dll","glew32.lib")
	## 要清理的文件
	rm_files=("*.dll","*.exe","*.ilk","*.idb","*.lib","*.ncb","*.pdb","*.suo","*.user")
	## 要删除的目录
	rm_dirs=("out",)

	os.chdir("..")
	for pathname in rm_dirs:
		print("删除目录:",pathname)
		if not just_print:
			shutil.rmtree(pathname,False,rmtree_exception_handler)
	clean_directory("bin",keep_files,rm_files,(".svn",),just_print)
	clean_directory("build\\msvc",keep_files,rm_files,(".svn",),just_print)
	clean_directory("sdk\\lib",keep_files,rm_files,(".svn",),just_print)

'''
移除SVN目录
'''
def rm_svn(path):
	for curdir, subdirs, files in os.walk(path):
		if ".svn" in subdirs:
			subdirs.remove(".svn")
			os.system("rd /S /Q %s\\.svn"%curdir)

def confirm_input():
	print("这项操作将是不可逆的，真的要执行吗？")
	confirm=input("请输入[Yes]确定，任意键取消: ")
	return confirm.lower()=="yes"

if __name__=="__main__":
	while(True):
		print("请输入你要执行的操作：")
		print("  [1] 清理工程目录，包括所有的中间文件和临时文件")
		print("  [2] 清理.svn目录，这将会移除svn信息")
		print("  [q] 退出")
		cmdstr=input()
		if cmdstr=="1":
			if confirm_input():
				rm_tmp_files(False)
				input("执行成功")
			else:
				input("取消操作")
		elif cmdstr=="2":
			if confirm_input():
				rm_svn("..")
				input("执行成功")			
			else:
				input("取消操作")
		elif cmdstr.lower()=="q":
			print("程序退出")
			break
		else:
			input("无效的指令")
		os.system("cls")
		
