load("@rules_cc//cc:cc_binary.bzl", "cc_binary")
load("@rules_cc//cc:cc_library.bzl", "cc_library")

cc_library(
    name = "executor",
    srcs = ["executor.cc"],
    hdrs = ["executor.h"],
    deps = [
        "//src/clock:clock"
    ],
    visibility = ["//visibility:public"],
)