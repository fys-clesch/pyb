#ifndef __THREADHAND_CLASS_HEADER__
#define __THREADHAND_CLASS_HEADER__

#include <thread>
#include <condition_variable>
#include <mutex>
#include <atomic>

#include "header.hpp"

#ifndef ISLINUX
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

#endif
