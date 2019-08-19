#include "threadhand_class.h"

threadhand::threadhand(void)
{
    signalled = false;
    waiting.store(false, std::memory_order_relaxed);
}

threadhand::~threadhand(void)
{
    #ifndef IGYBA_NDEBUG
    iprint(stdout, "'%s': memory released\n", __func__);
    #endif
}

bool threadhand::wait(const bool test_it)
{
    if(test_it)
        return false;
    std::lock_guard<std::mutex> lck(mtx);
    while(!signalled)
    {
        waiting.store(true, std::memory_order_release); /**< Within the next
        acquire, the (visible) side effects have to be synchronized. */
        c_v.wait(mtx);
    }
    waiting.store(false, std::memory_order_release); /**< Same here. */
    return signalled;
}

void threadhand::signal(void)
{
    std::lock_guard<std::mutex> lck(mtx);
    signalled = true;
    c_v.notify_one();
}

void threadhand::reset(void)
{
    std::lock_guard<std::mutex> lck(mtx);
    signalled = false;
}

/** \brief This can be used as a spin lock.
 *
 * \param void
 * \return bool Tells whether or not the thread locked by the mutex is waiting.
 *
 */
bool threadhand::check(void)
{
    return waiting.load(std::memory_order_acquire); /**< All side effects from
    the last store operation are synchronized now. */
}
