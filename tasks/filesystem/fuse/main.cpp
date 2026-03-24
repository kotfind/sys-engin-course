/// @mainpage %Fuse %Cpu
///
/// # Introduction
///
/// This project allows creating a "virtual cpu" with FUSE filesystem interface.
///
/// # Quick Start
///
/// First, mount the filesystem:
///
/// ```bash
/// mkdir mnt
/// fuse_cpu 3 mnt
/// ```
///
/// The directory structure will look like this:
///
/// ```
/// mnt
/// ├── unit0
/// │   ├── lram
/// │   └── pram
/// ├── unit1
/// │   ├── lram
/// │   └── pram
/// ├── unit2
/// │   ├── lram
/// │   └── pram
/// └── ctrl
/// ```
///
/// Try writing a sample c++ program to `mnt/unit0/pram` (zero unit's program
/// memory). It **must** contain a function with the following signature:
/// `int entrypoint(std::uint32_t size, std::uint8_t* data)` .
///
/// ```bash
/// cat << EOF > mnt/unit0/pram
/// #include <algorithm>
/// #include <cstdint>
///
/// int entrypoint(std::uint32_t size, std::uint8_t* data) {
///     std::sort(data, data + size);
///     return 0;
/// }
/// EOF
/// ```
///
/// Write some data to `mnt/unit0/lram` (zero unit's local memory).
///
/// ```bash
/// printf 'Hello, world!' > mnt/unit0/lram
/// ```
///
/// Now you can run it.
///
/// ```bash
/// echo '0' > mnt/ctrl
/// ```
///
/// Wait a bit and verify, that `mnt/unit0/lram` contains all the characters in
/// the sorted order.
///
/// ```bash
/// cat mnt/unit0/lram
/// # !,Hdellloorw
/// ```
///
/// # Miscellaneous
///
/// - Most of the operations are blocking.
///
///     That is, `cat ... > mnt/unit0/pram` will block until the program
///     compiles or fails to compile.
///
/// - To see all command line arguments run `fuse_cpu -h` or `fuse_cpu --help`.

#include "args.hpp"
#include "run.hpp"

/// @brief An entry point to the program.
///
/// Just calls @ref parse_args and @ref run functions.
int main(int argc, char** argv) {
    return run(parse_args(argc, argv));
}
