#ifndef __HTTP_PZ_FILEIO_H
#define __HTTP_PZ_FILEIO_H

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

// 落盘用的文件描述符接口: Windows CRT 只有下划线版本, POSIX 用同名版本
// SEEK_SET/SEEK_END 由 <cstdio> 提供, MSVC 的 <io.h>/<fcntl.h> 不带
#include <cstdio>
#include <cstdint>
#ifdef _WIN32
#include <io.h>
#include <fcntl.h>
#include <share.h>

// _sopen_s 签名: errno_t _sopen_s(int *pfd, const char *filename, int oflag, int shflag, int pmode)
// 这里做 inline 封装, 对外保持 "int fd = pz_fd_open(path, flags[, pmode])" 的 POSIX 风格调用,
// 内部把 C4996 deprecated 的 _open 换掉, 调用点零改动。
// shflag 统一用 _SH_DENYNO (与 POSIX open 行为最接近, 不拒绝其他进程读写)。
inline int pz_fd_open(const char *filename, int oflag, int pmode = 0)
{
    int pfd = -1;
    // 调用点传的是 POSIX 口径的 0640/0666,CRT 只认 _S_IREAD/_S_IWRITE 这组位,
    // 先折成 CRT 口径,免得不同 CRT 版本对多余位的处理不一致。
    int wmode = (pmode & 0222) ? (_S_IREAD | _S_IWRITE) : _S_IREAD;
    _sopen_s(&pfd, filename, oflag, _SH_DENYNO, wmode);
    return pfd;
}
// MSVC 的 _lseek 用 long 返回/传偏移 (32 位), 大文件 seek 会溢出;
// 这里用 _lseeki64 统一成 int64_t, 和 POSIX lseek(off_t 通常也是 64 位) 对齐。
inline std::int64_t pz_fd_seek(int fd, std::int64_t offset, int whence)
{
    return _lseeki64(fd, offset, whence);
}
#define pz_fd_close _close
#define pz_fd_write _write
#define pz_fd_read  _read
#define pz_fd_trunc _chsize_s
#else
#include <unistd.h>
#include <fcntl.h>
inline int pz_fd_open(const char *filename, int oflag, int pmode = 0)
{
    return open(filename, oflag, pmode);
}
inline std::int64_t pz_fd_seek(int fd, std::int64_t offset, int whence)
{
    return static_cast<std::int64_t>(lseek(fd, static_cast<off_t>(offset), whence));
}
#define pz_fd_close close
#define pz_fd_write write
#define pz_fd_read  read
#define pz_fd_trunc ftruncate
#endif

// RAII fd 包装: 独占 int fd, 析构自动 pz_fd_close
// 隐式转 int, 所有 pz_fd_xxx / fcntl / _get_osfhandle 等 C 风格 API 零改动
// 放弃所有权可调用 .release()
struct fd_handle {
    int fd = -1;
    fd_handle() = default;
    fd_handle(int f) : fd(f) {}
    fd_handle(fd_handle &&o) noexcept : fd(o.fd) { o.fd = -1; }
    fd_handle &operator=(fd_handle &&o) noexcept {
        if (this != &o) {
            if (fd >= 0) pz_fd_close(fd);
            fd     = o.fd;
            o.fd   = -1;
        }
        return *this;
    }
    ~fd_handle() { if (fd >= 0) pz_fd_close(fd); }
    fd_handle(const fd_handle &)            = delete;
    fd_handle &operator=(const fd_handle &) = delete;
    operator int() const { return fd; }
    int get() const { return fd; }
    explicit operator bool() const { return fd >= 0; }
    int release() {
        int f = fd;
        fd    = -1;
        return f;
    }
};

// Windows CRT 的 _open 默认文本模式, 会对 \n 做 \r\n 转译, 导致读写字节数与
// 缓冲长度不一致, 破坏本工程里 "返回值 == 缓冲长度" 的判等 (日志 flush / session
// 落盘全部失效)。落盘点必须显式带 _O_BINARY 以二进制方式打开。
#ifdef _WIN32
#define PZ_O_BINARY _O_BINARY
#else
#define PZ_O_BINARY 0
#endif

// MSVC 的 long 是 32 位, 而 <cstdio> 的 fseek/ftell 用 long 传偏移和返回位置,
// 超过 2 GiB 的文件定位不到。这里把这两个名字映射到 CRT 自己的 64 位版本,
// 参数与返回值都按 64 位整数处理。
// 注意: 替换只对不带前缀的 fseek / ftell 生效; std::fseek / std::ftell 不会被替换,
// 但它们在 MSVC 下会展开成不存在的 std::_fseeki64 / std::_ftelli64, 所以一律写不带前缀的版本。
// 只有真的调用了 fseek/ftell 的编译单元才会被替换, 纯 include 不调用的单元不受影响。
#ifdef _WIN32
#define fseek _fseeki64
#define ftell _ftelli64
#endif

#endif
