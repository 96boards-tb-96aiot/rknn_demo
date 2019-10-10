
Select a BASE folder on device where Data files and Database will be kept. 
Suppose the path is /userdata/frsdk_demo/.

1. Copy b1.bin,  f300pr.bin,  f300rk.bin,  p0.bin,  p1tc.bin,  p2tc.bin,  p3rk.bin, q103rk.bin, q501rk.bin and s11rk.bin  to BASE folder /userdata/frsdk_demo
2. Create database folder "wffrdb" in BASE folder /userdata/frsdk_demo
3. Copy libraries libwfFR.so to system path where they can get loaded in program. For example to /usr/lib64/
   Copy other dependencies *.so files included in sdk if they are not present on device already
4. Use API header files FRLibrary.h and FRLibraryTypes.h in application build project. 
5. Recommended free RAM is 64 MB for 1280x720 resolution.



FR State machine for Door Lock and Intercom application
To use state machine for door lock and Intercom application, directly use testSdkStateMachine.cpp

Recognize
Sample code is in testSdkCompile.cpp in Samplecode/ folder

Enroll
Sample code is in testSdkCompile.cpp in Samplecode/ folder


After enrolling faces enrolled can be checked in database folder wffrdb in subfolders like pid0, pid1, pid2 etc 



ID Check
-----------------
Use header file FRLibraryID.h. Sample code is in testIDCheck.cpp in Samplecode/ folder


PC Enroll
-----------------
Use header file FRLibraryPC.h. Sample code is in testPCEnroll.cpp in Samplecode/ folder



