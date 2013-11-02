cCodeHead = \
"""
/* Copyright(C) 2013, OpenOSEK by Fan Wang(parai). All rights reserved.
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
import sys,os,string,re

# Global Data
global oilcc_I,oilcc_o,oilcc_S,oilcc_target

oilcc_I = ['.']
oilcc_o = '.'
oilcc_S = ''
oilcc_target = ''
# First step information from oil file
oilcc_texts = [] 

# Second step information from oilcc_texts
# each items in the format [(NAME,ATTRIBUTE),[...]]
# For example :[(TASK,Task1),(PRIORITY,10),[(AUTOSTART,TRUE),(APPMODE,AppMode1+AppMode2)...]]
oilcc_items = []

# OSEK OBJECTIVE List
oilcc_osekObjs = []

# special global value for EventHandle
oilcc_eventHandle = 0

# Oil compiler parse rules which will extract a value in group array
oilcc_getIt = { 
    # used to get the include file name
    'include':re.compile(r'\s*#include\s+["<]([^\s]+)[">]'),
    'name':re.compile(r'^\s*(\w+)\s*(\w+)'),
    'attribute':re.compile(r'(\w+)\s*=\s*["]*(\w+)["]*\s*')
}

oilcc_isIt = {
    # used to judge whether it is a start of osek object <TASK, ALARM ....> 
    'osekObj':re.compile(r'^\s*OS|^\s*TASK|^\s*ALARM|^\s*COUNTER|^\s*RESOURCE|^\s*EVENT|^\s*NM|^\s*IPDU'),
}

def pInt(str,base = None):
    if(base == None):
        if(str.find('0x') != -1):
            base = 16
        else:
            base = 10
    return int(str,base)
def GetIt(rule,cstr):
    "Will use the rule to get the correspond value from cstr"
    try:
        return oilcc_getIt[rule].search(cstr).groups()
    except:
        return None

def IsIt(rule,cstr):
    """To judge whether it is what we want"""
    try:
        if(oilcc_isIt[rule].search(cstr)):
            return True
        else:
            return False
    except:
        return False
# =================================  Class  START =========================================
class textToItems():
    """parse from oilcc_texts to OSEK Object, First Step"""
    def __init__(self,texts=oilcc_texts,items=oilcc_items):
        self.items = items
        for text in texts:
            if(IsIt('osekObj',text)):
                self.parse(text)

    def getAttributes(self,text):
        attrGrp = []
        for subtext in re.compile(r'{|;|}').split(text):
            attr = GetIt('attribute',subtext)
            if(attr != None):
                attrGrp.append(attr)
        return attrGrp
    
    def parse(self,text):
        name = GetIt('name',text)
        attributes = self.getAttributes(text)
        self.items.append((name,attributes))

class OeskAttribute():
    def __init__(self,name,value):
        self.name = name
        self.value = value
    def __str__(self):
        return 'ATTRIBUTE: %s = %s'%(self.name,self.value)        
class OeskObject():
    def __init__(self,category,name):
        self.category = category
        self.name = name
        self.attributes = []    # OeskAttribute []
    def addAttribute(self,attrName,value):
        """attrName: the name of the attribute, such as : TASK COUNTER ...."""
        attrName = attrName.upper()
        if(attrName == 'STACKSIZE'):
            attrName = 'STACK'  # special process
        for attr in self.attributes:
            if(attr.name == attrName):
                if(attr.name == 'APPMODE' or attr.name == 'EVENT' or attr.name == 'RESOURCE'):
                    if(attr.value == value):
                        print 'ERROR:attribute %s = %s for %s should has different value!'%(attrName,value,self.name)
                        sys.exit(-1)
                else:
                    print 'ERROR: multiply defined attribute %s for %s!'%(attrName,self.name)
                    sys.exit(-1)
        attr = OeskAttribute(attrName,value)
        self.attributes.append(attr)
    def removeAttribute(self,attrName,value):
        attrName = attrName.upper()
        for attr in self.attributes:
            if(attr.name == attrName and attr.value == value):
                self.attributes.remove(attr)
                break
    def modifyAttribute(self,attrName,value):
        attrName = attrName.upper()
        flag = False
        for attr in self.attributes:
            if(attr.name == attrName):
                attr.value = value
                flag = True
                break
        if(flag == False):
            print "DEBUG: attribute %s isn't existed!"%(attrName)
    def getAttribute(self,attrName):
        attrs = []
        for attr in self.attributes:
            if(attr.name == attrName):
                attrs.append(attr)
        return attrs
    def getValue(self,attrName):
        """For single attributes such as PRIPRITY/AUTOSTART/STACK,must exist"""
        try:
            value =  self.getAttribute(attrName)[0].value
        except:
            if(attrName == 'ALARMTIME' or attrName == 'CYCLETIME'): #default value for alarm
                return '0x00'
            else:
                print 'ERROR: no attribute %s definition for %s'%(attrName,self.name)
                sys.exit(-1)
        return value
    def checkAndParse(self,items):
        for item in items:
            for attr in item[1]:
                self.addAttribute(attr[0], attr[1])
    def __str__(self):
        cstr = '%s %s : {\n'%(self.category,self.name)
        for attr in self.attributes:
            cstr += '\t< %s = %s >;\n'%(attr.name,attr.value)
        cstr += '}'
        return cstr

def GetOsekObjects(category):
    objs = []
    for obj in oilcc_osekObjs:
        if(obj.category == category):
            objs.append(obj) 
    return objs 
    
class OsekOS(OeskObject):
    def __init__(self,oss):
        """os in the format of items, all the oss must has the same name"""
        OeskObject.__init__(self,oss[0][0][0],oss[0][0][1])
        self.checkAndParse(oss)
        self.addAttribute('PRIORITY', hex(0))
        self.addAttribute('TASK_NUM', hex(0))
        self.addAttribute('ALARM_NUM', hex(0))
        self.addAttribute('COUNTER_NUM', hex(0))
        self.addAttribute('EVENT_NUM', hex(0))
        self.addAttribute('STD_RES_NUM', hex(0))
        self.addAttribute('INT_RES_NUM', hex(0))
        self.addAttribute('CC', 'BCC1')
        self.addAttribute('SCHEDULE', 'Full')
        self.addAttribute('ACTIVATION', False)
        self.addAttribute('MULTIPLYPRIORITY', False)
    def postProcess(self):
        """ Resolve the relationship and add some attributes needed by OpenOSEK ,
        also, there is some values should be defined dynamically"""
        # step 1 : from all the objs extract the APPMODE
        for obj in oilcc_osekObjs:
            if(obj == self):
                continue  # skipped
            for APPMODE1 in obj.getAttribute('APPMODE'):
                flag = False
                for APPMODE2 in self.getAttribute('APPMODE'):
                    if(APPMODE1.value == APPMODE2.value):
                        flag = True
                if(flag == False):
                    self.addAttribute(APPMODE1.name, APPMODE1.value)
        # step 2 : parse the max priority needed
        for obj in oilcc_osekObjs:
            if(obj == self):
                continue  # skipped
            for priority in obj.getAttribute('PRIORITY'):
                PRIORITY1 = pInt(priority.value)
                PRIORITY2 = pInt(self.getValue('PRIORITY'))
                if(PRIORITY1 > PRIORITY2):
                    self.modifyAttribute('PRIORITY', hex(PRIORITY1))
        # step 3 : get TASK/ALARM/COUNTER/EVENT/INTERNAL_R/STANDARD_R number
        task = alarm = counter = internal = standard = event = 0
        for obj in oilcc_osekObjs:
            if(obj.category == 'TASK'):
                task += 1
                for ee in obj.getAttribute('EVENT'):
                    event += 1
                    break
            elif(obj.category == 'ALARM'):
                alarm += 1
            elif(obj.category == 'COUNTER'):
                counter += 1
            elif(obj.category == 'RESOURCE'):
                RESOURCEPROPERTY = obj.getValue('RESOURCEPROPERTY')      
                if(RESOURCEPROPERTY == 'STANDARD'):
                    standard += 1
                elif(RESOURCEPROPERTY == 'INTERNAL'):
                    internal += 1
        self.modifyAttribute('TASK_NUM', hex(task))
        self.modifyAttribute('ALARM_NUM', hex(alarm))
        self.modifyAttribute('COUNTER_NUM', hex(counter))
        self.modifyAttribute('EVENT_NUM', hex(event))
        self.modifyAttribute('STD_RES_NUM', hex(standard+1)) # +1 for RES_SCHEDULER
        self.modifyAttribute('INT_RES_NUM', hex(internal)) 
        # step 4: Check only 1 OS object
        if(len(GetOsekObjects('OS'))>1):
            print 'ERROR: more than 1 OS object is not allowed!'
            sys.exit(-1)
        # step 5: resolve OS conformance class
        cc = 'BCC';
        for task in GetOsekObjects('TASK'):
            if(len(task.getAttribute('EVENT'))>0):
                cc = 'ECC'
                break;
        cc2 = cc + '1'
        for task in  GetOsekObjects('TASK'):
            if(pInt(task.getValue('ACTIVATION')) > 1):
                cc2 = cc + '2'
                break;
            for task2 in GetOsekObjects('TASK'):
                if(task != task2 and pInt(task.getValue('PRIORITY')) == pInt(task2.getValue('PRIORITY'))):
                    cc2 = cc + '2'
                    break
        self.modifyAttribute('CC', cc2)
        """ Is Full/Non/Mixed Schedule"""
        isFull = isNon = True
        for task in GetOsekObjects('TASK'):
            if(task.getValue('SCHEDULE') == 'NON'):
                isFull = False
            elif(task.getValue('SCHEDULE') == 'FULL'):
                isNon = False
        if(isFull):
            self.modifyAttribute('SCHEDULE', 'Full')
        elif(isNon):
            self.modifyAttribute('SCHEDULE', 'Non')
        else:
            self.modifyAttribute('SCHEDULE', 'Mixed')
        """ Is Multiply Activation Enabled """
        for task in GetOsekObjects('TASK'):
            if(pInt(task.getValue('ACTIVATION')) > 1):
                self.modifyAttribute('ACTIVATION', True)
        """ Is Multiply Priority Enabled"""
        for task in GetOsekObjects('TASK'):
            for task2 in GetOsekObjects('TASK'):
                if(task != task2):
                    if(pInt(task.getValue('PRIORITY')) == pInt(task2.getValue('PRIORITY'))):
                        self.modifyAttribute('MULTIPLYPRIORITY', True)            
    def getHookValue(self,value):
        if(self.getValue(value) == 'TRUE'):
            return 1
        else:
            return 0
    def genH(self,fp):
        fp.write("\n/* ====================== General ======================= */\n")
        fp.write("#define cfgOS_STATUS %s\n"%(self.getValue('STATUS')))
        fp.write("#define cfgOS_ERRORHOOK %s\n"%(self.getHookValue('ERRORHOOK')))
        fp.write("#define cfgOS_PRETASKHOOK %s\n"%(self.getHookValue('PRETASKHOOK')))
        fp.write("#define cfgOS_POSTTASKHOOK %s\n"%(self.getHookValue('POSTTASKHOOK')))
        fp.write("#define cfgOS_SHUTDOWNHOOK %s\n"%(self.getHookValue('SHUTDOWNHOOK')))
        fp.write("#define cfgOS_STARTUPHOOK %s\n\n"%(self.getHookValue('STARTUPHOOK')))
        fp.write('#define cfgOS_TASK_NUM %s\n'%(self.getValue('TASK_NUM')))
        fp.write('#define cfgOS_FLAG_NUM %s\n'%(self.getValue('EVENT_NUM')))
        fp.write('#define cfgOS_S_RES_NUM %s\n'%(self.getValue('STD_RES_NUM')))
        fp.write('#define cfgOS_I_RES_NUM %s\n'%(self.getValue('INT_RES_NUM')))
        fp.write('#define cfgOS_COUNTER_NUM %s\n'%(self.getValue('COUNTER_NUM')))
        fp.write('#define cfgOS_ALARM_NUM %s\n'%(self.getValue('ALARM_NUM')))
        fp.write('#define cfgOS_MAX_PRIORITY %s\n'%(self.getValue('PRIORITY')))
        fp.write('#define cfgOS_CC %s\n'%(self.getValue('CC')))
        fp.write('#define cfgOS_MULTIPLY_ACTIVATION %s\n'%(int(self.getValue('ACTIVATION'))))
        fp.write('#define cfgOS_MULTIPLY_PRIORITY %s\n'%(int(self.getValue('MULTIPLYPRIORITY'))))
        fp.write('#define cfgOS_SCHEDULE os%sPreemptive\n\n'%(self.getValue('SCHEDULE')))
        fp.write('/* Application Modes */\n')
        offset = 1
        if(len(self.getAttribute('APPMODE')) > 32):
            print 'ERROR: Only 32 APPMODE are allowed but %s modes has been configured!'%(len(appmodes))
        for attr in self.getAttribute('APPMODE'):
            if(attr.value != 'OSDEFAULTAPPMODE'):
                fp.write('#define %s %s\n'%(attr.value,hex(1<<offset)))
                offset += 1
         
class OsekTask(OeskObject):
    def __init__(self,tasks):
        """tasks in the format of items, all the tasks must has the same name"""
        OeskObject.__init__(self,tasks[0][0][0],tasks[0][0][1])
        self.checkAndParse(tasks)
        self.addAttribute('RPRIORITY', self.getValue('PRIORITY'))
    def postProcess(self):
        """ Resolve the relationship and add some attributes needed by OpenOSEK ,
        also, there is some values should be defined dynamically"""
        # step 1: check each task can only has on internal resource
        inr = 0
        inResName = ''
        for attr1 in self.getAttribute('RESOURCE'):   
            for attr2 in GetOsekObjects('RESOURCE'):
                if(attr1.value == attr2.name and attr2.getValue('RESOURCEPROPERTY') == 'INTERNAL'):
                    inResName = attr2.name
                    inr += 1
        if(inr > 1):
            print 'ERROR:only one internal resource should be assigned to task <%s>!'%(self.name)
            sys.exit(-1)
        # step 2: parse task RUNNING-PRIORITY
        if(self.getValue('SCHEDULE') == 'NON'):
            self.modifyAttribute('RPRIORITY', GetOsekObjects('OS')[0].getValue('PRIORITY'))
        elif(inr == 1):
            # see step 1 : inResName
            for task in GetOsekObjects('TASK'):
                if(self == task):
                    continue
                for attr in task.getAttribute('RESOURCE'):
                    if(attr.value == inResName):
                        if(pInt(self.getValue('RPRIORITY')) < pInt(task.getValue('PRIORITY'))):
                            self.modifyAttribute('RPRIORITY', task.getValue('PRIORITY'))
        # step 3 : check Event of Task is valid
        for attr in self.getAttribute('EVENT'):
            flag = False
            for event in GetOsekObjects('EVENT'):
                if(attr.value == event.name):
                    flag = True
            if(flag == False):
                print "ERROR:No Event <%s> definition for Task <%s>!"%(attr.value,self.name)
        # step 5 : resolve EVENT Mask which has the value 'AUTO'
        for attr in self.getAttribute('EVENT'):
            for event in GetOsekObjects('EVENT'): # Get One Event of Task
                if(attr.value != event.name):
                    continue
                MASK = event.getValue('MASK')
                if(MASK == 'AUTO'):
                    for i in range(0,32):
                        flag = True
                        for attr2 in self.getAttribute('EVENT'):
                            for event2 in GetOsekObjects('EVENT'):
                                if(attr2.value != event2.name):
                                    continue
                                if(event != event2): # Get another one Event of Task
                                    MASK2 = event2.getValue('MASK')
                                    if(MASK2 != 'AUTO' and pInt(MASK2) == (1<<i)):
                                        flag = False
                        if(flag == True):
                            event.modifyAttribute('MASK',hex(1<<i))
                            break
                if(event.getValue('MASK') == 'AUTO'):
                    print 'ERROR: Too much Event for Task <%s>!'%(self.name)
                    sys.exit(-1)
        # step 6: check for AUTOSTART:
        if(self.getValue('AUTOSTART') == 'TRUE' and len(self.getAttribute('APPMODE')) == 0):
            print 'ERROR: No APPMODE for Task <%s> as Task is auto-start!'%(self.name)
            sys.exit(-1)
        elif(self.getValue('AUTOSTART') == 'FALSE' and len(self.getAttribute('APPMODE')) > 0):
            print 'ERROR: Has APPMODE for Task <%s> as Task is not auto-start!'%(self.name)
            sys.exit(-1)
        # if has Event,Activation must be 1
        for event in self.getAttribute('EVENT'):
            if(pInt(self.getValue('ACTIVATION')) != 1):
                print 'ERROR: Task %s has EVENTs, ACTIVATION must be 1.'%(self.name)
                sys.exit(-1);
        if(pInt(self.getValue('ACTIVATION')) == 0):
            print 'ERROR: Task %s\'s ACTIVATION must be bigger than 0.'%(self.name)
            sys.exit(-1)
    def genH(self,fp):
        global oilcc_eventHandle
        fp.write('\n\n/* %s configuation */\n'%(self.name))
        id = 0
        for task in GetOsekObjects('TASK'):
            if(task == self):
                break
            else:
                id += 1
        fp.write('#define %s %s\n'%(self.name,id))
        fp.write('#define %s_ipriority %s\n'%(self.name,self.getValue('PRIORITY')))
        fp.write('#define %s_rpriority %s\n'%(self.name,self.getValue('RPRIORITY')))
        cstr = '\t\t\t/* ['
        for attr in self.getAttribute('RESOURCE'):
            cstr += '%s,'%(attr.value)
        cstr += '] */\n'
        fp.write(cstr)
        fp.write('#define %s_activation %s\n'%(self.name,self.getValue('ACTIVATION')))
        fp.write('#define %s_stacksize %s\n'%(self.name,self.getValue('STACK')))
        fp.write('#define %s_schedule %s\n'%(self.name,self.getValue('SCHEDULE')))
        fp.write('#define %s_autostart %s\n'%(self.name,self.getValue('AUTOSTART')))
        mode = 'INVALID_APPMODE'
        for attr in self.getAttribute('APPMODE'):
            mode += ' | %s'%(attr.value);
        fp.write('#define %s_appmode (%s)\n'%(self.name,mode));
        if(len(self.getAttribute('EVENT')) > 0):
            fp.write('#define %s_eventhandle %s\n'%(self.name,oilcc_eventHandle))
            oilcc_eventHandle += 1
            fp.write('\t\t\t/* [')
            for attr in self.getAttribute('EVENT'):
                fp.write('%s,'%(attr.value))
            fp.write('] */\n')
        else:
            fp.write('#define %s_eventhandle INVALID_FLAG\n'%(self.name))
class OsekCounter(OeskObject):
    def __init__(self,counters):
        """counters in the format of items, all the counters must has the same name"""
        OeskObject.__init__(self,counters[0][0][0],counters[0][0][1])
        self.checkAndParse(counters) 
    def postProcess(self):
        """ Resolve the relationship and add some attributes needed by OpenOSEK ,
        also, there is some values should be defined dynamically"""
        flag = False
        for alarm in GetOsekObjects('ALARM'):
            if(alarm.getValue('COUNTER') == self.name):
                flag = True
        if(flag == False):
            print 'WARNING: Counter %s isn\'t referred by any alarm.'%(self.name)  
        if(pInt(self.getValue('MAXALLOWEDVALUE')) > (0xFFFFFFFF-1)/2):
            print 'ERROR: counter %s max allowed value cann\'t be bigger than %s'%(self.name,hex((0xFFFFFFFF-1)/2).upper())
            sys.exit(-1)   
    def genH(self,fp):
        id = 0
        for counter in GetOsekObjects('COUNTER'):
            if(self == counter):
                break
            else:
                id += 1
        fp.write('\n\n#define %s %s\n'%(self.name,id))
        fp.write('#define %s_maxallowedvalue %s\n'%(self.name,self.getValue('MAXALLOWEDVALUE')))
        fp.write('#define %s_ticksperbase %s\n'%(self.name,self.getValue('TICKSPERBASE')))
        fp.write('#define %s_mincycle %s\n'%(self.name,self.getValue('MINCYCLE')))           
class OsekAlarm(OeskObject):
    def __init__(self,alarms):
        """alarms in the format of items, all the alarms must has the same name"""
        OeskObject.__init__(self,alarms[0][0][0],alarms[0][0][1])
        self.checkAndParse(alarms)
    def postProcess(self):
        """ Resolve the relationship and add some attributes needed by OpenOSEK ,
        also, there is some values should be defined dynamically"""
        flag = False
        for counter in GetOsekObjects('COUNTER'):
            if(self.getValue('COUNTER') == counter.name):
                flag = True
        if(flag == False):
            print 'ERROR: Counter <%s> isn\'t defined for Alarm <%s>!'%(self.getValue('COUNTER'),self.name)       
            sys.exit(-1)
        # step 1: check for AUTOSTART:
        if(self.getValue('AUTOSTART') == 'TRUE' and len(self.getAttribute('APPMODE')) == 0):
            print 'ERROR: No APPMODE for Alarm <%s> as Task is auto-start!'%(self.name)
            sys.exit(-1)
        elif(self.getValue('AUTOSTART') == 'FALSE' and len(self.getAttribute('APPMODE')) > 0):
            print 'ERROR: Has APPMODE for Task <%s> as Alarm is not auto-start!'%(self.name)
            sys.exit(-1) 
    def genH(self,fp):
        id = 0
        for alarm in GetOsekObjects('ALARM'):
            if(self == alarm):
                break
            else:
                id += 1
        fp.write('\n\n#define %s %s\n'%(self.name,id))
        fp.write('#define %s_counter %s\n'%(self.name,self.getValue('COUNTER')))
        fp.write('#define %s_time %s\n'%(self.name,self.getValue('ALARMTIME')))
        fp.write('#define %s_cycle %s\n'%(self.name,self.getValue('CYCLETIME')))
        mode = 'INVALID_APPMODE'
        for attr in self.getAttribute('APPMODE'):
            mode += ' | %s'%(attr.value);
        fp.write('#define %s_appmode (%s)\n'%(self.name,mode));
        fp.write('#define %s_Action %s\n'%(self.name,self.getValue('ACTION')))
        if(self.getValue('ACTION') == 'ACTIVATETASK'):
            fp.write('#define %s_Task %s\n'%(self.name,self.getValue('TASK')))
        elif(self.getValue('ACTION') == 'SETEVENT'):
            fp.write('#define %s_Task %s\n'%(self.name,self.getValue('TASK')))
            fp.write('#define %s_Event %s\n'%(self.name,self.getValue('EVENT')))
        else:
            fp.write('#define %s_Cbk %s\n'%(self.name,self.getValue('ALARMCALLBACKNAME')))
class OsekEvent(OeskObject):           
    def __init__(self,events):
        """events in the format of items, all the events must has the same name"""
        OeskObject.__init__(self,events[0][0][0],events[0][0][1])
        self.checkAndParse(events)
    def postProcess(self):
        """ Resolve the relationship and add some attributes needed by OpenOSEK ,
        also, there is some values should be defined dynamically"""
        flag = False;tt = [];
        # step 1: check that event is referred by Task    
        for task in GetOsekObjects('TASK'):
            for attr in task.getAttribute('EVENT'):
                if(self.name == attr.value):
                    flag = True
                    tt.append(task) # Task has this EVENT
        if(flag == False):
            print 'WARNING: %s has not been referred by any task.'%(self.name)
        elif(len(tt) > 1):
            print 'ERROR: EVENT <%s> is not allowed to be defined for %s, %s ...'%(self.name, tt[0].name, tt[1].name)
            sys.exit(-1)
        else:
            self.addAttribute('TASK', tt[0].name)
    def genH(self,fp):
        fp.write('\n#define %s %s\t\t/* Of %s */\n'%(self.name,self.getValue('MASK'),self.getValue('TASK')))
class OsekResource(OeskObject):           
    def __init__(self,resources):
        """resources in the format of items, all the resources must has the same name"""
        OeskObject.__init__(self,resources[0][0][0],resources[0][0][1])
        self.checkAndParse(resources)
        self.addAttribute('PRIORITY', hex(0))
    def postProcess(self):
        """ Resolve the relationship and add some attributes needed by OpenOSEK ,
        also, there is some values should be defined dynamically"""
        flag = False
        for task in GetOsekObjects('TASK'):
            for attr1 in task.getAttribute('RESOURCE'):
                if(attr1.value == self.name):
                    flag = True
                    if(pInt(task.getValue('PRIORITY')) > pInt(self.getValue('PRIORITY'))):
                        self.modifyAttribute('PRIORITY', task.getValue('PRIORITY'))
        if(flag == False):
            print 'WARNING: %s hasn\'t been assigned to any task.'%(self.name)
    def genH(self,fp):
        id = 0
        for res in GetOsekObjects('RESOURCE'):
            if(res == self):
                break
            elif(res.getValue('RESOURCEPROPERTY') == self.getAttribute('RESOURCEPROPERTY')):
                id += 1
        fp.write('\n\n#define %s %s /* property = %s */\n'%(self.name,id,self.getValue('RESOURCEPROPERTY')))
        fp.write('#define %s_priority %s\n'%(self.name,self.getValue('PRIORITY')))

class OsekNM(OeskObject):           
    def __init__(self,NMs):
        OeskObject.__init__(self,NMs[0][0][0],NMs[0][0][1])
        self.checkAndParse(NMs) 
    def postProcess(self):
        return
    def genH(self,fp):
        """fp must be a comcfg.h file"""
        id = 0
        for nm in GetOsekObjects('NM'):
            if(self == nm):
                break
            else:
                id += 1
        fp.write('\n#define %s %s\n'%(self.name,id))
        fp.write('#define %s_TYPE NM_%s\n'%(self.name,self.getValue('TYPE')))
        fp.write('#define %s_tTyp %s\n'%(self.name,self.getValue('TTYP')))
        fp.write('#define %s_tMax %s\n'%(self.name,self.getValue('TMAX')))
        fp.write('#define %s_tError %s\n'%(self.name,self.getValue('TERROR')))
        fp.write('#define %s_tTx %s\n'%(self.name,self.getValue('TTX')))
        fp.write('#define %s_IDBASE %s\n'%(self.name,self.getValue('IDBASE')))
        fp.write('#define %s_WINDOWMASK %s\n'%(self.name,self.getValue('WINDOWMASK')))
        fp.write('#define %s_CONTROLLER %s\n'%(self.name,self.getValue('CONTROLLER')))

class OsekIPDU(OeskObject):           
    def __init__(self,IPDUs):
        OeskObject.__init__(self,IPDUs[0][0][0],IPDUs[0][0][1])
        self.checkAndParse(IPDUs) 
    def postProcess(self):
        """ Rule: When sizeInBits > 64, It must has the suffix "_RX" or "_TX"
          and It must has a partner in name,For Example:
            UDSDiag_RX and UDSDiag_TX."""
        length = pInt(self.getValue('SIZEINBITS'))
        if(length > 64):
            if(   (self.getValue('IPDUPROPERTY') == 'RECEIVED' and self.name[-3:] != '_RX') 
               or (self.getValue('IPDUPROPERTY') == 'SENT'     and self.name[-3:] != '_TX')  ):
                print 'ERROR: IPDU <%s> SizeInBit > 64, So its name must with the suffix "_RX" for RECEIVED and "_TX" for SENT.'%(self.name)
                sys.exit(-1)
            else:
                # check that it has a partner
                flag = False
                if(self.getValue('IPDUPROPERTY') == 'RECEIVED'):
                    for pdu in GetOsekObjects('IPDU'):
                        if(pdu != self and pdu.getValue('IPDUPROPERTY') == 'SENT' and pdu.name[:-3] == self.name[:-3]):
                            flag = True
                elif(self.getValue('IPDUPROPERTY') == 'SENT'):
                    for pdu in GetOsekObjects('IPDU'):
                        if(pdu != self and pdu.getValue('IPDUPROPERTY') == 'RECEIVED' and pdu.name[:-3] == self.name[:-3]):
                            flag = True  
                else:
                    print 'ERROR:IPDU <%s> invalid IPDUPROPERTY %s.'%(self.name,self.getValue('IPDUPROPERTY')) 
                    sys.exit(-1) 
                if(flag == False):
                    if(self.getValue('IPDUPROPERTY') == 'RECEIVED'):
                        print 'ERROR:IPDU <%s> has no partner with the name "%s_TX"'%(self.name,self.name[:-3])  
                    else:
                        print 'ERROR:IPDU <%s> has no partner with the name "%s_RX"'%(self.name,self.name[:-3])
    def genH(self,fp):  
        id = 0
        for nm in GetOsekObjects('IPDU'):
            if(self == nm):
                break
            else:
                if(self.getValue('IPDUPROPERTY') == nm.getValue('IPDUPROPERTY')):
                    id += 1
        fp.write('\n#define %s %s\n'%(self.name,id))
        fp.write('#define %s_SIZEINBITS %s\n'%(self.name,self.getValue('SIZEINBITS')))
        fp.write('#define %s_IPDUPROPERTY IPDU_%s\n'%(self.name,self.getValue('IPDUPROPERTY')))
        fp.write('#define %s_ID %s\n'%(self.name,self.getValue('ID')))
        CONTROLLER = ''
        for nm in GetOsekObjects('NM'):
            if(self.getValue('LAYERUSED') == nm.name):
                CONTROLLER = nm.getValue('CONTROLLER')
                break
        fp.write('#define %s_LAYERUSED %s\t/* %s */\n'%(self.name,CONTROLLER,self.getValue('LAYERUSED')))
     
OsekObjDict={
    'OS':OsekOS,
    'TASK':OsekTask,
    'COUNTER':OsekCounter,
    'ALARM':OsekAlarm,
    'EVENT':OsekEvent,
    'RESOURCE':OsekResource,
    'NM':OsekNM,
    'IPDU':OsekIPDU
}  
          
class itemsToOsekObj():
    def __init__(self,items=oilcc_items,osekObjs=oilcc_osekObjs): 
        self.osekObjs = osekObjs
        copyOfItems = []
        for it in items:
            copyOfItems.append(it)
        for it1 in copyOfItems:
            ites = []
            ites.append(it1)
            for it2 in copyOfItems:
                if(it1 != it2):
                    if(it1[0] == it2[0]): # equal Name
                        ites.append(it2)
                        copyOfItems.remove(it2)
            self.osekObjs.append(OsekObjDict[ites[0][0][0]](ites))    
# =================================  Class  END   =========================================    
def DropComment(text):
    """text should be just a line"""
    grp = re.compile(r'/\*[^/]*\*/').split(text)
    result = string.join(grp);
    grp = re.compile(r'//.*').split(result);
    result = string.join(grp);
    #result = string.join(result.split('\n')) #remove the line break
    return(' '+result);

def PrepareCompile(file):
    """Parse Oil file, item view get."""
    global oilcc_I,oilcc_o,oilcc_S,oilcc_target
    fp = open(file,'r')
    # some flags
    item = ''; #one item is minimum object such as TASK,ALARM ...
    barcenum = 0;
    flag = False; #has " { " encountered or not
    start = False #has match an obj start or not
    for line in fp.readlines():
        #firstly, filter out the comment on this line
        el = DropComment(line);
        if(start == False):
        #{
            item = ''; 
            barcenum = 0;
            flag = False;
            if(IsIt('osekObj',el)):
                start = True;
                item += el;
                if(el.count('{') > 0):  #so at comment should not include '{}'
                    flag = True;
                    barcenum += el.count('{');
                if(el.count('}') > 0):
                    barcenum -= el.count('}');
                if((flag == True) and (barcenum == 0)): #in one line
                    #filter out the multi-line comment
                    item = DropComment(item)
                    oilcc_texts.append(item);
                    start = False
            else: # special process for include
                inc = GetIt('include',el)
                if(inc != None): #include file
                    flag_inc = False
                    for I in oilcc_I:
                        finc = I + '/' + inc[0]
                        if(os.path.exists(finc)):
                            print 'INFO:parse include file <%s> in the path <%s>'%(inc[0],I)
                            PrepareCompile(finc);
                            flag_inc = True;
                    if(flag_inc == False):
                        print 'ERROR:cann\'t find out the file %s!'%(inc[0])
                        sys.exit(-1)
        #}
        else:
        #{
            if(el.count('{') > 0):  #so at comment should not include '{}'
                flag = True;
                barcenum += el.count('{');
            if(el.count('}') > 0):
                barcenum -= el.count('}');
            item += el;
            if((flag == True) and (barcenum == 0)):
                #filter out the multi-line comment
                item = DropComment(item)
                oilcc_texts.append(item);
                start = False
        #}
    fp.close()

def  osekComObjPostProcess():
    """ Move IPDU(size in bits > 64) to ahead."""
    for pdu in GetOsekObjects('IPDU'):
        if(pInt(pdu.getValue('SIZEINBITS')) > 64):
            if pdu.getValue('IPDUPROPERTY') == 'RECEIVED':
                for pdu2 in GetOsekObjects('IPDU'):
                    if(pdu != pdu2 and pdu.name[:-3] == pdu2.name[:-3]):
                        #move both pdu and pdu2 to head
                        oilcc_osekObjs.remove(pdu)
                        oilcc_osekObjs.remove(pdu2)
                        oilcc_osekObjs.insert(0,pdu)
                        oilcc_osekObjs.insert(0,pdu2)
            
def osekObjPostProcess():
    # special process for Counter
    flag = False
    for counter in GetOsekObjects('COUNTER'):
        if(counter.name == 'SystemTimer'):
            flag = True;
    if(flag == False):
        print 'WARNING:No OpenOSEK default counter <SystenTimer> configured,OILCC will add it with default value.'
        counter = OsekCounter([[('COUNTER','SystemTimer'),[('MAXALLOWEDVALUE','0x7FFFFFFF'),('TICKSPERBASE','1'),('MINCYCLE',1),('TYPE','SOFTWARE')]]])
        oilcc_osekObjs.insert(0,counter)
    else:
        oilcc_osekObjs.remove(counter)
        oilcc_osekObjs.insert(0,counter) 
    GetOsekObjects('OS')[0].postProcess()  # Do OS firstly
    for obj in oilcc_osekObjs:
        obj.postProcess()
    osekComObjPostProcess()
def GenerateOsCfgH():
    global oilcc_o
    fp = open('%s/oscfg.h'%(oilcc_o),'w')
    fp.write(cCodeHead)
    fp.write('\n#ifndef OSCFG_H_H\n#define OSCFG_H_H\n\n')
    # 1 : OS General
    GetOsekObjects('OS')[0].genH(fp)
    # 2 : TASKS
    for task in GetOsekObjects('TASK'):
        task.genH(fp)
    # 3 : COUNTERS
    for counter in GetOsekObjects('COUNTER'):
        counter.genH(fp)
    # 4 : ALARMS
    for alarm in GetOsekObjects('ALARM'):
        alarm.genH(fp)
    # 5 : STANDARD RESOURCE
    for resource in GetOsekObjects('RESOURCE'):
        if(resource.getValue('RESOURCEPROPERTY') == 'STANDARD'):
            resource.genH(fp)
    # 6 : INTERNAL RESOURCE
    for resource in GetOsekObjects('RESOURCE'):
        if(resource.getValue('RESOURCEPROPERTY') == 'INTERNAL'):
            resource.genH(fp)
    # 7 : EVENTS
    for event in GetOsekObjects('EVENT'):
        event.genH(fp)       
    fp.write('\n#endif /* OSCFG_H_H */\n\n')
    fp.close()
class GenerateOsCfgC():
    def __init__(self):
        global oilcc_o
        fp = open('%s/oscfg.c'%(oilcc_o),'w')
        fp.write(cCodeHead)
        fp.write('#include "osek_os.h"\n')
        fp.write('\n#ifdef SIMULATE_ON_WIN\n#define SetEvent osekSetEvent\n#endif\n')
        # Gen Tasks
        self.genTasks(fp);
        # Gen Counters
        self.genCountersBaseInfo(fp)
        # Gen Alarms
        self.genAlarms(fp)
        # Gen Resources
        self.genResources(fp)
        fp.close()
    def getPrioSize(self, priority):
        size = 0;
        for tsk in GetOsekObjects('TASK'):
            if(pInt(tsk.getValue('PRIORITY')) == priority):
                size += pInt(tsk.getValue('ACTIVATION'));
        for res in GetOsekObjects('RESOURCE'):
            if(pInt(res.getValue('PRIORITY')) == priority):
                flag = False;
                for tsk in GetOsekObjects('TASK'):
                    for attr in tsk.getAttribute('RESOURCE'):
                        if(res.name == attr.value):
                            flag = True;
                if(flag == True):
                    size += 1;
                    break;
        return size;
            
    def genTaskReadyQueue(self, fp):
        cstr = 'EXPORT RDYQUE knl_rdyque = \n{\n';
        cstr += '\t/* top_pri= */ NUM_PRI,\n'
        cstr +='\t{/* tskque[] */\n'
        max_prio = pInt(GetOsekObjects('OS')[0].getValue('PRIORITY')) 
        for i in range(0, max_prio+1):
            size = self.getPrioSize(i);
            if(size != 0):
                fp.write('LOCAL TaskType knl_%s_queue[%s];\n'%(i, size+1))
                cstr += '\t\t{/* head= */ 0,/* tail= */ 0,/* length= */ %s, /* queue= */ knl_%s_queue},\n'%(size+1, i);
            else:
                cstr += '\t\t{/* head= */ 0,/* tail= */ 0,/* length= */ 0, /* queue= */ NULL},\n';
        cstr +='\t},\n'
        cstr +='\t/* null */{/* head= */ 0,/* tail= */ 0,/* length= */ 0, /* queue= */ NULL},\n'
        cstr +='};\n\n'
        fp.write(cstr);
    
    def genTasks(self, fp):
        fp.write("\n/* ====================== Tasks ====================== */\n")
        # --------------- task pc
        for tsk in GetOsekObjects('TASK'):
            fp.write('IMPORT TASK(%s);\n'%(tsk.name))
        cstr = '\nEXPORT const FP knl_tcb_pc[] = \n{\n'
        for tsk in GetOsekObjects('TASK'):
            cstr += '\tTASK_PC(%s),\n'%(tsk.name)
        cstr += '};\n\n'
        fp.write(cstr);
        # -------------- task init-priority
        cstr = 'EXPORT const PriorityType knl_tcb_ipriority[] = \n{\n'
        for tsk in GetOsekObjects('TASK'):
            cstr += '\t%s_ipriority,\n'%(tsk.name)
        cstr += '};\n\n'
        fp.write(cstr);
        # -------------- task run-priority
        if( GetOsekObjects('OS')[0].getValue('SCHEDULE') != 'Non' 
            or pInt(GetOsekObjects('OS')[0].getValue('EVENT_NUM')) != 0):
            cstr = 'EXPORT const PriorityType knl_tcb_rpriority[] = \n{\n'
            for tsk in GetOsekObjects('TASK'):
                cstr += '\t%s_rpriority,\n'%(tsk.name)
            cstr += '};\n\n'
            fp.write(cstr);
        if(GetOsekObjects('OS')[0].getValue('SCHEDULE') != 'Non' or
           len(GetOsekObjects('EVENT')) != 0):
            # -------------- task stack buffer
            for tsk in GetOsekObjects('TASK'):
                fp.write('LOCAL uint8 knl_%s_stack[%s_stacksize];\n'%(tsk.name, tsk.name))
            # -------------- task stacksize list
            cstr = 'EXPORT const StackSizeType knl_tcb_stksz[] = \n{\n'
            for tsk in GetOsekObjects('TASK'):
                cstr += '\t%s_stacksize,\n'%(tsk.name)
            cstr += '};\n\n'
            fp.write(cstr);
            # -------------- task stackbuffer list
            cstr = 'EXPORT uint8* const knl_tcb_stack[] = \n{\n'
            for tsk in GetOsekObjects('TASK'):
                cstr += '\t(knl_%s_stack+%s_stacksize),\n'%(tsk.name, tsk.name)
            cstr += '};\n\n'
            fp.write(cstr);
        # -------------- task activation
        if GetOsekObjects('OS')[0].getValue('ACTIVATION') is True:
            cstr = 'EXPORT const uint8 knl_tcb_max_activation[] = \n{\n'
            for tsk in GetOsekObjects('TASK'):
                cstr += '\t(%s_activation - 1),\n'%(tsk.name)
            cstr += '};\n\n'
            fp.write(cstr);
        # -------------- task event handle
        if(len(GetOsekObjects('EVENT')) > 0):
            cstr = 'EXPORT const uint8 knl_tcb_flgid[] = \n{\n'
            for tsk in GetOsekObjects('TASK'):
                cstr += '\t%s_eventhandle,\n'%(tsk.name)
            cstr += '};\n\n'
            fp.write(cstr);
        # -------------- task mode
        cstr = 'EXPORT const AppModeType knl_tcb_mode[] = \n{\n'
        for tsk in GetOsekObjects('TASK'):
            cstr += '\t%s_appmode,\n'%(tsk.name)
        cstr += '};\n\n'
        fp.write(cstr);
        if( GetOsekObjects('OS')[0].getValue('ACTIVATION') == True or
           GetOsekObjects('OS')[0].getValue('MULTIPLYPRIORITY') == True):
            fp.write('\n/* ====================== Task Ready Queue ====================== */\n')
            self.genTaskReadyQueue(fp);
    
    def genCountersBaseInfo(self, fp):
        if(len(GetOsekObjects('COUNTER')) == 0):
            return
        fp.write("\n/* ====================== Counters ====================== */\n")
        cstr = 'EXPORT const TickType knl_ccb_max[] = \n{\n';
        for cnt in GetOsekObjects('COUNTER'):
            cstr += '\t%s_maxallowedvalue,\n'%(cnt.name)
        cstr += '};\n'
        fp.write(cstr);
        cstr = 'EXPORT const TickType knl_ccb_tpb[] = \n{\n';
        for cnt in GetOsekObjects('COUNTER'):
            cstr += '\t%s_ticksperbase,\n'%(cnt.name)
        cstr += '};\n'
        fp.write(cstr);
        cstr = 'EXPORT const TickType knl_ccb_min[] = \n{\n';
        for cnt in GetOsekObjects('COUNTER'):
            cstr += '\t%s_mincycle,\n'%(cnt.name)
        cstr += '};\n'
        fp.write(cstr);
        
    def genAlarms(self, fp):
        if(len(GetOsekObjects('ALARM')) == 0):
            return
        fp.write("\n/* ====================== Alarms ====================== */\n")
        cstr = 'EXPORT const CounterType knl_acb_counter[] = \n{\n';
        for alm in GetOsekObjects('ALARM'):
            cstr += '\t%s_counter,\n'%(alm.name)
        cstr += '};\n'
        fp.write(cstr);
        cstr = 'EXPORT const TickType knl_acb_time[] = \n{\n';
        for alm in GetOsekObjects('ALARM'):
            cstr += '\t%s_time,\n'%(alm.name)
        cstr += '};\n'
        fp.write(cstr);
        cstr = 'EXPORT const TickType knl_acb_cycle[] = \n{\n';
        for alm in GetOsekObjects('ALARM'):
            cstr += '\t%s_cycle,\n'%(alm.name)
        cstr += '};\n'
        fp.write(cstr);
        cstr = 'EXPORT const CounterType knl_acb_mode[] = \n{\n';
        for alm in GetOsekObjects('ALARM'):
            cstr += '\t%s_appmode,\n'%(alm.name)
        cstr += '};\n'
        fp.write(cstr);
        cstr = ''
        for alm in GetOsekObjects('ALARM'):
            if(alm.getValue('ACTION') == 'SETEVENT'):
                cstr += 'LOCAL void AlarmMain%s(void)\n{\n'%(alm.name)
                cstr += '\t(void)SetEvent(%s,%s);\n'%(alm.getValue('TASK'), alm.getValue('EVENT'))
                cstr += '}\n'
            elif(alm.getValue('ACTION') == 'ACTIVATETASK'):
                cstr += 'LOCAL void AlarmMain%s(void)\n{\n'%(alm.name)
                cstr += '\t(void)ActivateTask(%s);\n'%(alm.getValue('TASK'))
                cstr += '}\n'
            else: #if(alm.action == 'ALARMCALLBACK'):
                cstr += 'IMPORT void AlarmMain%s(void);\n'%(alm.name)
        fp.write(cstr)
        cstr = 'EXPORT const FP knl_acb_action[] = \n{\n'
        for alm in GetOsekObjects('ALARM'):
            if(alm.getValue('ACTION') != 'ALARMCALLBACK'):
                cstr += '\tAlarmMain%s,\n'%(alm.name)
            else:
                cstr += '\t%s_Cbk,\n'%(alm.name)
        cstr += '};\n'
        fp.write(cstr);
            
    def genResources(self, fp):
        for res in GetOsekObjects('RESOURCE'):
            if(res.name == 'RES_SCHEDULER'):
                print "WARNING:RES_SCHEDULER shouldn't be configured."
                oilcc_osekObjs.remove(res)
        fp.write("\n/* ====================== Resources ====================== */\n")
        cstr = 'EXPORT const PriorityType knl_rcb_priority[] = \n{\n';
        cstr += '\tcfgOS_MAX_PRIORITY,/* RES_SCHEDULER */\n'
        for res in GetOsekObjects('RESOURCE'):
            if(res.getValue('RESOURCEPROPERTY') == 'STANDARD'):
                cstr += '\t%s_priority,\n'%(res.name)
        cstr += '};\n'
        fp.write(cstr); 
def GenerateComCfgH():
    global oilcc_o
    fp = open('%s/comcfg.h'%(oilcc_o),'w')
    fp.write(cCodeHead)
    fp.write('\n#ifndef COMCFG_H_H\n#define COMCFG_H_H\n\n')  
    # NM
    fp.write('#define cfgNM_NET_NUM %s\n'%(len(GetOsekObjects('NM'))))
    for nm in GetOsekObjects('NM'):
        nm.genH(fp)
    # IPDU
    fp.write('\n#define cfgCOM_IPDU_NUM %s\n'%(len(GetOsekObjects('IPDU'))))
    rx = tx = 0
    tpTx = 0
    for pdu in GetOsekObjects('IPDU'):
        if(pdu.getValue('IPDUPROPERTY') == 'SENT'):
            if(pInt(pdu.getValue('SIZEINBITS')) > 64):
                tpTx += 1;
            tx += 1
        else:
            rx += 1
    fp.write('#define cfgCOM_TPIPDU_NUM %s /* IPDU should be processed by Transport Layer */\n'%(tpTx))
    fp.write('#define cfgCOM_TxIPDU_NUM %s\n'%(tx))
    fp.write('#define cfgCOM_RxIPDU_NUM %s\n'%(rx))
    for pdu in GetOsekObjects('IPDU'):
        if(pdu.getValue('IPDUPROPERTY') == 'SENT'):
            pdu.genH(fp)
    for pdu in GetOsekObjects('IPDU'):
        if(pdu.getValue('IPDUPROPERTY') == 'RECEIVED'):
            pdu.genH(fp)
    fp.write('\n#endif\n\n') 
    fp.close() 

class GenerateComCfgC():
    def __init__(self):
        global oilcc_o
        fp = open('%s/comcfg.c'%(oilcc_o),'w')
        fp.write(cCodeHead)
        fp.write('#include "Com.h"\n')
        # IPDU
        self.genIPDU(fp);
        fp.close()
    def genIPDU(self,fp):
        # Gen Buffer
        for pdu in GetOsekObjects('IPDU'):
            fp.write('LOCAL uint8 %s_buffer[%s];\n'%(pdu.name,(pInt(pdu.getValue('SIZEINBITS'))+7)/8))
        fp.write('\n')
        cstrT = 'EXPORT const Com_IPDUConfigType ComTxIPDUConfig[] = \n{\n'
        cstrR = 'EXPORT const Com_IPDUConfigType ComRxIPDUConfig[] = \n{\n'
        for pdu in  GetOsekObjects('IPDU'):
            if(pdu.getValue('IPDUPROPERTY') == 'SENT'):
                cstrT += '\t{\n'
                cstrT += '\t\t{%s_buffer,\tsizeof(%s_buffer)},\n'%(pdu.name,pdu.name)
                cstrT += '\t\t%s_LAYERUSED,\n'%(pdu.name)
                cstrT += '\t\t%s_ID,\n'%(pdu.name)
                cstrT += '\t},\n'
            else:
                cstrR += '\t{\n'
                cstrR += '\t\t{%s_buffer,\tsizeof(%s_buffer)},\n'%(pdu.name,pdu.name)
                cstrR += '\t\t%s_LAYERUSED,\n'%(pdu.name)
                cstrR += '\t\t%s_ID,\n'%(pdu.name)
                cstrR += '\t},\n'
        cstrT += '};\n\n'
        cstrR += '};\n\n'
        fp.write(cstrT+cstrR)
def GenerateCode():
    GenerateOsCfgH()
    GenerateOsCfgC()
    GenerateComCfgH()
    GenerateComCfgC()
def Compile(file = ''):
    "Start to Compile the oilcc_target OIL file."
    global oilcc_I,oilcc_o,oilcc_S,oilcc_target
    if(file == ''):
        file = oilcc_target
    if(os.path.exists(file)):
        print 'INFO:parse Main file <%s>.'%(file)
    else:
        print 'ERROR: file <%s> isn\'t existed'
    PrepareCompile(file)
    textToItems()
    itemsToOsekObj()
    osekObjPostProcess()
    GenerateCode()

def GetOption():
    global oilcc_I,oilcc_o,oilcc_S,oilcc_target
    ercd = True
    option = ''
    for arg in sys.argv:
        if(arg[0:2] == '-I'):
            if(len(arg) > 2):
                oilcc_I.append(arg[2:])
            else:
                option = '-I'
        elif(option == '-I'):
            oilcc_I.append(arg)
            option = ''
        elif(arg == '-o'):
            option = '-o'
        elif(option == '-o'):
            oilcc_o = arg
            option = ''
        elif(arg == '-S'):
            option = '-S'
        elif(option == '-S'):
            oilcc_S = arg
            option = ''
        elif(os.path.exists(arg) and arg[-4:].lower() == '.oil'):
            oilcc_target = arg
        elif(arg == sys.argv[0]):
            continue
        else:
            print 'Invalid Option : %s.'%(arg)
            sys.exit(-1)
    return ercd

def usage():
    print """
Usage: make [options] [target] ...
Options:
  -h, --help                  Print this message and exit.
  -I DIRECTORY, --include-dir=DIRECTORY
                              Search DIRECTORY for included files.
  -o PATH                     Output path for the generated files.
  -S [FILE]                   Output a pre-process file(Format in Excel *.csv ).
  -v, --version               Print the version number of make and exit.
  """
      
if __name__ == "__main__":
    if(len(sys.argv) == 1):
        print "No Input file!"
    elif(len(sys.argv) == 2 and (sys.argv[1] == '--help' or sys.argv[1] == '-h')):
        usage()
    elif(len(sys.argv) == 2 and (sys.argv[1] == '--version' or sys.argv[1] == '-v')):
        print 'OSEK oilcc version 2.5.'
    else:
        if False == GetOption():
            sys.exit(-1)
        Compile()
            