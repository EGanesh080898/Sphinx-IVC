# This rule uses a list comprehension to create a separate cc_binary target
# for each C++ source file found in this directory.

[
    cc_binary(
        name = src.replace(".cpp", ""),
        srcs = [src],
        copts = [
            "-fpermissive",
            "-g",
        ],
        # NOTE: Your CMake links against 'ixfcm_ivcreaderwriter', but these
        # tests are for the 'shmem-hal' library. The dependency below is
        # the more logical choice.
        deps = ["//ixfcm/src/driver/hv/acrn/shmem-hal"],
    )
    for src in glob(["*.cpp"])
]
