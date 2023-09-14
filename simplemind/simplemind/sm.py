"""sm runner
This script is a wrapper for SM runner executable

Parameters
----------


Spec
----
runner(image_path, sm_model, output_dir)


Examples
--------


TODO
----
user-friendly optimizer wrapper

"""
import sys, os
import logging
from argparse import ArgumentParser
import numpy as np
import subprocess
import threading

class LogPipe(threading.Thread):
    """Reference: 
    https://codereview.stackexchange.com/questions/6567/redirecting-subprocesses-output-stdout-and-stderr-to-the-logging-module
    """
    def __init__(self, level, log):
        """Setup the object with a logger and a loglevel
        and start the thread
        """
        threading.Thread.__init__(self)
        self.log = log
        self.daemon = False
        self.level = level
        self.fdRead, self.fdWrite = os.pipe()
        self.pipeReader = os.fdopen(self.fdRead)
        self.start()

    def fileno(self):
        """Return the write file descriptor of the pipe
        """
        return self.fdWrite

    def run(self):
        """Run the thread, logging everything.
        """
        for line in iter(self.pipeReader.readline, ''):
            self.log.log(self.level, line.strip('\n'))
            # logging.log(self.level, line.strip('\n'))

        self.pipeReader.close()

    def close(self):
        """Close the write end of the pipe.
        """
        os.close(self.fdWrite)

def _execute(cmd, log):
    """helper function for execution
    Parameters
    ----------
    cmd : str
        command line to execute

    Returns
    -------
    """
    logpipe = LogPipe(logging.INFO, log)
    with subprocess.Popen(cmd, stdout=logpipe, stderr=logpipe, shell=False) as s:
        logpipe.close()
        s.wait()

def runner(image_path, sn_entry_path, output_dir, 
            working_directory='', user_resource_directory='', 
            force_overwrite=False,
            chromosome='', watcher='', 
            skip_tensorboard=False, 
            skip_png_training=True, skip_png_prediction=False, 
            logging_path = None,
            verbose=2):
    """SM execution for segmentation

    Parameters
    ----------
    image_path : str
        path of the image
    sn_entry_path : str
        path of the entry file of the semantic network (sn)
    output_dir : str
        path of the output directory 
    working_directory : str
        path of the working_directory (Default="")
    user_resource_directory : str
        path of the user_resource_directory (Default="")
    force_overwrite : bool
        to force overwrite any previous results in output_dir (Default=False)
    chromosome : str
        chromosome to run. (Default=None)
    watcher : str
        path of the watcher directory. This will be disregarded. -- WARNING: Not intended for active use
    skip_tensorboard : bool
        to skip Tensorboard during training (Default=False)
    skip_png_training : bool
        to skip PNG generation during training (Default=True)
    skip_png_prediction : bool
        to skip PNG generation during prediction (Default=False)
    logging_path : str
        path to write the logging (Default="")
    verbose : int
        logging level
    
    Returns
    -------


    """

    """logging: using basic logger from logging
    TODO: 
    set color
    https://stackoverflow.com/questions/45923290/how-to-get-the-current-log-level-in-python-logging-module
    make clean / add cnn_node_log file
    """
    # logging.basicConfig(format = '[%(name)-10s|%(levelname)-8s|%(filename)-20s:%(lineno)-3s] %(message)s',
    #                 level=logging.DEBUG)
    log = logging.getLogger()
    log.propagate = False
    # if verbose >= 2: log.setLevel(logging.DEBUG)
    # elif verbose >= 1: log.setLevel(logging.INFO)
    # else: setLevel(logging.WARNING) # logging.ERROR / logging.CRITICAL
    
    # formatter = logging.Formatter('[%(asctime)s|%(name)-10s|%(levelname)-8s|%(filename)-20s:%(lineno)-3s] %(message)s')
    formatter = logging.Formatter('[%(asctime)s] %(message)s', '%Y-%m-%d %H:%M:%S')
    stream_ch = logging.StreamHandler()
    stream_ch.setFormatter(formatter)
    if logging_path:
        print(f'Logging file: {logging_path}')
        file_ch = logging.FileHandler(logging_path)
        file_ch.setFormatter(formatter)
    if not len(log.handlers):
        log.addHandler(stream_ch)
        if logging_path:
            log.addHandler(file_ch)
    if verbose >= 2: log.setLevel(logging.DEBUG)
    elif verbose >= 1: log.setLevel(logging.INFO)
    else: log.setLevel(logging.WARNING) # logging.ERROR / logging.CRITICAL
    
    log.info('---------------------------------------------------------------')
    log.info(f'Simple Mind AI')
    log.info('---------------------------------------------------------------')
    
    log.info('---------------------------------------------------------------')
    log.info('SM runner')
    log.info('---------------------------------------------------------------')
    log.info(f'Image path: {image_path}')
    log.info(f'Semantic Network (SN) entry file path: {sn_entry_path}')
    log.info(f'Output directory path: {output_dir}')
    if working_directory: log.info(f'Working directory path: {working_directory}')
    if user_resource_directory: log.info(f'User resource directory path: {user_resource_directory}')
    # if args.chromosome: log.info(f'Chromosome: {args.chromosome}')
    log.info('---------------------------------------------------------------')

    current_path = os.path.realpath(__file__)
    """TODO: fix"""
    sm_runner = os.path.join(os.path.dirname(current_path), 'think', 'bin', 'sm', 'sm')
    log.info(f'SM runner executable path:  {sm_runner}')
    
    cmd = [sm_runner, image_path, sn_entry_path, output_dir]
    if working_directory: cmd.extend(['-d', working_directory])
    if user_resource_directory: cmd.extend(['-u', user_resource_directory])
    if force_overwrite: cmd.extend(['-f'])
    if chromosome: cmd.extend(['-c', chromosome])
    if watcher: cmd.extend(['-w', watcher])
    if skip_tensorboard: cmd.extend(['-t'])
    if skip_png_training: cmd.extend(['-it'])
    if skip_png_prediction: cmd.extend(['-i'])
    cmd.extend(['-p'])
    log.info(f"SM Runner Command: {cmd}")
    _execute(cmd, log)

    log.info('---------------------------------------------------------------')
    log.info('SM runner computation finished.')
    log.info('---------------------------------------------------------------')

