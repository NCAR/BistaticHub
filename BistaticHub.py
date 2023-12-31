#!/usr/bin/env python

# BistaticHub.py
# This is the GUI front-end used to run the BistaticHub program for collecting
# and processing bistatic radar data
#
# Copyright © 1999 Binet Incorporated
# Copyright © 1999 University Corporation for Atmospheric Research
#
#   Licensed under the Apache License, Version 2.0 (the "License");
#   you may not use this file except in compliance with the License.
#   You may obtain a copy of the License at
#
#
#       http:#www.apache.org/licenses/LICENSE-2.0
#
#
#   Unless required by applicable law or agreed to in writing, software
#   distributed under the License is distributed on an "AS IS" BASIS,
#   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#   See the License for the specific language governing permissions and
#   limitations under the License.

import os
import sys
import re
import signal
import fcntl
import time
import tkinter
from tkinter.simpledialog import Dialog
from tkinter import Frame, Label, Button, Text, Scrollbar, Pack, IntVar, \
                    Radiobutton, Entry, StringVar, Menu, Menubutton

class BistaticHub( Frame ):
    def __init__( self, master=None ):
        Frame.__init__( self, master )
        self.debuglevel = 0
        self.ncpthresh = -1.0
        self.winfo_toplevel().title("Bistatic Hub Control and Status")
        self.createWidgets()
        Pack.config( self )
        self.rcvrs = {}
        
        signal.signal( signal.SIGCHLD, self.childDied )
        signal.signal( signal.SIGTERM, self.sigExit )
        signal.signal( signal.SIGINT, self.sigExit )
        signal.signal( signal.SIGSEGV, self.sigExit )
        signal.signal( signal.SIGQUIT, self.sigExit )
        self.startclient()
        self.tk.createfilehandler( self.from_client.fileno(), tkinter.READABLE, self.incoming )

    def createWidgets( self ):
        #
        # bottom buttons
        #
        buttonframe = Frame( self )
        buttonframe.pack( side=tkinter.BOTTOM, fill=tkinter.X )

        self.recbutton = Button( buttonframe, text="Hub Recording is: unknown",
                                 command=self.toggleRecording )
        self.recbutton.pack( side=tkinter.LEFT )

        self.debugbutton = Button( buttonframe, text="Debug Level: unknown",
                                   command=self.debugDialog )
        self.debugbutton.pack( side=tkinter.LEFT )

        self.ncpbutton = Button( buttonframe, text="NCP threshold: unknown",
                             command=self.ncpDialog )
        self.ncpbutton.pack( side=tkinter.LEFT )

        self.sendbutton = Button( buttonframe, text="Send Command",
                                  command=self.rcvrCmdDialog )
        self.sendbutton.pack( side=tkinter.LEFT )

        Button( buttonframe, text="Shut Down", command=self.done,
                fg="red4" ).pack( side=tkinter.RIGHT )

        #
        # text area for general messages
        #
        textframe = Frame( self )
        textframe.pack( side=tkinter.BOTTOM, fill=tkinter.BOTH )
        self.textarea = Text( textframe )
        self.textarea.pack( side=tkinter.LEFT, fill=tkinter.BOTH )
        scroll = Scrollbar( textframe )
        scroll.config( command=self.textarea.yview )
        scroll.pack( side=tkinter.RIGHT, fill=tkinter.BOTH )
        self.textarea.config( yscrollcommand=scroll.set, height=15, width=80,
                              wrap=tkinter.CHAR )
        self.textarea.column = 0

    def incoming( self, fd, unknown ):
        # Read byte-by-byte until we hit a newline
        byteline = b""  # empty byte string
        while 1:
            try:
                c = self.from_client.read( 1 )
                if (not c):
                    break

                byteline += c
                if (c == b"\n"):
                    break
            except IOError:
                break

        # Now convert the line of text bytes into 'normal' Python string
        # representation to make further parsing simpler
        line = byteline.decode()

        #
        # Read stuff from BistaticHub, with special handling for certain
        # lines; the rest just gets written to our logging area
        #
        if (len( line ) == 0):
            return
        #
        # Look for the leading strings we handle specially
        #
        specials = [
            ("receiver:", self.handleRcvr),
            ("recording:", self.handleRecStatus),
            ("debuglevel:", self.handleDebugLevel),
            ("ncpthresh:", self.handleNCPThresh),
            ("status:", self.handleStatus),
            ("syncinfo:", self.handleSyncInfo)
        ]

        for string, handler in specials:
            match = re.search( string, line )
            if (match):
                start = match.start()
                if (start != 0):
                    self.log( line[:start] )    # log any extraneous stuff preceding the special
                handler( line[start:] )

                # Return after we handle a match
                return

        # If the line wasn't handled above, just log it as an informational message
        self.log( line )

    def handleRcvr( self, line ):
        #
        # Deal with receiver info line:
        #   receiver: <rnum> <rname> <enabled> [<extra info>]
        #
        match = re.search( 'receiver: *([0-9]+) +([^ ]+) +([0-9]+) (.*)', line )
        if (match == None):
            self.log( "Bad 'receiver' line: " + line )
            return

        rnum = match.group(1)
        name = match.group(2)
        enabled = match.group(3)
        extra = match.group(4)

        #
        # If we already have this receiver in our list, update its state,
        # otherwise create it
        #
        try:
            self.rcvrs[name].updateEnableState( enabled )
        except KeyError:
            self.rcvrs[name] = Receiver( name, rnum, enabled,
                                             self.sendToClient, self )
            self.pack()
            #
            # If we got extra information, put it into the receiver status
            # in red.  (We also blank the sync info for the receiver)
            #
            self.rcvrs[name].updateStatus( extra, "red2" )
            self.rcvrs[name].updateSyncInfo( "" )

    def handleStatus( self, line ):
        #
        # Deal with status line:
        #   status: <rname> <status stuff...>
        #
        fields = line.split()
        rname = fields[1]
        status = ' '.join(fields[2:])

        try:
            self.rcvrs[rname].updateStatus( status )
        except KeyError:
            self.log( "Attempt to update status for unknown receiver " +
                      rname + "\n" )

    def handleSyncInfo( self, line ):
        #
        # Deal with syncinfo line:
        #   syncinfo: <rname> p <p0> ... <p7> v <v0> ... <v7>
        #
        fields = line.split()
        rname = fields[1]
        p = fields[3:11]
        v = fields[12:20]
        line = "P " + " ".join(p) + "    V " + " ".join(v)
        self.rcvrs[rname].updateSyncInfo( line )

    def handleRecStatus( self, line ):
        self.recording = eval( line.split()[1] )
        
        if (self.recording):
            statusString = "ON"
            color = "green4"
        else:
            statusString = "OFF"
            color = "red2"
        self.recbutton.config( text="Hub Recording is: " + statusString,
                               fg=color )

    def debugDialog( self ):
        newlevel = DebugLevelDialog( self, self.debuglevel ).result
        if (newlevel != None):
            self.sendToClient( "debuglevel " + repr(newlevel) )

    def ncpDialog( self ):
        newthresh = NCPThreshDialog( self, self.ncpthresh ).result
        if (newthresh != None):
            self.sendToClient( "ncpthresh " + newthresh )

    def rcvrCmdDialog( self ):
        try:
            cmd, recipient = RcvrCmdDialog( self, self.rcvrs ).result
        except TypeError:
            return
        
        if (recipient == "All"):
            rnum = -1
        else:
            rnum = self.rcvrs[recipient].rcvrNum
            
        self.sendToClient( "rcvrcommand " + cmd + ' ' + repr(rnum) )

    def handleDebugLevel( self, line ):
        self.debuglevel = eval( line.split()[1] )
        self.debugbutton.config( text="Debug Level: " + repr(self.debuglevel) )

    def handleNCPThresh( self, line ):
        self.ncpthresh = eval( line.split()[1] )
        self.ncpbutton.config( text="NCP Threshold: " + repr(self.ncpthresh) )

    def log( self, text ):
        #
        # force line breaking if necessary
        #
        width = self.textarea.cget( "width" )

        nleft = width - self.textarea.column
        while (len( text ) > nleft):
            partial = text[:nleft]
            self.textarea.insert( tkinter.END, partial + "\n" )
            text = text[nleft-1:]
            self.textarea.column = 0
            nleft = width

        self.textarea.insert( tkinter.END, text )
        if (text[-1] == '\n'):
            self.textarea.column = 0
        else:
            lastline = '\n'.split(text)[-1]
            self.textarea.column = self.textarea.column + len( lastline )
        #
        # Just eval the line.column index as a float, then convert it
        # to an int to get our line count
        #
        nlines = int( eval( self.textarea.index( tkinter.END ) ) )
        #
        # Keep our size down to 200 lines or fewer
        #
        nkill = nlines - 200
        if (nkill > 0):
            self.textarea.delete( "0.0", repr(nkill) + ".0" )
        #
        # Scroll so that we keep the new stuff in the window
        #
        totallines = int( eval( self.textarea.index( tkinter.END ) ) )
        winheight = self.textarea.cget( "height" )
        top = 1.0 - (float( winheight - 4 ) / totallines)
        if top < 0:
            top = 0
        self.textarea.yview( "moveto", top )

    def dumplog( self ):
        fname = "BistaticHub.log"
        try:
            f = open( fname, "w" )
            f.write( self.textarea.get( 1.0, tkinter.END ) )
            f.close()
            sys.stderr.write("Log information dumped to " + fname + "\n" )
        except:
            sys.stderr.write( "Failure writing log file " + fname + "\n" )
            

    def startclient( self ):
        pipe_a = os.pipe()
        pipe_b = os.pipe()

        if (os.fork()):
            os.close( pipe_a[0] )
            os.close( pipe_b[1] )
            self.to_client = os.fdopen( pipe_a[1], 'wb', 0 )
            self.from_client = os.fdopen( pipe_b[0], 'rb', 0 )
            #
            # set up our input file for non-blocking reads
            #
            fcntl.fcntl( self.from_client.fileno(), fcntl.F_SETFL, os.O_NDELAY )
        else:
            os.close( pipe_a[1] )
            os.close( pipe_b[0] )
            os.dup2( pipe_a[0], 0 )
            os.dup2( pipe_b[1], 1 )
            os.dup2( 1, 2 )

            prog = "BistaticHub"
            args = (prog,) + tuple( sys.argv[1:] )
            os.execv( prog, args )
            #
            # if execv returned, it's a problem...
            #
            sys.exit( 1 )

    # Send the given string command to our client as 8-bit characters
    def sendToClient( self, cmd ):
        cmdbytes = (cmd + "\n").encode("utf-8")
        self.to_client.write( cmdbytes )

    def toggleRecording( self ):
        if self.recording:
            self.sendToClient( "writeoff" )
        else:
            self.sendToClient( "writeon" )

    def haltChild( self ):
        signal.signal( signal.SIGCHLD, signal.SIG_DFL )
        self.sendToClient( "exit" )

    def done( self ):
        self.haltChild()
        sys.exit( 0 )

    def sigExit( self, signum, stack ):
        sys.stderr.write( "Exiting on signal " + repr(signum) )
        self.log( "Exiting on signal " + repr(signum) + "\n" )
        self.dumplog()
        self.haltChild()
        sys.exit( 1 )

    def childDied( self, signum, stack ):
        sys.stderr.write( "Child program died, so I'm quitting!\n" )
        #
        # Try to grab any dying message we may have been left...
        #
        self.incoming( self.from_client.fileno(), 0 )
        self.log( "(BistaticHub.py exiting on SIGCHLD)\n" )
        self.dumplog()
        sys.exit( 1 )


