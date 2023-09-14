
************************************************
VS Code Setup
************************************************
#. Install vs-code in your local computer  

   * https://code.visualstudio.com/

#. set ssh setting for the linux server that you want to use

   #. First install remote-explorer

      #. ctrl+shift+p and remote
      #. select Install Remote Development Extensions
      #. install Remote - SSH

   #. click the remote-explorer icon (see monitor shape in Figure 1: Remote Explorer)

   #. use + button to add new ssh host or use configure to set the ssh targets (I used ssh setting below)

      * For example, for Matt Brown to connect to supernova would be mbrown@REDLRADADM23589.ad.medctr.ucla.edu

#. open the window under any linux computing server to use

   * this will required mednet VPN connection first
   * vs-code will ask to log in using ssh setting information

#. install extensions (docker, and others that usually VS-code recommended when opening the file)
#. edit scripts by open file
#. run the scripts in terminal (recommend)

   * recommend using screen
   * then run docker container if needed
   * run the code

   
.. image:: ./remote-explorer.png
Figure 1: Remote Explorer

FYI, VS-code keyboard shortcut:

* https://code.visualstudio.com/shortcuts/keyboard-shortcuts-windows.pdf


