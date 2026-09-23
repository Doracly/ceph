# scripts
# check-generated.sh and readable.sh each fan out to $(nproc) jobs of their own,
# every job forking several concurrent ceph-dencoder processes.  Cap that
# fan-out and declare it to ctest, so neither test takes the whole machine while
# occupying a single slot.
set(CEPH_DENCODER_TEST_JOBS 8 CACHE STRING
  "number of concurrent jobs the ceph-dencoder corpus tests may use")
add_ceph_test(check-generated.sh ${CMAKE_CURRENT_SOURCE_DIR}/check-generated.sh
  MAX_PARALLEL_JOBS ${CEPH_DENCODER_TEST_JOBS})
add_ceph_test(readable.sh ${CMAKE_CURRENT_SOURCE_DIR}/readable.sh
  MAX_PARALLEL_JOBS ${CEPH_DENCODER_TEST_JOBS})
