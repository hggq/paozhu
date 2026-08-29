#ifndef SESSION_BUF_H
#define SESSION_BUF_H

#include <list>
#include <mutex>
#include <vector>
#include <cstdint>

// ===========================================================================
// session_buf: 框架级跨连接 vector 容量池
// ---------------------------------------------------------------------------
// 设计意图:
//   - client_session 构造时 acquire 借走 vector (容量保留), 析构时 release 归还
//   - write() 每笔写独立 acquire 一个缓冲, 发送完成即 release (并发写互不覆盖在途数据)
//   - 新连接复用旧连接释放的容量, 避免每次新连接首次分配峰值
//   - 仅在连接构造/析构点与每笔写 acquire/release 时锁; read() 内复用连接级成员, 不锁
class session_buf
{
  public:
    // 借一个空 vector (clear 后 capacity 保留); 池空则返回新建空 vector
    std::vector<uint8_t> acquire()
    {
        std::lock_guard<std::mutex> lk(mtx_);
        if (pool_.empty())
            return {};
        std::vector<uint8_t> v = std::move(pool_.back());
        pool_.pop_back();
        v.clear();// size=0, capacity 保留
        return v;
    }

    void release(std::vector<uint8_t> &&v)
    {
        if (v.capacity() == 0)
            return;
        std::lock_guard<std::mutex> lk(mtx_);
        if (pool_.size() >= MAX_POOL_SIZE)
            return;// 池满, 让 v 析构
        pool_.push_back(std::move(v));
    }

    size_t size() const
    {
        std::lock_guard<std::mutex> lk(mtx_);
        return pool_.size();
    }

    // 说明: 用 std::list 而非 std::vector 外层容器, 因 release push_back 严格 O(1) 无 grow 抖动
    //       acquire 走 back + pop_back + clear, LIFO 语义, 与 vector 行为一致
    //       (list 节点稳定 / FIFO / splice 等特性在 LIFO+pop_push 模式下用不上)
    size_t cleanup_pool()
    {
        std::lock_guard<std::mutex> lk(mtx_);
        size_t n = pool_.size();
        pool_.clear();
        return n;
    }

  private:
    // 池上限: 防止空闲连接释放后池无限增长 (4096 个空闲 vector, 每个 ≤16KB = 64MB 上限)
    static constexpr size_t MAX_POOL_SIZE = 4096;
    mutable std::mutex mtx_;
    std::list<std::vector<uint8_t>> pool_;
};

#endif// SESSION_BUF_H
