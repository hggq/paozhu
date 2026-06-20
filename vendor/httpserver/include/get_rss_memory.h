

#ifndef GET_RSS_MEMORY_H
#define GET_RSS_MEMORY_H
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <atomic>
 

#ifdef _WIN32
    #include <windows.h>
    #include <psapi.h>
    // psapi.lib 需要在链接时加入（CMake: target_link_libraries(... psapi)）
#elif defined(__APPLE__)
    #include <mach/mach.h>
#else
    // Linux / FreeBSD 等类 Unix 系统
    #include <unistd.h>
#endif

namespace http
{
size_t get_rss_kb() {
#ifdef _WIN32
    PROCESS_MEMORY_COUNTERS pmc;
    if (!GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc))) {
        return 0;
    }
    // WorkingSetSize 单位为字节，转换为 KB
    return static_cast<size_t>(pmc.WorkingSetSize / 1024);

#elif defined(__APPLE__)
    struct mach_task_basic_info info;
    mach_msg_type_number_t count = MACH_TASK_BASIC_INFO_COUNT;
    kern_return_t kr = task_info(mach_task_self(),
                                 MACH_TASK_BASIC_INFO,
                                 reinterpret_cast<task_info_t>(&info),
                                 &count);
    return (kr == KERN_SUCCESS)
        ? static_cast<size_t>(info.resident_size / 1024)
        : 0;

#else
    // Linux: 解析 /proc/self/statm（高性能方案）
    FILE* f = fopen("/proc/self/statm", "r");
    if (!f) return 0;

    long resident_pages = 0;
    int ret = fscanf(f, "%*ld %ld", &resident_pages);
    fclose(f);

    if (ret != 1 || resident_pages <= 0) return 0;

    static const long page_size_kb = sysconf(_SC_PAGESIZE) / 1024;
    return static_cast<size_t>(resident_pages * page_size_kb);
#endif
}
}
#endif