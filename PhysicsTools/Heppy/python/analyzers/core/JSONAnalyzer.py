import json
import os
from PhysicsTools.Heppy.analyzers.core.Analyzer import Analyzer

from FWCore.PythonUtilities.LumiList import LumiList
from PhysicsTools.Heppy.utils.rltinfo import RLTInfo

from DataFormats.FWLite import Lumis

class JSONAnalyzer( Analyzer ):
    '''Apply a json filter, and creates an RLTInfo TTree.
    See PhysicsTools.HeppyCore.utils.RLTInfo for more information

    example:
    
    jsonFilter = cfg.Analyzer(
      "JSONAnalyzer",
      )

    The path of the json file to be used is set as a component attribute.

    The process function returns:
      - True if
         - the component is MC or
         - if the run/lumi pair is in the JSON file
         - if the json file was not set for this component
      - False if the component is MC or embed (for H->tau tau),
          and if the run/lumi pair is not in the JSON file.
    '''

    def __init__(self, cfg_ana, cfg_comp, looperName):
        super(JSONAnalyzer, self).__init__(cfg_ana, cfg_comp, looperName)
        if not cfg_comp.isMC:
            if self.cfg_comp.json is None:
                raise ValueError('component {cname} is not MC, and contains no JSON file. Either remove the JSONAnalyzer for your path or set the "json" attribute of this component'.format(cname=cfg_comp.name))
            self.lumiList = LumiList(os.path.expandvars(self.cfg_comp.json))
        else:
            self.lumiList = None
        
        if hasattr(self.cfg_comp, 'additionaljson'):
            self.additionalLumiList = LumiList(os.path.expandvars(self.cfg_comp.additionaljson))
            self.twojson = True
        else: 
            self.twojson = False
        
        self.useLumiBlocks = self.cfg_ana.useLumiBlocks if (hasattr(self.cfg_ana,'useLumiBlocks')) else False

        self.rltInfo = RLTInfo()

    def beginLoop(self, setup):
        super(JSONAnalyzer,self).beginLoop(setup)
        self.counters.addCounter('JSON')
        self.count = self.counters.counter('JSON')
        self.count.register('All Events')
        self.count.register('Passed Events')
        if self.twojson:
            self.count.register('Additional JSON Passed Events')

        if self.useLumiBlocks and not self.cfg_comp.isMC and not self.lumiList is None:
            lumis = Lumis(self.cfg_comp.files)
            for lumi in lumis:
                lumiid = lumi.luminosityBlockAuxiliary().id()
                run, lumi = lumiid.run(), lumiid.luminosityBlock()
                if self.lumiList.contains(run,lumi):
                    self.rltInfo.add('dummy', run, lumi)
            

    def process(self, event):
        self.readCollections( event.input )
        evid = event.input.eventAuxiliary().id()
        run = evid.run()
        lumi = evid.luminosityBlock()
        eventId = evid.event()

        event.run = run
        event.lumi = lumi
        event.eventId = eventId

        if self.cfg_comp.isMC:
            return True

        if self.lumiList is None:
            return True

        self.count.inc('All Events')
        if self.lumiList.contains(run,lumi):
            self.count.inc('Passed Events')
            if not self.useLumiBlocks:
                self.rltInfo.add('dummy', run, lumi)
            if self.twojson:
                if self.additionalLumiList.contains(run,lumi):
                    event.passSecondJSON = True
                    self.count.inc('Additional JSON Passed Events')
                else:
                    event.passSecondJSON = False
            return True
        else:
            return False
        

    def write(self, setup):
        super(JSONAnalyzer, self).write(setup)
        self.rltInfo.write( self.dirName )

