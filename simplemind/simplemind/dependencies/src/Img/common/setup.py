import os
import subprocess
from Cython.Build import cythonize
# THIS_DIR = os.path.dirname(os.path.abspath(__file__))
THIS_DIR = os.path.join(os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__)))), 'bin')
cythonize((
        os.path.join(THIS_DIR,r"qia/common/img/statistics.pyx"),
        os.path.join(THIS_DIR,r"qia/common/img/image.pyx"), 
        os.path.join(THIS_DIR,r"qia/common/img/element.pyx"), 
        os.path.join(THIS_DIR,r"qia/common/img/overlay.pyx"),
        os.path.join(THIS_DIR,r"qia/common/img/filter.pyx"),
        os.path.join(THIS_DIR,r"qia/common/img/lut.pyx"),
        os.path.join(THIS_DIR,r"qia/common/img/measure.pyx"),
),language_level=3)
    
if 'linux' in sys.platform:
    print('cythonization complete, exiting!')
    sys.exit(0)
    
devenv_exe_path = os.path.join(os.path.dirname(os.path.dirname(os.environ['VS140COMNTOOLS'])), 'IDE', 'devenv.exe')
sln_path = os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))), 'Img.sln')
command_simple_args = [devenv_exe_path, sln_path, r'/rebuild', 'Release|x64'] 

pipe = subprocess.DEVNULL
p = subprocess.Popen(command_simple_args, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
stat = p.wait()
if stat != 0:
    print("ERROR: Execution failed: ", subprocess.list2cmdline(command_simple_args))
else:
    print('Done.')
