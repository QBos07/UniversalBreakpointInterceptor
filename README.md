# Universal Breakpoint Interceptor (UBI)
UBI is a library designed to automaticly configure the UBC (User Break Controller) of SH4A cpus.
It tries to make setting more than 2 breakpoint simpler while allowing to execute ones own code and giving powerfull
tools to change the state at interception point. Nested exceptions can be conditionally allowed.

## Dependencies
- libstdcxx (C++ 23 compatible)
- libc

`libubi.a` is build as a fat LTO archive. A gint build will be detected using a weak definition of `ubc_setDBR` at link
time.

## Usage
Look in the example directory for a working test of UBI's capabilities. Most interactions with the library are trougth
`UBI::Singleton::instance`. The instance will be created with the program's startup (residing in the data section). You
will need to choose when it is appropriate to call the destructor. Changes to `handlers` is applied on a call to
`enable()` or `recompute_registers()`. Not changing `SPC` or disabling the point in any way in a `BeforeExecution`
handler will result in a soft lock. Sticking to one type of breakpoint should enhance performance.

### This software is currently in aplha/beta
- A C interface is yet to come.
- Performance might be bad.
- Binaries are big.