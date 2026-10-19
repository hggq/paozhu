#ifndef __HTTP_PZ_FILEIO_H
#define __HTTP_PZ_FILEIO_H

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

// 落盘用的文件描述符接口: Windows CRT 只有下划线版本, POSIX 用同名版本
// SEEK_SET/SEEK_END 由 <cstdio> 提供, MSVC 的 <io.h>/<fcntl.h> 不带
#include <cstdio>
#ifdef _WIN32
#include <io.h>
#include <fcntl.h>
#define pz_fd_open  _open
#define pz_fd_close _close
#define pz_fd_write _write
#define pz_fd_read  _read
#define pz_fd_seek  _lseek
#define pz_fd_trunc _chsize_s
#else
#include <unistd.h>
#include <fcntl.h>
#define pz_fd_open  open
#define pz_fd_close close
#define pz_fd_write write
#define pz_fd_read  read
#define pz_fd_seek  lseek
#define pz_fd_trunc ftruncate
#endif

// Windows CRT 的 _open 默认文本模式, 会对 \n 做 \r\n 转译, 导致读写字节数与
// 缓冲长度不一致, 破坏本工程里 "返回值 == 缓冲长度" 的判等 (日志 flush / session
// 落盘全部失效)。落盘点必须显式带 _O_BINARY 以二进制方式打开。
#ifdef _WIN32
#define PZ_O_BINARY _O_BINARY
#else
#define PZ_O_BINARY 0
#endif

#endif
