A version of RTKLIB optimized for single and dual frequency low cost GPS receivers, especially u-blox receivers.  It is based on RTKLIB 2.4.3 and is kept reasonably closely synced to that branch.  Documentation for RTKLIB is available at rtklib.com.   This software is provided “AS IS” without any warranties of any kind so please be careful, especially if using it in any kind of real-time application.  

Binaries and tutorials for this code, and sample GPS data sets at http://rtkexplorer.com/  

The latest version of the user manual is at: https://github.com/rtklibexplorer/RTKLIB/blob/demo5/doc/manual_demo5.pdf

----------------------------------------------------------------

The demo5_b34_dev branch is a work in progress merging in changes from the 2.4.3 b34 version of RTKLIB.  Current status is:

GUI apps:
   - RTKLAUNCH - builds, runs, no obvious issues
   - RTKCONV   - builds, runs, no obvious issues
   - RTKPOST   - builds, runs, no obvious issues
   - RTKPLOT   - builds, runs, no obvious issues
   - STRSVR    - builds, runs, no obvious issues
   - RTKNAVI   - builds, runs, not tested
   - RTKGET    - builds, runs, not tested
   - SRCTBLBROWS - builds, runs, not tested
   
CUI apps:
   - CONVBIN  - builds, runs, not tested
   - RNX2RTKP - builds, runs, no obvious issues
   - POS2KML  - builds, runs, no obvious issues


Functional receivers/raw data formats:
   - U-blox, Novatel, Hemisphere, Skytraq, Javad, NVS, Trimble
   - Septentrino, RTCM2, RTCM3, RINEX, BINEX

Non-functional receivers/raw data formats:
   - SwiftNav, ComNav, Tersus

Non-functional features:
   - ????

Some less commonly used features, especially those unique to the demo5 code may not be fully supported yet.  Let me know if you find any features that are working in the demo5 b33 code but not in this code.  

   
To build and install code for Windows with Embarcadero compiler:

GUIs: 
1) Build executables with app/winapp/rtklib_winapp.groupproj project file 
2) Install executables and DLLs to ../RTKLIB/bin by runnning app/winapp/install_winapp.bat

CUIs:
1) Build executables with app/consapp/rtklib_consapp.groupproj project file 
2) Install executables to ../RTKLIB/bin by runnning app/consapp/install_consapp.bat
