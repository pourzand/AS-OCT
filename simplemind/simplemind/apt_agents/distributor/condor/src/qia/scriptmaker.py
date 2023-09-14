import os
import sys
import inspect
import pickle
from simplemind.apt_agents.distributor.condor.src.qia.exceptions import NotReady

SCRIPT_FILENAME = "script.py"
INPUT_FILENAME = "input"
OUTPUT_FILENAME = "output"
ERROR_FILENAME = "error"

def retrieve(outpath):
    result_file = os.path.join(outpath, OUTPUT_FILENAME)
    if not os.path.exists(result_file):
        print(result_file)
        print("Error: Output file retrieved too early or doesn't exist.")
        raise NotReady     #MWW
        # return NotReady   #MWW 083020
    with open(result_file, "rb") as f:
        try:
            return pickle.load(f)
        except:
            print("Failed to load:", result_file)
            raise("Failed to Load.")
            
class ScriptObject:
    def __init__(self, path):
        self._path = path
        
    def __getattr__(self, name):
        if name=="script":
            return os.path.join(self._path, SCRIPT_FILENAME)
        elif name=="value":
            return retrieve(self._path)
        raise AttributeError(name)
       
def is_same(outpath, func, *args, **kwargs):
    if not os.path.exists(os.path.join(outpath, INPUT_FILENAME)):
        return False
    input = pickle.dumps((func, args, kwargs))
    with open(os.path.join(outpath, INPUT_FILENAME), "rb") as f:
        if f.read()==input:
            return True
    return False
       
def get_import_statement(func):
    if func.__module__=="__main__":
        main_globals = sys.modules["__main__"].__dict__
        main_file = os.path.abspath(main_globals["__file__"])
        return "from %s import %s" % (os.path.basename(main_file).rsplit(".", 1)[0], func.__name__)
    else:
        return "from %s import %s" % (func.__module__, func.__name__)
    
def make(outpath, prepcode, func, *args, **kwargs):
    outpath = os.path.abspath(outpath)
    if not os.path.exists(outpath):
        os.makedirs(outpath)
    main_globals = sys.modules["__main__"].__dict__
    try: main_file = os.path.abspath(main_globals["__file__"])
    except: pass
    
    with open(os.path.join(outpath, INPUT_FILENAME), "wb") as f:
        pickle.dump((func, args, kwargs), f)
    
    script = []
    script.append("import sys, os")
    try: script.append("sys.path.append(%s)" % repr(os.path.dirname(main_file)))
    except: pass
    if func.__module__!="__main__":
        # Importing the all important dunder files
        for k,v in main_globals.items():
            if k!="__builtins__" and k.startswith("__") and k.endswith("__") and inspect.ismodule(v):
                script.append("import %s" % v.__name__)
    
    try:
        if func.__module__=="__main__":
            script.append("from %s import %s" % (os.path.basename(main_file).rsplit(".", 1)[0], func.__name__))
        else:
            script.append("from %s import %s" % (func.__module__, func.__name__))
    except:
        script.append("from %s import %s" % (func.__module__, func.__name__))
    if prepcode is not None:
        if isinstance(prepcode, str):
            script.append(prepcode)
        else:
            script.extend(prepcode)

    script.append("""
import pickle

from simplemind.apt_agents.distributor.condor.src.qia.exceptions import ErrorValue
root_path = os.path.dirname(os.path.abspath(__file__))
input_file = os.path.join(root_path, %s)
output_file = os.path.join(root_path, %s)
try:
    with open(input_file, "rb") as f:
        input = pickle.load(f)
    output = input[0](*input[1], **input[2])
    with open(output_file, "wb") as f:
        pickle.dump(output, f)
except:
    with open(output_file, "wb") as f:
        pickle.dump(ErrorValue(sys.exc_info()), f)
    raise
    """ % (repr(INPUT_FILENAME), repr(OUTPUT_FILENAME)))
    script_file = os.path.join(outpath, SCRIPT_FILENAME)
    with open(script_file, "w") as f:
        f.write("\n".join(script))
    return ScriptObject(outpath)