class DebugLevelDialog( Dialog ):
    def __init__( self, master, currentlevel ):
        self.newlevel = IntVar()
        self.newlevel.set( currentlevel )
        Dialog.__init__( self, master )
        
    def body( self, master ):
        for i in range(4):
            Radiobutton( master, text=repr(i), variable=self.newlevel,
                         value=i ).grid( row=0, column=i )

    def apply( self ):
        self.result = self.newlevel.get()


class NCPThreshDialog( Dialog ):
    def __init__( self, master, currentthresh ):
        self.currentval = currentthresh
        Dialog.__init__( self, master )
        
    def body( self, master ):
        Label( master, text="NCP threshold:" ).grid( row=0, column=0 )
        self.entry = Entry( master )
        self.entry.insert( 0, self.currentval )
        self.entry.grid( row=0, column=1 )
        self.entry.config( width=5 )

    def apply( self ):
        self.result = self.entry.get()


class RcvrCmdDialog( Dialog ):
    def __init__( self, master, rcvrs ):
        self.rcvrs = rcvrs
        self.towhom = StringVar()
        self.towhom.set( "All" )
        Dialog.__init__( self, master )

    def body( self, master ):
        Label( master, text="Send:" ).grid( row=0, column=0 )
        self.entry = Entry( master )
        self.entry.grid( row=0, column=1 )
        self.entry.config( width=5 )
        Label( master, text="to" ).grid( row=0, column=2 )
        self.mb = Menubutton( master, text=self.towhom.get(), borderwidth=2,
                         relief=tkinter.RAISED )
        self.mb.grid( row=0, column=3 )
        menu = Menu( self.mb )
        menu.add_radiobutton( label="All", variable=self.towhom, value="All",
                              indicatoron=0, command=self.updateMenuButton )

        for r in list(self.rcvrs.keys()):
            menu.add_radiobutton( label=r, variable=self.towhom,
                                  value=r, indicatoron=0,
                                  command=self.updateMenuButton )
        self.mb['menu'] = menu

    def updateMenuButton( self ):
        self.mb.config( text=self.towhom.get() )

    def apply( self ):
        self.result = ( self.entry.get(), self.towhom.get() )


