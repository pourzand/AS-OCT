

************************************************
Resource monitoring
************************************************

CPU resource monitoring
============================================

**htop**
  >>> htop
  for more detailed information
  ctrl+c for exit

**htop options**
  https://linux.die.net/man/1/htop
  htop -u --user=USERNAME
  Show only the processes of a given user

**Interactive Commands**

The following commands are supported while in htop:

:F5 or t: Tree view: organize processes by parenthood, and layout the relations between them as a tree. Toggling the key will switch between tree and your previously selected sort view. Selecting a sort view will exit tree view.
          
          +, -
          
          When in tree view mode, expand or collapse subtree. When a subtree is collapsed a "+" sign shows to the left of the process name.

:u: Show only processes owned by a specified user.

:M: Sort by memory usage (top compatibility key).

:P: Sort by processor usage (top compatibility key).

:T: Sort by time (top compatibility key).

Example usage:
  e.g. run “htop -u youngwonchoi” and press “t”

.. image:: ./htop.png

**top**
  >>> top
  for simple and lighter way to check
  ctrl+c for exit

GPU resource monitoring
============================================

**nvidia-smi**
  >>> nvidia-smi

if you want to watch resource continuously
  >>> watch -n 1 nvidia-smi
  this will refresh nvidia-smi every 1 sec
  ctrl+c for exit
