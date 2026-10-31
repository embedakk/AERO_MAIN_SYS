find_package(GTest REQUIRED)
enable_testing()

set(AERO_TEST_SUPPORT_DIR "${AERO_REPO_ROOT}/test_support")
set(AERO_ESP_MOCK_SRC "${AERO_TEST_SUPPORT_DIR}/esp_idf_mocks/esp_idf_mocks.c")

function(aero_add_gtest TARGET_NAME)
    cmake_parse_arguments(AERO "" "" "SOURCES" ${ARGN})
    add_executable(${TARGET_NAME}
        ${AERO_SOURCES}
        ${AERO_ESP_MOCK_SRC}
    )
    target_include_directories(${TARGET_NAME} PRIVATE
        "${AERO_TEST_SUPPORT_DIR}/esp_idf_mocks"
        "${AERO_TEST_SUPPORT_DIR}/inc_shim"
        "${AERO_REPO_ROOT}"
    )
    target_compile_options(${TARGET_NAME} PRIVATE -Wall -Wextra -Wno-unused-parameter)
    target_link_libraries(${TARGET_NAME} PRIVATE GTest::gtest_main)
    add_test(NAME ${TARGET_NAME} COMMAND ${TARGET_NAME})
endfunction()

