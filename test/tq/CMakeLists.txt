add_executable(tq-perf tq_perf.cpp)
target_link_libraries(tq-perf Core)
if (BCL_COMPILER_IS_GCC_COMPATIBLE)
  target_compile_options(tq-perf PRIVATE -O3)
endif()

include(CTest)

add_executable(tq-empty tq_empty.cpp)
target_link_libraries(tq-empty Core)
add_test(tq-empty tq-empty)

add_executable(tq-eq-one tq_eq_one.cpp)
target_link_libraries(tq-eq-one Core)
add_test(tq-eq-one tq-eq-one)

add_executable(tq-eq-two tq_eq_two.cpp)
target_link_libraries(tq-eq-two Core)
add_test(tq-eq-two tq-eq-two)

add_executable(tq-lt-two-one tq_lt_two_one.cpp)
target_link_libraries(tq-lt-two-one Core)
add_test(tq-lt-two-one tq-lt-two-one)

add_executable(tq-lt-three tq_lt_three.cpp)
target_link_libraries(tq-lt-three Core)
add_test(tq-lt-three tq-lt-three)

set(TQ_PERF_TARGETS tq-perf)
set(TQ_TEST_TARGETS tq-empty tq-eq-one tq-eq-two tq-lt-two-one tq-lt-three)

set_target_properties(${TQ_PERF_TARGETS} PROPERTIES FOLDER "BCL benchmarks")
set_target_properties(${TQ_TEST_TARGETS} PROPERTIES FOLDER "BCL tests")

if(BCL_INSTALL)
  install(TARGETS ${TQ_PERF_TARGETS} ${TQ_TEST_TARGETS}
    EXPORT BCLExports DESTINATION bin)
  install(FILES tq_perf.cpp tq_empty.cpp tq_eq_one.cpp tq_eq_two.cpp
    tq_lt_two_one.cpp tq_lt_three.cpp
    DESTINATION test/tq/)
endif()
