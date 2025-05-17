
if(NOT "/app/source/cvs/third-party/fetch/libevent-subbuild/libevent-populate-prefix/src/libevent-populate-stamp/libevent-populate-gitinfo.txt" IS_NEWER_THAN "/app/source/cvs/third-party/fetch/libevent-subbuild/libevent-populate-prefix/src/libevent-populate-stamp/libevent-populate-gitclone-lastrun.txt")
  message(STATUS "Avoiding repeated git clone, stamp file is up to date: '/app/source/cvs/third-party/fetch/libevent-subbuild/libevent-populate-prefix/src/libevent-populate-stamp/libevent-populate-gitclone-lastrun.txt'")
  return()
endif()

execute_process(
  COMMAND ${CMAKE_COMMAND} -E rm -rf "/app/source/cvs/third-party/fetch/libevent-src"
  RESULT_VARIABLE error_code
  )
if(error_code)
  message(FATAL_ERROR "Failed to remove directory: '/app/source/cvs/third-party/fetch/libevent-src'")
endif()

# try the clone 3 times in case there is an odd git clone issue
set(error_code 1)
set(number_of_tries 0)
while(error_code AND number_of_tries LESS 3)
  execute_process(
    COMMAND "/usr/bin/git"  clone --no-checkout --config "advice.detachedHead=false" "https://github.com/libevent/libevent.git" "libevent-src"
    WORKING_DIRECTORY "/app/source/cvs/third-party/fetch"
    RESULT_VARIABLE error_code
    )
  math(EXPR number_of_tries "${number_of_tries} + 1")
endwhile()
if(number_of_tries GREATER 1)
  message(STATUS "Had to git clone more than once:
          ${number_of_tries} times.")
endif()
if(error_code)
  message(FATAL_ERROR "Failed to clone repository: 'https://github.com/libevent/libevent.git'")
endif()

execute_process(
  COMMAND "/usr/bin/git"  checkout release-2.1.12-stable --
  WORKING_DIRECTORY "/app/source/cvs/third-party/fetch/libevent-src"
  RESULT_VARIABLE error_code
  )
if(error_code)
  message(FATAL_ERROR "Failed to checkout tag: 'release-2.1.12-stable'")
endif()

set(init_submodules TRUE)
if(init_submodules)
  execute_process(
    COMMAND "/usr/bin/git"  submodule update --recursive --init 
    WORKING_DIRECTORY "/app/source/cvs/third-party/fetch/libevent-src"
    RESULT_VARIABLE error_code
    )
endif()
if(error_code)
  message(FATAL_ERROR "Failed to update submodules in: '/app/source/cvs/third-party/fetch/libevent-src'")
endif()

# Complete success, update the script-last-run stamp file:
#
execute_process(
  COMMAND ${CMAKE_COMMAND} -E copy
    "/app/source/cvs/third-party/fetch/libevent-subbuild/libevent-populate-prefix/src/libevent-populate-stamp/libevent-populate-gitinfo.txt"
    "/app/source/cvs/third-party/fetch/libevent-subbuild/libevent-populate-prefix/src/libevent-populate-stamp/libevent-populate-gitclone-lastrun.txt"
  RESULT_VARIABLE error_code
  )
if(error_code)
  message(FATAL_ERROR "Failed to copy script-last-run stamp file: '/app/source/cvs/third-party/fetch/libevent-subbuild/libevent-populate-prefix/src/libevent-populate-stamp/libevent-populate-gitclone-lastrun.txt'")
endif()

