#ifndef __VIEWERCOM_HEADER_H__
#define __VIEWERCOM_HEADER_H__

#include "../../src/header.hpp"
#include "../../src/threadhand_class.h"

class viewercom
{
private:

	std::atomic<bool> map_mode_c,
	                  lmb_down_c,
	                  rmb_down_c,
	                  rot_box_c,
	                  move_box_c,
	                  esc_down_c;

public:

	viewercom(void);
	~viewercom(void);

};

#endif
