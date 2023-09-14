import traceback
import logging
import logging.handlers
import warnings

import os
import logging
import logging.handlers

DEFAULT_FORMAT = '%(asctime)s - %(levelname)s - %(module)s - %(lineno)s - %(message)s'
DEFUALT_SUFFIX = '%Y-%m-%d'

# basic file handler for logging
def get_basic_file_handler(filename,suffix=DEFUALT_SUFFIX,format=DEFAULT_FORMAT):    
    warnings.warn('not tested.')
    hdlr = logging.FileHandler(filename)
    hdlr.suffix = suffix
    formatter = logging.Formatter(format)
    hdlr.setFormatter(formatter)    
    return hdlr
    
# time rotating file handler for logging
def get_time_rotating_file_handler(filename,suffix=DEFUALT_SUFFIX,format=DEFAULT_FORMAT,when='midnight'):
    hdlr = logging.handlers.TimedRotatingFileHandler(filename,when=when)
    hdlr.suffix = suffix
    formatter = logging.Formatter(format)
    hdlr.setFormatter(formatter)
    return hdlr

# console handler for logging
def get_console_handler(format=DEFAULT_FORMAT):
    console_handler = logging.StreamHandler()
    formatter = logging.Formatter(format)
    console_handler.setFormatter(formatter)
    return console_handler
        
# custom logger 
def get_custom_logger(handlers,log_name='root',log_level=logging.DEBUG,):
    logger = logging.getLogger(log_name)
    logger.setLevel(log_level)
    for hdlr in handlers:
        logger.addHandler(hdlr)
    return logger


# '--verbose': (optional) default=0
# set the logging level
#     0: no logging (critical error only)
#     1: info level
#     2: debug level
def ga_logger(verbose):
    log = logging.getLogger()
    # if args.verbose >= 2: log.setLevel(logging.DEBUG)
    # elif args.verbose >= 1: log.setLevel(logging.INFO)
    # else: log.setLevel(logging.WARNING) # logging.ERROR / logging.CRITICAL
    
    formatter = logging.Formatter('[%(asctime)s|%(name)-10s|%(levelname)-8s|%(filename)-25s:%(lineno)-3s] %(message)s')
    ch = logging.StreamHandler()
    if verbose >= 2: log.setLevel(logging.DEBUG)
    elif verbose >= 1: log.setLevel(logging.INFO)
    else: log.setLevel(logging.WARNING) # logging.ERROR / logging.CRITICAL
    ch.setFormatter(formatter)
    log.addHandler(ch)
    return log


def default_logger():
    return ga_logger(0)


### if name is not supplied, it will return the root logger
### highly recommend to supply name

def generic_logger(verbose, central_error_path=None, filepath=None, name=None):
    log = logging.getLogger(name)
    log.setLevel(logging.DEBUG)
    # if args.verbose >= 2: log.setLevel(logging.DEBUG)
    # elif args.verbose >= 1: log.setLevel(logging.INFO)
    # else: log.setLevel(logging.WARNING) # logging.ERROR / logging.CRITICAL
    
    formatter = logging.Formatter('[%(asctime)s|%(name)-10s|%(levelname)-8s|%(filename)-25s:%(lineno)-3s] %(message)s')
    # stdout_handler = logging.StreamHandler()
    # stdout_handler.setFormatter(formatter)
    # if verbose >= 2: stdout_handler.setLevel(logging.DEBUG)
    # elif verbose >= 1: stdout_handler.setLevel(logging.INFO)
    # else: stdout_handler.setLevel(logging.WARNING) # logging.ERROR / logging.CRITICAL
    # log.addHandler(stdout_handler)

    if filepath is not None:
        file_handler = logging.FileHandler(filepath)
        file_handler.setLevel(logging.DEBUG)
        file_handler.setFormatter(formatter)
        log.addHandler(file_handler)

    if central_error_path is not None:
        file_handler2 = logging.FileHandler(central_error_path)
        file_handler2.setLevel(logging.WARNING)
        file_handler2.setFormatter(formatter)
        log.addHandler(file_handler2)

    return log