if __name__=='__main__':
    """Parsing the input arguments"""
    parser = ArgumentParser(description='Script to run a CNN model prediction from miu')
    # parser.add_argument('-action', '--action', type=str, dest='action',
    #                     choices=[
    #                         'runner',
    #                         'optimizer',
    #                         ],
    #                     help="action for script")
    parser.add_argument('-image_path', '--image_path', type=str, dest='image_path',
                        help="image path")
    parser.add_argument('-sn_entry_path', '--sn_entry_path', type=str, dest='sn_entry_path', 
                        help="path of the entry file of the semantic network (sn)")
    parser.add_argument('-output_dir', '--output_dir', type=str, dest='output_dir', 
                        help="output directory path")
    parser.add_argument('-working_directory', '--working_directory', type=str, dest='working_directory',
                        default='',
                        help="working_directory")
    parser.add_argument('-user_resource_directory', '--user_resource_directory', type=str, dest='user_resource_directory',
                        default='',
                        help="user_resource_directory")
    parser.add_argument('-f', '--force_overwrite', type=str, dest='force_overwrite', 
                        default='False', action='store_true',
                        help="Force overwrite any previous results in output_dir")
    parser.add_argument('-chromosome', '--chromosome', type=str, dest='chromosome', 
                        default='',
                        help="chromosome")
    parser.add_argument('-watcher', '--watcher', type=str, dest='watcher', 
                        default='',
                        help="watcher directory -- WARNING: Not intended for active use")
    parser.add_argument('-skip_tensorboard', '--skip_tensorboard', type=str, dest='skip_tensorboard', 
                        default='False', action='store_true',
                        help="Skip Tensorboard during training.")
    parser.add_argument('-skip_png_training', '--skip_png_training', type=str, dest='skip_png_training', 
                        default='True', action='store_true',
                        help="Skip PNG generation during training. By default this is enabled.")
    parser.add_argument('-skip_png_prediction', '--skip_png_prediction', type=str, dest='skip_png_prediction', 
                        default='False', action='store_true',
                        help="Skip PNG generation during testing.")
    parser.add_argument('-logging_path', '--logging_path', type=str, dest='logging_path',
                        default='',
                        help="logging_path")
    parser.add_argument('-v', '--verbose', type=str, dest='verbose', 
                        default='2',
                        help="logging level")
    args = parser.parse_args()

    # action = args.action.strip()
    try: verbose = int(args.verbose.strip())
    except : verbose = 2
    if args.force_overwrite.strip().lower() == "true": force_overwrite = True
    else: force_overwrite = False
    if args.skip_tensorboard.strip().lower() == "true": skip_tensorboard = True
    else: skip_tensorboard = False
    if args.skip_png_training.strip().lower() == "true": skip_png_training = True
    else: skip_png_training = False
    if args.skip_png_prediction.strip().lower() == "true": skip_png_prediction = True
    else: skip_png_prediction = False
    try:
        assert(np.all([args.image_path!=None, args.sn_entry_path!=None, args.output_dir!=None]))
        runner(args.image_path, args.sn_entry_path, args.output_dir, 
                working_directory=args.working_directory, 
                user_resource_directory=args.user_resource_directory, 
                force_overwrite=force_overwrite,
                chromosome=args.chromosome, watcher=args.watcher,
                skip_tensorboard=skip_tensorboard, 
                skip_png_training=skip_png_training, skip_png_prediction=skip_png_prediction, 
                logging_path=args.logging_path,
                verbose=verbose)
    except Exception as e:
        print(f'SM runner failed with error {e}\n\
        {[args.image_path!=None, args.sn_entry_path!=None, args.output_dir!=None]}')
        print(f'Image path: {args.image_path}')
        print(f'Semantic Network (SN) entry file path: {args.sn_entry_path}')
        print(f'Output directory path: {args.output_dir}')
        if args.working_directory: print(f'Working directory path: {args.working_directory}')
        if args.user_resource_directory: print(f'User resource directory path: {args.user_resource_directory}')
        # if args.chromosome: log.info(f'Chromosome: {args.chromosome}')
        print('---------------------------------------------------------------')

        raise ValueError(e)
