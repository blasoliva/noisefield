# Small standalone developer tools. Built with the rest of the tree so they stay compilable,
# but not part of the app or the test suite.
add_executable(calibrate_noise calibrate_noise.cpp)
target_link_libraries(calibrate_noise PRIVATE noisefield_dsp noisefield_warnings)
