# vim: syntax=cmake
cmake_minimum_required(VERSION 2.8.11)
add_compile_options(-g -rdynamic -ldl -funwind-tables -fPIC -O0)

include_directories(.)
include_directories(./include)
include_directories(./common)
include_directories(./ui)
include_directories(./rknn/rknn_api)
include_directories(./rknn/ssd)
set(RKNN_DEMO_SRC
      rknn_camera.c
      common/rknn_msg.c
      common/v4l2camera.c
      common/yuv.c
      common/buffer.c
      common/device_name.c
      ui/ui_res.c
    )

add_definitions(-DNEED_RKNNAPI=${NEED_RKNNAPI})

include_directories(./rknn/ssd/ssd_1808)
set(RKNNAPI_SRC rknn/ssd/ssd_1808/ssd.c)
#        rknn/ssd/ssd_1808/ssd_post.c)


set(ENABLE_SSD 1)

add_definitions(-DENABLE_SSD=${ENABLE_SSD})
include_directories(./ui/ssd)
set(RKNN_MODEL_SRC
  ui/ssd/ssd_ui.c
  ${RKNNAPI_SRC}
)

# set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -fsanitize=address -fsanitize-recover=address -static-libasan")

link_libraries("libv4l2.so")
link_libraries("libjpeg.so")
link_libraries("libm.so")
link_libraries("libdrm.so")
link_libraries("libminigui_ths.so")
link_libraries("libpng12.so")
link_libraries("libpthread.so")
link_libraries("librga.so")
link_libraries("libfreetype.so")
link_libraries("librkisp.so")
link_libraries("${CMAKE_SOURCE_DIR}/libwfFR.so")

add_executable(rknn_demo ${RKNN_DEMO_SRC} ${RKNN_MODEL_SRC})

install(FILES libwfFR.so DESTINATION lib)
install(FILES libjpeg.so DESTINATION lib)
install(FILES libjpeg.so.8 DESTINATION lib)
install(FILES libjpeg.so.8.1.2 DESTINATION lib)
install(FILES FR/b2.bin DESTINATION share/rknn_demo/frsdk_demo)
install(FILES FR/f300rk.bin DESTINATION share/rknn_demo/frsdk_demo)
install(FILES FR/p0.bin DESTINATION share/rknn_demo/frsdk_demo)
install(FILES FR/p1tc.bin DESTINATION share/rknn_demo/frsdk_demo)
install(FILES FR/p2tc.bin DESTINATION share/rknn_demo/frsdk_demo)
install(FILES FR/p3rk.bin DESTINATION share/rknn_demo/frsdk_demo)
install(FILES FR/q103rk.bin DESTINATION share/rknn_demo/frsdk_demo)
install(FILES FR/q501rk.bin DESTINATION share/rknn_demo/frsdk_demo)
install(FILES FR/s11rk.bin DESTINATION share/rknn_demo/frsdk_demo)
install(DIRECTORY FR/wffrdb DESTINATION share/rknn_demo/frsdk_demo)
install(DIRECTORY FR/wffrdbpc DESTINATION share/rknn_demo/frsdk_demo)

install(FILES ${MINIGUI_CFG} DESTINATION ../etc RENAME MiniGUI.cfg)
install(DIRECTORY minigui DESTINATION local/share)
install(DIRECTORY resource DESTINATION local/share/rknn_demo)

#for qt desktop
# install(FILES ui/qt/terminator.desktop DESTINATION /usr/share/applications/)

install(TARGETS rknn_demo RUNTIME DESTINATION bin)
