#ifndef __OAUTH_AUDIT_H__
#define __OAUTH_AUDIT_H__

#include <string>

namespace http
{
namespace oauth
{

// 把一行审计写进框架那条活着的历史汇：add_error_lists() 抢 log_mutex 入 error_loglist，
// httpwatch 每拍 httpwatch_flush_error_log() 追加进 log/error.log（O_APPEND + 文件写锁）。
// 不用 DEBUG_LOG —— terminal_color.h 里它在 #ifdef DEBUG 之下，而 DEBUG 只有 Debug 档定义，
// Release 档整条宏展开成空。
// 不用 LOG_ERROR —— debug_log.h 的 out_str() 第一条语句把 issave 写 0，三个文件分支恒假，
// 两档都不落盘。
// 行内带函数名和行号、不带文件名：调用点全在 controller/src/oauth/ 与 libs/oauth/ 里，函数名已唯一。
void audit_log(const char *func, unsigned int line, const std::string &msg);

}// namespace oauth
}// namespace http

// 参数是一个 std::string 表达式，拼接由调用方写完再交进来。
#define OAUTH_AUDIT(msg) http::oauth::audit_log(__func__, __LINE__, (msg))

#endif
