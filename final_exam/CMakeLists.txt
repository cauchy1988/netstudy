file (GLOB FINAL_SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
add_library (final-source-lib ${FINAL_SOURCE_FILES})

add_executable(final_exam_server main.cpp)
target_link_libraries(final_exam_server final-source-lib)
