cmake_minimum_required (VERSION 3.7.2 FATAL_ERROR)


project (msk144decoder
  VERSION 2.4.0.0
  LANGUAGES C CXX Fortran
  )

# set CMAKE_BUILD_TYPE to Release if not defined
if (NOT CMAKE_BUILD_TYPE)
  set (CMAKE_BUILD_TYPE RELEASE CACHE STRING
    "Choose the type of build, options are: None Debug Release."
    FORCE)
endif (NOT CMAKE_BUILD_TYPE)

option (SWAP_ENDIANS "Swap bytes in readed 16bits samples." OFF)

include (GNUInstallDirs)

set (CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/CMake/Modules ${CMAKE_MODULE_PATH})

find_package (FFTW3 COMPONENTS single threads REQUIRED)
find_package(CURL)

include_directories(${FFTW3_INCLUDE_DIRS})
include_directories(${CURL_INCLUDE_DIR})

# project definitions
add_definitions (-DBIGSYM=1 )
if(SWAP_ENDIANS)
  add_definitions (-DSWAP_ENDIANS=1)
endif()

set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pthread")
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --std=gnu++11 -pthread")


set (General_FFLAGS "-Wall -Wno-conversion -fno-second-underscore")
# gfortran specific settings
set (CMAKE_Fortran_FLAGS_RELEASE "${CMAKE_Fortran_FLAGS_RELEASE} -fbounds-check -funroll-all-loops -fno-f2c ${General_FFLAGS}")
set (CMAKE_Fortran_FLAGS_DEBUG   "${CMAKE_Fortran_FLAGS_DEBUG} -fbounds-check -fno-f2c ${General_FFLAGS}")


set(WSJT_BASE_DIR ${CMAKE_SOURCE_DIR}/deps/wsjtx CACHE STRING "Provide path to WSJT sorurces")


set (ka9q_CSRCS
  ${WSJT_BASE_DIR}/lib/ftrsd/decode_rs.c
  ${WSJT_BASE_DIR}/lib/ftrsd/encode_rs.c
  ${WSJT_BASE_DIR}/lib/ftrsd/init_rs.c
  )
set_source_files_properties (${ka9q_CSRCS} PROPERTIES COMPILE_FLAGS -Wno-sign-compare)


set (wsjt_msk144_CSRCS
  ${WSJT_BASE_DIR}/lib/wsprd/nhash.c
  )

set (wsjt_jt65_CSRCS
  ${WSJT_BASE_DIR}/lib/igray.c
  ${WSJT_BASE_DIR}/lib/wsprd/nhash.c
  ${WSJT_BASE_DIR}/lib/ftrsd/ftrsdap.c
  ${WSJT_BASE_DIR}/lib/wrapkarn.c
  )

set (wsjt_q65_CSRCS
  ${WSJT_BASE_DIR}/lib/qra/qracodes/qra12_63_64_irr_b.c
  ${WSJT_BASE_DIR}/lib/qra/qracodes/qra13_64_64_irr_e.c
  ${WSJT_BASE_DIR}/lib/qra/q65/npfwht.c
  ${WSJT_BASE_DIR}/lib/qra/q65/pdmath.c
  ${WSJT_BASE_DIR}/lib/qra/q65/qracodes.c
  ${WSJT_BASE_DIR}/lib/qra/q65/normrnd.c
  ${WSJT_BASE_DIR}/lib/qra/q65/qra15_65_64_irr_e23.c
  ${WSJT_BASE_DIR}/lib/qra/q65/q65.c
  ${WSJT_BASE_DIR}/lib/qra/q65/q65_subs.c
  )

set (wsjt_CXXSRCS
  ${WSJT_BASE_DIR}/lib/crc13.cpp
  )

# deal with a GCC v6 UB error message
set_source_files_properties (
  ${WSJT_BASE_DIR}/lib/crc13.cpp
  PROPERTIES COMPILE_FLAGS -fpermissive)

set (wsjt_common_FSRCS
  ${WSJT_BASE_DIR}/lib/packjt.f90
  ${WSJT_BASE_DIR}/lib/fftw3mod.f90
  ${WSJT_BASE_DIR}/lib/four2a.f90
  ${WSJT_BASE_DIR}/lib/timer_module.f90
  ${WSJT_BASE_DIR}/lib/deg2grid.f90
  ${WSJT_BASE_DIR}/lib/grid2deg.f90
  ${WSJT_BASE_DIR}/lib/fmtmsg.f90
  )


