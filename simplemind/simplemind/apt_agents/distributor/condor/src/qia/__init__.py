import os

### technically used by qia.temp

# The following class is meant for identifying objects that encapsulates a file or path that is meant to be accessed via __str__
class FilePathObject:
    pass
    
def is_file_path_object(input):
    return isinstance(input, FilePathObject)

GLOBAL_DEBUG = False

class _DebugArtifacts:
    def __init__(self):
        self.artifacts = []

    def __del__(self):
        if self.artifacts:
            print("Warning: Following resources are not automatically deleted due to GLOBAL_DEBUG being true:")
            for i in self.artifacts:
                print(i)
                
    def append(self, item):
        self.artifacts.append(item)
        
    def __iter__(self):
        return iter(self.artifacts)

GLOBAL_DEBUG_ARTIFACTS = _DebugArtifacts()

def set_global_debug():
    global GLOBAL_DEBUG
    GLOBAL_DEBUG = True
    
def reset_global_debug():
    global GLOBAL_DEBUG
    GLOBAL_DEBUG = False
    
def is_global_debug():
    return GLOBAL_DEBUG
