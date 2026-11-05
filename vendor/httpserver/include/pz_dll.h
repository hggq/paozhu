#ifndef PZ_DLL_H
#define PZ_DLL_H

#include <cstdint>
#include <stdexcept>
#include <string>
#include <utility>

#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#else
#include <dlfcn.h>
#endif

#if defined(_WIN32)
#define PZ_SYMBOL_EXPORT __declspec(dllexport)
#else
#define PZ_SYMBOL_EXPORT __attribute__((visibility("default")))
#endif

// Keep the class visible across the module boundary.
#define PZ_SYMBOL_VISIBLE PZ_SYMBOL_EXPORT

// Export a C-linkage data slot named AliasName whose value is the address of FunctionOrVar.
// The loader reads that slot through dlsym/GetProcAddress and dereferences it, so only modules
// built with this macro are resolvable. A module built with the old BOOST_DLL_ALIAS exports a
// one-byte symbol under the same name (the target lives in its section name, not its value);
// dereferencing such a symbol yields a wild pointer instead of a "not found" error, so those
// modules must be rebuilt.
#if defined(_WIN32)
#define PZ_DLL_ALIAS(FunctionOrVar, AliasName)                                                   \
    namespace pz_dll_aliases                                                                     \
    {                                                                                            \
        extern "C" PZ_SYMBOL_EXPORT const void *AliasName;                                       \
        __declspec(selectany) const void *AliasName =                                            \
            reinterpret_cast<const void *>(reinterpret_cast<std::intptr_t>(&FunctionOrVar));     \
    }
#else
#define PZ_DLL_ALIAS(FunctionOrVar, AliasName)                                                   \
    namespace pz_dll_aliases                                                                     \
    {                                                                                            \
        extern "C" PZ_SYMBOL_EXPORT const void *AliasName;                                       \
        __attribute__((used, visibility("default"))) const void *AliasName =                     \
            reinterpret_cast<const void *>(reinterpret_cast<std::intptr_t>(&FunctionOrVar));     \
    }
#endif

namespace pz
{
namespace dll
{
// Minimal RAII wrapper over dlopen/dlsym/dlclose (LoadLibrary/GetProcAddress/FreeLibrary on Windows).
// Only the operations used by the framework are covered: open, has(), get_alias<T>().
class library
{
  public:
    explicit library(const std::string &path)
    {
        open(path);
    }

    ~library()
    {
        close();
    }

    library(const library &)            = delete;
    library &operator=(const library &) = delete;

    library(library &&other) noexcept
    {
        std::swap(handle_, other.handle_);
        path_ = std::move(other.path_);
    }

    library &operator=(library &&other) noexcept
    {
        if (this != &other)
        {
            close();
            std::swap(handle_, other.handle_);
            path_ = std::move(other.path_);
        }
        return *this;
    }

    bool is_open() const noexcept
    {
        return handle_ != nullptr;
    }

    bool has(const std::string &name) const noexcept
    {
        return symbol(name) != nullptr;
    }

    // Alias symbols exported by PZ_DLL_ALIAS are data slots holding the target address,
    // so the slot has to be dereferenced before it is used as a function pointer.
    template <typename T>
    T get_alias(const std::string &name) const
    {
        void *slot = symbol(name);
        if (slot == nullptr)
        {
            throw std::runtime_error("pz::dll: symbol not found: " + name + " in " + path_);
        }
        const void *target = *static_cast<const void *const *>(slot);
        if (target == nullptr)
        {
            throw std::runtime_error("pz::dll: null alias target: " + name + " in " + path_);
        }
        return reinterpret_cast<T>(const_cast<void *>(target));
    }

    const std::string &path() const noexcept
    {
        return path_;
    }

  private:
    void open(const std::string &path)
    {
        path_ = path;
#if defined(_WIN32)
        handle_ = reinterpret_cast<void *>(LoadLibraryA(path.c_str()));
        if (handle_ == nullptr)
        {
            throw std::runtime_error("pz::dll: LoadLibrary failed: " + path);
        }
#else
        handle_ = dlopen(path.c_str(), RTLD_NOW | RTLD_LOCAL);
        if (handle_ == nullptr)
        {
            const char *err = dlerror();
            throw std::runtime_error("pz::dll: dlopen failed: " + path + " : " +
                                     (err == nullptr ? "unknown error" : err));
        }
#endif
    }

    void close() noexcept
    {
        if (handle_ == nullptr)
        {
            return;
        }
#if defined(_WIN32)
        FreeLibrary(reinterpret_cast<HMODULE>(handle_));
#else
        dlclose(handle_);
#endif
        handle_ = nullptr;
    }

    void *symbol(const std::string &name) const noexcept
    {
        if (handle_ == nullptr)
        {
            return nullptr;
        }
#if defined(_WIN32)
        return reinterpret_cast<void *>(GetProcAddress(reinterpret_cast<HMODULE>(handle_), name.c_str()));
#else
        return dlsym(handle_, name.c_str());
#endif
    }

    void *handle_ = nullptr;
    std::string path_;
};
} // namespace dll
} // namespace pz

#endif
