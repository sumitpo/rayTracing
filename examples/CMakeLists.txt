# examples/CMakeLists.txt
SET(TARGET_DEMO raytracer-demo)
FIND_PACKAGE(argtable3 CONFIG REQUIRED)
ADD_EXECUTABLE(${TARGET_DEMO} demo.c cli.c)
TARGET_LINK_LIBRARIES(${TARGET_DEMO} PRIVATE raytracer-c log4c::log4c
                                             argtable3::argtable3)

# ASan for example (if enabled in parent)
IF(ENABLE_ASAN)
  TARGET_COMPILE_OPTIONS(${TARGET_DEMO} PRIVATE -fsanitize=address
                                                -fno-omit-frame-pointer -g)
  TARGET_LINK_OPTIONS(${TARGET_DEMO} PRIVATE -fsanitize=address)
ENDIF()

INSTALL(
  TARGETS ${TARGET_DEMO}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})
