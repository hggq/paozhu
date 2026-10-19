#pragma once
#ifndef HTTP_RESIDENT_STOP_H
#define HTTP_RESIDENT_STOP_H

namespace http
{
// 常驻出站客户端（redis 订阅 / websocket / 裸 TCP / mqtt）的停机自检路由，
// 注册为 GET /resident_stop?kind=redis|ws|sock|mqtt&name=<conf 段名>。
// 必须在 router_init_sites() 之后调用，否则未知 site 的注册会被丢弃。
void reg_resident_stop_route();
}// namespace http

#endif
