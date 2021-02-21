
message("Bundle ${TARGET}")

file(COPY ${CONAN_QT_ROOT}/plugins/platforms/    DESTINATION ${RUNTIME_OUTPUT_DIRECTORY}/platforms)
file(COPY ${CONAN_QT_ROOT}/plugins/generic/      DESTINATION ${RUNTIME_OUTPUT_DIRECTORY}/generic)
file(COPY ${CONAN_QT_ROOT}/plugins/imageformats/ DESTINATION ${RUNTIME_OUTPUT_DIRECTORY}/imageformats)

include(BundleUtilities)
fixup_bundle("${TARGET}" "" "")
