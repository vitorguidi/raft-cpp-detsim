load("@rules_cc//cc:cc_binary.bzl", "cc_binary")
load("@rules_cc//cc:cc_library.bzl", "cc_library")

cc_binary(
    name = "sleeper_simulation",
    srcs = ["sleeper_simulation.cc"],
    # Binary rules usually don't have separate 'hdrs' or 'visibility' 
    # unless they are being used as a dependency elsewhere.
    deps = [
        "//src/rng:rng",
        "//src/clock:clock",
        "//src/executor:executor",
        "//src/scheduler:scheduler",
        "//src/system:system",
        "//src/node:node",
    ],
)

cc_binary(
    name = "pinger_simulation",
    srcs = ["pinger_simulation.cc"],
    # Binary rules usually don't have separate 'hdrs' or 'visibility' 
    # unless they are being used as a dependency elsewhere.
    deps = [
        "//src/rng:rng",
        "//src/clock:clock",
        "//src/executor:executor",
        "//src/scheduler:scheduler",
        "//src/routing:routing",
        "//src/system:system",
        "//src/node:node",
    ],
)