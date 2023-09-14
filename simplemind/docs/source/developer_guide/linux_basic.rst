
************************************************
Linux Basics
************************************************
Starting a screen session in ssh

* https://blog.thibaut-rousseau.com/2015/12/04/screen-terminal-multiplexer.html

.. code-block:: console

  screen -S cnn (argument is name of job, so that my job keeps going even if I end the session)
  Ctrl+d (to kill a session while attached)
  Ctrl+a d (to detach a session and keep it running)
  screen -ls (to list all sessions)
  screen -S cnn -X quit (to terminate the detached session)
  screen -x cnn (to return to the session)

.screenrc (screen setup to scroll)

.. code-block:: console

  # Enable mouse scrolling and scroll bar history scrolling
  termcapinfo xterm* ti@:te@
  source ~/.bash_profile

.bash_profile

.. code-block:: console

  # .bash_profile
  # Place as ~/.bash_profile
 
  # Get the aliases and functions
  if [ -f ~/.bashrc ]; then
    . ~/.bashrc
  fi

  # User specific environment and startup programs
  export PYTHONPATH=/cvib2/apps/personal/youngwonchoi/lib
  export MEDQIA=/cvib2/apps/personal/youngwonchoi/lib
  
  # (regacy from phd) User specific environment and startup programs
  #alias python=python3
  #alias pip=pip3
 
  #alias git=hub
  #PATH=$PATH:$HOME/tools/cling/bin
  #export PATH=~/.local/bin:/opt/openmpi-3.0.0:$PATH:$HOME/bin
  #export CPATH=$CPATH:~/.local/include
  #export LIBRARY_PATH=~/.local/lib:/usr/local/cuda/lib64:$LIBRARY_PATH
  #export LD_LIBRARY_PATH=/usr/local/lib:~/.local/lib:/usr/local/cuda/lib64:/usr/local/cuda/extras/CUPTI/lib64:$LD_LIBRARY_PATH
 
  #export CUDA_VISIBLE_DEVICES=2,3
  #export CUDA_HOME=/usr/local/cuda/
  #export CUDA_HOME=~/.local/

key cmds
  >>> cd {directory_path}
  >>> ls
    ls -l
  >>> rm {file_to_delete}
    rm -r {directory_to_delete}
  >>> cp {source} {target}
    cp -r {source_dir} {target_dir}
  >>> chmod {permission} {file or dir}
    chmod -R {permission} {dir}
    e.g. “chmod -R 777 * “ to make Windows AD/LDAP can access to all files in current directory 
  >>> cat {file}
  >>> head {file}
  >>> tail {file}

Also, recommend to use “tap”, “upper arrow”, “lower arrow” to find or complete the cmd

export parameter
  >>> export parameter=information
    e.g. export parameter sm
  >>>export sm=/cvib2/apps/personal/youngwonchoi/project/QIA/miu_stable/bin/miu/miu
    see the value of parameter with “echo”
  >>> echo $sm
    use it with ${parameter}
  >>> $sm -h

Symbolic link (similar thing to windows shortcut)
  ln -s {source} {target}
  e.g.
  >>> ln -s /scratch/youngwon dingo_youngwon_home

save log of stdout to file with "tee"
  >>> your_running_cmd | tee log_file_path

Find some text, use “greb”
  e.g.
  >>> cat log | greb youngwon

Check the number of file/dirs under some directory
  >>> ls | wc -l

Process check
  pid: The is the process ID (PID) of the process you call the Process.
  ppid: The PID of the parent process (the process that spawned the current one). 

Display process
  >>> ps -ef

display a tree of precess
  >>> ps axjf
  or
  >>> ps -ejH
  or
  >>> pstree

