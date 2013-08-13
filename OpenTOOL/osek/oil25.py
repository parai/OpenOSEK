cCodeHead = \
"""/* Copyright(C) 2013, OpenOSEK by Fan Wang(parai). All rights reserved.
 *
 * This file is part of OpenOSEK.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 * Email: parai@foxmail.com
 * Sourrce Open At: https://github.com/parai/OpenOSEK/
 */
"""
import re, string, os, sys

########################### Common Functions
def bool(isTrue):
	"""isTrue must be the type of string"""
	if(isTrue.upper() == 'TRUE'):
		return True
	else:
		return False

def findItByName(list, name):
    """each obj in list should has the attribute 'name' """
    for obj in list:
        if(obj.name == name):
            return obj # find
    return None # no find

######################### Open OSEK Objectives
# define all the regular expressions to parse a item
# 1: for comment 
re_comment_type1 = re.compile(r'/\*[^/]*\*/');
re_comment_type2 = re.compile(r'//.*');
def filter_out_comment(text):
    """text should be just a line"""
    #过滤形如 “/* .. */” 的注释
    grp = re_comment_type1.split(text)
    result = string.join(grp);
    #过滤形如 “//....” 的注释
    grp = re_comment_type2.split(result);
    result = string.join(grp);
    result = string.join(result.split('\n')) #remove the line break
    return(' '+result);
    
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

    def gen(self, fp):
        fp.write("\n/* ====================== General ======================= */\n")
        fp.write("#define cfgOS_STATUS %s\n"%(self.status))
        fp.write("#define cfgOS_ERRORHOOK %s\n"%(int(self.errorhook)))
        fp.write("#define cfgOS_PRETASKHOOK %s\n"%(int(self.pretaskhook)))
        fp.write("#define cfgOS_POSTTASKHOOK %s\n"%(int(self.errorhook)))
        fp.write("#define cfgOS_SHUTDOWDHOOK %s\n"%(int(self.shutdownhook)))
        fp.write("#define cfgOS_STARTUPHOOK %s\n"%(int(self.startuphook)))

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
        self.id = 0
    
    def gen(self, fp):
        fp.write('\n/* %s configuation */\n'%(self.name))
        fp.write('#define %s %s\n'%(self.name,self.id))
        fp.write('#define %s_priority %s\n'%(self.name,self.priority))
        fp.write('#define %s_activation %s\n'%(self.name,self.activation))
        fp.write('#define %s_stacksize %s\n'%(self.name,self.stacksize))
        fp.write('#define %s_autostart %s\n'%(self.name,str(self.autostart).upper()))
        fp.write('#define %s_schedule %s\n'%(self.name,self.schedule))

    def parse(self,item):
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
        if(re_task_STACKSIZE.search(item)):
            self.stacksize = int(re_task_STACKSIZE.search(item).groups()[0]);
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
class OsCounter():
    def __init__(self):
        self.name = 'unname'
        self.maxallowedvalue = 65575
        self.ticksperbase = 1
        self.mincycle = 1

    def parse(self,item):
        grp = re_oil_os_counter.search(item).groups();
        if(grp[0] != 'COUNTER'):
            return
        name = grp[1];
        if(re_counter_MAXALLOWEDVALUE.search(item)):
            self.maxallowedvalue = int(re_counter_MAXALLOWEDVALUE.search(item).groups()[0]); 
        if(re_counter_TICKSPERBASE.search(item)):
            self.ticksperbase = int(re_counter_TICKSPERBASE.search(item).groups()[0]); 
        if(re_counter_MINCYCLE.search(item)):
            self.mincycle = int(re_counter_MINCYCLE.search(item).groups()[0]); 

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

