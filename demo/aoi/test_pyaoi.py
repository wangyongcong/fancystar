# encoding utf-8
import gc
import sys
import pyaoi

GROUP_PLAYER = 1
GROUP_MONSTER = 2

class CSceneObject(object):

	def __init__( self, name, size, group, group_filter, callback_enter=None, callback_leave=None ):
		if callback_enter is None:
			callback_enter = self.on_enter
		if callback_leave is None:
			callback_leave = self.on_leave
		self._trigger = pyaoi.create_trigger(name=name, size=size, scene_obj=self, callback_enter=callback_enter, callback_leave=callback_leave, group=group, group_filter=group_filter)

	def destroy(self):
		self._trigger = None

	def join(self, aoi_mgr, x, y=0, z=0):
		aoi_mgr.add(self._trigger,x,y,z)

	def leave(self):
		self._trigger.leave()

	def move_to(self, x, y=0, z=0):
		self._trigger.move_to(x,y,z)

	def on_enter (self, obj, trigger_name, other_obj, other_trigger_name):
		assert(self is obj)
		print "[%s] enter [%s]"%(other_trigger_name,trigger_name)

	def on_leave (self, obj, trigger_name, other_obj, other_trigger_name):
		assert(self is obj)
		print "[%s] leave [%s]"%(other_trigger_name,trigger_name)

def monster_on_player_enter(monster, monster_name, player, player_name):
	print '[%s] enter [%s] scope'%(player_name, monster_name,)

def monster_on_player_leave(monster, monster_name, player, player_name):
	print '[%s] leave [%s] scope'%(player_name, monster_name,)

def fequal(f1,f2):
	return abs(f1-f2)<0.000001

def basic_test():
	print('Testing basic functions...')
	mgr = pyaoi.create_manager(1)
	assert(mgr.dimension==1)
	mgr = pyaoi.create_manager(dimension=2)
	assert(mgr.dimension==2)
	try:
		mgr = pyaoi.create_manager(4)
		assert(0)
	except ValueError:
		pass
	mgr = pyaoi.create_manager()
	assert(mgr.dimension==3)

	trlist = []

	tr = pyaoi.create_trigger(name='T1', size=(1,1,1), callback_enter=monster_on_player_enter, callback_leave=monster_on_player_leave, group=GROUP_PLAYER, group_filter=GROUP_PLAYER)
	trlist.append(tr)
	assert(3==sys.getrefcount(tr)) # 1 hold by tr, 1 hold by trlist, 1 hold by sys.getrefcount()
	mgr.add(tr,1,2,3)
	assert(3==sys.getrefcount(tr)) # manager never hold reference to trigger
	assert(fequal(tr.x,1))
	assert(fequal(tr.y,2))
	assert(fequal(tr.z,3))
	pos = tr.pos
	assert(type(pos) is tuple)
	x, y, z = pos
	assert(fequal(x,1))
	assert(fequal(y,2))
	assert(fequal(z,3))
	r = tr.radius
	assert(type(r) is tuple)
	x, y, z = r
	assert(fequal(x,1))
	assert(fequal(y,1))
	assert(fequal(z,1))
	assert(tr.name=='T1')
	assert(tr.scene_obj==None)
	assert(tr.group==GROUP_PLAYER)
	assert(tr.group_filter==GROUP_PLAYER)

	tr = pyaoi.create_trigger(name='T2', size=(1,1), group=GROUP_PLAYER, group_filter=GROUP_PLAYER)
	trlist.append(tr)
	mgr.add(tr,1,1)
	x,y,z=tr.pos
	assert(fequal(x,1))
	assert(fequal(y,1))
	assert(fequal(z,0))
	x,y,z = tr.radius
	assert(fequal(x,1))
	assert(fequal(y,1))
	assert(fequal(z,0))
	assert(tr.name=='T2')
	assert(tr.scene_obj==None)
	assert(tr.group==GROUP_PLAYER)
	assert(tr.group_filter==GROUP_PLAYER)

	tr = pyaoi.create_trigger(name='T3', size=(1,), group=GROUP_MONSTER, group_filter=GROUP_PLAYER)
	trlist.append(tr)
	mgr.add(tr,1)
	x,y,z=tr.pos
	assert(fequal(x,1))
	assert(fequal(y,0))
	assert(fequal(z,0))
	x,y,z = tr.radius
	assert(fequal(x,1))
	assert(fequal(y,0))
	assert(fequal(z,0))
	assert(tr.name=='T3')
	assert(tr.scene_obj==None)
	assert(tr.group==GROUP_MONSTER)
	assert(tr.group_filter==GROUP_PLAYER)

	tr=None
	tr_count = len(trlist)
	assert(2+tr_count==sys.getrefcount(mgr))
	mgr.remove(trlist[0])
	assert(2+tr_count-1==sys.getrefcount(mgr))
	trlist=[]
	assert(2==sys.getrefcount(mgr))

def walk_test ():
	print 'Testing walking...'
	mgr = pyaoi.create_manager()
	m1 = CSceneObject(name='monster1', size=(2,2,2), group=GROUP_MONSTER, group_filter=GROUP_PLAYER, callback_leave=monster_on_player_leave, callback_enter=monster_on_player_enter)
	m1.join(mgr,0,0,0)
	p1=CSceneObject(name='player1', size=(1,1,1), group=GROUP_PLAYER, group_filter=GROUP_PLAYER)
	p1.join(mgr,-10)
	p2=CSceneObject(name='player2', size=(1,1,1), group=GROUP_PLAYER, group_filter=GROUP_PLAYER)
	p2.join(mgr, 10)
	print 'initially: monster(0,0,0), player1(-10,0,0), player2(10,0,0)'
	print '-'*79
	print 'all players move to (0,0,0)'
	p1.move_to(0,0,0)
	p2.move_to(0,0,0)
	print '-'*79
	print 'player1 move to (10,0,0)'
	p1.move_to( 10)
	print '-'*79
	print 'player2 leave'
	p2.leave()
	print '-'*79
	print 'player2 join at (0,0,0)'
	p2.join(mgr,0,0,0)
	print '-'*79
	print 'player2 move to (-10,0,0), than pass through monster to (10,0,0)'
	p2.move_to(-10,0,0)
	p2.move_to( 10,0,0)
	print '-'*79
	print 'all players move to (0,0,0)'
	p1.move_to(0,0,0)
	p2.move_to(0,0,0)
	print '-'*79
	print 'monster leave'
	m1.leave()

if __name__ == "__main__":
	gc.set_debug(gc.DEBUG_LEAK)	
	basic_test()
	walk_test()
	gc.collect()
	if gc.garbage:
		print '-'*79
		print 'Leaks:'
		print gc.garbage
		print '-'*79
		assert(0)
	print '-'*79
	print 'All tests are passed.'

