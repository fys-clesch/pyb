#include "viewercom_class.h"

viewercom::viewercom(void)
{
	/* atomic<bool> */
	map_mode_c.store(false, std::memory_order_relaxed);
	lmb_down_c.store(false, std::memory_order_relaxed);
	rmb_down_c.store(false, std::memory_order_relaxed);
	rot_box_c.store(false, std::memory_order_relaxed);
	move_box_c.store(true, std::memory_order_relaxed);
	esc_down_c.store(true, std::memory_order_relaxed); /* 6 */
}

viewercom::~viewercom(void)
{
	#ifndef IGYBA_NDEBUG
	iprint(stdout, "'%s': memory released\n", __func__);
	#endif
}
