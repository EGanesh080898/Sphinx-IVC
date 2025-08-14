# File: ixfcm/src/driver/hv/acrn/shmem-hal/BUILD -- CORRECTED
cc_library(
    name = "shmem-hal",
    srcs = glob(["*.cpp"]),
    hdrs = glob(["*.hpp"]),
    # Add this line to make its headers directly includable
    includes = ["."],
    copts = [
        "-fPIC",
        "-g",
        "-fpermissive",
    ],
    linkopts = ["-lpthread"],
    visibility = ["//visibility:public"],
    deps = [
        "//logger",
    ],
)
