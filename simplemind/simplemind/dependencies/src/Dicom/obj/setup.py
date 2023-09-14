import os,sys
import subprocess
from Cython.Build import cythonize
THIS_DIR = os.path.join(os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__)))), 'bin')
cythonize((
    os.path.join(THIS_DIR,r"qia/common/dicom/obj.pyx"),
),language_level=3)

if 'linux' in sys.platform:
    print('cythonization complete, exiting!')
    sys.exit(0)
    
devenv_exe_path = os.path.join(os.path.dirname(os.path.dirname(os.environ['VS140COMNTOOLS'])), 'IDE', 'devenv.exe')
sln_path = os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))), 'Dicom.sln')
command_with_args = [devenv_exe_path, sln_path, r'/rebuild', 'Release|x64', r'/project', 
                     r'obj\obj.vcxproj', r'/projectconfig', 'Release|x64']

pipe = subprocess.DEVNULL
p = subprocess.Popen(command_with_args, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
stat = p.wait()
if stat != 0:
    print("ERROR: Execution failed: ", subprocess.list2cmdline(command_with_args))
else:
    print('Done.')