#ifndef __VIEWER_HEADER_H__
#define __VIEWER_HEADER_H__

#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <float.h>
#include <ctype.h>
#include <chrono>
#include <thread>
#include <atomic>

#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/freeglut.h>

#include "../../src/header.hpp"
#include "../../src/threadhand_class.h"

class viewer
{
private:

	double *res_pt rgb, /**< colour at a given point */
		   *res_pt data, /**< data stored as x, data, z */
		   *res_pt cb, /**< colour box colouring stripes */
		   *res_pt rgbcb, /**< colour box colour at each stripe */
		   min_norm, /**< minimum of data with -1 <= min_norm <= 0 */
		   min_val, /**< minimum of data */
		   max_norm, /**< maximum of data with 0 < max_norm <= 1 */
		   /* if the absolute maximum value is positive, max_norm = 1., else min_norm = -1. */
		   max_val, /**< maximum of data */
		   wc[3], /**< world coordinates at the mouse position */
		   rot_y,
		   rot_x,
		   mov_y,
		   mov_x,
		   mov_lvl,
		   zoom;

	uint nstrps, /**< Number of stripes for the colour box */
	     win_width,
	     win_height,
	     row_slice,
	     col_slice,
	     glrows,
	     glcols; /**< Number of the datapoints in each direction */

	std::atomic<uint> atmc_glrows,
	                  atmc_glcols; /**< This is an atomic version of glcols.
	                  it has to be atomic to prevent data races between the
	                  copy and viewer thread. A relaxed memory order is OK,
	                  as long as there's no visible side effect. */

	int win_main,
	    win_sub;

	bool lmb_down,
	     rmb_down,
	     move_box,
	     map_mode; /**< A non atomic version to reduce thread safe
	     store and load operations. */

	std::atomic<bool> map_mode_atm,
	                  allocd,
	                  filled,
	                  running,
	                  update_animate,
	                  new_data,
	                  esc_down,
	                  constant_animate,
	                  rot_box;

	static const double cwhite[4],
	              cblack[4],
	              ctrans[4],
	              cgrey_trans[4],
	              cred[4],
	              cgreen[4],
	              cblue[4];

	static const float light_amb[4],
					   light_diff[4],
					   light_spec[4],
					   light_pos[4],
					   mat_amb[4],
					   mat_diff[4],
					   mat_spec[4],
					   high_shine[1];

	constexpr static double near_clip = -1e1,
	                        far_clip = 1e1,
	                        zoom_std = 200.,
	                        rot_y_std = 45.,
	                        rot_x_std = -45,
							zoom_max = zoom_std * (1. + 1.5),
							zoom_min = zoom_std * .2;

	constexpr static uint std_win_width = 800,
	                      std_win_height = 600;

	enum get_TrackballState
	{
		VIEWER_MOUSEMOTION = 0,
		VIEWER_KNOWMOUSEBUTTON,
		VIEWER_RESET
	};

	static void KeyboardHandler(const uchar key, const int x, const int y);
	static void ArrowKeysHandler(const int a_keys, const int x, const int y);
	static void TrackballHandler(const int mode, const int button,
								const int state, const int x, const int y);
	static void MotionHandler(const int x, const int y);
	static void PassiveMotionHandler(const int x, const int y);
	static void MouseHandler(const int button, const int state,
							const int x, const int y);

	static void animate_View(void);
	static void check_Visibility(const int vis);
	static void check_WindowState(const int state);
	static void set_DisplayMain(void);
	void exchange_Atomics(void);
	static void set_DisplayMainThread(void);
	static void set_SubDisplay(void);
	static void reshape_View(const int width, const int height);
	static void set_OnExitMain(void);
	static void close_All(void);
	void reset_MemberVariables(const bool make_free = false);
	void fill_DrawingData(const double *res_pt const m_in, const bool noisy);
	void print_BlockString2d(const char *s,
						const uint lw,
						double sx, double sy,
						const double ix, const double iy);
	void print_String(const char *s);
	void display_MapView(const bool init);
	void display_3dView(const bool init);
	void draw_Colorbox(void);
	void draw_CoordinateOverview(void);
	void print_Level(const double *res_pt pos_z,
					const double *res_pt vmax,
					const uint prpr);
	void print_LevelColorbox(const double *res_pt vmin,
							const double *res_pt vmax,
							const uint prpr,
							const double xl,
							const double ymin, const double ymax);
	void init_Main(int argc, char **argv);

	void reset_DrawingControls(void);
	void wait_UntilViewerPause(const std::string &text);

	static void launch_FreeglutTest(int argc, char **argv);

	void store_MapMode(const bool b);
	bool load_MapMode(void);
	void store_RotateBox(const bool b);
	bool load_RotateBox(void);
	void store_AllocatedMemory(const bool b);
	bool load_AllocatedMemory(void);
	void store_AllocatedMemoryRelease(const bool b);
	bool load_AllocatedMemoryAcquire(void);
	bool load_NewDataAvailable(void);
	void store_ConstantAnimation(const bool b);
	bool load_ConstantAnimation(void);
	void store_IsRunning(const bool b);

	void store_GlRows(const uint n);
	uint load_GlRows(void);
	void store_GlCols(const uint n);
	uint load_GlCols(void);
	bool load_PressedEsc(void);
	void store_PressedEsc(const bool b);
	void store_UpdateAnimate(const bool b);
	bool load_UpdateAnimate(void);

public:

	threadhand event_SwapDataToViewer;

	viewer(void);
	~viewer(void);

	bool load_FilledMemory(void);
	void init_Colorbox(void);
	void close_Freeglut(void);
	void toggle_Idling(void);
	void toggle_Map3DMode(void);
	void toggle_Rotation(void);
	void store_NewDataAvailable(const bool b);
	void alloc_DataFromMemory(const uint nrows, const uint ncols);
	static void launch_Freeglut(int argc, char **argv,
								const bool threading = false);
	bool load_IsRunning(void);
	static void calc_DrawingData(const double *res_pt const m_in,
							const bool noisy,
							double *res_pt max_val_out,
							double *res_pt min_val_out,
							double *res_pt max_norm_out,
							double *res_pt min_norm_out,
							const uint row_in, const uint col_in,
							double *res_pt data_out,
							double *res_pt rgb_out);
	void set_DrawingData(const double *res_pt max_val_out,
					const double *res_pt min_val_out,
					const double *res_pt max_norm_out,
					const double *res_pt min_norm_out,
					const double *res_pt data_out,
					const double *res_pt rgb_out);
	void store_FilledMemory(const bool b);
	void alloc_DataFromFile(const std::string &fname);
	void fill_DataFromFile(const std::string &fname);

};

#endif
