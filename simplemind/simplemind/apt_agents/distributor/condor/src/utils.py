import warnings
import os
import subprocess
import datetime
import re
import time
import xml.etree.ElementTree as ET
import logging 

from simplemind.apt_agents.distributor.condor.src.qia.executecmd import execute_command, execute_command_with_output
from simplemind.apt_agents.distributor.condor.src.qia.exceptions import TimedOut, JobStopped, NonZeroExitCode
from simplemind.apt_agents.distributor.condor.src.qia.logging_utils import default_logger


CONDOR_UNEXPANDED=0
CONDOR_IDLE=1
CONDOR_RUNNING=2
CONDOR_REMOVED=3
CONDOR_COMPLETED=4
CONDOR_HELD=5
CONDOR_TRANSFERRING_OUTPUT=6
CONDOR_SUSPENDED=7

CONDOR_STATUS_STR = {
    CONDOR_UNEXPANDED:"unexpanded",
    CONDOR_IDLE:"idle",
    CONDOR_RUNNING:"running",
    CONDOR_REMOVED:"removed",
    CONDOR_COMPLETED:"completed",
    CONDOR_HELD:"held",
    CONDOR_TRANSFERRING_OUTPUT:"transferring output",
    CONDOR_SUSPENDED:"suspended"
}

_CLUSTER_ID_PATTERN = re.compile(r".*(submitted (.*as|to) cluster \d+).*")

UTILS_LOG = logging.getLogger("dist.condordag")   # log for this file


def get_cluster_id(msg):
    msg_list = msg.split("\n")
    msg_list.reverse()
    for i in msg_list:
        if i.strip():
            matched = _CLUSTER_ID_PATTERN.match(i.lower())
            if matched:
                return int(matched.group(1).rsplit(" ",1)[1])

def _general_execute(cmd,arg,show):
    if isinstance(arg, str):
        cmd = subprocess.list2cmdline(cmd)+" "+arg
    else:
        try:
            cmd.extend((str(i) for i in arg))
        except:
            cmd.append(str(arg))
    return execute_command(cmd, show=show)

def condor_rm(arg, show=False):
    cmd = ["condor_rm"]
    return _general_execute(cmd,arg,show)
    
def condor_hold(arg, show=False):
    cmd = ["condor_hold"]
    return _general_execute(cmd,arg,show)
    
def condor_release(arg, show=False):
    cmd = ["condor_release"]
    return _general_execute(cmd,arg,show)
    
def condor_suspend(arg, show=False):
    warnings.warn('Not tested')
    cmd = ["condor_suspend"]
    return _general_execute(cmd,arg,show)
    
def condor_continue(arg, show=False):
    warnings.warn('Not tested!')
    cmd = ["condor_continue"]
    return _general_execute(cmd,arg,show)
    
def condor_wait(log_file, wait=None, show=False):
    warnings.warn('Not tested!')
    cmd = ["condor_wait"]
    if wait is not None:
        cmd.append("-wait")
        cmd.append(str(wait))
    cmd.append(log_file)
    return execute_command(cmd, show=show)
    
def condor_submit(arg, cwd=None, show=False):
    cmd = ["condor_submit"]
    if isinstance(arg, str):
        cmd = subprocess.list2cmdline(cmd)+" "+arg
    else:
        cmd.extend(arg)
    stat, out, err = execute_command_with_output(cmd, cwd=cwd, show=show)
        
    return get_cluster_id(out)
    
def condor_submit_dag(arg, cwd=None, max_jobs=None, show=False):
    
    cmd = ["condor_submit_dag"]
    if max_jobs is not None:
        cmd.extend(["-maxjobs", str(max_jobs)])
    if isinstance(arg, str):
        cmd = subprocess.list2cmdline(cmd)+" "+arg
    else:
        cmd.extend(arg)
    stat, out, err = execute_command_with_output(cmd, cwd=cwd, show=show)

    UTILS_LOG.debug("Submitted DAG output:")
    UTILS_LOG.debug(stat)
    UTILS_LOG.debug(out)
    UTILS_LOG.debug(err)
    return get_cluster_id(out)

