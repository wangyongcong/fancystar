#encoding utf-8

#----------------------------------------------------------------
# version: Python 2.x
# desc: convert CEGUI imageset file (xml) to JSON
#----------------------------------------------------------------

import os
import os.path
import xml.parsers.expat

imageset=None
images=[]

def XmlDeclHandler(version, encoding, standalone):
	print 'version:',version
	print 'encoding:',encoding
	print 'standalone:',standalone

def StartElementHandler(name, attributes):
	global imageset
	global images
	if name == "Imageset":
		imageset=attributes
	elif name == "Image":
		images.append(attributes)

	if __debug__:
		print 'Element:',name
		for key, val in attributes.iteritems():
			print '\t',key,':',val

def EndElementHandler(name):
	if __debug__:
		print 'Element END:',name

def cegui_imageset_to_json(file_name):
	parser=xml.parsers.expat.ParserCreate()
	xml_file_name="".join(("../res/",file_name,".imageset"))
	try:
		xml_file=open(xml_file_name)
	except IOError:
		print "[ERROR] Can't open imageset file:",xml_file_name
		return False
	parser.XmlDeclHandler=XmlDeclHandler
	parser.StartElementHandler=StartElementHandler
	parser.EndElementHandler=EndElementHandler
	parser.ParseFile(xml_file)
	xml_file.close()
	global imageset
	global images
	json_file_name="".join(("../res/",file_name,".json"))
	try:
		json_file=open(json_file_name,"w")
	except IOError:
		print "[ERROR] Can't open JSON file:",json_file_name
		return False
	json_file.write("{\n")
	json_file.write("\t\"name\": \"%s\",\n"%(imageset['Name'],))
	json_file.write("\t\"image_file\": \"res/%s\",\n"%(imageset['Imagefile'],))
	json_file.write("\t\"texture_width\": %s,\n"%(imageset.get('TextureWidth',0),))
	json_file.write("\t\"texture_height\": %s,\n"%(imageset.get('TextureHeight',0),))
	json_file.write("\t\"image_count\": %s,\n"%(len(images),))
	json_file.write("\t\"images\": \n\t[\n")
	for img in images:
		json_file.write("\t\t{\n")
		json_file.write("\t\t\t\"name\": \"%s\",\n"%img["Name"])
		json_file.write("\t\t\t\"xpos\": %s,\n"%img["XPos"])
		json_file.write("\t\t\t\"ypos\": %s,\n"%img["YPos"])
		json_file.write("\t\t\t\"width\":  %s,\n"%img["Width"])
		json_file.write("\t\t\t\"height\": %s,\n"%img["Height"])
		json_file.write("\t\t},\n")
	json_file.write("\t],\n}") # end of imageset
	json_file.close()
	print "Converted successfully"
	return True
	

if __name__ == "__main__":
	cegui_imageset_to_json("DefaultLook")