set (wsjt_msk144_FSRCS
  ${WSJT_BASE_DIR}/lib/crc.f90
  ${WSJT_BASE_DIR}/lib/hashing.f90
  ${WSJT_BASE_DIR}/lib/77bit/packjt77.f90
  ${WSJT_BASE_DIR}/lib/analytic.f90
  ${WSJT_BASE_DIR}/lib/bpdecode40.f90
  ${WSJT_BASE_DIR}/lib/bpdecode128_90.f90
  ${WSJT_BASE_DIR}/lib/ft8/chkcrc13a.f90
  ${WSJT_BASE_DIR}/lib/chkcall.f90
  ${WSJT_BASE_DIR}/lib/determ.f90
  ${WSJT_BASE_DIR}/lib/encode_msk40.f90
  ${WSJT_BASE_DIR}/lib/encode_128_90.f90
  ${WSJT_BASE_DIR}/lib/genmsk_128_90.f90
  ${WSJT_BASE_DIR}/lib/genmsk40.f90
  ${WSJT_BASE_DIR}/lib/hash.f90
  ${WSJT_BASE_DIR}/lib/indexx.f90
  ${WSJT_BASE_DIR}/lib/msk40decodeframe.f90
  ${WSJT_BASE_DIR}/lib/msk144decodeframe.f90
  ${WSJT_BASE_DIR}/lib/msk40spd.f90
  ${WSJT_BASE_DIR}/lib/msk144spd.f90
  ${WSJT_BASE_DIR}/lib/msk40sync.f90
  ${WSJT_BASE_DIR}/lib/msk144sync.f90
  ${WSJT_BASE_DIR}/lib/msk40_freq_search.f90
  ${WSJT_BASE_DIR}/lib/msk144_freq_search.f90
  ${WSJT_BASE_DIR}/lib/mskrtd.f90
  ${WSJT_BASE_DIR}/lib/msk144signalquality.f90
  ${WSJT_BASE_DIR}/lib/platanh.f90
  ${WSJT_BASE_DIR}/lib/polyfit.f90
  ${WSJT_BASE_DIR}/lib/tweak1.f90
  ${WSJT_BASE_DIR}/lib/update_recent_calls.f90
  ${WSJT_BASE_DIR}/lib/update_msk40_hasharray.f90
  )



set (wsjt_jt65_FSRCS
  ${WSJT_BASE_DIR}/lib/fchisq0.f90 
  ${WSJT_BASE_DIR}/lib/lorentzian.f90 
  ${WSJT_BASE_DIR}/lib/subtract65.f90
  ${WSJT_BASE_DIR}/lib/graycode.f90
  ${WSJT_BASE_DIR}/lib/hint65.f90
  ${WSJT_BASE_DIR}/lib/move.f90
  ${WSJT_BASE_DIR}/lib/interleave63.f90 
  ${WSJT_BASE_DIR}/lib/graycode65.f90 
  ${WSJT_BASE_DIR}/lib/chkhist.f90 
  ${WSJT_BASE_DIR}/lib/demod64a.f90 
  ${WSJT_BASE_DIR}/lib/prog_args.f90 
  ${WSJT_BASE_DIR}/lib/extract.f90
  ${WSJT_BASE_DIR}/lib/twkfreq65.f90 
  ${WSJT_BASE_DIR}/lib/ccf2.f90 
  ${WSJT_BASE_DIR}/lib/fchisq65.f90 
  ${WSJT_BASE_DIR}/lib/afc65b.f90
  ${WSJT_BASE_DIR}/lib/fil6521.f90
  ${WSJT_BASE_DIR}/lib/sh65snr.f90
  ${WSJT_BASE_DIR}/lib/sh65.f90
  ${WSJT_BASE_DIR}/lib/filbig.f90
  ${WSJT_BASE_DIR}/lib/decode65a.f90
  ${WSJT_BASE_DIR}/lib/decode65b.f90
  ${WSJT_BASE_DIR}/lib/shell.f90
  ${WSJT_BASE_DIR}/lib/db.f90
  ${WSJT_BASE_DIR}/lib/peakup.f90
  ${WSJT_BASE_DIR}/lib/pctile.f90
  ${WSJT_BASE_DIR}/lib/slope.f90
  ${WSJT_BASE_DIR}/lib/flat65.f90
  ${WSJT_BASE_DIR}/lib/xcor.f90
  ${WSJT_BASE_DIR}/lib/setup65.f90
  ${WSJT_BASE_DIR}/lib/sync65.f90
  ${WSJT_BASE_DIR}/lib/symspec65.f90
  ${WSJT_BASE_DIR}/lib/smo121.f90
  ${WSJT_BASE_DIR}/lib/jt65_mod.f90
  ${WSJT_BASE_DIR}/lib/jt65_decode.f90
  )

