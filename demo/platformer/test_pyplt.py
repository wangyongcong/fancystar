#-*- coding: utf-8 -*-
import gc
import random
import math
import sys

import pyplt
#import map_util

def test_pyplt():
	scn_w = 1000
	scn_h = 1000
	scn = pyplt.create_scene(0,0,scn_w,scn_h,20,20)
	print 'scene:'
	print '\tsize:',scn.size
	print '\tgrid:',scn.grid_size
	print '\tLOD:',scn.lod
	agents = []
	count = 10
	print "add [%d] agents"%count
	for i in xrange(count):
		x = random.random()*scn_w
		y = random.random()*scn_h
		if random.random():
			t = 0
		else:
			t = 1
		a = pyplt.create_agent(x,y,10,10,t,1)
		print '\t agent[%d]:'%i, a.pos, a.radius, a.group, a.type
		assert(a.type == t)
		scn.add_agent(a)
		agents.append(a)
	print 'agent count:', scn.agent_count
	assert(scn.agent_count==count)
	# 每个agent持有1个ref + scn的ref + sys.getrefcount的ref
	assert(sys.getrefcount(scn) == scn.agent_count+2)
	print 'static test'
	idx = 0
	for a in agents:
		cd_list = scn.static_test(a,1)
		print '\tagents[%d]: %d collisions'%(idx, len(cd_list)), sys.getrefcount(cd_list)
		idx+=1
	print 'sweep test'
	idx = 0
	for a in agents:
		x = random.random()*scn_w
		y = random.random()*scn_h
		obj, t, normal = scn.sweep_test(a,(x,y),1)
		if obj:
			idx2 = agents.index(obj)
			print '\tagents[%d]: hit agent[%d]'%(idx,idx2),t,normal
		else:
			print '\tagents[%d]: no collisions'%idx
		idx+=1
	print 'sweep intersect'
	idx = 0
	for a in agents:
		x = random.random()*scn_w
		y = random.random()*scn_h
		cd_list = scn.sweep_intersect(a,(x,y),1)
		print '\tagents[%d]: %d collisions'%(idx, len(cd_list)), sys.getrefcount(cd_list)
		idx+=1

	cd_list = None
	for i in xrange(len(agents)/2):
		agents.pop()
		print 'del random agent'
		assert(sys.getrefcount(scn) == scn.agent_count+2)
	print 'clear all agents'
	for a in agents:
		scn.del_agent(a)
		assert(sys.getrefcount(scn) == scn.agent_count+2)
	assert(scn.agent_count==0)
	for a in agents:
		assert(a.parent is None)

def test_load():
	filename = "E:\\iTownSDK\\ws\\res\\scene\\test-scene\\test.map"
	map = map_util.load_map(filename)
	scn = pyplt.create_scene(map.bounding_box[0],map.bounding_box[1],
		map.bounding_box[2],map.bounding_box[3],map.min_obj_size[0],map.min_obj_size[1])
	print 'scene:'
	print '\tsize:',scn.size
	print '\tgrid:',scn.grid_size
	print '\tLOD:',scn.lod
	agents=[]
	for lx,ly,ux,uy,t in map.objs:
		x = (ux+lx)*0.5
		y = (uy+ly)*0.5
		w = (ux-lx)*0.5
		h = (uy-ly)*0.5
		a = pyplt.create_agent(x,y,w,h,t,1)
		assert(a.type == t)
		scn.add_agent(a)
		agents.append(a)
	assert(scn.agent_count==len(agents))
	for a in agents:
		scn.del_agent(a)
	assert(scn.agent_count==0)

def test_normal_filter():
	print "testing normal filter..."
	scn_w = 1000
	scn_h = 1000
	scn = pyplt.create_scene(0,0,scn_w,scn_h,20,20)
	a0 = pyplt.create_agent(0,0,10,10,0,1)
	scn.add_agent(a0)
	a1 = pyplt.create_agent(0,0,10,10,0,1)
	r = 88.0*math.pi/180.0
	min_dir = (math.cos(r),math.sin(r))
	r = 92.0*math.pi/180.0
	max_dir = (math.cos(r),math.sin(r))
	a1.pos = (-30,0)
	obj, t, normal = scn.sweep_test_ex(a1,(0,0),1,1,min_dir,max_dir)
	print obj, t, normal
	assert(obj is None)
	a1.pos = ( 30,0)
	obj, t, normal = scn.sweep_test_ex(a1,(0,0),1,1,min_dir,max_dir)
	print obj, t, normal
	assert(obj is None)
	a1.pos = (0, 30)
	obj, t, normal = scn.sweep_test_ex(a1,(0,0),1,1,min_dir,max_dir)
	print obj, t, normal
	assert(obj is not None)
	assert(normal[0]<0.0001)
	assert(abs(normal[1]-1)<0.0001)
	a1.pos = (0,-30)
	obj, t, normal = scn.sweep_test_ex(a1,(0,0),1,1,min_dir,max_dir)
	print obj, t, normal
	assert(obj is None)

if __name__ == "__main__":
	gc.set_debug(gc.DEBUG_LEAK)	
	print 'pyplt', pyplt.version()
	#-------------------------------
	# run test functions
	test_pyplt()
	#test_load()
	test_normal_filter()
	#-------------------------------
	gc.collect()
	if gc.garbage:
		print '-'*79
		print 'Leaks:'
		print gc.garbage
		print '-'*79
		assert(0)
	print '-'*79
	print 'All tests are passed.'

