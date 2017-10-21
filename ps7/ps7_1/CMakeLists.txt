add_library(dha dha.h dha.cpp)
target_include_directories(dha PUBLIC .)
target_link_libraries(dha geblkernel)

add_executable(ps7_1 MACOSX_BUNDLE main.cpp glutil.cpp glutil.h)
target_link_libraries(ps7_1 fslazywindow geblkernel dha)

