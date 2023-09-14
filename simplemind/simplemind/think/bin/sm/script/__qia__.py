import sys
import os

path_root = os.path.dirname(os.path.dirname(os.path.dirname(os.path.dirname(os.path.dirname(os.path.realpath(__file__))))))
path_dependencies = os.path.join(path_root,'dependencies', 'bin')
sys.path.append(path_dependencies)