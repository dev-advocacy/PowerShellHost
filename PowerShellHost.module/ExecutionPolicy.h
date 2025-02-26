#pragma once
#pragma once
// Define a C++ enum for ExecutionPolicy
enum class ExecutionPolicy {
    Unrestricted = 0,
    RemoteSigned = 1,
    AllSigned = 2,
    Restricted = 3,
    Default = 3, // Default - The most restrictive policy available.
    Bypass = 4,
    Undefined = 5
};
