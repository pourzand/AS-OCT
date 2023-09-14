.. highlight:: shell

************************************************
SM Blackboard Viewer
************************************************

The following instructions are for running on Windows.

1. *Open a Remote Desktop Connection to cvibserver0.cvib.ucla.edu

2. Login with LDAP credentials

3. Open a command window

The **SM viewer** is run as follows


.. code-block:: console
    
    C:\Python34\python.exe M:\DEVELOPMENT\MEDVIEW\dev3.4\test\miu_viewer.py \\dingo\scratch\mbrown\miu_viewer_example\seg


The argument (seg folder) should be the SM output you wish to view.

Note that the image file path in dicom.seri must be a **windows path, not Linux**

The viewer example command above should open a window as shown below.

* Scroll down through the Solution Elements to the one from the SM model you wish to view (e.g., trachea).
* If one was found, the viewer will display the roi from the best matched candidate.
* The opacity of the displayed ROI can be adjusted using the slider near the bottom of the right panel.
* If using a 3D modality, checking the "Auto navigate" box before selecting the Solution Element will automatically navigate to the slice in which the ROI occurs.

.. image:: https://gitlab.cvib.ucla.edu/qia/qia/-/raw/miu/doc/source/user_guide/viewer.png
  :width: 700


**Instructions**

* Formatting: Can select which windows to display and also drag the right pane to stretch it horizontally.
* Measurements: ctrl+drag (tick scale is also useful)
* Pan: shift+drag
* Position of cursor (in image coordinates) in the top left
* Current slice being shown for axial/coronal/Sagittal images
* Slice spacing and slice thickness (i have to double check that the slice thickness display is working properly, for this test case it didn't extract it from the header)

**Displaying Candidates**

* By default the MatchedPrimitive ROI is displayed when you select a node.
* When you load Nodule (or something with multiple roi's), you can double left-click on the ROI that you want to isolate to and it would bring it up
* If you want to go back to seeing all of the nodules (instead of the one you've isolated by double-clicking), you can press CTRL-R to reset to seeing all of the nodules. (When testing the double-click functionality, it was a bother to scroll all the way up to the top of the list of nodule ROIs to see all of the nodules, so I decided to add this in)
