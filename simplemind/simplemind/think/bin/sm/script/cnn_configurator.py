"""Configurator

This script contains the configurator class

"""

import os
import configparser
import sys
import time
 
class Configurator(object):
    def __init__(self, config_file, log):
        self.log = log
        self.time_counter = 0
        self.time_to_wait = 300
        self.load_config(config_file)
 
    def load_config(self,config_file):
        while not os.path.exists(config_file):
            time.sleep(1)
            self.time_counter += 1
            if self.time_counter > self.time_to_wait:
                break
            self.log.debug(f"Sleep on ini for {self.time_counter} sec")
            sys.stdout.flush()
            
        if os.path.exists(config_file)==False:
            raise Exception("%s file does not exist.\n" % config_file)    
        self.config = configparser.ConfigParser()
        self.log.debug('Configuration with {}'.format(config_file))
        self.config.read(config_file)

    def get_section_map(self):
        return self.config.sections()
        
    def set_config_map(self, section_map):
        self.log.debug('Set configuration map with section {}'.format(section_map))
        self.config_map = dict(zip(section_map, map(lambda section : dict(self.config.items(section)), section_map)))
    
    def print_config_map(self):
        self.log.info('    Configuration map')
        for section, item in self.config_map.items():
            self.log.info('        {} \t: {}'.format(section, item.keys()))
        
    def get_config_map(self):
        return self.config_map