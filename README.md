# CppSynth

> Note: in its current state, this repository isn't really meant for public eyes.
> I will make it private again soon.

## Description

This is my little ongoing synthesiser/DAW project I work o from time to time. Hopefully eventually it'll grow into something greater.

Technical details:

| Spec                     | Technology                                      |
|--------------------------|-------------------------------------------------|
| Language                 | **C++20**                                        |
| Frameworks               | **OpenAL**, possibly **wxWidgets** later for UI |
| Build system             | **CMake**                                       |
| Requirements (as of now) | **MSVC**, **Powershell**                        |
| Supported platforms      | **Windows 10**                                  |

## Build

Currently I only have the Windows Debug configuration set up. Since it's essentially just a hobby of mine, the other configurations are not a priority.
I will add the other options when I feel like the project has grown big enough to warrant it.

Also since I'm developing the project on Windows, I use MSVC to compile the project.
Not sure whether using GCC or Clang would work, but, then again, I will only test that when I see the benefit of expanding to other platforms.

A regular CMake build pipeline should work:

```bash
# Clone the repository
git clone https://github.com/Ex-ce-pt/CppSynth
cd CppSynth
# Set up a build directory
mkdir build
cd build
# Build the project with CMake
cmake ..
cmake --build .
```
