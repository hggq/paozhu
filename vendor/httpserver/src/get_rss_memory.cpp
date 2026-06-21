
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <atomic>
 #include "get_rss_memory.h"

#ifdef _WIN32
    #ifndef WIN32_LEAN_AND_MEAN
        #define WIN32_LEAN_AND_MEAN
    #endif
    #include <windows.h>
    #include <psapi.h>
#elif defined(__APPLE__)
    #include <mach/mach.h>
    #include <sys/types.h>
    #include <sys/sysctl.h> 
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

size_t get_total_memory_kb() {
#ifdef _WIN32
    MEMORYSTATUSEX mem_status;
    mem_status.dwLength = sizeof(mem_status);
    if (!GlobalMemoryStatusEx(&mem_status)) {
        return 0;
    }
    return static_cast<size_t>(mem_status.ullTotalPhys / 1024);

#elif defined(__APPLE__)
    uint64_t total_mem = 0;
    size_t length = sizeof(total_mem);
    if (sysctlbyname("hw.memsize", &total_mem, &length, nullptr, 0) != 0) {
        return 0;
    }
    return static_cast<size_t>(total_mem / 1024);

#else
    long pages = sysconf(_SC_PHYS_PAGES);
    long page_size = sysconf(_SC_PAGESIZE);
    if (pages <= 0 || page_size <= 0) return 0;
    return static_cast<size_t>((pages * page_size) / 1024);
#endif
}

}