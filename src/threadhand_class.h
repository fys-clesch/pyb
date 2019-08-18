#ifndef __THREADHAND_CLASS_HEADER__
#define __THREADHAND_CLASS_HEADER__

#include <thread>
#include <condition_variable>
#include <mutex>
#include <atomic>

#include "header.hpp"

#if !defined(ISLINUX) && !defined(__WINPTHREADS_VERSION)
#include "mingw.thread.h"
#include "mingw.mutex.h"
#include "mingw.condition_variable.h"
#endif

class threadhand
{
private:
	bool signalled;
	std::atomic<bool> waiting;
	std::condition_variable_any c_v;
	std::mutex mtx;

public:
    threadhand(void);
    ~threadhand(void);
    bool wait(const bool test_it = false); /**< optional bool */
    void signal(void);
    void reset(void);
    bool check(void);
};

/* A simple lock */
class SimpleLock
{
private:

	std::atomic<uint8_t> v;

public:

	SimpleLock(void)
	{
		v.store(0);
	}
    /** \brief
     *
     * \param void
     * \return bool True if 'z' compares equal to 'v'.
     *
     * Compares the contents of the 'v' value with 'z':
     * If true, it replaces the 'v' value with 1 (like store).
     * If false, it replaces 'z' with the 'v' value.
     */
	bool try_lock(void)
	{
		uint8_t z = 0;
		return v.compare_exchange_strong(z, 1);
	}
	void lock(void)
	{
		while(try_lock() == false)
			std::this_thread::yield();
	}
	void unlock(void)
	{
		v.store(0);
	}
};

#endif