def condor_q(arg, show=False, log_lookup=None):
    cmd = ["condor_q", "-xml"]
    if isinstance(arg, str):
        cmd = subprocess.list2cmdline(cmd)+" "+arg
    else:
        try:
            cmd.extend((str(i) for i in arg))
        except:
            cmd.append(str(arg))
    status_dict = {}
    out = None
    # Retry section included by MWW because condor_q would freeze if too many were done at the same time
    max_retries = 5
    interval_seconds = 3
    for i in range(max_retries):
        try:
            stat, out, err = execute_command_with_output(cmd, show=show, shell=False) # may need "shell=False"
            root = ET.fromstring(out)
            break
        except NonZeroExitCode:
            UTILS_LOG.info("NonZeroExitCode caught, try #{} in {} seconds...".format(i+2, interval_seconds))
            UTILS_LOG.debug(cmd)
            time.sleep(interval_seconds)
            pass
        except FileNotFoundError:   # MWW 07012020
            out = None
            UTILS_LOG.info("File not found, try #{} in {} seconds...".format(i+2, interval_seconds))
            time.sleep(interval_seconds)
            pass
        except:
            UTILS_LOG.info("Other error caught, try #{} in {} seconds...".format(i+2, interval_seconds))
    if out is None:
        # raise("File not found or too many retries needed")
        UTILS_LOG.error("Warning: File not found or too many retries needed")
        return status_dict
    for job in root.iter("c"):
        cluster = None
        process = None
        status = None
        log = None
        for entry in job.iter("a"):
            entry_str = entry.get("n").lower()
            if entry_str=="clusterid":
                cluster = int(entry[0].text)
            elif entry_str=="procid":
                process = int(entry[0].text)
            elif entry_str=="jobstatus":
                status = int(entry[0].text)
        status_dict[(cluster, process)] = status

        if log_lookup:  #unsure of purpose
            log_lookup[(cluster, process)] = log.replace("/". os.sep)
    return status_dict   

# reading the .log file
def iter_log(log_file, id=None, pid=None):
    create_time = os.stat(log_file).st_ctime
    create_datetime = datetime.datetime.fromtimestamp(create_time)
    year = create_datetime.strftime("%Y")
    with open(log_file, "r") as f:
        info = None
        for l in f:
            line = l.strip()
            if line:
                if info is None:
                    temp = line.split(" ", 4)
                    cluster_info = temp[1][1:-1].split(".")
                    try:
                        #for windows
                        timestamp=datetime.datetime.strptime(" ".join((temp[2], year, temp[3])), "%m/%d %Y %H:%M:%S"),
                    except:
                        #for linux
                        timestamp=datetime.datetime.strptime(" ".join((temp[2], temp[3])), "%Y-%m-%d %H:%M:%S"),
                    info = dict(
                        index=int(temp[0]),
                        id=int(cluster_info[0]),
                        pid=int(cluster_info[1]),
                        timestamp=timestamp,
                        title=temp[4],
                        content=[]
                    )
                elif line=="...":
                    if id is None and pid is None:
                        UTILS_LOG.debug("<<<< id and pid is None")
                        yield info
                    else:
                        match = True
                        if id is not None and info["id"]!=id:
                            match = False
                        if pid is not None and info["pid"]!=pid:
                            match = False
                        if match:
                            yield info
                    info = None
                else:
                    info["content"].append(line)

