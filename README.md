<p align="center">
  <img width="600" src="assets/logo.jpg">
  <p align="center">
	<img alt="license" src="https://img.shields.io/badge/license-GPL-blue">
  <img alt="bazel" src="https://img.shields.io/badge/Build%20Tool-Bazel-green">
  <img alt="zephyr" src="https://img.shields.io/badge/RTOS-Zephyr-blueviolet">
  </p>
</p>
<br>

## About
Since at this point in time, there is very little information available on how to take an already existing bazel project, build it via the ARM toolchain and then link it into a Zephyr project, this repository will give you a brief overview on how you can achieve this goal.

This approach is based on the [bazel-embedded](https://github.com/bazelembedded/bazel-embedded) project, since this is by far the simplest way to compile the appropriate ARM binary. Please take a look at this project if you want to get further information on how you can use bazel to not only build the project itself but also to flash your ARM device.

## Structure
To be able to create an easy to follow hands-on example, the simplest way is to split the whole tutorial into three steps:
- **Step 0**: Library Code
- **Step 1**: Creation of the ARM binary with Bazel
- **Step 2**: Integration of the ARM binary into Zephyr 

## Step 0: Library Code
Since every hands-on example requires code, I opted to create a very simple and understandable calculator library called `CalculationMachine`, which basically can only add two integer numbers and return the result as specified in the `int sum(int a, int b);` function.

This `CalculationMachine` is split into a header file and a source file as you can see below.

##### calculationMachine.hh
```c++
class CalculationMachine {
    public:
        int sum(int a, int b);
};
```

##### calculationMachine.cc
```c++
#include "calculationMachine.hh"

int CalculationMachine::sum(int a, int b){
   return a + b;
}
```

Now we need to tie everything together by creating a `BUILD` file, which tells Bazel what to compile. This file can be as simple as this:
```py
cc_library(
    name = "calculationMachine",
    srcs = ["calculationMachine.cc"],
    hdrs = ["calculationMachine.hh"],
)
```

If we now also create an empty file called `WORKSPACE`, Bazel has everything which is needed to build the library and now you can test it out via the following command:
```bash
bazel build //... 
```

**Note:** You can find the whole `CalculationMachine` code of this step in the corresponding folder `Step0`.

## Step 1: Creation of the ARM binary with Bazel
Since the created `libcalculationMachine.a` file by Bazel in the Step 0 is built with the host platform compiler/toolchain but we need to have a binary compiled on the ARM toolchain, we now have to get help from the [bazel-embedded](https://github.com/bazelembedded/bazel-embedded) project and make some changes.

First, add the following code to the `WORKSPACE` file
```py
load("@bazel_tools//tools/build_defs/repo:git.bzl", "git_repository")

git_repository(
    name = "bazel_embedded",
    commit = "d3cbe4eff9a63d3dee63067d61096d681daca33b",
    remote = "https://github.com/silvergasp/bazel-embedded.git",
    shallow_since = "1585022166 +0800",
)

load("@bazel_embedded//:bazel_embedded_deps.bzl", "bazel_embedded_deps")

bazel_embedded_deps()

load("@bazel_embedded//platforms:execution_platforms.bzl", "register_platforms")

register_platforms()

load(
    "@bazel_embedded//toolchains/compilers/gcc_arm_none_eabi:gcc_arm_none_repository.bzl",
    "gcc_arm_none_compiler",
)

gcc_arm_none_compiler()

load("@bazel_embedded//toolchains/gcc_arm_none_eabi:gcc_arm_none_toolchain.bzl", "register_gcc_arm_none_toolchain")

register_gcc_arm_none_toolchain()

load("@bazel_embedded//tools/openocd:openocd_repository.bzl", "openocd_deps")

openocd_deps()
```

Then, create a `.bazelrc` file and add the following lines:
```
# Enable toolchain resolution with cc
build --incompatible_enable_cc_toolchain_resolution
```

Now, the ARM toolchain integration should be complete and depending on the exact ARM chip (nrf5340 in my case) you are using, you can build the library via the following command:
```bash
bazel build //... --platforms=@bazel_embedded//platforms:cortex_m3     
```

🎉  we successfully compiled the `libcalculationMachine.a` on the ARM toolchain for the corresponding ARM chip!

**Note:** You can find the whole `CalculationMachine` code of this step in the corresponding folder `Step1`.

## Step 2: Integration of the ARM binary into Zephyr 
Generally speaking, to be able to integrate a static library (in this case) into Zephyr, we need to tell the `CMakeLists.txt` file where the `.a` file and the corresponding header file is located. 

To make things easy, I assume you use a standard Zephyr Hello World example. There I created a folder named `lib` in the root of the Zephyr project, into which  I copied the previously generated `libcalculationMachine.a` file and the `calculationMachine.hh` header file.

Now you should have a project structure which looks like this:
```txt
├── lib
│   ├── calculationMachine.hh
│   └── libcalculationmachine.a
│
├── src
│   └── main.cpp
│
├── CMakeLists.txt
│
└── prj.conf

```

To tie everything together, we now need to change the `CMakeLists.txt` file in the following way:
```cmake
cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(bazel2zephyr)

FILE(GLOB app_sources src/*.cpp)
target_sources(app PRIVATE ${app_sources})

set(LIB_DIR  ${CMAKE_CURRENT_SOURCE_DIR}/lib)
include_directories(${LIB_DIR})

add_library(calculationMachine STATIC IMPORTED GLOBAL)
set_target_properties(calculationMachine PROPERTIES IMPORTED_LOCATION ${LIB_DIR}/libcalculationMachine.a)
target_link_libraries(app PUBLIC calculationMachine)
```
If you want to find out why and how this is done in this way, please feel free to read PrimalCortex's great article about [static libraries on a Zephyr RTOS Project](https://primalcortex.wordpress.com/2021/03/13/using-static-libraries-on-a-zephyr-rtos-project/).

🎉  we successfully integrated the `CalculationMachine` library into the Zephyr project!

The last step is now to include the header file and all the `CalculationMachine` library as any other library:
```c++
#include <zephyr.h>
#include <sys/printk.h>
#include "calculationMachine.hh"

void main(void) {
	while(1) {
		CalculationMachine machine = CalculationMachine();
		printk("Sum of 5 and 3 = %d\n", machine.sum(5, 3));
		k_sleep(K_MSEC(1000));
	}
}
```
**Note:** You can find the whole `CalculationMachine` Zephyr code of this step in the corresponding folder `Step2`.

## Limitations

This Software is provided as-is!

Please feel free to adapt it to your needs and contribute to the project. I would be very grateful to include your improvements. Thanks for your support!

**WARNING:** Everyone is responsible for what he/she is doing! I am not responsible if you hurt yourself, torch your house or anything that kind trying to do build something new! You are doing everything at your own risk!
