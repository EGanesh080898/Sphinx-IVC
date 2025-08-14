# File: logger/BUILD -- THE CORRECTED VERSION
cc_library(
    name = "logger",
    hdrs = glob(["*.h", "*.hpp"]),
    includes = ["."],
    # Add this line to link this library against jsoncpp
    linkopts = ["-ljsoncpp"],
    visibility = ["//visibility:public"],
)