class OsAlarm():
    def __init__(self):
        self.name = 'unname'
        self.counter = ''
        self.action = ''
        self.task = ''
        self.event = ''
        self.alarmcallbackname = ''
        self.autostart = False
        self.appmodes = []
        self.alarmtime = 0
        self.cycletime = 100

    def parse(self,item):
        grp = re_oil_os_alarm.search(item).groups();
        if(grp[0] != 'ALARM'):
            return
        name = grp[1];
        if(re_alarm_COUNTER.search(item)):
            self.counter = str(re_alarm_COUNTER.search(item).groups()[0]);
        if(re_alarm_ACTION.search(item)):
            action = re_alarm_ACTION.search(item).groups(); 
        if(action[0] == 'ACTIVATETASK'):
            self.action = 'ActivateTask';
        if(re_action_TASK.search(action[1])):
            self.task = re_action_TASK.search(action[1]).groups()[0]
        elif(action[0] == 'SETEVENT'):
            self.action = 'SetEvent';
        if(re_action_TASK.search(action[1])):
            self.task = re_action_TASK.search(action[1]).groups()[0]
        if(re_action_EVENT.search(action[1])):
            self.event = re_action_EVENT.search(action[1]).groups()[0]
        elif(action[0] == 'ALARMCALLBACK'):
            self.action = 'ALARMCALLBACK';
        if(re_action_ALARMCALLBACKNAME.search(action[1])):
            self.alarmcallbackname = re_action_ALARMCALLBACKNAME.search(action[1]).groups()[0]
        if(re_alarm_AUTOSTART.search(item)):
            self.autostart = bool(re_alarm_AUTOSTART.search(item).groups()[0]);
        if(self.autostart == True):
            if(re_alarm_appmode_list.search(item)):
                appmode = re_alarm_appmode_list.search(item).groups()[0];
                for mode in appmode.split(';'):
                    if(re_alarm_APPMODE.search(mode)):
                        modename = re_alarm_APPMODE.search(mode).groups()[0]
                        self.appmodes.append(modename)
        if(re_alarm_ALARMTIME.search(item)):
            self.alarmTime = int(re_alarm_ALARMTIME.search(item).groups()[0])
        if(re_alarm_CYCLETIME.search(item)):
            self.cycleTime = int(re_alarm_CYCLETIME.search(item).groups()[0])

# 6: for resource
re_oil_os_resource = re.compile(r'^\s*(RESOURCE)\s*(\w+)')
re_resource_property = re.compile(r'RESOURCEPROPERTY\s*=\s*(STANDARD|LINKED|INTERNAL)\s*;')
class OsResource():
	def __init__(self):
		self.name = 'unname'
		self.property = 'STANDARD'

	def parse(self,item):
		grp = re_oil_os_resource.search(item).groups();
		if(grp[0] != 'RESOURCE'):
			return
		self.name = grp[1];
		if(re_resource_property.search(item)):
			self.property = re_resource_property.search(item).groups()[0];

# 7: for event
re_oil_os_event = re.compile(r'^\s*(EVENT)\s*(\w+)')
re_event_MASK = re.compile(r'MASK\s*=\s*(\w+)\s*;')
class OsEvent():
    def __init__(self):
        self.name = 'unname'
        self.mask = ''

    def parse(self, item):
        grp = re_oil_os_event.search(item).groups();
        if(grp[0] != 'EVENT'):
            return
        self.name = grp[1];
        if(re_event_MASK.search(item)):
            self.mask = str(re_event_MASK.search(item).groups()[0])