class Process:
    def __init__(self, id, pid, status=None, log=None, auto_refresh_interval=1):
        self._id = id
        self._pid = pid
        self._log = log
        self._status = status
        self._last_refresh = time.perf_counter()
        self._auto_refresh_interval = auto_refresh_interval
        self._in_queue = True
        
    def set_auto_refresh_interval(self, val):
        self._auto_refresh_interval = val
        
    def __getattr__(self, name):
        if name=="id":
            return self._id
        elif name=="pid":
            return self._pid

        elif name == "status":
            if self._in_queue:
                self._auto_refresh()
            if self._status is None and self._in_queue:
                if self._log is None:
                    log_lookup = {}
                else:
                    log_lookup = None  # why is it None rather than {}
                # condor_q_output = condor_q("%s.%s" % (self._id, self._pid), log_lookup=log_lookup)  # why does the lookup have the pid instead of just the id
                condor_q_output = condor_q(self._id,
                                           log_lookup=log_lookup) 
                self._status = condor_q_output.get((self._id, self._pid))

                if log_lookup:
                    self._log = log_lookup.get((self._id, self._pid))
                if self._status is None:
                    self._in_queue = False
                else:
                    self._queued_before = True

                self._last_refresh = time.perf_counter()
            return self._status
            r""" MWW 07012020 -- maybe needed in Windows?
            if self._in_queue:
                self._auto_refresh()

            if self._status is None and self._in_queue:
                if self._log is None:
                    log_lookup = {}
                else:
                    log_lookup = None
                self._status = condor_q("%s.%s" % (self._id, self._pid), log_lookup=log_lookup).get((self._id, self._pid))
                if log_lookup:
                    self._log = log_lookup.get((self._id, self._pid))
                if self._status is None:
                    self._in_queue = False
                self._last_refresh = time.perf_counter()
            return self._status
            
            """
        elif name=="log":
            if self._log is None:
                self.status
            return self._log
        raise AttributeError(name)

    def _auto_refresh(self):
        if self._auto_refresh_interval is None:
            return
        if time.perf_counter()-self._last_refresh>self._auto_refresh_interval:
            self._status = None
        
    def is_active(self):
        status = self.status
        if status==CONDOR_UNEXPANDED or status==CONDOR_RUNNING or status==CONDOR_IDLE:
            return True
        return False
        
    def refresh(self):
        self._status = None
        
    def remove(self):
        warnings.warn('Not tested!')
        self.refresh()
        self.status
        if self.status is True:
            condor_rm("%s.%s" % (self._id, self._pid))
            self._status = None
            self._in_queue = False

    def resume(self):
        warnings.warn('Not tested!')
        if self.status==CONDOR_HELD:
            condor_release("%s.%s" % (self._id, self._pid))
            self.status = None
        elif self.status==CONDOR_SUSPENDED:
            condor_continue("%s.%s" % (self._id, self._pid))
            self.status = None
        
    def _is_done(self):
        if not self._in_queue or self.status==CONDOR_COMPLETED or self.status==CONDOR_REMOVED:
            return True
        
    def wait(self, timeout=None, interval_seconds=5):
        start_time = time.perf_counter()
        if self._is_done():
            return
        if self.status == CONDOR_HELD or self.status == CONDOR_SUSPENDED:
            raise JobStopped
        while True:
            if timeout is not None and time.perf_counter() - start_time > timeout:
                UTILS_LOG.warning("PROCESS TIMED OUT")
                raise TimedOut
            time.sleep(interval_seconds)

            self.refresh()
            if self._is_done():
                return

            if self.status == CONDOR_HELD or self.status == CONDOR_SUSPENDED:
                return JobStopped

