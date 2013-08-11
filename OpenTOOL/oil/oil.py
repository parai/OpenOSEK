"""
 Copyright 2012, Fan Wang(Parai). All rights reserved.

 This file is part of OpenOSEK.
 
 This version is just for oil25.
"""
import re, string, os

########################### Common Functions
def bool(isTrue):
	"""isTrue must be the type of string"""
	if(isTrue.upper() == 'TRUE'):
		return True
	else:
		return False
	
def gcfindObj(list, name):
    """Global Common API to find an object in list by name"""
    for obj in list:
        if(name==obj.name):
            return obj;
    return None;

######################### Open OSEK Objectives
# define all the regular expressions to parse a item
# 1: for comment 
re_comment_type1 = re.compile(r'/\*[^/]*\*/');
re_comment_type2 = re.compile(r'//.*');
## include 
re_include = re.compile(r'\s*#include\s+["<]([^\s]+)[">]');

# 2: for os obj
re_oil_os_obj = re.compile(r'^\s*OS|^\s*TASK|^\s*ALARM|^\s*COUNTER|^\s*RESOURCE||^\s*EVENT')

# 3: for os <general>
re_oil_os_general = re.compile(r'^\s*(OS)\s*(\w+)')
re_general_STATUS = re.compile(r'STATUS\s*=\s*(\w+)\s*;')
re_general_ERRORHOOK = re.compile(r'ERRORHOOK\s*=\s*(\w+)\s*;')
re_general_PRETASKHOOK = re.compile(r'PRETASKHOOK\s*=\s*(\w+)\s*;')
re_general_POSTTASKHOOK = re.compile(r'POSTTASKHOOK\s*=\s*(\w+)\s*;')
re_general_SHUTDOWNHOOK = re.compile(r'SHUTDOWNHOOK\s*=\s*(\w+)\s*;')
re_general_STARTUPHOOK = re.compile(r'STARTUPHOOK\s*=\s*(\w+)\s*;')

class OsGeneral():
	def __init__(self):
		self.status = 'STANDARD'
		self.errorhook = False
		self.pretaskhook = False
		self.posttaskhook = False
		self.shutdownhook = False
		self.startuphook = False

	def parse(self,item):
		grp = re_oil_os_general.search(item).groups();
		if(grp[0] != 'OS'):
			return;
		if(re_general_STATUS.search(item)):
			self.status = re_general_STATUS.search(item).groups()[0];
		if(re_general_ERRORHOOK.search(item)):
			self.errorhook = bool(re_general_ERRORHOOK.search(item).groups()[0]);
		if(re_general_PRETASKHOOK.search(item)):
			self.pretaskhook = bool(re_general_PRETASKHOOK.search(item).groups()[0]);
		if(re_general_POSTTASKHOOK.search(item)):
			self.posttaskhook = bool(re_general_POSTTASKHOOK.search(item).groups()[0]);
		if(re_general_SHUTDOWNHOOK.search(item)):
			self.shutdownhook = bool(re_general_SHUTDOWNHOOK.search(item).groups()[0]);
		if(re_general_STARTUPHOOK.search(item)):
			self.startuphook = bool(re_general_STARTUPHOOK.search(item).groups()[0]);

# 4: for task
re_oil_os_task = re.compile(r'^\s*(TASK)\s*(\w+)')
re_task_SCHEDULE = re.compile(r'SCHEDULE\s*=\s*(\w+)\s*;')
re_task_PRIORITY = re.compile(r'PRIORITY\s*=\s*(\w+)\s*;')
re_task_ACTIVATION = re.compile(r'ACTIVATION\s*=\s*(\w+)\s*;')
re_task_AUTOSTART = re.compile(r'AUTOSTART\s*=\s*(\w+)\s*[;{]')
re_task_STACKSIZE = re.compile(r'STACKSIZE\s*=\s*(\w+)\s*;') 
re_task_appmode_list = re.compile(r'AUTOSTART\s*=\s*TRUE\s*{([^{}]*)}\s*;')
re_task_APPMODE = re.compile(r'APPMODE\s*=\s*(\w+)')
re_task_RESOURCE = re.compile(r'RESOURCE\s*=\s*(\w+)')
re_task_EVENT = re.compile(r'EVENT\s*=\s*(\w+)')

