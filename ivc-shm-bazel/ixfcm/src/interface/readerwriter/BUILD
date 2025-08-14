# File: ixfcm/src/interface/readerwriter/BUILD -- THE CORRECTED VERSION
cc_library(
    name = "ivc-readerwriter",
    srcs = glob(["*.cpp"]),
    hdrs = glob(["*.hpp"]),
    # Add this line to expose headers to dependents like the tests
    includes = ["."],
    copts = ["-fPIC"],
    linkopts = ["-ljsoncpp"],
    visibility = ["//visibility:public"],
    deps = [
        "//ixfcm/src/driver/hv/acrn/ivcreaderwriter:ivc-acrn-rw",
        #"//ixfcm/src/driver/hv/acrn/memorymanager",
        #"//ixfcm/src/driver/hv/acrn/shmem-hal",
        "//logger",
    ],
)
