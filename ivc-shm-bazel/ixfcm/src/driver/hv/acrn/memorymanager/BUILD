# File: ixfcm/src/driver/hv/acrn/memorymanager/BUILD -- PROACTIVELY CORRECTED

cc_library(
    name = "memorymanager",
    srcs = glob(["*.cpp"]),
    hdrs = glob(["*.hpp"]),
    # You are right, this is needed for other libraries that depend on this one.
    includes = ["."],
    copts = [
        "-fPIC",
        "-g",
    ],
    linkopts = ["-lpthread"],
    visibility = ["//visibility:public"],
    # This dependency is needed to fix the error you just saw.
    deps = [
        "//ixfcm/src/driver/hv/acrn/shmem-hal",
    ],
)
