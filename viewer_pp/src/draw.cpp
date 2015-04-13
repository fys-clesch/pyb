#include "viewer_class.h"

void viewer::check_Visibility(const int vis)
{
	if(vis == GLUT_VISIBLE)
		glutIdleFunc(animate_View);
	else
	{
		glutSetWindowTitle("viewer - idle");
		glutIdleFunc(NULL);
	}
}

void viewer::check_WindowState(const int state)
{
	if(state == GLUT_FULLY_RETAINED)
		glutIdleFunc(animate_View);
	else
	{
		glutSetWindowTitle("viewer");
		glutIdleFunc(NULL);
	}
}

void viewer::set_OnExitMain(void)
{
	iprint(stdout, "'%s': i'm leaving\n", __func__);
}
