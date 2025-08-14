# This rule uses a list comprehension to create a separate cc_binary target
# for each C++ test source file found in this directory.

[
    cc_binary(
        name = src.replace(".cpp", ""),
        srcs = [src],
        copts = ["-g"],
        deps = ["//ixfcm/src/interface/readerwriter:ivc-readerwriter"],
    )
    for src in glob(["*.cpp"])
]