def ga_watcher_logger(verbose, central_error_path=None, filepath=None, ):
    log = logging.getLogger()
    log.setLevel(logging.DEBUG)
    # if args.verbose >= 2: log.setLevel(logging.DEBUG)
    # elif args.verbose >= 1: log.setLevel(logging.INFO)
    # else: log.setLevel(logging.WARNING) # logging.ERROR / logging.CRITICAL
    
    formatter = logging.Formatter('[%(asctime)s|%(name)-10s|%(levelname)-8s|%(filename)-25s:%(lineno)-3s] %(message)s')
    stdout_handler = logging.StreamHandler()
    stdout_handler.setFormatter(formatter)
    if verbose >= 2: stdout_handler.setLevel(logging.DEBUG)
    elif verbose >= 1: stdout_handler.setLevel(logging.INFO)
    else: stdout_handler.setLevel(logging.WARNING) # logging.ERROR / logging.CRITICAL
    print(stdout_handler)
    log.addHandler(stdout_handler)

    if filepath is not None:
        file_handler = logging.FileHandler(filepath)
        file_handler.setLevel(logging.DEBUG)
        file_handler.setFormatter(formatter)
        log.addHandler(file_handler)

    if central_error_path is not None:
        file_handler2 = logging.FileHandler(central_error_path)
        file_handler2.setLevel(logging.WARNING)
        file_handler2.setFormatter(formatter)
        log.addHandler(file_handler2)

    return log

"""
how to use: 
log.debug('debug message')
log.info('info message')
log.warning('warn message')
log.error('error message')
log.critical('critical message')

Useful read: https://stackoverflow.com/questions/50714316/how-to-use-logging-getlogger-name-in-multiple-modules
"""
DEFAULT_SM_FORMATTER = logging.Formatter('[%(asctime)s|%(name)-10s|%(levelname)-8s|%(filename)-25s:%(lineno)-3s] %(message)s')
### Sets up the root logger
def logger_setup_default(verbose=2):
    log = logging.getLogger()
    log.setLevel(logging.DEBUG) ### TODO: might have to change this

    formatter = logging.Formatter('[%(asctime)s|%(name)-10s|%(levelname)-8s|%(filename)-25s:%(lineno)-3s] %(message)s')
    stdout_handler = logging.StreamHandler()
    stdout_handler.setFormatter(formatter)
    if verbose >= 2: stdout_handler.setLevel(logging.DEBUG)
    elif verbose >= 1: stdout_handler.setLevel(logging.INFO)
    else: stdout_handler.setLevel(logging.WARNING) # logging.ERROR / logging.CRITICAL
    log.addHandler(stdout_handler)
    return log

def simplemind_logger_setup(verbose=2, log_path=None, entry_point="simplemind"):
    log = logger_setup_default(verbose=verbose)
    formatter = DEFAULT_SM_FORMATTER
    if log_path is not None:
        central_log_file = os.path.join(log_path, "{}.log".format(entry_point))
        file_handler = logging.FileHandler(central_log_file)
        file_handler.setLevel(logging.DEBUG)
        file_handler.setFormatter(formatter)
        log.addHandler(file_handler)

        central_log_file = os.path.join(log_path, "{}.log".format("error"))
        file_handler = logging.FileHandler(central_log_file)
        file_handler.setLevel(logging.WARNING)
        file_handler.setFormatter(formatter)
        log.addHandler(file_handler)

        ## setting up sub logs
        log_dict = dict(
            engine = os.path.join(log_path, "engine.log"),
            dist = os.path.join(log_path, "dist.log"),
            opt = os.path.join(log_path, "opt.log"),
        )
        for k, log_file in log_dict.items():
            sub_log = logging.getLogger(k)
            file_handler = logging.FileHandler(log_file)
            file_handler.setFormatter(formatter)
            file_handler.setLevel(logging.DEBUG)
            sub_log.addHandler(file_handler)

    return log


def get_log_path(log_path):
    if log_path is None: return None
    import datetime
    now = datetime.datetime.now()
    timestamp = now.strftime("%Y%m%d_%H%M%S")
    # print (now.strftime("%Y-%m-%d %H:%M:%S"))
    
    ### currently something simple like a new log dir for each run       
    log_path = os.path.join(log_path, timestamp)
    os.makedirs(log_path, exist_ok=True)
    return log_path


# def test_this():
#     file1 = "/cvib2/apps/personal/wasil/trash/file1.log"
#     file2 = "/cvib2/apps/personal/wasil/trash/file2.log"
#     log1 = generic_logger(2, file1, file2, "log1")
#     log1.debug("hi from log1")

#     log2 = log1

#     log2.debug("hi from log2")
    
#     file3 = "/cvib2/apps/personal/wasil/trash/file3.log"
#     file_handler = logging.FileHandler(file3)
#     file_handler.setLevel(logging.DEBUG)
#     formatter = logging.Formatter('[%(asctime)s|%(name)-10s|%(levelname)-8s|%(filename)-25s:%(lineno)-3s] %(message)s')
#     file_handler.setFormatter(formatter)
#     log2.addHandler(file_handler)


#     log1.debug("hello from log1")
#     log2.debug("hello from log2")


#     log3 = logging.getLogger("log1.test")
#     log3.debug("hello from log3")


# def test_that():
#     log4 = logging.getLogger("wasil")
#     log4.debug("hello from log4")
#     log4.debug("wasil is %s", "cool")


# logger_setup_default()
# test_this()
# test_that()
