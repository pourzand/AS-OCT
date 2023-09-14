"""Reporter

This script will generate reporting from predictions.
    * main - the main function of the script

Parameters
----------
--output_dir_list: list of str
    list of output_dir paths to compare
-v: int
    '--verbose': (optional) default=0
    set the logging level
        0: no logging (critical error only)
        1: info level
        2: debug level
"""

import os
import sys
import logging
from argparse import ArgumentParser
import numpy as np

def main(log, args):
    pass


if __name__=='__main__':
    """Parsing the input arguments"""
    parser = ArgumentParser(description='Script to generating reports with predictions.')
    parser.add_argument('-o', '--output_dir_list', type=str, dest='output_dir_list', 
                        help="list of output_dir paths to compare")
    parser.add_argument('-v', '--verbose', type=int, dest='verbose', default=0,
                        help="logging level")
    args = parser.parse_args()

    """logging: using basic logger from logging
    TODO: set color
    https://stackoverflow.com/questions/45923290/how-to-get-the-current-log-level-in-python-logging-module
    """
    logging.basicConfig(format = '[%(name)-10s|%(levelname)-8s|%(filename)-20s:%(lineno)-3s] %(message)s',
                    level=logging.DEBUG)
    log = logging.getLogger()
    if args.verbose >= 2: log.setLevel(logging.DEBUG)
    elif args.verbose >= 1: log.setlevel(logging.INFO)
    else: log.setlevel(logging.WARNING) # logging.ERROR / logging.CRITICAL
    
    log.info('---------------------------------------------------------------')
    log.info('Python Environments')
    log.info('---------------------------------------------------------------')
    log.info('python v. %d.%d.%d' % (sys.version_info[:3]))
    log.info(f'numpy v. {np.__version__}')
    
    main(log, args)