class OsConfig():
    def __init__(self):
        self.general = OsGeneral();
        self.tasks = [];
        self.counters = [];
        self.alarms = [];
        self.resources = [];
        self.internalresoures = [];
        self.events = [];
    
    def processGeneral(self, item):
        self.general.parse(item)
        
    def processTask(self, item):
        #get task name firstly
        grp = re_oil_os_task.search(item).groups();
        if(grp[0] != 'TASK'):
            return
        name = grp[1];
        tsk = findItByName(self.tasks, name)
        if(tsk == None): # no find
            tsk = OsTask()
            tsk.name = name
            self.tasks.append(tsk)
        tsk.parse(item)
    
    def processCounter(self, item):
        grp = re_oil_os_counter.search(item).groups();
        if(grp[0] != 'COUNTER'):
            return
        name = grp[1];
        counter = findItByName(self.counters, name)
        if(counter == None): # no find
            counter = OsCounter()
            counter.name = name
            self.counters.append(counter)
        counter.parse(item)
    
    def processAlarm(self, item):
        grp = re_oil_os_alarm.search(item).groups();
        if(grp[0] != 'ALARM'):
            return
        name = grp[1];
        alm = findItByName(self.alarms, name)
        if(alm == None): # no find
            alm = OsAlarm()
            alm.name = name
            self.alarms.append(alm)
        alm.parse(item)
    
    def processResource(self, item):
        grp = re_oil_os_resource.search(item).groups();
        if(grp[0] != 'RESOURCE'):
            return
        name = grp[1];
        res = findItByName(self.resources, name)
        if(res == None): # no find
            res = OsResource()
            res.name = name
            self.resources.append(res)
        res.parse(item)
    
    def processResource(self, item):
        grp = re_oil_os_event.search(item).groups();
        if(grp[0] != 'EVENT'):
            return
        name = grp[1];
        event= findItByName(self.events, name)
        if(event == None): # no find
            event = OsEvent()
            event.name = name
            self.events.append(event)
        event.parse(item)
        
    def preProcessEach(self, item):
        if(re_oil_os_task.search(item)):
            self.processTask(item);
        elif(re_oil_os_general.search(item)):
            self.processGeneral(item);
        elif(re_oil_os_counter.search(item)):
            self.processCounter(item);
        elif(re_oil_os_alarm.search(item)):
            self.processAlarm(item);
        elif(re_oil_os_resource.search(item)):
            self.processResource(item);
        elif(re_oil_os_event.search(item)):
            self.processEvent(item);
            
    def preProcess(self, oilfile):
        fp = open(oilfile, 'r');
        oneitem = ''; #one item is minimum object such as TASK,ALARM ...
        barcenum = 0; #remember the brace number，when encounter " { ", +1; when " } " -1.
        brace_flag = False; #has " { " encountered or not
        process_one_item_start = False #has match an obj start or not
        for el in fp.readlines():
            #firstly, filter out the comment on this line
            el = filter_out_comment(el);
            if(process_one_item_start == False):
            #{
                oneitem = ''; 
                barcenum = 0;
                brace_flag = False;
                if(re_oil_os_obj.search(el)):
                    process_one_item_start = True;
                    oneitem += el;
                    if(el.count('{') > 0):  #so at comment should not include '{}'
                        brace_flag = True;
                        barcenum += el.count('{');
                    if(el.count('}') > 0):
                        barcenum -= el.count('}');
                    if((brace_flag == True) and (barcenum == 0)): #in one line
                        #filter out the multi-line comment
                        oneitem = filter_out_comment(oneitem)
                        self.preprocessEach(oneitem);
                        process_one_item_start = False
                elif(re_include.search(el)): #include file
                    basep = os.path.dirname(oilfile)
                    file = re_include.search(el).groups()[0];
                    file = basep+'/'+file;
                    self.preprocess(file);
            #}
            else:
            #{
                if(re_include.search(el)): #include file
                    basep = os.path.dirname(oilfile)
                    file = re_include.search(el).groups()[0];
                    file = basep+'/'+file;
                    to_oscfg(file, oscfg);
                    continue;
                if(el.count('{') > 0):  #so at comment should not include '{}'
                    brace_flag = True;
                    barcenum += el.count('{');
                if(el.count('}') > 0):
                    barcenum -= el.count('}');
                oneitem += el;
                if((brace_flag == True) and (barcenum == 0)):
                    #filter out the multi-line comment
                    oneitem = filter_out_comment(oneitem)
                    self.preProcessEach(oneitem);
                    process_one_item_start = False
            #}
        fp.close();

    def postProcess(self):
        id = 0
        for tsk in self.tasks:
            tsk.id = id
            id += 1

    def parse(self, oilfile):
        self.preProcess(oilfile)
        self.postProcess()

    def gen(self, path):
        self.genH(path)
    
    def genH(self, path):
        fp = open('%s/oscfg.h'%(path), 'w')
        fp.write(cCodeHead)
        fp.write('\n#ifndef OSCFG_H_H\n#define OSCFG_H_H\n\n')
        # =================================================
        # 1. General
        self.general.gen(fp)
        # 2. Tasks
        fp.write("\n/* ====================== Tasks ====================== */\n")
        for tsk in self.tasks:
            tsk.gen(fp)
        # =================================================
        fp.write('\n#endif /* OSCFG_H_H */\n\n')
        fp.close()

class OsekConfig():
	def __init__(self):
		self.oscfg = OsConfig();
		
	def parse(self, oilfile):
		self.oscfg.parse(oilfile);

	def gen(self, path):
		self.oscfg.gen(path);
	
class Oil():
    def __init__(self):
        self.osekcfg = OsekConfig();
        
    def parse(self, oilfile):
        self.osekcfg.parse(oilfile);
        
    def gen(self, path):
        self.osekcfg.gen(path);
        print '>>>>>>>>>>>>>>>>> DONE <<<<<<<<<<<<<<<<<<<<<'


