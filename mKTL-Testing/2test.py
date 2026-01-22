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

    def stop(self):
        '''
        Stop Stage
        '''

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
        pos = mktl.get('atcf.fwcon')
        return self.to_payload(pos.value)

    def perform_set(self, value):
        '''
        Set Filter Wheel Position
        '''
        pos = int(value)
        cur = mktl.get('atcf.fwcon')
        print("Current Position:", cur.value)
        cur.set(pos)
        print("New Position:", cur.value)
        return self.to_payload(pos)

