add_executable(noisefield_tests
    test_build_info.cpp
    test_sine_oscillator.cpp
    test_white_noise.cpp
    test_noise_tint.cpp
    test_scope_buffer.cpp
    test_param_smoother.cpp
    test_soft_limiter.cpp
    test_gain.cpp
    test_level_detector.cpp)

target_link_libraries(noisefield_tests PRIVATE
    noisefield_core
    noisefield_warnings
    Catch2::Catch2WithMain)

include(Catch)
catch_discover_tests(noisefield_tests)