class Cluster:
    def __init__(self, id, watcher):
        self._id = id
        self.watcher = watcher
        self.log = logging.getLogger("dist.condordag.cluster")
    def __getattr__(self, name):
        if name=="id":
            return self._id

    def get_processes(self):
        ret = []
        log_lookup = {}
        for k,v in condor_q(self._id, log_lookup=log_lookup).items():
            ret.append(Process(k[0], k[1], status=v, log=log_lookup.get(k)))
        return ret
        
    def __iter__(self):
        return iter(self.get_processes())
        
    def remove(self):
        condor_rm(self._id)
        
    def is_active(self):
        for p in self.get_processes():
            if p.is_active():
                return True
        return False

    def wait(self, timeout=None, interval_seconds=15): ### NOTE: This is where you set the frequency that the watcher checks the status
        start_time = time.perf_counter()
        init = True
        while True:
            # self.log.debug("x Interval seconds {}".format(str(interval_seconds)))

            ### If being first initialized, give system time to start process
            if not init:
                time.sleep(interval_seconds)
            init = False
            
            ### Check if process is done, via the method get_processes
            #   if job is held or suspended then it will raise JobStopped error (not sure how it is handled, to be honest)
            process_list = self.get_processes()
            if not process_list:
                return
            all_done = True
            for p in process_list:
                if p.status!=CONDOR_COMPLETED:
                    all_done = False
                if p.status==CONDOR_HELD or p.status==CONDOR_SUSPENDED:
                    raise JobStopped
            if all_done:
                return
            

            # self.log.debug("Checking GA watcher dir")
            if self.watcher is not None:
                # self.log.debug("{} CLUSTER {} {} | Interval seconds".format(self.watcher.watcher_dir,timeout, time.perf_counter()-start_time, interval_seconds ))
                errors = self.watcher.watch()
                if errors:
                    ### TODO: UNTESTED
                    for error in errors:
                        self.log.error("GA Watcher Fatal Error: %s"% str(error.get("ClusterId"))) ### TODO: This is probably not supposed to be warning
                    self.remove()
                    raise NonZeroExitCode

            else:
                self.log.warning("Watcher not instantiated.")
            if timeout is not None and time.perf_counter()-start_time>timeout:
                raise TimedOut
            time.sleep(interval_seconds)



### Expect that jobs is a list of dictionaries, with each dict following {"hierarchy": (int), "job_id": (str), "job_path": (str) }
def set_dag_job_hierarchy(jobs):
    # (1) get the contents
    # Determine levels of hierarchy
    hie = [b.get("hierarchy") for b in jobs] # list of hierarchy levels in order
    lookup = set()
    # below is a list of unique hierarchy levels in order (negative to positive value priority)
    # will be used as an index for organizing hierarchy in parent_lines
    levels = [x for x in hie if x not in lookup and lookup.add(x) is None]
    levels = [int(x) for x in levels]
    levels = sorted(levels)

    # Accounts for the number of transitions between parent_lines
    transitions = len(levels) - 1

    # Compile JOBS and organize PARENT:CHILD lines
    parent_lines = [dict(parents=[], children=[]) for p in range(transitions)] #prints parents and subsequent children based on hierarchy
    job_lines = [] #prints Jobs organized by job_id and job_path
    for i, j in enumerate(jobs):
        job_id = j["job_id"]
        job_path = j["job_path"]
        job_lines.append("Job %s %s"%(job_id, job_path))
        
        for n in levels:
            # if there is one hierarchy for all jobs (i.e. all 0), then there's no dependencies on each other
            # no parent_lines were initialized and will just print job_lines in contents
            # break out of loop and print jobs; otherwise, continue to sort hierarchies
            if transitions == 0:
                break

            # if 0-th index of levels -> highest hierarchy, must only be a PARENT
            if levels.index(int(j["hierarchy"])) == 0:
                parent_lines[levels.index(int(j["hierarchy"]))]["parents"].append(job_id)
                break

            # if last index of levels -> lowest hierarchy, must only be a CHILD
            if int(j["hierarchy"]) == levels[-1]:
                parent_lines[levels.index(int(j["hierarchy"])) - 1]["children"].append(job_id)
                break
            
            # if no to previous, must be a parent at current level
            # Therefore, must be a child of the previous level
            if int(j["hierarchy"]) == n:
                parent_lines[levels.index(int(j["hierarchy"]))]["parents"].append(job_id)
                parent_lines[levels.index(int(j["hierarchy"])) - 1]["children"].append(job_id)

    contents = "\n".join(job_lines)

    for parent_line in parent_lines:
        parents = " ".join(parent_line["parents"])
        children = " ".join(parent_line["children"])
        line = "PARENT %s CHILD %s"%(parents, children)
        contents+="\n"+line
    return contents
    


"""Configurator

This script contains the configurator class

"""

import configparser
import sys
 
class Configurator(object):
    def __init__(self, config_file, log):
        self.log = log
        self.load_config(config_file)
        self.time_to_wait = 300
        self.time_counter = 0
 
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



