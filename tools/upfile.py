#encoding utf-8

import sys, os, os.path, shutil, time

def walk_through(indir, include, exclude, command):
	cnt=0
	err=0
	begt=time.clock()
	for curdir, subdirs, files in os.walk(indir):
		print("search",curdir)
		ret=command(curdir,files)
		cnt+=ret[0]
		err+=ret[1]
		if len(include)>0:
			newdirs=[]
			for d in subdirs:
				if d in include:
					newdirs.append(d)
			## swap two lists
			num=len(newdirs)
			i=0
			while i<num:
				subdirs[i]=newdirs[i]
				i+=1
			i=len(subdirs)
			while i>num:
				i-=1
				subdirs.pop(i)
		for d in exclude:
			if d in subdirs:
				subdirs.remove(d)
	endt=time.clock()
	print("  %d files were handled"%cnt)
	print("  %d errors occurred"%err)
	print("  %.4f seconds passed"%(endt-begt))

def append_splitter(sdir):
	if len(sdir)>0:
		c=sdir[-1]
		if c!="\\" and c!="/":
			sdir+=os.sep
	return sdir

def fake_proc():
	pass

class file_updater:

	def __init__(self, exts, outdir, skip=[], root=None, just_print=True):
		self.filter=None
		self.__exts=exts
		self.__output=append_splitter(outdir)
		self.__skip=skip
		self.__root=root
		self.touched_files=[]
		if just_print:
			self.command=fake_proc
			self.makedirs=fake_proc
		else:
			self.command=shutil.copy2
			self.makedirs=os.makedirs
		if not os.path.exists(self.__output):
			print("[mkdir]",self.__output)
			self.makedirs(self.__output)
	
	def process(self, curdir, files):
		cnt=0
		err=0
		dstdir=None
		for f in files:
			if f in self.__skip:
				continue
			base, ext=os.path.splitext(f)
			## 过滤后缀
			if ext not in self.__exts:
				continue
			## 应用自定义过滤器
			if self.filter and not self.filter(base):
				continue
			if not dstdir:
				dstdir=self.__output
				if self.__root:
					n=len(self.__root)
					if n<len(curdir) and curdir[:n]==self.__root:
						dstdir=append_splitter(self.__output+curdir[n+1:])
						if not os.path.exists(dstdir):
							print("[mkdir]",dstdir)
							self.makedirs(dstdir)
			src=os.path.join(curdir,f)
			dst=dstdir+f
			if not os.path.exists(dst):
				print("[create]",dstdir,f)
				self.command(src,dst)
				self.touched_files.append(dst)
				cnt+=1
			else:
				fst=os.stat(dst)
				fst2=os.stat(src)
				if fst2.st_mtime-fst.st_mtime>0.0001:
					print("[update]",dstdir,f)
					self.command(src,dst)
					self.touched_files.append(dst)
					cnt+=1
		return (cnt,err)

class libfile_filter:
	
	def __init__(self, prefixs, suffixs, basenames):
		self.__prefixs=prefixs
		self.__suffixs=suffixs
		self.__basenames=basenames
		if "*" in self.__prefixs:
			self.__prefixs.remove("*")
			self.__nullprefix=True
		else: self.__nullprefix=False
		if "*" in self.__suffixs:
			self.__suffixs.remove("*")
			self.__nullsuffix=True
		else: self.__nullsuffix=False

	def filter(self, fname):
		basebeg=0
		basend=len(fname)
		if self.__prefixs:
			for pre in self.__prefixs:
				if fname.startswith(pre):
					basebeg=len(pre)
					break
			if not self.__nullprefix and basebeg==0:
				return False
		if self.__suffixs:
			for suf in self.__suffixs:
				if fname.endswith(suf):
					basend=len(fname)-len(suf)
					break
			if not self.__nullsuffix and basend==len(fname):
				return False
		if self.__basenames:
			base=fname[basebeg:basend]
			if base not in self.__basenames:
				return False
		return True

	def __str__(self):
		s="[libfile_filter]\n"
		if self.__prefixs:
			s+="prefix: "+"; ".join(self.__prefixs)+"\n"
		if self.__suffixs:
			s+="suffix: "+"; ".join(self.__suffixs)+"\n"
		if self.__basenames:
			s+="libname: "+"; ".join(self.__basenames)
		return s
	
def parse_cmdline():
	options={}
	opname=""
	n=len(sys.argv)
	i=1
	while i<n:
		arg=sys.argv[i]
		if arg[0]=="-":
			opname=arg[1:]
			options[opname]=""
		else:
			if len(options[opname])>0:
				options[opname]+=" "
			options[opname]+=arg
		i+=1
	return options

'''
upfile -in 输入目录 -out 输出目录 [-filter] [-include] [-exclude]

将输入目录中的文件更新到输出目录中。

-p:			只是输出，但不执行实际的操作，用于DEBUG
-filter:	扩展名列表，只有在列表中的文件类型才进行比较
			例如： -filter .h .cpp .py
-include:	目录列表，只有在列表中的子目录才会被扫描，默认扫描所有子目录
-exclude:	排除目录列表，在列表中的目录会被排除，默认排除: .svn, .git
-keep:		保持目录结构，会自动创建子目录
-lib:		开启库文件过滤器
-libprefix:	只在-lib开启后生效，指定需要更新的库文件前缀
-libsuffix:	只在-lib开启后生效，指定需要更新的库文件后缀
-libname:	只在-lib开启后生效，指定需要更新的库文件名（移除前缀和后缀）
'''
def upfile(options):
	indir=options["in"]
	indir=os.path.abspath(indir)
	outdir=options["out"]
	outdir=os.path.abspath(outdir)
	ext=options.get("filter","").split()
	skip=options.get("skip","").split()
	include=options.get("include","").split()		
	exclude=[".svn",".git"]
	if "exclude" in options:
		exclude+=options["exclude"].split()
	just_print=options.get("p")!=None
	if "keep" in options:
		root=indir
	else: 
		root=None
	custom_filter=None
	if "lib" in options:
		prefix=options.get("libprefix","").split()
		suffix=options.get("libsuffix","").split()
		base=options.get("libname","").split()
		custom_filter=libfile_filter(prefix,suffix,base)
	print("===============================================")
	print("scan:",indir)
	print("output:",outdir)
	indent=" "*4
	if include:
		print("scanned sub-dirs:")
		for s in include:
			print(indent+s)
	if exclude:
		print("skipped sub-dirs:")
		for s in exclude:
			print(indent+s)
	if ext:
		print("file filters:","*"+"; *".join(ext))
	if skip: 
		print("skipped files:")
		for s in skip:
			print(indent+s)
	if custom_filter:
		print("custom filter:")
		s=str(custom_filter)
		s=indent+s.replace("\n","\n"+indent)
		print(s)
	print("===============================================")
	fu=file_updater(ext,outdir,skip,root,just_print)
	if custom_filter:
		fu.filter=custom_filter.filter
	walk_through(indir,include,exclude,fu.process)
	return fu.touched_files

if __name__=="__main__":
	upfile(parse_cmdline())
