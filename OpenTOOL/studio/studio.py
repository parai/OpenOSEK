import Tkinter

class ConsoleMenu(Tkinter.Menu):
    __master = None
    def __init__(self,master=None):
        self.__master = master
        Tkinter.Menu.__init__(self, master)
        self.createFileMenu()
        self.createToolMenu()
        self.createHelpMenu()
        master['menu']=self  
    def actionFileOpen(self,event=None):
        print 'Hello World'
    def createFileMenu(self):
        fm = Tkinter.Menu(self,tearoff=0)
        fm.add_command(label = 'Open File... <Ctrl+O>',command = self.actionFileOpen)
        self.__master.bind('<Control-KeyPress-o>',self.actionFileOpen)
        fm.add_separator()
        fm.add_command(label = 'New          <Ctrl+N>',command = None)
        fm.add_separator()
        fm.add_command(label = 'Save         <Ctrl+S>',command = None)
        fm.add_command(label = 'Save As...',command = None)
        fm.add_separator()
        fm.add_command(label = 'Exit',command = self.__master.destroy)
        self.add_cascade(label = 'File',menu = fm)
    def createToolMenu(self):
        tm = Tkinter.Menu(self,tearoff=0)
        tm.add_command(label = 'Import Oil',command = None)
        tm.add_command(label = 'Export Oil',command = None)
        self.add_cascade(label = 'Tool',menu = tm)
    def createHelpMenu(self):
        hm = Tkinter.Menu(self,tearoff=0)
        hm.add_command(label = 'About',command = None)
        hm.add_command(label = 'Help',command = None)
        self.add_cascade(label = 'Help',menu = hm)
        
class OsekConsole(Tkinter.Tk):
    def __init__(self):
        Tkinter.Tk.__init__(self,None,None,'@Console of Osek Studio')
        ConsoleMenu(self)
    def mainloop(self):
        Tkinter.Tk.mainloop(self)


def main():
    OsekConsole().mainloop()

if __name__ == '__main__':
    main()
    print 'Exit Studio'

