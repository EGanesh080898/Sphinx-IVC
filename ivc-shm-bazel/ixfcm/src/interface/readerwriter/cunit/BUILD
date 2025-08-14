# This rule builds the ivcrw-cunit-test executable and defines it
# as a test that can be run with the `bazel test` command.

cc_test(
    name = "ivcrw-cunit-test",
    srcs = ["ivcrw-cuint-test.cpp"],
    copts = ["-g"],
    linkopts = ["-lcunit"],
    deps = [
        # Link against the main ivcreaderwriter library
        "//ixfcm/src/interface/readerwriter:ivc-readerwriter",
    ],
)