set (wsjt_q65_FSRCS
  ${WSJT_BASE_DIR}/lib/prog_args.f90
  ${WSJT_BASE_DIR}/lib/spec64.f90
  ${WSJT_BASE_DIR}/lib/twkfreq.f90 
  ${WSJT_BASE_DIR}/lib/ft8/ft8apset.f90
  ${WSJT_BASE_DIR}/lib/smo121.f90
  ${WSJT_BASE_DIR}/lib/chkcall.f90
  ${WSJT_BASE_DIR}/lib/indexx.f90
  ${WSJT_BASE_DIR}/lib/77bit/packjt77.f90
  ${WSJT_BASE_DIR}/lib/sec0.f90
  ${WSJT_BASE_DIR}/lib/shell.f90
  ${WSJT_BASE_DIR}/lib/db.f90
  ${WSJT_BASE_DIR}/lib/peakup.f90
  ${WSJT_BASE_DIR}/lib/pctile.f90
  ${WSJT_BASE_DIR}/lib/ana64.f90
  ${WSJT_BASE_DIR}/lib/qra/q65/q65_loops.f90
  ${WSJT_BASE_DIR}/lib/qra/q65/genq65.f90
  ${WSJT_BASE_DIR}/lib/qra/q65/q65_set_list.f90
  ${WSJT_BASE_DIR}/lib/qra/q65/q65.f90
  ${WSJT_BASE_DIR}/lib/qra/q65/q65_ap.f90
  ${WSJT_BASE_DIR}/lib/q65_decode.f90 
  )

set (project_common_SRCS 
  src/wavfile.cpp src/wavfile.h
  src/report_tasks.cpp src/report_tasks.h
  src/utils.cpp src/utils.h
  src/http_reporter.cpp src/http_reporter.h
  src/decode_result.cpp src/decode_result.h
  src/context.cpp src/context.h
  )

add_executable (msk144decoder 
  src/msk144_main.cpp 
  src/msk144_context.h src/msk144_context.cpp 
  ${project_common_SRCS}
  ${wsjt_common_FSRCS} 
  ${wsjt_msk144_FSRCS} 
  ${wsjt_CXXSRCS} 
  ${wsjt_msk144_CSRCS})
target_link_libraries (msk144decoder  ${FFTW3_LIBRARIES}  ${CURL_LIBRARIES} )

add_executable (jt65decoder 
  src/jt65_main.cpp 
  src/jt65_context.h src/jt65_context.cpp 
  ${project_common_SRCS} 
  ${wsjt_common_FSRCS} 
  ${wsjt_jt65_CSRCS} 
  ${wsjt_jt65_FSRCS}
  ${ka9q_CSRCS}
  )
target_link_libraries (jt65decoder  ${FFTW3_LIBRARIES}  ${CURL_LIBRARIES} )

add_executable (q65decoder 
  src/q65_main.cpp 
  src/q65_context.h src/q65_context.cpp 
  ${project_common_SRCS} 
  ${wsjt_common_FSRCS} 
  ${wsjt_q65_CSRCS} 
  ${wsjt_q65_FSRCS}
  )
target_link_libraries (q65decoder  ${FFTW3_LIBRARIES}  ${CURL_LIBRARIES} )

install (TARGETS msk144decoder jt65decoder q65decoder DESTINATION ${CMAKE_INSTALL_BINDIR})

source_group("wsjt_fortran" FILES ${wsjt_common_FSRCS} )
source_group("wsjt_c" FILES ${ka9q_CSRCS} ${wsjt_CXXSRCS} ${wsjt_CSRCS} ${wsjt_jt65_CSRCS})
source_group("src" FILES ${project_common_SRCS})