class Receiver( Frame ):
    def __init__( self, name, rnum, enabled, sender, master=None ):
        Frame.__init__( self, master, relief=tkinter.SUNKEN, borderwidth=2 )
        self.name = name
        self.pack( side=tkinter.TOP, fill=tkinter.X )
        self.rcvrNum = rnum
        self.createWidgets()
        self.sendToClient = sender
        self.updateEnableState( enabled )

    def updateEnableState( self, enabled ):
        self.enabled = enabled
        if self.enabled:
            self.label.config( fg="black" )
        else:
            self.label.config( fg="gray60" )
            self.status.config( text="" )
            self.sync.config( text="" )

    def updateStatus( self, status, color="black" ):
        self.status.config( text=status, fg=color )
 
    def updateSyncInfo( self, info ):
        self.sync.config( text=info )

    def createWidgets( self ):
        self.label = Label( self, text=self.name, width=20, fg="red",
                                anchor=tkinter.E )
        self.label.pack( side=tkinter.LEFT )
        Button( self, command=self.toggleEnable ).pack( side=tkinter.LEFT )
        self.status = Label( self, text="", anchor=tkinter.W )
        self.status.pack( side=tkinter.TOP, fill=tkinter.X )
        self.sync = Label( self, text="", anchor=tkinter.W )
        self.sync.pack( side=tkinter.BOTTOM, fill=tkinter.X )

    def toggleEnable( self ):
        if (self.enabled):
            cmd = "disable " + repr(self.rcvrNum)
        else:
            cmd = "enable " + repr(self.rcvrNum)
    
        self.sendToClient( cmd )

        

if __name__ == '__main__':
    BistaticHub().mainloop()
