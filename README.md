# KextRW

A macOS kernel extension offering several features useful for security/vulnerability research against XNU.

The features provided by this kernel extension include:
* Virtual kernel read/write
* Physical read/write
* Getting the kernel base and slide
* Kernel memory allocation and freeing
* Kernel call primitive
* Address translation

The codebase is originally based on [IOKernelRW](https://github.com/Siguza/IOKernelRW), where you can find installation instructions, as they will be the same for this project. Any binary that wishes to create a userclient for this kernel extension must possess the `com.apple.security.alfie.kext-rw` entitlement. An easy-to-use test program can be found in the `tests/` folder, but the offsets and addresses are specific to my machine.

You can find the path to your kernelcache by running `kmutil inspect` - it will be printed at the top. This will be necessary if you would like to update the offsets and addresses in `kextrw_test.c`. I have not tested this on anything other than macOS 15.2.