add_executable(ps6 MACOSX_BUNDLE main.cpp)
add_library(ps6lib ps6lib.h ps6lib.cpp)
target_include_directories(ps6lib PUBLIC .)
target_link_libraries(ps6lib fslazywindow geblkernel utility	)
target_link_libraries(ps6 ps6lib)


