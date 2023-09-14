##  \package qia.common.executecmd
#   Convenience functions for executing commands in shell environment.
#   MWW 08262020 - Note that shell is automatically False in Linux environment
import subprocess
import time
import os
from threading import Thread
from simplemind.apt_agents.distributor.condor.src.qia.temp import get_temp_file
from simplemind.apt_agents.distributor.condor.src.qia.exceptions import NonZeroExitCode

def special_bool(v):
    if v.lower() in ('yes', 'true', 't', '1'):
        return True
    elif v.lower() in ('no', 'false', 'f', '0'):
        return False
    elif v.lower() in ('none',):
        return None
    else:
        raise NotImplementedError('Invalid value, cannot parse!')
            
            
def _is_unc_path(path):
    temp = path.replace("\\\\?\\", "", 1)
    if temp[0:2]==r"\\":
        return True
    return False

##  Executes a command in shell environment.
#   \param[in]  args    The command and arguments to be executed. Can be either a string or a list.as
#   \param[in]  cwd     Current working directory. Default is /c None.
#   \param[in[  env     Custom environment variables. Default is /c None.
#   \param[in]  show    If \c True, will print outputs during execution. Default is \c True.
#   \param[in]  throw   If not None, raise RuntimeError if the the exit code evaluates to true. Default function evaluates to true if the exit code is not 0.
#   \returns    The exitcode of the execution.
def execute_command(args, cwd=None, env=None, show=True, shell=False, throw=lambda x: x!=0, bufsize=0,close_fds=True):
    # if cwd is not None and shell:
        # if _is_unc_path(cwd):
            # temp_mount = TempMount(cwd)
            # cwd = str(temp_mount)+"\\"
    pipe = None
    if not show:
        pipe = subprocess.DEVNULL
    p = subprocess.Popen(args, shell=shell, cwd=cwd, env=env, stdout=pipe, stderr=pipe, bufsize=bufsize)
    [outStream, errStream] = p.communicate()    
    stat = p.returncode
    if throw is not None and throw(stat):
        raise NonZeroExitCode("".join((
            "Exit code ", str(stat), " encountered when executing:\n", args if isinstance(args, str) else subprocess.list2cmdline(args)
        )))
    return stat


def _file_print(outfile, errfile, done, wait_time=None):
    if wait_time is None:
        wait_time = 0.01
    with open(outfile, "r") as o, open(errfile, "r") as e:
        for f in (o,e):
            while 1:
                line_empty = True
                where = f.tell()
                line = f.readline()
                if not line:
                    f.seek(where)
                else:
                    print(line, end="")
                    line_empty = False
                if line_empty:
                    if done[0]:
                        break
                    time.sleep(wait_time)
    
##  Executes a command in shell environment and returns the outputs generated.
#   \note   No output will be printed on screen during execution.
#   \param[in]  args    The command and arguments to be executed. Can be either a string or a list.as
#   \param[in]  cwd     Current working directory. Default is /c None.
#   \param[in]  env     Custom environment variables. Default is /c None.
#   \param[in]  stringout  If \c True, converts output to Python string format. Default is \c True.
#   \param[in]  throw   If not None, raise RuntimeError if the the exit code evaluates to true. Default function evaluates to true if the exit code is not 0.
#   \returns    A tuple containing the exitcode of the execution, output from stdout and output from stderr.
def execute_command_with_output(args, cwd=None, env=None, stringout=True, shell=False, show=True, wait_time=None, throw=lambda x: x!=0):
    # if cwd is not None and shell:
        # if _is_unc_path(cwd):
        #     temp_mount = Mount(cwd)
        #     cwd = str(temp_mount)+"\\"
    stdout_file = get_temp_file()
    stderr_file = get_temp_file()
    stdout = open(stdout_file, "wb+")
    stderr = open(stderr_file, "wb+")
    
    if show:
        done = [False]
        show_thread = Thread(target=_file_print, args=(stdout_file, stderr_file, done, wait_time))
        show_thread.start()

    # args = ["echo", "'Date: '", "wasil"]
    # args = ["condor_q",]
#    args = ["python", "/cvib2/apps/personal/wasil/lib/qia/qia_linux/temp/data/12122019_/fake_function-6092091CF1FC4CF086A40E6C399A84B07B2C3CFE6_LIDC-IDRI-0004_1576221824/script.py"]
 #   cwd = None
    # print(args, shell, cwd, stdout, stderr, shell, "\n", env, )

    p = subprocess.Popen(args, shell=shell, universal_newlines=True, cwd=cwd, env=env, stdout=stdout, stderr=stderr)
    stat = p.wait()
    stdout.seek(0)
    stderr.seek(0)
    # print(stat, "\n", stdout, "\n", stderr)
    if show:
        done[0] = True # Stops thread
        show_thread.join()

    if throw is not None and throw(stat):
        msg = stdout.read()
        err = stderr.read()
        raise NonZeroExitCode("".join((
            "Exit code ", str(stat), " encountered when executing:\n", args if isinstance(args, str) else subprocess.list2cmdline(args)
        )))
    if stringout:
        msg = stdout.read()
        stdout.close()
        err = stderr.read()
        stderr.close()
        return (stat, msg.decode("utf-8"), err.decode("utf-8"))
    else:
        return (stat, stdout, stderr)