class OsTask():
	def __init__(self):
		self.name = 'unname'
		self.schedule = 'FULL'
		self.priority = 0
		self.activation = 1
		self.stacksize = 200
		self.autostart = True
		self.appmodes = []
		self.resources = []
		self.events = []
	def parse(self,item,oscfg):
		grp = re_oil_os_task.search(item).groups();
		if(grp[0] != 'TASK'):
			return
		self.name = grp[1];
		#now start to process it
		if(re_task_SCHEDULE.search(item)):
			self.schedule = re_task_SCHEDULE.search(item).groups()[0];
		if(re_task_PRIORITY.search(item)):
			self.priority = int(re_task_PRIORITY.search(item).groups()[0]);
		if(re_task_ACTIVATION.search(item)):
			self.activation = int(re_task_ACTIVATION.search(item).groups()[0]);
		if(re_task_AUTOSTART.search(item)):
			self.autostart = bool(re_task_AUTOSTART.search(item).groups()[0]);
        if(self.autostart == True):
			if(re_task_appmode_list.search(item)):
                appmode = re_task_appmode_list.search(item).groups()[0];
                for mode in appmode.split(';'):
                    if(re_task_APPMODE.search(mode)):
                        modename = re_task_APPMODE.search(mode).groups()[0]
                        self.appmodes.append(modename)
		if(re_task_StackSize.search(item)):
			self.stacksize = int(re_task_STACKSIZER.search(item).groups()[0]);
		#for resource
		for subitem in item.split(';'): #maybe sereval resource
			if(re_task_RESOURCE.search(subitem)): 
				resname = re_task_RESOURCE.search(subitem).groups()[0]
				self.resources.append(resname)
		# for EVENT
		for subitem in item.split(';'): #maybe sereval event
			if(re_task_EVENT.search(subitem)): 
				name = re_task_EVENT.search(subitem).groups()[0]
				tsk.events.append(name)

# 6: for counter
re_oil_os_counter = re.compile(r'^\s*(COUNTER)\s*(\w+)')
re_counter_MAXALLOWEDVALUE = re.compile(r'MAXALLOWEDVALUE\s*=\s*(\w+)\s*;')
re_counter_TICKSPERBASE = re.compile(r'TICKSPERBASE\s*=\s*(\w+)\s*;')
re_counter_MINCYCLE = re.compile(r'MINCYCLE\s*=\s*(\w+)\s*;')

# 5: for alarm
re_oil_os_alarm = re.compile(r'^\s*(ALARM)\s*(\w+)')
re_alarm_COUNTER = re.compile(r'COUNTER\s*=\s*(\w+)\s*;')
re_alarm_ACTION = re.compile(r'ACTION\s*=\s*(ACTIVATETASK|SETEVENT|ALARMCALLBACK)\s*{([^{}]+)}\s*;')
re_action_TASK = re.compile(r'TASK\s*=\s*(\w+)\s*;')
re_action_EVENT = re.compile(r'EVENT\s*=\s*(\w+)\s*;')
re_action_ALARMCALLBACKNAME = re.compile(r'ALARMCALLBACKNAME\s*=\s*"(\w+)"\s*;')
re_alarm_AUTOSTART = re.compile(r'AUTOSTART\s*=\s*(\w+)\s*[;{]')
re_alarm_appmode_list = re.compile(r'AUTOSTART\s*=\s*TRUE\s*{([^{}]*)}\s*;')
re_alarm_APPMODE = re.compile(r'APPMODE\s*=\s*(\w+)')
re_alarm_ALARMTIME = re.compile(r'ALARMTIME\s*=\s*(\w+)')
re_alarm_CYCLETIME = re.compile(r'CYCLETIME\s*=\s*(\w+)')

# 6: for resource
re_oil_os_resource = re.compile(r'^\s*(RESOURCE)\s*(\w+)')
re_resource_property = re.compile(r'RESOURCEPROPERTY\s*=\s*(STANDARD|LINKED|INTERNAL)\s*;')

# 7: for event
re_oil_os_event = re.compile(r'^\s*(EVENT)\s*(\w+)')
re_event_MASK = re.compile(r'MASK\s*=\s*(\w+)\s*;')

clsaa OsConfig():
	def __init__(self,oilfile):
		self.oilfile = oilfile;
		self.general = OsGeneral();
		self.tasks = [];
		self.counters = [];
		self.alarms = [];
		self.resources = [];
		self.internalresoures = [];
		self.events = [];

	def preprocess(self):
		return

	def postprocess(self):
		return
	
	def parse(self):
		self.preprocess()
		self.postprocess()
		
	def gen(self):
		return
		
		
class OsekConfig():
	def __init__(self,oilfile):
		self.oscfg = OsConfig(oilfile);
		
	def parse(self):
		self.oscfg.parse();

	def gen(self):
		self.osekcfg.gen();
	
class Oil():
	def __init__(self,oilfile):
		self.osekcfg = OsekConfig(oilfile);
		
	def parse(self):
		self.osekcfg.parse();
		
	def gen(self):
		self.osekcfg.gen();
