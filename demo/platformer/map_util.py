#-*- coding: utf-8 -*-
import struct

class CMapData (object):
	def __init__(self):
		self.version = 1
		self.bounding_box = (0,0,0,0)
		self.min_obj_size = (0,0)
		self.objs = []

def load_map (filename):
	f = open(filename,"r")
	s = f.read(4)
	if len(s)!=4:
		f.close()
		return None
	ver = struct.unpack("<B3x",s)[0]
	if ver!=1:
		f.close()
		return None
	s = f.read(16)
	if len(s)!=16:
		f.close()
		return None
	bbox = struct.unpack("<4f",s)
	s = f.read(8)
	if len(s)!=8:
		f.close()
		return None
	min_size = struct.unpack("<2f",s)
	objs = []
	obj_size = 20
	max_obj_count = 10000
	for i in xrange(max_obj_count):
		s=f.read(obj_size)
		if len(s)<obj_size:
			break
		objs.append(struct.unpack("<4fB3x",s))
	f.close()
	map = CMapData()
	map.version = ver
	map.bounding_box = bbox
	map.min_obj_size = min_size
	map.objs = objs
	return map
	