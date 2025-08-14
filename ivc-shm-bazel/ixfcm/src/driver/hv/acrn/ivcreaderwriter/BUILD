# This rule builds the ixfcm_acrn_ivcreaderwriter static library.

cc_library(
    name = "ivc-acrn-rw",
    srcs = glob(["*.cpp"]),
    hdrs = glob(["*.hpp"]),
    includes = ["."],
    copts = [
        "-fPIC",
        "-g",
    ],
    linkopts = ["-lpthread"],
    visibility = ["//visibility:public"],
    deps = [
        # Dependency for IvcReaderWriterInterface.hpp
        "//ixfcm/src/driver/interface/readerwriter:readerwriter-interface",

        # Dependencies from project structure (likely needed by IvcAcrnRW.cpp)
        "//ixfcm/src/driver/hv/acrn/memorymanager",
        "//ixfcm/src/driver/hv/acrn/shmem-hal",
    ],
)
