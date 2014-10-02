if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  include(CheckCXXCompilerFlag)
  check_cxx_compiler_flag(--std=c++11 SUPPORTS_STD_CXX11)
  if(SUPPORTS_STD_CXX11)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -stdlib=libc++")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c++11 -stdlib=libc++")
  else()
    message(ERROR "Compiler does not support -std=c++11.")
  endif()
endif()


if (CMAKE_COMPILER_IS_GNUCXX)
  include(CheckCXXCompilerFlag)
  check_cxx_compiler_flag(--std=c++11 SUPPORTS_STD_CXX11)
  if(SUPPORTS_STD_CXX11)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c++11")
  else()
    message(ERROR "Compiler does not support -std=c++11.")
  endif()
endif()