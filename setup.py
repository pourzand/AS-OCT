import os
import sys
import setuptools
import skbuild
import shutil
import traceback


# NOTE: This file must remain Python 2 compatible for the foreseeable future,
# to ensure that we error out properly for people with outdated setuptools
# and/or pip.
min_version = (3, 5)
if sys.version_info < min_version:
    error = """
simplemind does not support Python {0}.{1}.
Python {2}.{3} and above is required. Check your Python version like so:
python3 --version
This may be due to an out-of-date pip. Make sure you have pip >= 9.0.1.
Upgrade pip like so:
pip install --upgrade pip
""".format(*(sys.version_info[:2] + min_version))
    sys.exit(error)

here = os.path.abspath(os.path.dirname(__file__))

with open(os.path.join(here, 'README.md'), encoding='utf-8') as readme_file:
    readme = readme_file.read()

with open(os.path.join(here, 'requirements.txt')) as requirements_file:
    # Parse requirements.txt, ignoring any commented-out lines.
    requirements = [line for line in requirements_file.read().splitlines()
                    if not line.startswith('#')]

try:
    skbuild.setup(
    # setuptools.setup(
        name='simplemind',
        description="SimpleMind: a framework for cognitive AI",
        long_description=readme,
        author="SimpleMind AI",
        author_email='simplemind.ai@gmail.com',
        url='https://gitlab.com/sm-ai-team/simplemind.git',
        python_requires='>={}'.format('.'.join(str(n) for n in min_version)),
        packages=setuptools.find_packages(exclude=['docs', 'tests']),
        entry_points={
            'console_scripts': [
                # 'command = some.module:some_function',
            ],
        },
        include_package_data=True,
        package_data={
            'simplemind': [
                # When adding files here, remember to update MANIFEST.in as well,
                # or else they will not be included in the distribution on PyPI!
                'docs/source/data/*',
                'tools/quick_start_template/*.yml',
                'tools/quick_start_template/bash/*.sh',
                'think/bin/sm/script/*.ini',
                'apt_agents/optimizer/ga/config/*/*.tpl',
                'apt_agents/tools/ga/default_metrics.yml',
            ]
        },
        install_requires=requirements,
        license="BSD (3-clause)",
        classifiers=[
            'Development Status :: 2 - Pre-Alpha',
            'Natural Language :: English',
            'Programming Language :: Python :: 3',
        ],
        cmake_install_dir='simplemind',
        cmake_args=[f'-DPCL_INCLUDE_DIR={here}/simplemind/dependencies/include/PCL/include'],
    )
except:
    traceback.print_exc()
    raise ValueError('setup.py failed at setup!')