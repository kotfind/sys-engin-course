#include "program.hpp"
#include "log.hpp"

#include <array>
#include <cstdio>
#include <cstring>
#include <dlfcn.h>
#include <format>
#include <fstream>
#include <string>
#include <string_view>

Program::Program(
    void* dynlib_handle,
    const std::string& source_code,
    EntryPointFn entry_point_fn
)
    : dynlib_handle(dynlib_handle), source_code(source_code),
      entry_point_fn(entry_point_fn) {
}

Program::~Program() {
    if (this->dynlib_handle != nullptr && dlclose(this->dynlib_handle) != 0) {
        error("failed to close a dynamic library: {}", dlerror());
    }
}

int Program::run(std::uint32_t size, std::uint8_t* data) const {
    return this->entry_point_fn(size, data);
}

static bool create_temdir(std::string& tmp_dir) {
    tmp_dir = "/tmp/fuse_cpu.XXXXXX";

    if (mkdtemp(tmp_dir.data()) == nullptr) {
        error("failed to create a temp directory: {}", strerror(errno));
        return false;
    }

    return true;
}

static bool write_code_file(
    const std::string& code_file, const std::string& source_code
) {

    std::ofstream code_fout{code_file};
    if (!code_fout.is_open()) {
        error("failed to open a code file");
        return false;
    }

    code_fout << source_code;
    code_fout.close();

    return true;
}

static bool compile_dynlib(
    const std::string& code_file, const std::string& dynlib_file
) {
    info("Compiling a dynamic program");

    auto cmd = std::format(
        "clang++ -shared -fPIC {} -fuse-ld=mold -O2 -g0 -o {} 2>&1",
        code_file,
        dynlib_file
    );
    info("Running: {}", cmd);

    auto* pipe = popen(cmd.c_str(), "r");
    if (pipe == nullptr) {
        error("failed to run clang: {}", strerror(errno));
        return false;
    }

    std::string clang_output;
    std::array<char, 1024> buf;
    while (fgets(buf.data(), sizeof(buf), pipe) != nullptr) {
        clang_output += buf.data();
    }

    auto clang_status = pclose(pipe);
    if (clang_status == -1) {
        error("failed to close clang process: {}", strerror(errno));
        return false;
    }

    if (clang_status != 0) {
        error(
            "clang exited with non-zero status: status={}, output=\n{}",
            clang_status,
            clang_output
        );
        return false;
    }

    success("Successfully compiled a dynamic program");
    return true;
}

Program* Program::compile(const std::string& source_code) {
    std::string tmp_dir;
    if (!create_temdir(tmp_dir)) {
        return nullptr;
    }

    auto code_file = std::format("{}/code.cpp", tmp_dir);
    auto dynlib_file = std::format("{}/libdynlib.so", tmp_dir);

    if (!write_code_file(code_file, source_code)) {
        return nullptr;
    }

    if (!compile_dynlib(code_file, dynlib_file)) {
        return nullptr;
    }

    void* dynlib_handle = dlopen(dynlib_file.c_str(), RTLD_LAZY);
    if (dynlib_handle == nullptr) {
        error("failed to load dynlib file: {}", dlerror());
        return nullptr;
    }

    auto entry_point_fn =
        (EntryPointFn)dlsym(dynlib_handle, entry_point_fn_mangled_name.data());
    if (entry_point_fn == nullptr) {
        error("failed to get entrypoint function from dynlib: {}", dlerror());
        return nullptr;
    }

    return new Program(dynlib_handle, source_code, entry_point_fn);
}

static int dummy_entry_point_fn(std::uint32_t, std::uint8_t*) {
    return 0;
}

static constexpr std::string_view dummy_entry_point_fn_code = R"(
    #include <cstdint>

    int dummy_entry_point_fn(std::uint32_t, std::uint8_t*) {
        return 0;
    }
)";

Program* Program::dummy() {
    return new Program(
        nullptr, std::string(dummy_entry_point_fn_code), dummy_entry_point_fn
    );
}
