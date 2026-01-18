load("@rules_cc//cc:cc_binary.bzl", "cc_binary")
load("@rules_cc//cc:cc_library.bzl", "cc_library")

cc_library(
    name = "system",
    srcs = [],
    hdrs = ["system.h"],
    deps = [
        "//src/scheduler:scheduler",
        "//src/rng:rng",
        "//src/clock:clock",
        "//src/io:io"
    ],
    visibility = ["//visibility:public"],
)