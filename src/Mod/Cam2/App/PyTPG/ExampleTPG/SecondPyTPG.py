'''
#/***************************************************************************
# *   Copyright (c) 2012 Andrew Robinson  (andrewjrobinson@gmail.com)       *
# *                                                                         *
# *   This file is part of the FreeCAD CAx development system.              *
# *                                                                         *
# *   This library is free software; you can redistribute it and/or         *
# *   modify it under the terms of the GNU Library General Public           *
# *   License as published by the Free Software Foundation; either          *
# *   version 2 of the License, or (at your option) any later version.      *
# *                                                                         *
# *   This library  is distributed in the hope that it will be useful,      *
# *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
# *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
# *   GNU Library General Public License for more details.                  *
# *                                                                         *
# *   You should have received a copy of the GNU Library General Public     *
# *   License along with this library; see the file COPYING.LIB. If not,    *
# *   write to the Free Software Foundation, Inc., 59 Temple Place,         *
# *   Suite 330, Boston, MA  02111-1307, USA                                *
# *                                                                         *
# ***************************************************************************/
Created on 16/06/2013

@author: arobinson
'''


import Cam
from PyCam.PyCam import PyTPGBase, UnimplementedTPError


__all__ = ['SecondPyTPG']

class SecondPyTPG(PyTPGBase):
    '''A second TPG to test ui'''
    settings = None
    
    # The static identification and descriptive information 
    id = u'299d8e22-3155-11e2-98ec-08002734b94f' #this NEEDS to be unique!!!  @see PyTPGBase.id
    name = u'SecondPyTPG' # should match classname
    description = u'A second Python TPG for testing UI'
    
    def _defineSettings(self):
        '''Support method to setup the settings structure'''
        
        self.settings = Cam.TPGSettings()
        self.settings.addSettingDefinition('default', Cam.TPGSettingDefinition('geometry', 'Geometry', 'Cam::Textbox', '1', 'mm', 'How close to run tool to final depth'))
        self.settings.addSettingDefinition('default', Cam.TPGSettingDefinition('tolerance', 'Tolerance', 'Cam::Textbox', '1', 'mm', 'How close to run tool to final depth'))
    
    def getActions(self):
        '''Returns a list of actions that this TPG offers.  Actions are like
        methods on a class; they allow a TPG to perform user-selectable tasks.
        i.e. they might calculate the ToolPath using different strategies, 
        they might allow a multi-step strategy to be performed in individual
        steps OR they might provide testing or debug tasks.  If the TPG only
        supports a single action then it should be called 'default'.'''
        
        if self.settings is None:
            self._defineSettings()
        
        return self.settings.keys()

    def getSettingDefinitions(self):
        '''Returns a list of settings that the TPG uses for each 'Action'.
        
        @return: Cam.TPGSettings instance containing the settings
        '''
        if self.settings is None:
            self._defineSettings()

        return self.settings
        
    def run(self, settings, toolpath, action):
        '''Runs the selected action and returns the resulting TP'''
        
        # First tell the UI we have begun processing
#        self.updateProgress('STARTED', 1)
        
        # get the Input geometry (use getModel*(name) methods to get geometry in various formats)
#        name = settings.getSetting('InputGeometry') # NOTE: this isn't implemented yet
#        geom = self.getModelFreeCAD(name)
        
        if action == 'default':
            # Add toolpath primitives
            # TODO: actually use the geometry
#            self.updateProgress('RUNNING', 2)
            toolpath.addToolPath('rapid(0,0,0.1)')
            toolpath.addToolPath('feed(0,0,-0.5)')
            toolpath.addToolPath('feed(1,0,-0.5)')
            toolpath.addToolPath('feed(1,1,-0.5)')
#            self.updateProgress('RUNNING', 55)
            toolpath.addToolPath('feed(0,1,-0.5)')
            toolpath.addToolPath('feed(0,0,-0.5)')
            toolpath.addToolPath('rapid(0,0,0.1)')
#            self.updateProgress('RUNNING', 99)
        elif action == 'test':
            print 'Testing ExampleTPG'
        
        # Let the UI know we are done
#        self.updateProgress('FINISHED', 100)
        
        return toolpath
    # end run()
    
    