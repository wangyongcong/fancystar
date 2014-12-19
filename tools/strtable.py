# encoding utf-8

import os, os.path, string, struct, sys
import wycpy

def process_strdef(filename, tables):
	try:
		doc=open(filename,'r')
	except:
		print("[ERROR] can't open file:",filename)
		return
	table_name=''
	entries=None
	lineIndex=0
	try:
		for line in doc:
			lineIndex+=1
			line=line.strip()
			if not line:
				continue
			if table_name:
				if line=='{':
					continue
				elif line=='}':
					print('process [%s]...OK'%table_name)
					table_name=""
				else:
					pair=line.split(':',1)
					assert len(pair)==2
					entry_name=pair[0].strip()
					entry_value=pair[1].strip()
					assert len(entry_name)>0 and "no entry name"
					assert len(entry_value)>0 and "no entry value"
					entries[entry_name]=entry_value
			else:
				table_name=line
				entries={}
				tables[table_name]=entries
	except:
		doc.close()
		print('[ERROR] Line[%d] Error!'%lineIndex)
		raise
	doc.close()
	

def generate_tables(tableID, tables, output_file, header_file, codec='gb2312'):
	## generate binary data
	out=open(output_file,'wb')
	data=[]
	table_key=[]
	entry_key=[]
	pack=struct.pack("I",100)
	out.write(pack)
	pack=struct.pack("I",len(tables))
	out.write(pack)
	skip=len(pack)*(2+2*len(tables))
	for table_name, entries in tables.items():
		#print "pos:", skip
		hashkey=wycpy.strhash(table_name)
		pack=struct.pack("II%dsb"%len(table_name),len(table_name),hashkey,table_name,0)
		offset=0
		table_key.append((offset<<8)|(0xFF&tableID))
		print("[%s]: len=%d, key=0x%X (%d)"%(table_name,len(table_name),hashkey,offset))
		offset+=len(pack)
		for entry_name, entry_value in entries.items():
			bstr=entry_value.encode(codec)
			hashkey=wycpy.strhash(bstr)
			s=struct.pack("II%dsb"%len(bstr),len(bstr),hashkey,bstr,0)
			pack+=s
			entry_key.append((offset<<8)|(0xFF&tableID))
			print("\t[%s]: len=%d, key=0x%X (%d)"%(entry_name,len(bstr),hashkey,offset))
			offset+=len(s)
		data.append(pack)
		out.write(struct.pack('II',skip,len(pack)))
		skip+=len(pack)
		tableID+=1
	for pack in data:
		out.write(pack)
	out.close()
	pack=None
	data=None
	## generate c++ header
	out=open(header_file,'w')
	idx=0
	out.write("enum STRING_TABLE_ID {\n")
	for table_name in tables.keys():
		out.write('\tSTAB_%s = 0x%X,\n'%(table_name.upper(),table_key[idx]))
		idx+=1
	out.write("};\n")
	out.write("enum STRING_ID {\n")
	idx=0
	for table_name, entries in tables.items():
		for entry_name in entries.keys():
			out.write('\tCSTR_%s = 0x%X,\n'%(entry_name.upper(),entry_key[idx]))
			idx+=1
	out.write("};\n")
	out.close()
	return tableID

def format_path(path):
	if not os.path.isabs(path):
		os.path.abspath(path)
	path=path.replace("\\","/")
	if path[-1]=='/':
		path=path[:-1]
	return path

def dump_strtable(tables):
	for fname, table_group in tables.items():
		print("[%s]"%fname)
		for table_name, entries in table_group.items():
			print("\ttable: [%s]"%table_name)
			for name, value in entries.items():
				print("\t\t%s : %s"%(name, value))

def scan_directory(tables, input_directory, excludes):
	names=os.listdir(input_directory)
	for pathname in names:
		if pathname in excludes:
			continue
		fullpath='/'.join((input_directory,pathname))
		if os.path.isdir(fullpath):
			scan_directory(tables,fullpath,tables)
		elif os.path.isfile(fullpath):
			if pathname[-4:]=='.def':
				print("scanning:", pathname, "(%s)"%fullpath)
				strinfo={}
				process_strdef(fullpath,strinfo)
				tables[pathname[:-4]]=strinfo
	return tables

def generate_string_table(input_directory, output_directory, header_directory, excludes, codec='gb2312'):
	input_directory=format_path(input_directory)
	output_directory=format_path(output_directory)
	tables={}
	tableID=0
	scan_directory(tables,input_directory,excludes)
	print('scan string definition files...OK')
	#dump_strtable(tables)
	if not os.path.exists(output_directory):
		os.makedirs(output_directory)
	allTables={}
	for pathname, table_group in tables.items():
		for table_name, entries in table_group.items():
			tab=allTables.get(table_name,None)
			if not tab:
				allTables[table_name]=entries
			else: tab.update(entries)
	outfile="%s/string.stab"%(output_directory,)
	header="%s/stringid.h"%(header_directory,)
	tableID=generate_tables(tableID,allTables,outfile,header,codec)
	print('output string table and headers...OK')

if __name__=="__main__":
	if len(sys.argv)<2:
		print('Argument error! The command should be:')
		print('\tpython strtable.py strdef_dir strtab_dir [header_dir]')
	else:
		strdef_dir=sys.argv[1]
		strtab_dir=sys.argv[2]
		if len(sys.argv)>3:
			header_dir=sys.argv[3]
		else:
			header_dir=strdef_dir
		generate_string_table(strdef_dir,strtab_dir,header_dir,['.svn'])

