'''
Daemon Testing -- mKTL 
'''
#import time # pylint: disable=W0611
import mktl # pylint: disable=C0114, E0401, W0611, C0103

from hispec.util.thorlabs.fw102c import FilterWheelController #pylint: disable = E0401,E0611


class Testfw(mktl.Daemon):
    '''
    Test the mKTL ATCFWHEEL interface
    '''

    def __init__(self, store, alias, *args, **kwargs):
        '''
        Initialize the Daemon type anmd service
        '''
        items = {
            "fwcon": {
                "type": "numeric",
                "description": "Filter Wheel Position"
            }
        }
        mktl.config.authoritative(store, alias, items)
        mktl.Daemon.__init__(self, store, alias,*args, **kwargs)
        self.store.fw = FilterWheelController()
        self.store.fw.connect(host = "192.168.29.100", port = 10010)
        self.store.fw.initialize()

    def setup(self):
        '''
        Setup FilterWheel items/services
        '''
        self.add_item(Fwcon, "fwcon")
        #self.add_item(Fwnamedpos, "fwnamedpos")

    def setup_final(self):
        '''
        Final Setup Stage
        '''
        pass

    def cleanup(self):
        '''
        Cleanup Stage
        '''
        self.store.fw.set_pos(1)
        self.store.fw.disconnect()

    def stop(self):
        '''
        Stop Stage
        '''
        self.store.fw.set_pos(1)
        self.store.fw.disconnect()

class Fwcon(mktl.Item):
    '''
    Filter Wheel Controller Item
    '''
    def __init__(self, *args, **kwargs):
        mktl.Item.__init__(self, *args, **kwargs)

    def perform_get(self):
        '''
        Get Filter Wheel Position
        '''
        pos = self.store.fw.get_pos()
        return self.to_payload(pos)

    def perform_set(self, value):
        '''
        Set Filter Wheel Position
        '''
        pos = int(value)
        self.store.fw.set_pos(pos)
        return self.to_payload(pos)
#
#class Fwnamedpos(mktl.Item):
#    '''
#    Filter Wheel Named Position Item
#    '''
#    def __init__(self, *args, **kwargs):
#        mktl.Item.__init__(self, *args, **kwargs)
#
#    def perform_get(self):
#        '''
#        Get Filter Wheel Named Position
#        '''
#
#    def perform_set(self, value):
#        '''
#        Set Filter Wheel Named Position
#        '''
#        pass

