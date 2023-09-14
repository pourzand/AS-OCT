import glob
import os
import re
import filecmp
import warnings
# import traceback
from shutil import copy2, rmtree
from pkg_resources import require
from simplemind.apt_agents.distributor.condor.src.utils import condor_submit, condor_submit_dag, Cluster, set_dag_job_hierarchy, Configurator

condor_root = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
DEFAULT_CONDOR_CONFIG = os.path.join(condor_root, "cvib.condor")

from simplemind.apt_agents.distributor.condor.src.qia.logging_utils import default_logger, ga_watcher_logger

### formerly in QIA stringtools ###
def find_unique_pattern(string, pattern, prefix="!", postfix="!"):
    pattern = pattern
    while string.find(pattern)>=0:
        pattern = prefix+pattern+postfix
    return pattern
    
DEFAULT_AUTO_MOUNT_ONLY = DEFAULT_EXEC_ONLY = DEFAULT_CONDOR_TEMPLATE = DEFAULT_AUTO_MOUNT_EXEC = DEFAULT_AUTO_MOUNT_CONF = ""


DEFAULT_LOG_FILE = "log/$(cluster).$(process).log"
DEFAULT_ERR_FILE = "log/$(cluster).$(process).err"
DEFAULT_OUT_FILE = "log/$(cluster).$(process).out"

