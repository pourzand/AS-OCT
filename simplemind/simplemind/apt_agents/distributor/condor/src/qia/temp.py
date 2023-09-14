import os
import shutil
import string
import random
import tempfile
import time
from simplemind.apt_agents.distributor.condor.src.qia import FilePathObject, is_global_debug, GLOBAL_DEBUG_ARTIFACTS

DEFAULT_TEMP_DIR = tempfile.gettempdir()

def set_default_temp_dir(path):
    global DEFAULT_TEMP_DIR
    DEFAULT_TEMP_DIR = path
    
def reset_default_temp_dir(path):
    global DEFAULT_TEMP_DIR
    DEFAULT_TEMP_DIR = tempfile.gettempdir()

# This class is meant for usage by _TempFilePath only!
class _Resource:
    def __init__(self, resource, delete_on_exit=None):
        self._resource = resource
        if delete_on_exit is None:
            self._delete_on_exit = not is_global_debug()
        else:
            self._delete_on_exit = delete_on_exit
        if is_global_debug() and not self._delete_on_exit:
            GLOBAL_DEBUG_ARTIFACTS.append(resource)

    def no_delete_on_exit(self):
        self._delete_on_exit = False

    def __del__(self):
        if self._delete_on_exit:
            if os.path.exists(self._resource):
                if os.path.isdir(self._resource):
                    while os.path.exists(self._resource):
                        try:
                            shutil.rmtree(self._resource, ignore_errors=True)
                        except:
                            pass
                        if os.path.exists(self._resource):
                            time.sleep(0.05)
                else:
                    while os.path.exists(self._resource):
                        try:
                            os.remove(self._resource)
                        except:
                            pass
                        if os.path.exists(self._resource):
                            time.sleep(0.05)

    def __str__(self):
        return self._resource


class _TempFilePath(str, FilePathObject):
    def __new__(cls, str_repr, resource):
        ret_obj = str.__new__(cls, str(str_repr))
        ret_obj.__resource = resource
        return ret_obj

    def alias(self, s=None):
        if s is None:
            s = str(self.__resource)
        return _TempFilePath(s, self.__resource)
        
    def no_delete_on_exit(self):
        self.__resource.no_delete_on_exit()
    
    def __enter__(self):
        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        del self.__resource
        return False

##  A counter based string generator.
class CounterGenerator:
    ##  Constructor of the class.
    #   \param[in]  format  The format of the output string. For example \c format="test(%s)" will generate strings such as \c test(1), \c test(2) and so forth.
    #   \param[in]  start_num   A s
    def __init__(self, format="%s", start_num=0):
        self._format = format
        self._start_num = start_num

    ##  Returns the generated string.
    #   Will increment the internal counter, thus ensuring each call returns a different string.
    #   \returns    The generated string.
    def get(self):
        return_val = (self._format % self._start_num)
        self._start_num += 1
        return return_val

##  A random string generator.
class RandomStringGenerator:
    ##  Constructor of the class.
    #   \param[in]  num The length of the random string generated.
    #   \param[in]  string_list A string containing the list of valid characters for the random string generator. Default is all alpha numerical characters (both upper and lower case).
    def __init__(self, num, string_list=string.ascii_uppercase+string.ascii_lowercase+string.digits):
        self._string_list = string_list
        self._num = num

    ##  Returns a randomly generated string.
    #   \returns    A randomly generated string.
    def get(self):
        return "".join(random.choice(self._string_list) for _ in range(self._num))

##  Obtain a resource name that are guaranteed to not exist.
#   \param[in]  prefix  Prefix of the name of the temporary resource generated.
#   \param[in]  generator   The string generator for the resource name. Default is RandomStringGenerator() with 6 characters.
#   \param[in]  postfix Prefix of the name of the temporary resource generated. Default is an empty string.
#   \param[in]  must_use_generator  If \c False, will start with the resource name obtained via \c prefix+postfix, before trying \c prefix+generator.getString()+postfix. Default is \c False.
#   \param[in]  existsfunc   A function to test the existence of a generated resource name. Default test function uses \c os.path.exists() to test for existence.
#   \returns    A resource name that are guaranteed to not exist based on \c test_function.
def get_unique_name(prefix, generator=RandomStringGenerator(6), postfix="", must_use_generator=False, existsfunc=os.path.exists):
    if not must_use_generator:
        cur_string = prefix+postfix
    else:
        cur_string = prefix+generator.get()+postfix
    while existsfunc(cur_string):
        cur_string = prefix+generator.get()+postfix
    return cur_string

##  Returns a TempResource object containing a directory.
#   \param[in]  directory   The root directory where the temp directory is generated. Default is temporary directory of the operating system.
#   \param[in]  generator   The string generator for the resource name. Default is RandomStringGenerator() with 6 characters.
#   \param[in]  makedir    If \c True, the temp directory is created automatically. Default is \c True.
def get_temp_dir(directory=DEFAULT_TEMP_DIR, prefix="gtd-", postfix="", generator=RandomStringGenerator(6), makedir=True):
    temp_path = get_unique_name(os.path.join(directory, prefix), generator, postfix=postfix, must_use_generator=True)
    if makedir:
        os.makedirs(temp_path)
    return _TempFilePath(temp_path, _Resource(temp_path))

##  Returns a TempResource object containing a directory.
#   \param[in]  directory   The root directory where the temp directory is generated. Default is temporary directory of the operating system.
#   \param[in]  generator   The string generator for the resource name. Default is RandomStringGenerator() with 6 characters.
def get_temp_file(directory=DEFAULT_TEMP_DIR, prefix="gtf-", postfix="", generator=RandomStringGenerator(6)):
    temp_file = get_unique_name(os.path.join(directory, prefix), generator, postfix=postfix, must_use_generator=True)
    return _TempFilePath(temp_file, _Resource(temp_file))
