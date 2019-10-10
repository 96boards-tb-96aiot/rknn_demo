# rknn_demo_usb_intercom
- Compile with latest sdk.
- Go to sdk directory which container envsetup.sh and buildroot etc.
- source envsetup.sh
- make rknn_demo-rebuild to compile or make rknn_demo-dirclean to clean build directory.
- Source Directory : ~/RKNN/Try/linux/external/rknn_demo/
- Build Directory : ~/RKNN/Try/linux/buildroot/output/rockchip_rk1808/build/rknn_demo-1.0.0/
- Copy files from files to device and per install operations shows on output of "make rknn_demo-rebuild
- Copy b2.bin,  f300rk.bin,  p0.bin,  p1tc.bin,  p2tc.bin,  p3rk.bin,  q501rk.bin and s11rk.bin  to BASE folder /userdata/frsdk_demo (Hardcoded path in rknn_demo source)
- Create database folder "wffrdb" in BASE folder /userdata/frsdk_demo (Hardcoded path in rknn_demo source)