class _FormattedContainer(list):
    def __init__(self, joinstr, prepostfix=None, global_prepostfix=None, force=True, callback=None, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.joinstr = joinstr
        self.prepostfix = prepostfix
        self.global_prepostfix = global_prepostfix
        self.force = force
        self.callback = callback
    
    def _append_raw(self, val):
        if self.global_prepostfix is not None:
            val = val.strip().lstrip(self.global_prepostfix[0]).rstrip(self.global_prepostfix[1])
        super().append(val)
        if self.callback:
            self.callback(val)

    def _get_val(self, val):
        val = val.strip()
        if self.prepostfix is None:
            return val
        else:
            if self.force:
                return self.prepostfix[0]+val+self.prepostfix[1]
            else:
                if self.prepostfix[0]!=val[0]:
                    val = self.prepostfix[0]+val
                if self.prepostfix[1]!=val[-1]:
                    val = val+self.prepostfix[1]
        
    def append(self, val):
        if isinstance(val, str):
            val = self._get_val(val)
        else:
            val = (val[0], self._get_val(val[1]))
        super().append(val)
        if self.callback:
            self.callback(val)
            
    def extend(self, val_list):
        for i in val_list:
            self.append(i)
    
    def __str__(self):
        if not self:
            return ""
        res = []
        if self.global_prepostfix is not None:
            res.append(self.global_prepostfix[0])
        if isinstance(self[0], str):
            res.append(self[0])
        else:
            res.append(self[0][1])
        for i in range(1, len(self)):
            item = self[i]
            if isinstance(item, str):
                res.append(self.joinstr+item)
            else:
                res.append("".join(item))
        if self.global_prepostfix is not None:
            res.append(self.global_prepostfix[1])
        return "".join(res)
        

_LIST_FIELDS = {
    "dont_encrypt_input_files":lambda x: _FormattedContainer(",", callback=x),
    "dont_encrypt_output_files":lambda x: _FormattedContainer(",", callback=x),
    "encrypt_input_files":lambda x: _FormattedContainer(",", callback=x),
    "encrypt_output_files":lambda x: _FormattedContainer(",", callback=x),
    "transfer_output_files":lambda x: _FormattedContainer(",", callback=x),
    "compress_files":lambda x: _FormattedContainer(",", callback=x),
    "fetch_files":lambda x: _FormattedContainer(",", callback=x),
    "local_files":lambda x: _FormattedContainer(",", callback=x),
    "transfer_output_remaps":lambda x: _FormattedContainer(";", global_prepostfix=('"','"'), callback=x),
    "buffer_files":lambda x: _FormattedContainer(";", global_prepostfix=('"','"'), callback=x),
    "file_remaps":lambda x: _FormattedContainer(";", global_prepostfix=('"','"'), callback=x),
    "requirements":lambda x: _FormattedContainer("&&", prepostfix=('(',')'), callback=x),
    "leave_in_queue":lambda x: _FormattedContainer("&&", prepostfix=('(',')'), callback=x),
    "next_job_start_delay":lambda x: _FormattedContainer("&&", prepostfix=('(',')'), callback=x),
    "on_exit_hold":lambda x: _FormattedContainer("&&", prepostfix=('(',')'), callback=x),
    "on_exit_remove":lambda x: _FormattedContainer("&&", prepostfix=('(',')'), callback=x),
    "periodic_hold":lambda x: _FormattedContainer("&&", prepostfix=('(',')'), callback=x),
    "periodic_release":lambda x: _FormattedContainer("&&", prepostfix=('(',')'), callback=x),
    "periodic_remove":lambda x: _FormattedContainer("&&", prepostfix=('(',')'), callback=x),
}

_CONTAINER_FIELDS = set(_LIST_FIELDS.keys()) | {"transfer_input_files"}

_FIELD_ORDER = (
    "executable",
    "transfer_input_files",
    "output",
    "error",
    "log",
    "requirements",
    "universe"
)

def _makedirs(path):
    if not os.path.exists(path):
        os.makedirs(path)
        
def _double_quote_iterator(text):
    strbeg = 0
    for m in re.finditer(r'"(.*?)"', text):
        yield text[strbeg:(m.start())]
        yield m.group(0)
        strbeg = m.end()
    yield text[strbeg:]
    
def _format_argument(string):
    result = ""
    double_quote = find_unique_pattern(string, "DBQ")
    string = string.replace("\\\"", double_quote)
    for i in _double_quote_iterator(string):
        if not i:
            continue
        if i[0]=='"' and i[-1]=='"':
            if len(i)>2:
                cur = i[1:-1]
                cur = cur.replace(double_quote, '""').replace("'", "''")
                result += "'"+cur+"'"
        elif i:
            cur = i.replace("'", "''''").replace(double_quote, '""')
            result += cur
    return '"'+result+'"'
    
class _ArgumentContainer(list):
    def __init__(self, callback=None, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.callback = callback
        
    def append(self, val, raw=False):
        if not raw:
            val = _format_argument(val)
        super().append(val)
        if self.callback:
            self.callback(val)
            
    def extend(self, val_list, raw=False):
        for i in val_list:
            self.append(i, raw)
        
class _TransferFileContainer(list):
    def __init__(self, callback, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.callback = callback
        
    def _append_raw(self, val):
        for i in val.split(","):
            self.append(i.strip())

    def append(self, val, raw=False):
        added = False
        if raw: #MWW 07272020
            super().append(self.callback(val))  #MWW
            return
        for f in glob.glob(val):
            if os.path.isfile(f):
                added = True
                super().append(self.callback(os.path.abspath(f)))
        if not added:
            warnings.warn("No files found when adding transfer input file(s) via "+val)
            
    def extend(self, val_list):
        for i in val_list:
            self.append(i)
            
    def __str__(self):
        return ",".join(self)



### TODO: allow compute_env to be used here too
def make_condor_file(script_file, outside_params=None, template=DEFAULT_CONDOR_CONFIG, copy_to=None, sh_exec=None, compute_env=None, log_path=None):
    ret = new(script_file, template=template, copy_to=copy_to, log_path=log_path)
    condor_dir = os.path.dirname(sh_exec)
    ret["executable"] = sh_exec #MWW 02222021
    ret["transfer_executable"] = "True"    # probably take this out or make true?
    ret["initialdir"] = condor_dir   # probably take this out or make true?
    ret["priority"] = 15
    if compute_env is not None: 
        if compute_env.get("image") is not None: 
            # print("User-set the docker image:", compute_env["image"])
            ret["docker_image"] = compute_env["image"]
    if outside_params is not None:
        # print("outside_params: ", outside_params)
        for k,v in outside_params.items():
            if k == "condor_memory_filter":
                ret["requirements"] = "(%s) && (%s)"%(ret["requirements"], "GPUMEM >= %s"%(str(v)) )
                ret["priority"] = 20    # increases priority if there is GPU requirement
                continue
            elif k == "cpu_memory_filter":
                ret["RequestMemory"] = str(v)
                continue
            ret[k] = v
    ret.create()
    return ret


    
def prep_jobs(job_listings_file, exec_save_dir, loaded_resource, compute_env=None, log_path=None):
    with open(job_listings_file, 'r') as f:
        contents = f.read()
    job_lines = contents.split("\n")
    
    job_id = 0
    jobs = []
    hierarchy = 0
    for job_line in job_lines:
        # (1) Make .sh file
        sh_file = os.path.join(exec_save_dir, "%s.sh"%str(job_id))
        log_file = os.path.join(exec_save_dir, "%s.log"%str(job_id))
        log_finish_file = os.path.join(exec_save_dir, "%s_finish.log"%str(job_id))
        with open(sh_file, 'w') as f:
            contents = """#!/bin/bash
set -e
date
export PATH=/usr/sbin:$PATH
"""
            contents+=job_line+" 2>&1 | tee "+log_file+ "; export exitcode=${PIPESTATUS[0]}" +"\n"
            contents+="echo Exit code: $exitcode\n"
            contents+="""if [ $exitcode -gt 0 ]
then 
    exit 10
fi
"""
            contents+="touch "+log_finish_file
            contents+="\necho 'python job finished.' "
            f.write(contents)

        # (2) Make condor file
        condor_file = os.path.join(exec_save_dir, "%s.condor"%str(job_id))
        make_condor_file(condor_file, sh_exec=sh_file, outside_params=loaded_resource, compute_env=compute_env, log_path=log_path)
        # (3) Populate jobs 
        jobs.append(dict(job_id=job_id, job_path=condor_file, hierarchy=hierarchy, log_file=log_file, log_finish_file=log_finish_file))
        
        job_id+=1 
        if hierarchy < 3: 
            hierarchy+=1

    return jobs

def make_condor_dag_file(jobs, condor_dag_file,):
    contents = set_dag_job_hierarchy(jobs)
    with open(condor_dag_file, 'w') as f:
        f.write(contents)




try:
    import htcondor
    htcondor.enable_debug()
except:
    # print("Couldn't import htcondor")
    pass
import time

### Class for tracking time and alerting
### TODO: Add identifier for which job this is
class TimeTracker():
    def __init__(self, timeout=None, log=None):
        self.start_time = time.perf_counter()
        self.hours_passed = 0
        self.timeout = timeout
        self.log = log
    def alert_hour(self, print_alert=False, identifier=""):
        time_passed = self.get_time_passed()
        # print(time_passed, self.hours_passed)
        if time_passed<=0:
            return False
        if self.hours_passed > 0:
            seconds = int(time_passed) % int(self.hours_passed)
        else:
            seconds = time_passed
        if seconds > 3600:
            if self.hours_passed == 0:
                self.hours_passed = 1
                seconds -= 3600
            else:
                added_hours_passed = int(time_passed/self.hours_passed)
                self.hours_passed += added_hours_passed
            ### TODO: replace this with a better logging later
            if print_alert:
                if identifier:
                    identifier = " for job {}".format(identifier)    
                if self.log is not None:
                    self.log.warning("Alert! {} hours have passed since job submission{}!".format(str(self.hours_passed), identifier))
                else:
                    print("Alert!", self.hours_passed, "hours have passed since job submission{}!".format(identifier))
                    
            return True
        else:
            return False
    def get_time_passed(self):
        current_time = time.perf_counter()
        return current_time - self.start_time

REQUEST_MEMORY_THRESHOLDS = ["10000", # Casanova++
                            "20000", # Supernova++
                            "40000", # Lambda2++
                            "60000",
                            "80000", # Lambda2 is dynamic so you can request as much as up to 1000 Gb
                            "100000",
                            "120000",
                            ] 

GPU_MEMORY_THRESHOLDS = [       "10000", # Casanova++
                                "20000", # Supernova++
                                "40000", # Lambda2++
                                ] 

class DAG_Watcher():
    def __init__(self, dag_file, done_file=None, error_file=None, jobs=None, dag_watcher_id="watcher_job", timeout=None, global_error_path=None, resubmit_limit=3, initial_cpu_req=None, log=None):
        self.dag_file = dag_file
        self.dag_watcher_id = dag_watcher_id
        self.schedd = htcondor.Schedd()   
        self.metrics_file = dag_file+".metrics"
        self.dag_job_log = dag_file+".nodes.log"
        self.done_file = done_file
        self.error_file = error_file
        self.global_error_path = global_error_path
        self.log = log
        self._setup_log()
        self.jobs = jobs
        self.timeout = timeout
        self.start_time = time.perf_counter()
        self.times_resubmitted = 0  ### TODO: Set cap on resubmissions
        self.resubmit_limit = 3 
        self.log.debug("Timeout params set as: {} {} {}".format(self.timeout, self.start_time, self.resubmit_limit))
        self.tracker = TimeTracker(timeout=self.timeout, log=self.log)
        # self.request_memory_index = 0 ## for setting request memory threshold -- starts at 0 but increments for each 
        # 02062023, S. Wei: 
        # From now on, the request_memory_index is initialized as floored to the closest threshold that is less than initial_cpu_req
        if initial_cpu_req is not None:
            for i, threshold in enumerate(REQUEST_MEMORY_THRESHOLDS):
                if int(threshold) > initial_cpu_req:
                    self.request_memory_index = i
                    break
            self.log.info("Initial CPU memory request: %s Mb", initial_cpu_req)
            self.log.debug("Initial request_memory_index = %s", str(self.request_memory_index))
        else:
            self.request_memory_index = 0
        self.error_status = dict()
        self.error_log = ""
        self.sub_job_ids = []
        return

    def _setup_log(self):
        if self.log is None:
            if self.global_error_path is not None:
                os.makedirs(self.global_error_path, exist_ok=True)
                log_file = os.path.join(self.global_error_path, "{}.log".format(self.dag_watcher_id))
                self.log = logging.getLogger("dist.condordag.watcher")
                # self.log = ga_watcher_logger(2, filepath=log_file)
                print(log_file)
            else:
                self.log = ga_watcher_logger(2)

        return 
    def submit(self, max_jobs=5):
        
        if self.done_file is not None and os.path.exists(self.done_file):
            self.log.info("Done file exists already... skipping submission.")
                # print("Done file exists already... skipping submission.")
            self.log.debug(self.done_file)
            # print(self.done_file)
            return False
        self.cluster_id = condor_submit_dag((os.path.basename(self.dag_file),), cwd=os.path.dirname(self.dag_file), max_jobs=max_jobs)
        self.start_time = time.perf_counter()
        return self.cluster_id

    def exit(self, status):
        # print(status)
        self.log.debug(str(status))
        self.rm()
        self.error_status = status
        if status.get("HoldReason", -1) == 35:
            self.log.error("Specified Docker image was invalid. Job was held.")
            self.error_status["FatalError"] = "Specified Docker image was invalid. Job was held."
            self.error_log += "Specified Docker image was invalid. Job was held.\n"
        elif status.get("HoldReason", -1) == 34:
            self.log.error("CPU memory exceeded. Job was held.")
            self.error_status["FatalError"] = "CPU memory exceeded. Job was held."
            self.error_log += "CPU memory exceeded. Job was held.\n"
        # raise("Error: Ran into fatal error.")
        return 

    def manage_held_job(self, status):
        # print("Checking why job was held....",status.get("HoldReasonCode"),  status.get("HoldReason"), )
        self.log.warning("Checking why job was held.... {} {}".format(status.get("HoldReasonCode"),  status.get("HoldReason")) )
        self.error_log += "Job was held... figuring out reason now:"+str(status.get("HoldReasonCode"))+str(status.get("HoldReason"))+"\n"
        job_finished = False

        ### complete exit conditionals ###
        # 35 = Specified Docker image was invalid.
        if status["HoldReasonCode"] in [35,]:
            self.log.warning("Reason 35, Docker image was invalid.")
            # print("Reason 35, Docker image was invalid.")
            self.exit(status)
            job_finished = True
        ### simple release conditional ###
        # 46 = The job’s allowed duration was exceeded.
        # 47 = The job’s allowed execution time was exceeded.
        elif status["HoldReasonCode"] in [46, 47]:
            # print("Reason 46/47, Time limit exceeded.")
            self.log.warning("Reason 46/47, Time limit exceeded.")
            self.error_log += "Reason 46/47, Time limit exceeded.\n"
            if self.times_resubmitted < self.resubmit_limit:
                self.release(status["ClusterId"])
                self.times_resubmitted+=1

        ### special release with different conditions ###
        # 34 = Memory usage exceeds a memory limit.
        elif status["HoldReasonCode"] in [34,]: 
            # print("Reason 34, cpu memory exceeded.")
            self.log.warning("Reason 34, cpu memory exceeded.")
            self.error_log += "Reason 34, cpu memory exceeded.\n"
            self.increase_cpu_memory_req(status)
            
        return job_finished

    def _oom_exists(self, log_file):
        self.log.debug("Testing to see if OOM occurred.")
        self.log.debug(log_file)
        
        if not os.path.exists(log_file): return False
        with open(log_file, 'r') as f:
            lines = f.readlines()
            contents = "\n".join(lines[-100:]) # reads last 100 lines
        
        if "Resource exhausted: OOM when allocating" in contents:
            self.log.warning("Warning: OOM Error.")
            self.error_log += "Found GPU OOM error, in GA watcher log file.\n"
            return True

        return False

    def increase_gpu_memory_req(self):
        # gpu_memory_filter = None
        # # do a query to check the value of attribute foo
        # ## somehow get the current GPU memory 
        # ## maybe by a query of the characteristics
        # # requirements = self.schedd.query(
        # #         constraint=f"ClusterId == {self.cluster}",
        # #         projection=["ClusterId", "ProcId", "JobStatus", "Requirements"],
        # #     )
        # ## or by passing it to this object when initializing 

        # ## if we query the requirements, need to parse the value from the requirements string
        # # gpu_memory_filter = #do something
        # ## sometimes there won't be memory here

        # if gpu_memory_filter is not None:
        #     gpu_memory_filter *= 2 # either multiply previous or just set it to go up pre-set tiers of memory tiers we know we have
        #                             # aka casanova is 10 GB, supernova is 30GB, lambdas are 40GB (as applicable filters) 
        #     ### may need to instead overwrite the Requirements instead...
        #     self.schedd.edit(f"ClusterId == {self.cluster}", "foo", "\"bar\"")

        #     ### release the job

        ### Assuming the GPU OOM causes the job to crash, we need to read in the script.condor submit file
        ### Read in "Requirements" from job submit files
        ### Compare to established thresholds
        ### Increase threshold

        self.error_log += "\nIncreasing the GPU memory threshold.\n"
        self.log.warning("Increasing the GPU memory threshold.")
        threshold_increased = False
        for job in self.jobs:
            requirements = ""
            self.log.debug(job["job_path"])
            with open(job["job_path"], 'r') as f:
                contents = f.read()
            current_gpu_memory_req = 0
            new_gpu_memory_req = None
            search_str = None
            replace_str = None
            append_str = None
            for line in contents.split("\n"):
                components = line.split("=")
                if components[0].strip().lower() == "requirements":
                    requirements = line
                    if "GPUMEM" in requirements:
                        ### Example: || (Machine  == "REDLRADADM11249.ad.medctr.ucla.edu")) && (GPUMEM >= 30000) blah blah
                        str_start_pos = requirements.find("(GPUMEM >= ")
                        str_end_pos = requirements.find(")", str_start_pos)+1
                        current_gpu_memory_req = int(requirements[ str_start_pos+len("(GPUMEM >= )")-1: str_end_pos-1])
                        self.log.info("GPUMEM is currently set at {}.".format(str(current_gpu_memory_req)))
                        self.error_log += "GPUMEM is currently set at {}.\n".format(str(current_gpu_memory_req))
                        search_str = requirements[str_start_pos:str_end_pos]
                    
                        ### compare to established thresholds
                        replaced_threshold=False
                        for i, threshold in enumerate(GPU_MEMORY_THRESHOLDS):
                            if current_gpu_memory_req < int(threshold):
                                ### if less than existing threshold, then increase to this threshold
                                new_gpu_memory_req = threshold
                                replace_str = search_str.replace(str(current_gpu_memory_req), str(new_gpu_memory_req))
                                self.error_log += "GPUMEM is updated to be set at {}.\n".format(str(new_gpu_memory_req))
                                self.log.info("GPUMEM is updated to be set at {}.".format(str(new_gpu_memory_req)))
                                replaced_threshold = True
                                break
                        if not replaced_threshold:
                            self.error_log += "GPUMEM was not updated.\n"
                            self.log.warning("GPUMEM was not updated")

                    else:
                        requirements = line
                        new_gpu_memory_req = GPU_MEMORY_THRESHOLDS[0]
                        search_str = requirements
                        replace_str = str(search_str)+" && (GPUMEM >= %s)"%new_gpu_memory_req
                        self.error_log += "GPUMEM is added as a requirement, set at {}.\n".format(str(new_gpu_memory_req))
                        self.log.info("GPUMEM is added as a requirement, set at {}.".format(str(new_gpu_memory_req)))

                    break
            if new_gpu_memory_req is not None and search_str is not None and replace_str is not None:
                contents = contents.replace(search_str, replace_str)
                self.log.debug("Updating the condor submit file {} with new memory req: {}".format(job["job_path"], new_gpu_memory_req))
                self.error_log += "Updating the Condor submit file: {}.\n".format(job["job_path"])
                with open(job["job_path"], 'w') as f:
                    f.write(contents)
                threshold_increased = True
        return threshold_increased

    def _check_oom(self):
        ### see if GPU issue ###
        resubmitted = False
        gpu_issue = False
        # parse the log 
        for job in self.jobs:
            log_file = job["log_file"]
            gpu_issue = self._oom_exists(log_file)
            if gpu_issue: break
        if gpu_issue:
            gpu_memory_req_increased = self.increase_gpu_memory_req()
            if gpu_memory_req_increased:
                self.error_log += "GPU memory limit was successfully increased.\n"
                self.log.info("GPU memory limit was successfully increased.")
                self.log.debug("Resubmitting job.")
                self.submit()
                resubmitted = True
            else:
                self.log.debug("Couldn't resubmit job.")
                self.log.error("GPU Memory Limit led to OOM issue, and GPU requirement could not be increased any higher.")
                #report error 
                self.error_status["FatalError"] = "GPU Memory Limit led to OOM issue, and GPU requirement could not be increased any higher."
                self.error_log += "GPU Memory Limit led to OOM issue, and GPU requirement could not be increased any higher.\n"
        return resubmitted, gpu_issue

    def _check_stuck_finished_job(self):
        finished = False
        for job in self.jobs:
            log_finish_file = job["log_finish_file"]
            self.log.debug("Checking to see if job is stuck:".format(log_finish_file))
            if not os.path.exists(log_finish_file):
                return False
            else:
                self.log.warning("Job finished but is stuck: {} \n".format(log_finish_file))
                self.error_log += "Job finished but is stuck: {} \n".format(log_finish_file)
                finished = True
        return finished


    ### This should resubmit on the dag level ###
    def _manage_subjob_error(self):
        self.log.info("Managing subjob error...")
        finished = True
        
        ### see if GPU issue ###
        resubmitted, gpu_issue = self._check_oom()
        if resubmitted: finished=False

        ### section for checking any other issues ###
        
        # if Fatal Error exists then update this: self.error_status["FatalError"]

        ###########

        # gpu_issue = False
        # # parse the log 
        # for job in self.jobs:
        #     log_file = job["log_file"]
        #     gpu_issue = self._oom_exists(log_file)
        #     if gpu_issue: break
        # if gpu_issue:
        #     gpu_memory_req_increased = self.increase_gpu_memory_req()
        #     if gpu_memory_req_increased:
        #         print("Resubmitting job")
        #         self.submit()
        #         finished = False
        #     else:
        #         print("Couldn't resubmit job")
        #         #report error 
        #         self.error_status["FatalError"] = "GPU Memory Limit led to OOM issue, and GPU requirement could not be increased any higher."
        self.log.debug("Finished managing subjob...")
        return finished
        
    def _check(self):
        # 0 means finished, 1 means idle, 2 means running, 5 means held
        statuses = self.schedd.query(
            constraint=f"ClusterId == {self.cluster_id}",
            projection=["ClusterId", "HoldReason", "HoldReasonCode" "JobStatus", "LastRejMatchReason", "Requirements", "Out", "Err", "UserLog", "RequestCpus", "RequestGpus"],
        )
        


        if statuses:
            self.log.debug("Status exists for cluster {}".format(self.cluster_id))

            ### Timeout issues ###
            # Keep this on the DAG level for now
            finished = False # default   ### SHOULD BE FALSE????
            ### Check timeout if bypassed
            self.log.debug("Time passed: {}; Timeout threshold {}".format(int(time.perf_counter()) - int(self.start_time), self.timeout))
            # print(int(time.perf_counter()) - int(self.start_time), self.timeout)
            alert_triggered = self.tracker.alert_hour(print_alert=True, identifier=self.dag_watcher_id)
            if self.timeout is not None and (int(time.perf_counter()) - int(self.start_time)) > int(self.timeout):
                ### remove job, and resubmit
                self.log.warning("JOB_REMOVED: Removing job because of timeout.")
                # print("Removing job because of timeout")
                self.error_log += "JOB_REMOVED: Removing job because of timeout.\n"
                self.rm()
                if self.times_resubmitted < self.resubmit_limit:
                    self.log.debug("Resubmitting after timeout... {}".format(self.times_resubmitted))
                    self.times_resubmitted+=1
                    self.error_log += "JOB_RESUBMITTED #{}: Resubmitting after timeout.\n".format(str(self.times_resubmitted))
                    self.submit()
                else:
                    self.log.debug("Resubmit limit bypassed... just ending it.")
                    self.log.error( "Job is not resubmitted because it exceeded {} times of resubmission.\n".format(str(self.resubmit_limit)))
                    self.error_log += "Job is not resubmitted because it exceeded {} times of resubmission.\n".format(str(self.resubmit_limit))
                    # self.error_status = status
                    self.error_status["FatalError"] = "Resubmitted more than maximum of %s times."%str(self.resubmit_limit)
                    finished = True
            else:
                resubmitted, gpu_issue = self._check_oom()
                if gpu_issue and not resubmitted:   # then job should be killed.
                    self.error_log += "JOB_REMOVED: OOM Found and not resubmitted. Killing.\n"
                    self.log.error("Warning: OOM Found and not resubmitted. Killing.")
                    self.rm()
                    finished = True

                job_actually_finished = self._check_stuck_finished_job()
                if job_actually_finished:
                    self.error_log += "JOB_REMOVED: Job finished but not closed by Condor. Killing.\n"
                    self.log.warning("Alert: Job finished but not closed by Condor. Killing. {}".format( self.done_file))
                    self.rm()
                    finished = True
                    ### tempting to just do:
                    # return not finished
            ### Moved to sub-job level ###
            
            # ### Check job status if exist
            # for status in statuses:
            #     print("Job status:", status["JobStatus"])
            #     if status["JobStatus"] > 0:
            #         finished = False
            #         if "no match found " in status.get("LastRejMatchReason", ""):
            #             self.error_status = status
            #             self.error_status["FatalError"] = "Condor submission parameters led to no matched machines/nodes."
            #             self.rm()
            #             finished = True
            #             return not finished
            #             ## do stuff, pass the status along
            #     if status["JobStatus"]==5: # 5 means held
            #         print("Job status is held.")
            #         finished = self.manage_held_job(status)


        else:
            self.log.info( "After checking status through HTCONDOR, the job status of this cluster does not exist. Checking to see if it is finished or had an abnormal Exit Code.")
            self.error_log += "After checking status through HTCONDOR, the job status of this cluster does not exist. Checking to see if it is finished or had an abnormal Exit Code.\n"
            finished = True
            history = self.schedd.history(
                # constraint='true',
                constraint=f"ClusterId == {self.cluster_id}",
                # projection=['ProcId', 'ClusterId', 'JobStatus',  "ExitCode"],
                projection=["ExitCode","ClusterId"],
                match=10,  # limit to 10 returned results
                )
            self.log.debug("Before history >> ")
            for h in history:
                self.log.debug("h *** {} {}".format(h.get("ExitCode"), h.get("ClusterId")))
                ### ExitCode is for the DAG job actually, so it should have an ExitCode of 1 (not 10)
                if h.get("ExitCode") is not None:
                    if h.get("ExitCode")>0:
                        self.log.warning("Abnormal exit code: .\n".format(str(h.get("ExitCode"))))
                        self.error_log += "Abnormal exit code: .".format(str(h.get("ExitCode")))
                        finished = self._manage_subjob_error()  # may potentially resubmit this job, which redefines the cluster id and means the job is not finished
                else:
                    ### do something here if there is no exitcode... that means it was removed???? then should i remove the dag job???
                    pass
                #####
        sub_finished = True
        if os.path.exists(self.dag_job_log):
            dagman_job_events = htcondor.JobEventLog(str(self.dag_job_log)).events(stop_after=0)

            self.sub_job_ids = list(set([event.cluster for event in dagman_job_events]))
            sub_finished = self._check_sub()
        self.log.debug("main finished {} ; sub finished {} ; {}".format(finished, sub_finished, self.done_file))
        # print("main finished:", finished, "sub finished", sub_finished, self.done_file)
        return not (finished and sub_finished)

    def _check_sub(self):
        finished = True
        for sub_job_id in self.sub_job_ids:
            # 0 means finished, 1 means idle, 2 means running, 5 means held
            statuses = self.schedd.query(
                constraint=f"ClusterId == {sub_job_id}",
                # projection=["ClusterId", "ProcId", "Out", "HoldReason", "JobStatus", "LastRejMatchReason" "Requirements"],
                projection=["ClusterId", "HoldReason", "HoldReasonCode", "JobStatus", "LastRejMatchReason", "Requirements", "Out", "Err", "UserLog", "RequestCpus", "RequestGpus"],
            )
            
            for status in statuses:
                self.log.debug("Job status: {}".format(status["JobStatus"]))
                if status["JobStatus"] > 0:
                    finished = False
                    if "no match found " in status.get("LastRejMatchReason", ""):
                        self.error_status = status
                        self.error_status["FatalError"] = "Condor submission parameters led to no matched machines/nodes."
                        self.error_log += "Condor submission parameters led to no matched machines/nodes."
                        self.error_log += "JOB_HELD: Job is held until further user interaction."
                        self.log.error += "Condor submission parameters led to no matched machines/nodes."
                        self.log.error += "JOB_HELD: Job is held until further user interaction."
                        
                        ### Have it hold instead of remove the job
                        # self.rm()   ### Removes whole DAG
                        # finished = True
                        # return not finished
                        self.hold()
                        finished = True
                        return not finished
                        ## do stuff, pass the status along
                if status["JobStatus"]==5: # 5 means held
                    self.log.warning("Watcher sub-job was found to be held.")
                    self.log.warning("Cluster ID: {}".format(str(sub_job_id)))
                    self.error_log += "Watcher sub-job was found to be held. Cluster ID: {}\n".format(str(sub_job_id))
                    finished = self.manage_held_job(status)
        return finished


    def rm(self, cluster_id=None):
        if cluster_id is None:
            cluster_id = self.cluster_id
        self.log.debug("Removing job with Cluster ID: {}".format(str(cluster_id)))
        self.schedd.act(htcondor.JobAction.Remove, f"ClusterId == {cluster_id}")
        return

    def release(self, cluster_id=None):
        if cluster_id is None:
            cluster_id = self.cluster_id
        self.log.debug("Releasing job with Cluster ID: {}".format(str(cluster_id)))
        self.schedd.act(htcondor.JobAction.Release, f"ClusterId == {cluster_id}")
        return

    def hold(self, cluster_id=None):
        if cluster_id is None:
            cluster_id = self.cluster_id
        self.log.debug("Placing hold on job with Cluster ID: {}".format(str(cluster_id)))
        self.schedd.act(htcondor.JobAction.Hold, f"ClusterId == {cluster_id}")
        return

    ### do similar for disk space
    def increase_cpu_memory_req(self, status):
        if self.request_memory_index < len(REQUEST_MEMORY_THRESHOLDS):
            if self.times_resubmitted < self.resubmit_limit:
            # if True:
                # self.schedd.edit(f"ClusterId == {status['ClusterId']}", "request_memory", REQUEST_MEMORY_THRESHOLDS[self.request_memory_index])
                # print("RESUBMITTING WITH GREATER MEMORY", status['ClusterId'], "request_memory", REQUEST_MEMORY_THRESHOLDS[self.request_memory_index])
                self.schedd.edit(f"ClusterId == {status['ClusterId']}", "RequestMemory", REQUEST_MEMORY_THRESHOLDS[self.request_memory_index])
                self.log.debug("Resubmitting with greater CPU Memory.")
                self.log.debug("Cluster ID: {} ; RequestMemory: {}".format(status['ClusterId'], REQUEST_MEMORY_THRESHOLDS[self.request_memory_index]))
                # print("RESUBMITTING WITH GREATER CPU MEMORY", status['ClusterId'], "RequestMemory", REQUEST_MEMORY_THRESHOLDS[self.request_memory_index])
                self.release(status['ClusterId'])
                self.request_memory_index+=1
                self.times_resubmitted+=1
        else:
            self.exit(status)
            return 1
        return 0
    
    def _no_dag_error(self,):
        # try-except this
        if not os.path.exists(self.metrics_file):
            self.log.debug("{} doesn't exist.".format(self.metrics_file))
            return True
        self.log.debug("{} exists, examining to see if DAG exited normally.".format(self.metrics_file))
        with open(self.metrics_file, 'r') as f:
            contents = f.read()
        metrics = eval(contents)
        # print(metrics["exitcode"])
        # print(metrics["exitcode"]==0)
        # print(metrics["DagStatus"])
        # print(metrics["DagStatus"]==0)
        # print(not(metrics["exitcode"]==0 and metrics["DagStatus"]==0))
        self.log.debug("Exit Code: {} ; DagStatus: {}".format(metrics["exitcode"], metrics["DagStatus"]))
        if metrics["exitcode"]==0 and metrics["DagStatus"]==0:
            self.log.debug("Both exitcode and DagStatus are 0, so it exited normally.")
            return True
        else:
            self.log.debug("Either exitcode or DagStatus aren't 0, so it exited abnormally.")
            self.log.warning("The DAG job didn't exit properly, so suggest checking the DAG metrics file and its directory: {}".format(self.metrics_file))
            return False

    def finish(self):
        self.log.info("Finishing...")
        self.log.debug("self.error_status {}".format(self.error_status))
        self.log.debug("self.done_file {}".format(self.done_file))
        self.log.debug("self.error_file {}".format(self.error_file))
        # print("Finishing...")
        # print("self.error_status", self.error_status, self.done_file)
        # print(self.error_file,)
        finished_with_error = 0
        if not self._no_dag_error():
            self.log.warning("DAG error exists.")
            if self.error_file is not None:
                with open(self.error_file, 'a') as f:
                    f.write("DAG error exists in %s"%self.metrics_file)
                self.error_status["DAG_error"] = self.metrics_file
            finished_with_error = 1
        elif self.error_status:
            self.log.warning("There is a dag error: {}".format(self.error_status))
            self.log.debug(self.error_file)
            self.log.debug(self.global_error_path)
            if self.error_file is not None:
                with open(self.error_file, 'a') as f:
                    f.write("Fatal error: %s"%self.error_status.get("FatalError", "Unknown"))
            ### Report error to global error path too
            os.makedirs(self.global_error_path, exist_ok=True)
            global_error_file = os.path.join(self.global_error_path, "%s.err"%self.dag_watcher_id)
            with open(global_error_file, 'a') as f:
                f.write("Fatal error: %s"%self.error_status.get("FatalError", "Unknown"))
            finished_with_error = 1
        elif self.done_file is not None:
            
            self.log.info("No error detected.")
            self.log.debug(self.done_file)
            with open(self.done_file, 'w') as f:
                f.write("")
        else:
            self.log.warning("Nothing. (?)")
        return finished_with_error

# from qia.common.condor.utils import Configurator

def load_resource(model_config, log):
    ### make example job list
    """Model configuration maps"""
    config_model_class = Configurator(model_config, log)
    config_model_class.set_config_map(config_model_class.get_section_map())
    config_model = config_model_class.get_config_map()

    """Sanity check for the current model and cnn_train.py"""
    n_cpu = config_model.get('CPU', dict()).get('num_cpu_core', 1)
    gpu_list = config_model.get('GPU', dict()).get('gpu_cores', [])
    n_gpu = len(gpu_list)
    
    loaded_resources = dict(request_gpus=n_gpu,)
    if n_gpu > 0:
        condor_memory_filter = config_model.get('GPU', dict()).get('condor_memory_filter', 1)
        loaded_resources["condor_memory_filter"] = int(condor_memory_filter)
    
    cpu_memory_filter = config_model.get('CPU', dict()).get('cpu_memory_filter')
    if cpu_memory_filter is not None:
        loaded_resources["cpu_memory_filter"] = int(cpu_memory_filter)

    gpu_timeout = config_model.get('GPU', dict()).get('timeout')   # if time spent running exceeds this, job will be held
    cpu_timeout = config_model.get('CPU', dict()).get('timeout')   # if time spent running exceeds this, job will be held
    if gpu_timeout is None:
        if cpu_timeout is not None:
            loaded_resources["allowed_job_duration"] = cpu_timeout    # if time spent running exceeds this, job will be held
    else:
        loaded_resources["allowed_job_duration"] = gpu_timeout
        if cpu_timeout is not None:
            if gpu_timeout < cpu_timeout: loaded_resources["allowed_job_duration"] = cpu_timeout
    
    ### only before allowed_job_duration is implemented in latest Condor 
    loaded_resources["timeout"] = loaded_resources.get("allowed_job_duration")

    return loaded_resources


# ### Example: How to use watcher
# def initiate_watcher(job_listings_file, resource_file, log):
#     loaded_resource = load_resource(resource_file, log)

#     exec_save_dir = job_listings_file[:-4] # removes ".job"
#     os.makedirs(exec_save_dir, exist_ok=True)
#     jobs = prep_jobs(job_listings_file, exec_save_dir, loaded_resource=loaded_resource)


#     condor_dag_file = os.path.join(exec_save_dir, "dag.submit")
#     make_condor_dag_file(jobs, condor_dag_file)

#     done_file = exec_save_dir+".done"
#     error_file = exec_save_dir+".err"

#     DAG = DAG_Watcher(condor_dag_file, done_file=done_file, error_file=error_file)
#     DAG.submit()
#     DAG.watch()
#     return DAG.finish()

import glob, logging
#### PROCESS ####
class GA_Watcher():
    def __init__(self, watcher_dir, log_path, watcher_id=None, compute_env=None):
        # each chromosome has its own watcher dir
        self.watcher_dags = []
        self.watcher_dir = watcher_dir
        self.watcher_id=watcher_id
        if self.watcher_id is None: 
            self.watcher_id=os.path.basename(watcher_dir)
        self.compute_env = compute_env
        self.log = logging.getLogger("opt")
        self.dags = dict()
        self.log_path = log_path
        self.error_path = log_path
        self.error_log = os.path.join(log_path, "{}.log")
    # Watcher
    # every 30s, query glob.glob([watcher_dir]/*.job)
    def check_dir(self,):
        job_files = glob.glob(os.path.join(self.watcher_dir, "*.job"))

        # if exists, then check if previously used (persistent list of parallelized jobs) & also check if already done (/*.done)
        unsubmitted_jobs = []
        for job_file in job_files:
            if not job_file in self.watcher_dags: 
                done_file = job_file[:-4]+".done"
                if not os.path.exists(done_file):
                    ### TODO: don't automatically resubmit if there is a previous error... probably denoted by an error file or check the history
                    self.watcher_dags.append(job_file)  # so it's not initiated again
                    unsubmitted_jobs.append(job_file)
        return unsubmitted_jobs

    def submit(self, job_listings_file, resource_file):
        # if hasn't been run yet, then initiate
        loaded_resource = load_resource(resource_file, self.log)

        exec_save_dir = job_listings_file[:-4] # removes ".job"
        os.makedirs(exec_save_dir, exist_ok=True)
        
        task_name = os.path.basename(exec_save_dir)
        dag_watcher_id = "%s_%s"%(self.watcher_id,task_name)
        log_path = os.path.join(self.log_path, "watcher", dag_watcher_id)

        jobs = prep_jobs(job_listings_file, exec_save_dir, loaded_resource=loaded_resource, compute_env=self.compute_env, log_path=log_path)
        
        # /radraid/apps/personal/wasil/sandbox/ga_experiments/ga_cxr_debug/experiment/watcher/DEFAULT/trachea_cnn_weight_NA/dag.submit
        chrom = os.path.basename(os.path.dirname(exec_save_dir))
        sub_job_task = os.path.basename(exec_save_dir)
        dag_id = "%s_%s"%(chrom, sub_job_task)
        # condor_dag_file = os.path.join(exec_save_dir, "dag.submit")
        condor_dag_file = os.path.join(exec_save_dir, "%s.submit"%dag_id)
        make_condor_dag_file(jobs, condor_dag_file)
        done_file = exec_save_dir+".done"
        error_file = exec_save_dir+".err"

        dag_watcher_id = "%s_%s"%(self.watcher_id,task_name)
        DAG = DAG_Watcher(condor_dag_file, 
                          done_file=done_file, 
                          error_file=error_file, 
                          jobs=jobs, 
                          dag_watcher_id=dag_watcher_id, 
                          timeout=loaded_resource.get("timeout"), 
                          global_error_path=self.error_path,
                          initial_cpu_req=loaded_resource.get("cpu_memory_filter")
                          )
        self.log.debug("Submitting DAG")
        DAG.submit()
        self.dags[done_file] = DAG
        return DAG

    def check_dag(self,):
        delete_keys = []
        errors = []
        for k, dag in self.dags.items():
            not_finished = dag._check() # returns non-zero if still going, returns 0 if it finished
            dag_error_log = self.error_log.format(str(dag.cluster_id))
            with open(dag_error_log, 'w') as f:
                f.write(dag.error_log)
            if not not_finished:
                self.log.debug("%s finished!", k)
                finished_with_error = dag.finish()
                delete_keys.append(k)
                if finished_with_error > 0:
                    errors.append(dag.error_status)
        
        for k in delete_keys:
            del self.dags[k]
    
        return errors

    
    def watch(self,):
        
        ### Submits any new dags

        unsubmitted_jobs = self.check_dir()
        for job_listings_file in unsubmitted_jobs:
            resource_file = job_listings_file[:-4]+".resource"
            self.submit(job_listings_file, resource_file)
        
        ### Glances at any existing dags
        return self.check_dag()




class _Creator:
    def __init__(self, script_file, template=None, copy_to=None, log_path=None):
        self._script_file = os.path.abspath(script_file)
        self._work_path = os.path.dirname(self._script_file)
        if log_path is None:
            log_path = self._work_path
        os.makedirs(self._work_path, exist_ok=True)
        os.makedirs(log_path, exist_ok=True)

        self._copy_to = None
        self._modified = True
        self._cluster = None
        self._info = {
            "arguments": _ArgumentContainer(self._set_modified),
            "output": os.path.join(log_path, DEFAULT_OUT_FILE),
            "error": os.path.join(log_path, DEFAULT_ERR_FILE),
            "log": os.path.join(log_path, DEFAULT_LOG_FILE),
            "universe": "vanilla"
        }

        self.copy_to(copy_to)
        if template is not None:
            self._load_template(template)
            
    def _load_template(self, template_file):
        with open(template_file, "r") as f:
            line_count = 0
            for l in f:
                line_count += 1
                line = l.strip()
                if line:
                    temp = line.split("=", 1)
                    if len(temp)==1:    # be careful, this is ignoring "queue x"
                        pass
                        # print("Skipping invalid line encountered in", template_file, "at line", line_count)
                        # print("\t", l.strip())
                    else:
                        key = temp[0].strip().lower()
                        val = temp[1].strip()
                        self[key] = val
    def set_modified(self):
        self._modified = True
        
    def _set_modified(self, dummy):
        self.set_modified()
                            
    def cluster(self):
        return self._cluster
            
    def __getitem__(self, key):
        try:
            return self._info[key.lower()]
        except:
            key = key.lower()
            if key in _LIST_FIELDS:
                ret = _LIST_FIELDS[key](self._set_modified)
                self._info[key] = ret
                return ret
            elif key=="transfer_input_files":
                ret = _TransferFileContainer(self._transfer_input_file)
                self._info[key] = ret
                return ret
            else:
                raise
        
    def __setitem__(self, key, val):
        key = key.lower()
        if key in _CONTAINER_FIELDS:
            entry = self[key]
            entry.clear()
            entry._append_raw(val)
        elif key=="arguments":
            entry = self[key]
            entry.clear()
            entry.append(val)
        else:
            self.set_modified()
            self._info[key] = str(val)
        
    def copy_to(self, val):
        if val is None:
            self._copy_to = None
        else:
            if os.path.isabs(val):
                raise NameError("copy_to must be a relative path!")
            self._copy_to = val
            
    def _transfer_input_file(self, file):
        self.set_modified()
        if self._copy_to is not None:
            _makedirs(os.path.join(self._work_path, self._copy_to))
            filename = os.path.basename(file)
            dest_file = os.path.join(self._work_path, self._copy_to, filename)
            if os.path.isfile(dest_file) and filecmp.cmp(file, dest_file):
                #Do not copy file if the same file already exists
                pass
            else:
                copy2(file, dest_file)
            file = os.path.join(self._copy_to, filename)
        return file

    def __str__(self):
        if self._info.get("executable") is None:
            raise SyntaxError("executable is not set!")
        
        result = []
        fields = set(self._info.keys()) - set(_FIELD_ORDER)
        fields.remove("arguments")
        for k in list(_FIELD_ORDER)+sorted(fields):
            val = self._info.get(k)
            if val is not None:
                result.append("%s = %s" % (k, str(val)))
                
        arguments = self._info["arguments"]
        if arguments:
            for i in arguments:
                result.append("arguments = "+i)
                result.append("queue\n")
        else:
            result.append("queue\n")
        
        return "\n".join(result)
    
    # creates the condor submit file 
    def create(self):
        if self._cluster is not None:
            raise ValueError("Job is already submitted!")
        if self._modified:
            self._modified = False
            with open(self._script_file, "w") as f:
                f.write(str(self))
        os.makedirs(os.path.dirname(self["output"]), exist_ok=True)
        os.makedirs(os.path.dirname(self["error"]), exist_ok=True)
        os.makedirs(os.path.dirname(self["output"]), exist_ok=True)
        return self._script_file
    def submit(self):
        if self._cluster is not None:
            raise ValueError("Job is already submitted!")
        self.create()
        cluster_id = condor_submit((os.path.basename(self._script_file),), cwd=self._work_path)
        self._cluster = Cluster(cluster_id)
        return self._cluster
    def submit_dag(self, max_jobs=15, watcher_dir=None, log_path=None, compute_env=None):  # NOTE: 09072021 Change max jobs per chromosome here
        if self._cluster is not None:
            raise ValueError("Job is already submitted!")
        cluster_id = condor_submit_dag((os.path.basename(self._script_file),), cwd=self._work_path, max_jobs=max_jobs)
        watcher = None
        if watcher_dir is not None:
            if os.path.exists(watcher_dir):
                ## TODO: Turn this into debug print statement
                # print("Attempting to remove watcher dir")
                try:
                    rmtree(watcher_dir)
                except Exception:
                    # print(traceback.print_exc())
                    time.sleep(15)
                    # print("Attempting to remove watcher dir AGAIN")
                    rmtree(watcher_dir, ignore_errors=True, onerror=print("Warning: Couldn't delete one of the files (probably the MIU log file or the log dir) in watcher dir (probably non-fatal)."))
                    # rmtree(watcher_dir) 
                # print("Attempting to remove watcher dir FINISHED")
            os.makedirs(watcher_dir, exist_ok=True)
            print("WATCHER DIR:", watcher_dir)
            watcher = GA_Watcher(watcher_dir, log_path=log_path, compute_env=compute_env)
        self._cluster = Cluster(cluster_id, watcher=watcher)
        return self._cluster
"""
Make sure to transfer

"""


### Depracated 073122 MWW
# def new_bat_file(script_file, bat_exec=None, auto_mount_only=DEFAULT_AUTO_MOUNT_ONLY, condor_exec_only=DEFAULT_EXEC_ONLY, venv_bat="", os_env=""):
#     script_dir = os.path.dirname(script_file)
#     if bat_exec is None:
#         bat_exec = os.path.join(script_dir, "run.bat")
#     if venv_bat is None: venv_bat=""
#     if venv_bat:
#         venv_bat = "call "+venv_bat
#     # input_file = os.path.join(script_dir, "input")
#     # output_file = os.path.join(script_dir, "output")
#     py_file = os.path.join(script_dir, "script.py")

#     contents = """
# python %s %s
# %s
# python %s %s
# """ % (os.path.basename(auto_mount_only), os_env, venv_bat, os.path.basename(condor_exec_only), py_file)
#     with open(bat_exec, 'w') as f:
#         f.write(contents)
#     return bat_exec


def new_sh_file(sh_exec, script_file, venv_bat="", gpu_cores=None):
    script_dir = os.path.dirname(script_file)
    if sh_exec is None:
        sh_exec = os.path.join(script_dir, "run.sh")
    py_file = os.path.join(script_dir, "script.py")
    set_gpus = ""
    if gpu_cores is not None:
        set_gpus = "export CUDA_VISIBLE_DEVICES=%s"%",".join([str(i) for i in gpu_cores])
    contents = """#!/bin/bash
set -e
date
export PATH=/usr/sbin:$PATH
%s
%s
python %s || exit 10
wait
date
echo "Done"
""" % (set_gpus, venv_bat, os.path.basename(py_file))
    with open(sh_exec, 'w') as f:
        f.write(contents)
    return sh_exec


def new(script_file, template=True, copy_to=None, log_path=None):
    if template is True:
        template = DEFAULT_CONDOR_TEMPLATE
    return _Creator(script_file, template=template, copy_to=copy_to, log_path=log_path)

def new_sh(script_file, template=DEFAULT_CONDOR_CONFIG, copy_to=None, sh_exec=None, compute_env=None, log_path=None):
    ret = new(script_file, template=template, copy_to=copy_to, log_path=log_path)
    sh_exec = new_sh_file(sh_exec, script_file)
    # ret["executable"] = os.path.basename(sh_exec) #MWW 02222021
    condor_dir = os.path.dirname(sh_exec)
    ret["executable"] = sh_exec #MWW 02222021
    ret["transfer_executable"] = "True"    # probably take this out or make true?
    ret["initialdir"] = condor_dir   # probably take this out or make true?
    ret["transfer_input_files"].append(os.path.join(condor_dir,"script.py"), raw=True)  # MWW 03222020
    ret["transfer_input_files"].append(os.path.join(condor_dir,"input"),  raw=True)  # MWW 03222020
    if compute_env is not None: 
        if compute_env.get("image") is not None: 
            # print("User-set the docker image:", compute_env["image"])
            ret["docker_image"] = compute_env["image"]
    ret.copy_to(copy_to)
    return ret

