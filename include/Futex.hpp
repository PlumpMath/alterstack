#pragma once

#include <linux/futex.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <climits>

#include <atomic>

class Futex
{
public:
    Futex() = default;
    Futex(const Futex&) = delete;
    Futex(Futex&&)      = delete;
    Futex& operator=(const Futex&) = delete;
    Futex& operator=(Futex&&) = delete;
    ~Futex() = default;

    void wait();
    void notify(int32_t count = 1);
    void notify_all();

private:
    std::atomic<int>      m_work_avalable{ 1 };
    std::atomic<uint32_t> m_wait_counter { 0 };
};

inline void Futex::wait()
{
    bool have_work = m_work_avalable.load(std::memory_order_acquire);
    if( have_work != 0 )
    {
        have_work = m_work_avalable.exchange( 0, std::memory_order_release );
        if( have_work != 0 )
            return;
    }
    /*
     * int futex(int *uaddr, int op, int val, const struct timespec *timeout,
     *           int *uaddr2, int val3);
     */
    m_wait_counter.fetch_add( 1, std::memory_order_relaxed);
    syscall(SYS_futex, &m_work_avalable, FUTEX_WAIT, 0, NULL, NULL, 0 );
    m_wait_counter.fetch_sub( 1, std::memory_order_release );

    return;
}
inline void Futex::notify(int32_t count)
{
    if( m_work_avalable.load(std::memory_order_acquire) == 0 )
    {
        m_work_avalable.store( 1, std::memory_order_release );
    }
    if( m_wait_counter.load( std::memory_order_acquire ) > 0 )
        syscall(SYS_futex, &m_work_avalable, FUTEX_WAKE, count, NULL, NULL, 0 );
}

inline void Futex::notify_all()
{
    notify( INT_MAX );
}
