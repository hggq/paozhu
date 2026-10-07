
#pragma once
#include <chrono>
#include <thread>
#include "httppeer.h"

namespace http
{        
	std::string testfxpreonly(std::shared_ptr<httppeer> peer);
	std::string testfxguarded(std::shared_ptr<httppeer> peer);
	std::string testfxa(std::shared_ptr<httppeer> peer);
	std::string testfxb(std::shared_ptr<httppeer> peer);
	std::string testfxc(std::shared_ptr<httppeer> peer);
	std::string testfxd(std::shared_ptr<httppeer> peer);
	std::string testfxplain(std::shared_ptr<httppeer> peer);
	std::string testfxloop(std::shared_ptr<httppeer> peer);
	std::string testfxgone(std::shared_ptr<httppeer> peer);
	std::string testfxexit(std::shared_ptr<httppeer> peer);
	std::string testfxdepa(std::shared_ptr<httppeer> peer);
	std::string testfxdepb(std::shared_ptr<httppeer> peer);
	asio::awaitable<std::string> testfxcoro(std::shared_ptr<httppeer> peer);
	asio::awaitable<std::string> testfxcoroa(std::shared_ptr<httppeer> peer);
	asio::awaitable<std::string> testfxcorob(std::shared_ptr<httppeer> peer);
	std::string testfxsyncpushcoro(std::shared_ptr<httppeer> peer);
	asio::awaitable<std::string> testfxcoroc(std::shared_ptr<httppeer> peer);
	asio::awaitable<std::string> testfxcoropre(std::shared_ptr<httppeer> peer);
	std::string testfxcoropresync(std::shared_ptr<httppeer> peer);
	std::string testfxmixchain_sync(std::shared_ptr<httppeer> peer);
	asio::awaitable<std::string> testfxmixchain_coro(std::shared_ptr<httppeer> peer);
	std::string testfxmixchain_tail(std::shared_ptr<httppeer> peer);
	std::string testfxexc_sync_reg(std::shared_ptr<httppeer> peer);
	std::string testfxexc_sync_pre(std::shared_ptr<httppeer> peer);
	std::string testfxexc_sync_pre_reg(std::shared_ptr<httppeer> peer);
	asio::awaitable<std::string> testfxexc_coro_reg(std::shared_ptr<httppeer> peer);
	asio::awaitable<std::string> testfxexc_coro_pre(std::shared_ptr<httppeer> peer);
	std::string testfxexc_coro_pre_reg(std::shared_ptr<httppeer> peer);
	asio::awaitable<std::string> testfxexc_coro_unknown(std::shared_ptr<httppeer> peer);
	asio::awaitable<std::string> testfxexc_reject503_pre(std::shared_ptr<httppeer> peer);
	std::string testfxexc_reject503(std::shared_ptr<httppeer> peer);
	std::string testfxexc_after503(std::shared_ptr<httppeer> peer);
	std::string testfxlaneb(std::shared_ptr<httppeer> peer);
	std::string testfxlaneread(std::shared_ptr<httppeer> peer);
	asio::awaitable<std::string> testfxlanea(std::shared_ptr<httppeer> peer);
	asio::awaitable<std::string> testfxlanerejectonce(std::shared_ptr<httppeer> peer);
	std::string testfxloopslow(std::shared_ptr<httppeer> peer);
	std::string testfxlooptick(std::shared_ptr<httppeer> peer);
}
