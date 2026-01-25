load("@rules_cc//cc:cc_binary.bzl", "cc_binary")
load("@rules_cc//cc:cc_library.bzl", "cc_library")


cc_library(
    name = "fuzzer_lib",
    srcs = ["fuzzer.cc"],
    hdrs = ["fuzzer.h"],
    deps = [
        "//src/simulation:harness",
    ],
    visibility = ["//visibility:public"],
)

cc_binary(
    name = "raft_fuzzer",
    srcs = ["main.cc"],
    deps = [
        ":fuzzer_lib",
    ],
)
