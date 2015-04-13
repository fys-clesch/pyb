#include "viewer_class.h"

const double viewer::cwhite[4] = {1., 1., 1., 1.},
viewer::cblack[4] = {0., 0., 0., 1.},
viewer::ctrans[4] = {1., 1., 1., .1},
viewer::cgrey_trans[4] = {.8, .8, .8, .5},
viewer::cred[4] = {1., .01, .05, 1.},
viewer::cgreen[4] = {.01, 1., .01, 1.},
viewer::cblue[4] = {.01, .1, 1., 1.};

const float viewer::light_amb[4] = {0., 0., 0., .8}, /**< rgba */
viewer::light_diff[4] = {.8, .8, .8, .8}, /**< rgba */
viewer::light_spec[4] = {.8, .8, .8, .8}, /**< rgba */
viewer::light_pos[4] = {1., -0., 2., 0.}, /**< source at inf */
viewer::mat_amb[4] = {.7, .7, .7, .8},
viewer::mat_diff[4] = {.8, .8, .8, .8},
viewer::mat_spec[4] = {1., 1., 1., .8},
viewer::high_shine[1] = {100.};

static viewer *dv1p = nullptr;

viewer::viewer(void)
{
	/* viewer * */
	dv1p = static_cast<viewer *>(this);
	/* double * */
	rgb =
	data =
	cb =
	rgbcb = nullptr; /* 4 */
	/* double */
	min_norm = -1.;
	min_val = -DBL_MAX;
	max_norm = 1.;
	max_val = DBL_MAX;
	rot_y = rot_y_std;
	rot_x = rot_x_std,
	mov_y =
	mov_x = 0.;
	mov_lvl = .5;
	zoom = zoom_std; /* 10 */
	/* double[3] */
	for(double &x : wc)
		x = 0.; /* 1 */
	/* uint */
	nstrps = 256;
	win_width =
	win_height = 1;
	row_slice =
	col_slice =
	glrows =
	glcols = 0; /* 7 */
	/* atomic<uint> */
	atmc_glrows.store(0, std::memory_order_relaxed);
	atmc_glcols.store(0, std::memory_order_relaxed); /* 2 */
	/* int */
	win_main =
	win_sub = 0; /* 2 */
	/* bool */
	lmb_down =
	rmb_down =
	move_box = false; /* 3 */
	/* atomic<bool> */
	rot_box.store(false, std::memory_order_relaxed);
	esc_down.store(false, std::memory_order_relaxed);
	map_mode.store(false, std::memory_order_relaxed);
	running.store(false, std::memory_order_relaxed);
	allocd.store(false, std::memory_order_relaxed);
	filled.store(false, std::memory_order_relaxed);
	update_animate.store(false, std::memory_order_relaxed);
	constant_animate.store(true, std::memory_order_relaxed);
	new_data.store(true, std::memory_order_relaxed); /* 9 */
}

viewer::~viewer(void)
{
	if(load_AllocatedMemoryAcquire())
	{
		free(rgb);
		free(data);
		free(cb);
		free(rgbcb);
	}
	dv1p = nullptr;
	#ifndef IGYBA_NDEBUG
	iprint(stdout, "'%s': memory released\n", __func__);
	#endif
}

/** \brief Reset everything for a re-use of this instance.
 *
 * \param void
 * \return void
 *
 * The variables glrows, glcols, allocd, filled and the double
 * arrays don't need to be freed, this happens at the end of the
 * class instance.
 */
void viewer::reset_MemberVariables(const bool make_free)
{
	/* atomic<bool> */
	update_animate.store(true, std::memory_order_relaxed); /**< this makes sure that the next run is displayed immediately */
	store_MapMode(false);
	store_NewDataAvailable(true);
	esc_down.store(false, std::memory_order_relaxed);
	store_RotateBox(false);
	store_ConstantAnimation(true);
	store_IsRunning(false); /* 7 */
	/* bool */
	lmb_down =
	rmb_down =
	move_box = false; /* 3 */
	/* double */
	min_norm = -1.;
	min_val = -DBL_MAX;
	max_norm = 1.;
	max_val = DBL_MAX;
	rot_y = rot_y_std;
	rot_x = rot_x_std,
	mov_y =
	mov_x = 0.;
	mov_lvl = .5;
	zoom = zoom_std; /* 10 */
	/* double [3] */
	for(double &x : wc)
		x = 0.; /* 1 */
	/* uint */
	win_width =
	win_height = 1;
	row_slice =
	col_slice = 0; /* 4 */

	if(make_free)
	{
		free(rgb);
		free(data);
		free(cb);
		free(rgbcb);
		store_AllocatedMemoryRelease(false); /**< This ensures that all writes
		in the current thread are visible in other threads that acquire the same
		atomic. */
	}
}

void viewer::reset_DrawingControls(void)
{
	rot_y = rot_y_std;
	rot_x = rot_x_std;
	mov_x =
	mov_y = 0.;
	mov_lvl = .5;
	zoom = zoom_std;
	update_animate.store(true, std::memory_order_relaxed);
	row_slice =
	col_slice = 0;
}

void viewer::init_Main(int argc, char **argv)
{
	win_width = std_win_width;
	win_height = std_win_height;

	glutInit(&argc, argv);
	glutInitWindowSize(win_width, win_height);
	glutInitWindowPosition(0, 0);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH | GLUT_MULTISAMPLE);

	glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);
	glutCreateWindow("viewer");
	#ifndef ISLINUX
	glutSetCursor(GLUT_CURSOR_CROSSHAIR);
	#endif
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glEnable(GL_COLOR_MATERIAL);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glEnable(GL_LINE_SMOOTH);
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

	glEnable(GL_LIGHT0);
	glEnable(GL_COLOR_MATERIAL);
	glLightfv(GL_LIGHT0, GL_POSITION, light_pos);

	glLightfv(GL_LIGHT0, GL_AMBIENT, light_amb);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diff);
	glLightfv(GL_LIGHT0, GL_SPECULAR, light_spec);
	glMaterialfv(GL_FRONT, GL_AMBIENT, mat_amb);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diff);
	glMaterialfv(GL_FRONT, GL_SPECULAR, mat_spec);
	glMaterialfv(GL_FRONT, GL_SHININESS, high_shine);

	glClearColor(0., 0., 0., 0.);
	glClearDepth(1.);

	glViewport(0, 0, (int)win_width, (int)win_height);

	if(!glutGet(GLUT_DISPLAY_MODE_POSSIBLE))
		warn_msg("can not establish correct display mode. flawless display is unlikely.",
				ERR_ARG);
	iprint(stdout,"rendering with\n%-15s %s\n%-15s %s\n%-15s %s\n",
		"OGL version", glGetString(GL_VERSION),
		"renderer", glGetString(GL_RENDERER),
		"vendor", glGetString(GL_VENDOR));

	win_main = glutGetWindow();
}

void viewer::display_3dView(const bool init)
{
	if(!init)
	{
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glOrtho(-.5 * win_width, .5 * win_width,
				-.5 * win_height, .5 * win_height,
				near_clip, far_clip);
	}

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glScaled(zoom, zoom, 1.);

	glTranslated(mov_x, mov_y, 0.);
	glRotated(rot_x, 1., 0., 0.);
	glRotated(rot_y, 0., 0., 1.);
	{
		double ry = 1., rx = 1.;
		if(glcols > glrows)
			rx = glrows / (double)glcols;
		else
			ry = glcols / (double)glrows;

		glScaled(rx, ry, 1.);
	}

	if(zoom <= zoom_std)
		glLineWidth(2);
	else
		glLineWidth(3);

	/* frame */
	glColor3dv(viewer::cwhite);
	glBegin(GL_LINE_LOOP); /* back */
	glVertex3d(-1., -1., -1.); glVertex3d(-1., 1., -1.);
	glVertex3d(1., 1., -1.); glVertex3d(1., -1., -1.);
	glEnd();
	glBegin(GL_LINE_LOOP); /* left */
	glVertex3d(-1., -1., -1.); glVertex3d(-1., 1., -1.);
	glVertex3d(-1., 1., 1.); glVertex3d(-1., -1., 1.);
	glEnd();
	glBegin(GL_LINE_LOOP); /* front */
	glVertex3d(-1., 1., 1.); glVertex3d(-1., -1., 1.);
	glVertex3d(1., -1., 1.); glVertex3d(1., 1., 1.);
	glEnd();
	glBegin(GL_LINE_LOOP); /* right */
	glVertex3d(1., -1., 1.); glVertex3d(1., -1., -1.);
	glVertex3d(1., 1., -1.); glVertex3d(1., 1., 1.);
	glEnd();

	glLineWidth(1);

	glRasterPos3d(1.05, -1., -1.);
	glutBitmapCharacter(GLUT_BITMAP_8_BY_13, 'x');
	glRasterPos3d(-1., 1.05, -1.);
	glutBitmapCharacter(GLUT_BITMAP_8_BY_13, 'y');
	glRasterPos3d(-1., -1., 1.05);
	glutBitmapCharacter(GLUT_BITMAP_8_BY_13, 'z');

	/* points */
	glBegin(GL_POINTS);
	for(uint x = 0; x < glrows; x++)
		for(uint y = 0; y < glcols; y++)
		{
			uint idx = x * glcols * 3 + y * 3;
			glColor3dv(&rgb[idx]);
			glVertex3dv(&data[idx]);
		}
	glEnd();

	/* surface */
	glEnable(GL_LIGHTING);
	for(uint x = 0; x < (glrows - 1); x++)
	{
		glBegin(GL_QUAD_STRIP);
		const uint idxi = x * glcols * 3;
		for(uint y = 0; y < glcols; y++)
		{
			uint idx = idxi + y * 3;
			glColor3dv(&rgb[idx]);
			glVertex3dv(&data[idx]);
			idx = idxi + glcols * 3 + y * 3;
			glColor3dv(&rgb[idx]);
			glVertex3dv(&data[idx]);
		}
		glEnd();
	}
	glDisable(GL_LIGHTING);

	/* ground */
	for(uint x = 0; x < (glrows - 1); x++)
	{
		glBegin(GL_TRIANGLE_STRIP);
		const uint idxi = x * glcols * 3;
		for(uint y = 0; y < glcols; y++)
		{
			uint idx = idxi + y * 3;
			glColor3dv(&rgb[idx]);
			glVertex3d(data[idx + 0], data[idx + 1], -1.);
			idx = idxi + glcols * 3 + y * 3;
			glColor3dv(&rgb[idx]);
			glVertex3d(data[idx + 0], data[idx + 1], -1.);
		}
		glEnd();
	}

	/* transparent level */
	glPushMatrix();
	if(mov_lvl < min_norm)
		mov_lvl = min_norm;
	glTranslated(0., 0., mov_lvl);

	glColor4dv(ctrans);
	glBegin(GL_QUADS);
	glVertex3d(-1., -1., 0.);
	glVertex3d(-1., 1., 0.);
	glVertex3d(1., 1., 0.);
	glVertex3d(1., -1., 0.);
	glEnd();

	glColor4dv(viewer::cwhite);
	glRasterPos3d(-1.1, -1.1, 0.);
	print_Level(&mov_lvl, &max_val, 10);

	glPopMatrix();

	/* overview and and colorbox */
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0., 1., 0., 1., -2., 2.);
	glTranslated(0., 0., 1.); /**< the colorbox should be always in front of the drawing */

	draw_CoordinateOverview();
	draw_Colorbox();

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();

	glutSwapBuffers();
}

void viewer::display_MapView(const bool init)
{
	if(!init)
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		gluOrtho2D(-1., 1, -1., 1);
	}

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	{
		double ry = 1., rx = 1.;
		if(glcols > glrows)
			rx = glrows / (double)glcols;
		else
			ry = glcols / (double)glrows;

		glScaled(.5 * rx, .5 * ry, 1.);
	}

	glLineWidth(2);

	/* frame */
	glColor3dv(cwhite);
	glBegin(GL_LINE_LOOP);
	glVertex2d(-1., -1.);
	glVertex2d(-1., 1.);
	glVertex2d(1., 1.);
	glVertex2d(1., -1.);
	glEnd();

	/* axis */
	glRasterPos2d(1.05, -1.);
	glutBitmapCharacter(GLUT_BITMAP_8_BY_13, 'x');
	glRasterPos2d(-1., 1.05);
	glutBitmapCharacter(GLUT_BITMAP_8_BY_13, 'y');

	/* ground */
	for(uint x = 0; x < (glrows - 1); x++)
	{
		glBegin(GL_TRIANGLE_STRIP);
		const uint idxi = x * glcols * 3;
		for(uint y = 0; y < glcols; y++)
		{
			uint idx = idxi + y * 3;
			glColor3dv(&rgb[idx]);
			glVertex2dv(&data[idx]);
			idx = idxi + glcols * 3 + y * 3;
			glColor3dv(&rgb[idx]);
			glVertex2dv(&data[idx]);
		}
		glEnd();
	}

	/* side-projector */
	glPushMatrix();
	glTranslated(0., 0., .5);
	glColor3dv(cwhite);

	static double wc_ox = 0xDEADDEAD,
	              wc_oy = 0xDEADDEAD;
	if((wc_ox != wc[0] || wc_oy != wc[1]) &&
		(fabs(wc[0]) <= 1. && fabs(wc[1]) <= 1.)) /**< we're taking the mouse click position instead */
	{
		row_slice = (uint)rint((wc[0] + 1.) / 2. * glrows);
		col_slice = (uint)rint((wc[1] + 1.) / 2. * glcols);
		wc_ox = wc[0];
		wc_oy = wc[1];
	}
	row_slice %= glrows;
	col_slice %= glcols;

	const double tf_lin_a = 1. / (max_norm - min_norm),
	             tf_lin_b = min_norm / (min_norm - max_norm);
	char c[32];
	/* right */
	const double off_x = .2,
	             width_x = .5;
	glLineWidth(2);
	glBegin(GL_LINE_LOOP);
	glVertex2d(-1. - off_x, -1.);
	glVertex2d(-1. - off_x, 1.);
	glVertex2d(-1. - (off_x + width_x), 1.);
	glVertex2d(-1. - (off_x + width_x), -1.);
	glEnd();
	/* this is the horizontal hair */
	glBegin(GL_LINES);
	glVertex2d(data[row_slice * glcols * 3 + 0 * 3], 1.);
	glVertex2d(data[row_slice * glcols * 3 + 0 * 3], -1.);
	glEnd();

	glLineWidth(1);
	glBegin(GL_LINE_STRIP);
	for(uint y = 0; y < glcols; y++)
	{
		double yy = y / (double)glcols * 2. - 1.,
		       xx = data[row_slice * glcols * 3 + y * 3 + 2];
		xx = (tf_lin_a * xx + tf_lin_b) * width_x; /**< normalize span to unity and rescale span to the box height */
		xx -= 1. + off_x + width_x;
		glVertex2d(xx, yy);
	}
	glEnd();

	snprintf(c, 32, "slice %u", row_slice);
	glRasterPos2d(1.05, 0.);
	print_String(c);

	/* bottom */
	const double off_y = .2,
	             height_y = .5;
	/* the box first */
	glLineWidth(2);
	glBegin(GL_LINE_LOOP);
	glVertex2d(-1., -1. - off_y);
	glVertex2d(1., -1. - off_y);
	glVertex2d(1., -1. - (off_y + height_y));
	glVertex2d(-1., -1. - (off_y + height_y));
	glEnd();
	snprintf(c, 32, "%g", min_val);
	glRasterPos2d(1.05, -1. - (off_y + height_y));
	print_String(c);
	snprintf(c, 32, "%g", max_val);
	glRasterPos2d(1.05, -1. - off_y);
	print_String(c);
	/* this is the vertical hair */
	glBegin(GL_LINES);
	glVertex2d(1., data[0 * glcols * 3 + col_slice * 3 + 1]);
	glVertex2d(-1., data[0 * glcols * 3 + col_slice * 3 + 1]);
	glEnd();

	glLineWidth(1);
	glBegin(GL_LINE_STRIP);
	for(uint x = 0; x < glrows; x++)
	{
		double xx = x / (double)glrows * 2. - 1.,
		       yy = data[x * glcols * 3 + col_slice * 3 + 2];
		yy = (tf_lin_a * yy + tf_lin_b) * height_y;
		yy -= 1. + off_y + height_y;
		glVertex2d(xx, yy);
	}
	glEnd();

	snprintf(c, 32, "slice %u", col_slice);
	glRasterPos2d(0., 1.05);
	print_String(c);

	mov_lvl = data[row_slice * glcols * 3 + col_slice * 3 + 2];
	snprintf(c, 32, "value %f", mov_lvl * max_val);
	glRasterPos2d(0., 1.5);
	print_String(c);

	glPopMatrix();

	/* colorbox */
	glColor4dv(cwhite);
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluOrtho2D(0., 1., 0., 1.);

	draw_Colorbox();

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();

	glutSwapBuffers();
}

void viewer::draw_Colorbox(void)
{
	constexpr double width = .05,
	                 height = .2;
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	glTranslated(.84, .75, 0.);
	glScaled(1., 1., 1.);
	/* outer box */
	glBegin(GL_LINE_LOOP);
	glVertex2d(0., 0.);
	glVertex2d(width, 0.);
	glVertex2d(width, height);
	glVertex2d(0., height);
	glEnd();

	glBegin(GL_QUAD_STRIP);
	for(uint i = 0; i < nstrps; i++)
	{
		glColor3dv(&rgbcb[i * 3]);
		glVertex2d(0., cb[i] * height);
		glVertex2d(width, cb[i] * height);
	}
	if(nstrps % 4) /* add 2 vertices to finish quad strip */
	{
		glVertex2d(0., cb[nstrps - 1] * height);
		glVertex2d(width, cb[nstrps - 1] * height);
	}
	glEnd();

	glColor3dv(cwhite);
	print_LevelColorbox(&min_val, &max_val, 10,
						width + width * .2, 0., height);
	glPopMatrix();
}

void viewer::draw_CoordinateOverview(void)
{
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	glTranslated(.1, .1, 0.);
	glRotated(rot_x, 1., 0., 0.);
	glRotated(rot_y, 0., 0., 1.);

	const double zero[3] = {0., 0., 0.},
	             end_p = .05,
	             end_char = .07;
	glBegin(GL_LINES);
	glVertex3dv(zero);
	glVertex3d(end_p, 0., 0.);
	glVertex3dv(zero);
	glVertex3d(0., end_p, 0.);
	glVertex3dv(zero);
	glVertex3d(0., 0., end_p);
	glEnd();

	glRasterPos3d(end_char, 0., 0.);
	glutBitmapCharacter(GLUT_BITMAP_8_BY_13, 'x');
	glRasterPos3d(0., end_char, 0.);
	glutBitmapCharacter(GLUT_BITMAP_8_BY_13, 'y');
	glRasterPos3d(0., 0., end_char);
	glutBitmapCharacter(GLUT_BITMAP_8_BY_13, 'z');

	glPopMatrix();
}

void viewer::print_BlockString2d(const char *s,
								const uint lw,
								double sx, double sy,
								const double ix, const double iy)
{
	char *buf = (char *)calloc(lw + 1, sizeof(char));
	const uint len = strlen(s);
	for(uint i = 0; i < len;)
	{
		strncpy(buf, &s[i], lw);
		if(isspace(buf[0]))
		{
			memmove(buf, &buf[1], lw - 1);
			if(i + lw < len)
			{
				buf[lw - 1] = s[i + lw];
				i++;
			}
		}
		if(isalpha(buf[lw - 1]) && i + lw < len && isalpha(s[i + lw]))
		{
			buf[lw - 1] = '-';
			i += (lw - 1);
		}
		else
			i += lw;
		glRasterPos2d(sx, sy);
		sx += ix;
		sy += iy;
		for(uint j = 0; j < strlen(buf); j++)
			glutBitmapCharacter(GLUT_BITMAP_8_BY_13, buf[j]);
	}
	free(buf);
}

void viewer::print_String(const char *s)
{
	for(uint i = 0; i < strlen(s); i++)
		glutBitmapCharacter(GLUT_BITMAP_8_BY_13, s[i]);
}

void viewer::print_Level(const double *res_pt pos_z, const double *res_pt vmax,
						const uint prpr)
{
	double val = (*vmax) * (*pos_z);
	char c[32],
	     cf[12];
	snprintf(cf, 12, "%%-%ug", prpr);
	snprintf(c, 32, cf, val);
	print_String(c);
}

void viewer::print_LevelColorbox(const double *res_pt vmin,
								const double *res_pt vmax,
								const uint prpr,
								const double xl,
								const double ymin,
								const double ymax)
{
	char c[32],
	     cf[12];
	snprintf(cf, 12, "%%-%ug", prpr);
	snprintf(c, 32, cf, *vmin);
	glRasterPos2d(xl, ymin);
	print_String(c);
	snprintf(c, 32, cf, (*vmax - *vmin) *.5 + *vmin);
	glRasterPos2d(xl, (ymax - ymin) * .5 + ymin);
	print_String(c);
	snprintf(c, 32, cf, *vmax);
	glRasterPos2d(xl, ymax);
	print_String(c);
}

void viewer::fill_DrawingData(const double *res_pt const m_in, const bool noisy)
{
	max_val = -DBL_MAX;
	min_val = DBL_MAX;
	find_minmax(m_in, nullptr, nullptr, nullptr, nullptr,
				glrows, glcols, &min_val, &max_val);

	constexpr double def_col[3] = {1., 0., 0.};

	/* check if the coordinate system has changed */
	const double testx = data[(glcols + 1) * 3],
	             testy = data[(glcols + 1) * 3 + 1],
	             mapx = -1. + 2. * 1. / (glrows - 1),
	             mapy = -1. + 2. * 1. / (glcols - 1);
	if(testx != mapx || testy != mapy)
		for(uint x = 0; x < glrows; x++)
			for(uint y = 0; y < glcols; y++)
			{
				const uint idx = (x * glcols + y) * 3;
				/* x, y: */
				data[idx + 0] = -1. + 2. * x / (glrows - 1);
				data[idx + 1] = -1. + 2. * y / (glcols - 1);
			}
	/* check if the data is normal... */
	if(max_val == -DBL_MAX || min_val == DBL_MAX ||
		!isfinite(max_val) || !isfinite(min_val))
	{
		warn_msg("the input data is not finite", ERR_ARG);
		min_val =
		max_val = 0.;
		min_norm = -1.;
		max_norm = 1;
		for(uint i = 0; i < glrows * glcols; i++)
		{
			data[i * 3 + 2] = 0.;
			memcpy(&rgb[i * 3], def_col, 3 * sizeof(double));
		}

		return;
	}
	/* ...otherwise, continue as usual */
	const double diff = max_val - min_val;
	double norm;

	if(fabs(min_val) > max_val)
		norm = fabs(min_val);
	else
		norm = max_val;

	min_norm = min_val / norm;
	max_norm = max_val / norm;

	for(uint i = 0; i < glcols * glrows; i++)
	{
		double temp = m_in[i];
		const uint idx = i * 3;
		/* z holds the data */
		data[idx + 2] = temp / norm;
		temp = (temp - min_val) / diff;
		if(temp < .5)
		{
			rgb[idx + 0] = 0.;
			sincosd(M_PI * temp, &rgb[idx + 2], &rgb[idx + 1]);
		}
		else if(temp <= 1.)
		{
			sincosd(M_PI * temp, &rgb[idx + 0], &rgb[idx + 1]);
			rgb[idx + 0] = -rgb[idx + 0];
			rgb[idx + 2] = 0.;
		}
		else
		{
			#ifndef IGYBA_NDEBUG
			if(noisy)
				warn_msg("scaling error", ERR_ARG);
			#endif
			memcpy(&rgb[idx], def_col, 3 * sizeof(double));
		}
	}
}

void viewer::calc_DrawingData(const double *res_pt const m_in, const bool noisy,
							double *res_pt max_val_out,
							double *res_pt min_val_out,
							double *res_pt max_norm_out,
							double *res_pt min_norm_out,
							const uint row_in, const uint col_in,
							double *res_pt data_out, double *res_pt rgb_out)
{
	*max_val_out = -DBL_MAX;
	*min_val_out = DBL_MAX;
	find_minmax(m_in, nullptr, nullptr, nullptr, nullptr,
				row_in, col_in, min_val_out, max_val_out);

	constexpr double def_col[3] = {1., 0., 0.};

	/* check if the coordinate system has changed */
	const double testx = data_out[(col_in + 1) * 3],
	             testy = data_out[(col_in + 1) * 3 + 1],
	             mapx = -1. + 2. * 1. / (row_in - 1),
	             mapy = -1. + 2. * 1. / (col_in - 1);
	if(testx != mapx || testy != mapy)
		for(uint x = 0; x < row_in; x++)
			for(uint y = 0; y < col_in; y++)
			{
				const uint idx = (x * col_in + y) * 3;
				/* x, y: */
				data_out[idx + 0] = -1. + 2. * x / (row_in - 1);
				data_out[idx + 1] = -1. + 2. * y / (col_in - 1);
			}
	/* check if the data is normal... */
	if(*max_val_out == -DBL_MAX || *min_val_out == DBL_MAX ||
		!isfinite(*max_val_out) || !isfinite(*min_val_out))
	{
		warn_msg("the input data is not finite", ERR_ARG);
		*min_val_out = 0.;
		*max_val_out = 0.;
		*min_norm_out = 0.;
		*max_norm_out = 1;
		for(uint i = 0; i < row_in * col_in; i++)
		{
			data_out[i * 3 + 2] = 0.;
			memcpy(&rgb_out[i * 3], def_col, 3 * sizeof(double));
		}
		return;
	}
	/* ...and not 'flat'... */
	double norm = (fabs(*min_val_out) > *max_val_out) ?
	fabs(*min_val_out) : *max_val_out;
	*min_norm_out = *min_val_out / norm;
	*max_norm_out = *max_val_out / norm;
	const double diff = *max_val_out - *min_val_out;
	if(fabs(diff) <= DBL_EPSILON * 1e2)
	{
		*min_norm_out = 0.;
		*max_norm_out = 1;
		warn_msg("the input data is too flat", ERR_ARG);
		for(uint i = 0; i < row_in * col_in; i++)
		{
			data_out[i * 3 + 2] = 0.;
			memcpy(&rgb_out[i * 3], def_col, 3 * sizeof(double));
		}
		return;
	}
	/* ...then continue as normal */
	bool scl_err = false;
	for(uint i = 0; i < col_in * row_in; i++)
	{
		double temp = m_in[i];
		const uint idx = i * 3;
		/* z holds the data */
		data_out[idx + 2] = temp / norm;
		temp = (temp - *min_val_out) / diff;
		if(temp < .5)
		{
			rgb_out[idx + 0] = 0.;
			sincosd(M_PI * temp, &rgb_out[idx + 2], &rgb_out[idx + 1]);
		}
		else if(temp <= 1.)
		{
			sincosd(M_PI * temp, &rgb_out[idx + 0], &rgb_out[idx + 1]);
			rgb_out[idx + 0] = -rgb_out[idx + 0];
			rgb_out[idx + 2] = 0.;
		}
		else
		{
			scl_err = true;
			memcpy(&rgb_out[idx], def_col, 3 * sizeof(double));
		}
	}
	#ifndef IGYBA_NDEBUG
	if(noisy && scl_err)
		warn_msg("scaling error", ERR_ARG);
	#endif
}

void viewer::init_Colorbox(void)
{
	const double diff = 1. / (nstrps - 1.);
	constexpr double def_col[3] = {1., 0., 0.};
	for(uint i = 0; i < nstrps; i++)
	{
		const double temp = diff * i;
		const uint idx = i * 3;
		cb[i] = temp;
		if(temp < .5)
		{
			rgbcb[idx + 0] = 0.;
			sincosd(M_PI * temp, &rgbcb[idx + 2], &rgbcb[idx + 1]);
		}
		else if(temp <= 1.)
		{
			sincosd(M_PI * temp, &rgbcb[idx + 0], &rgbcb[idx + 1]);
			rgbcb[idx + 0] = -rgbcb[idx + 0];
			rgbcb[idx + 2] = 0.;
		}
		else
		{
			#ifndef IGYBA_NDEBUG
			warn_msg("scaling error", ERR_ARG);
			#endif
			memcpy(&rgbcb[idx], def_col, 3 * sizeof(double));
		}
	}
}

void viewer::alloc_DataFromFile(const std::string &fname)
{
	uint nrow,
	     ncol;
	FILE *readfile = fopen(fname.c_str(), "r");
	if(readfile == NULL)
	{
		file_error_msg(fname.c_str(), ERR_ARG);
		exit(EXIT_FAILURE);
	}
	count_entries(readfile, &nrow, &ncol);
	fclose(readfile);
	if(!load_AllocatedMemoryAcquire())
	{
		glrows = nrow;
		glcols = ncol;
		rgb = alloc_3mat(glrows, glcols);
		rgbcb = alloc_3mat(1, nstrps);
		cb = alloc_vec(nstrps);
		data = alloc_3matrix(glrows, glcols, 0.); /**< important to prevent an uninitialized read */
		store_AllocatedMemoryRelease(true);
	}
	else if(load_AllocatedMemory() &&
			(nrow != glrows || ncol != glcols))
	{
		glrows = nrow;
		glcols = ncol;
		col_slice =
		row_slice = 0;
		rgb = realloc_3mat(rgb, glrows, glcols);
		data = realloc_3mat(data, glrows, glcols);
	}
}

void viewer::alloc_DataFromMemory(const uint nrows, const uint ncols)
{
	if(!load_AllocatedMemoryAcquire())
	{
		atmc_glrows.store(nrows, std::memory_order_relaxed);
		glrows = nrows;
		atmc_glcols.store(ncols, std::memory_order_relaxed);
		glcols = ncols;

		rgb = alloc_3mat(glrows, glcols);
		rgbcb = alloc_3mat(1, nstrps);
		cb = alloc_vec(nstrps);
		data = alloc_3matrix(glrows, glcols, 0.);
		store_AllocatedMemoryRelease(true);
	}
	else if((nrows != atmc_glrows.load(std::memory_order_relaxed)) ||
			(ncols != atmc_glcols.load(std::memory_order_relaxed)))
	{
		/* viewer is alive... */
		if(load_IsRunning())
		{
			/* ...then tell it about the news... */
			store_NewDataAvailable(true);
			/* ...and waiting: */
			wait_UntilViewerPause("reallocation takes place.");
		}
		/* then the reallocation can take place safely.
		 * the check in the 'else if' clause has to be atomic, though.
		 */
		atmc_glrows.store(nrows, std::memory_order_relaxed);
		glrows = nrows;
		atmc_glcols.store(ncols, std::memory_order_relaxed);
		glcols = ncols;
		col_slice =
		row_slice = 0;
		rgb = realloc_3mat(rgb, glrows, glcols);
		data = realloc_3mat(data, glrows, glcols);
	}
}

void viewer::fill_DataFromFile(const std::string &fname)
{
	if(!load_AllocatedMemoryAcquire())
		error_msg("memory has not been allocated yet", ERR_ARG);

	if(!load_FilledMemory())
		init_Colorbox();

	double *m = alloc_mat(glrows, glcols);
	fpfile2double(fname.c_str(), m, glrows, glcols);

	fill_DrawingData(m, true);
	free(m);
	store_FilledMemory(true);
}

void viewer::set_DrawingData(const double *res_pt max_val_out,
							const double *res_pt min_val_out,
							const double *res_pt max_norm_out,
							const double *res_pt min_norm_out,
							const double *res_pt data_out,
							const double *res_pt rgb_out)
{
	max_val = *max_val_out;
	min_val = *min_val_out;
	max_norm = *max_norm_out;
	min_norm = *min_norm_out;
	memcpy(data, data_out, 3 * glrows * glcols * sizeof(double));
	memcpy(rgb, rgb_out, 3 * glrows * glcols * sizeof(double));
}

void viewer::launch_Freeglut(int argc, char **argv, const bool threading)
{
	static viewer *vwr = dv1p;

	(*vwr).init_Main(argc, argv);
	atexit(set_OnExitMain);
	if(threading)
		glutDisplayFunc(set_DisplayMainThread);
	else
		glutDisplayFunc(set_DisplayMain);
	glutReshapeFunc(reshape_View);
	glutCloseFunc(close_All);
	glutMotionFunc(MotionHandler);
	glutMouseFunc(MouseHandler);
	glutKeyboardFunc(KeyboardHandler);
	glutSpecialFunc(ArrowKeysHandler);
	glutVisibilityFunc(check_Visibility);
	glutWindowStatusFunc(check_WindowState);
	(*vwr).win_sub = glutCreateSubWindow((*vwr).win_main,
										5, 5,
										(*vwr).win_width / 4,
										(*vwr).win_height / 4);
	glutDisplayFunc(set_SubDisplay);
	(*vwr).store_IsRunning(true);
	glutMainLoop();
}

void viewer::launch_FreeglutTest(int argc, char **argv)
{
	std::string fname = "../viewer/elohtyp.dat";

	static viewer *vwr = dv1p;

	(*vwr).alloc_DataFromFile(fname);
	(*vwr).fill_DataFromFile(fname);

	(*vwr).init_Main(argc, argv);
	atexit(set_OnExitMain);
	glutDisplayFunc(set_DisplayMain);
	glutReshapeFunc(reshape_View);
	glutCloseFunc(close_All);
	glutMotionFunc(MotionHandler);
	glutMouseFunc(MouseHandler);
	glutKeyboardFunc(KeyboardHandler);
	glutSpecialFunc(ArrowKeysHandler);
	glutVisibilityFunc(check_Visibility);
	glutWindowStatusFunc(check_WindowState);
	(*vwr).win_sub = glutCreateSubWindow((*vwr).win_main,
										5, 5,
										(*vwr).win_width / 4,
										(*vwr).win_height / 4);
	glutDisplayFunc(set_SubDisplay);
	(*vwr).store_IsRunning(true);
	glutMainLoop();
}

void viewer::close_Freeglut(void)
{
	if(!load_IsRunning())
		warn_msg("called for no reason", ERR_ARG);
	else
	{
		if(event_SwapDataToViewer.check())
			event_SwapDataToViewer.signal();
		close_All();
	}
}

void viewer::KeyboardHandler(const uchar key, const int x, const int y)
{
	switch(key)
	{
		case 27:
			(*dv1p).esc_down.store(true);
			close_All();
			break;
		case 'm':
			(*dv1p).move_box = !(*dv1p).move_box;
			if((*dv1p).move_box)
				glutSetCursor(GLUT_CURSOR_INFO);
			else
				glutSetCursor(GLUT_CURSOR_CROSSHAIR);
			break;
		case 'R':
			(*dv1p).reset_DrawingControls();
			break;
		case 'p':
			(*dv1p).update_animate.store(true, std::memory_order_relaxed);
			(*dv1p).toggle_Map3DMode();
			break;
		default:
			#ifndef IGYBA_NDEBUG
			iprint(stdout, "key: %i, pos (%i, %i)\n", key, x, y);
			#else
			break;
			#endif
	}
}

void viewer::ArrowKeysHandler(const int a_keys, const int x, const int y)
{
	switch(a_keys)
	{
		case GLUT_KEY_F1:
			(*dv1p).toggle_Rotation();
			break;
		case GLUT_KEY_F12:
			glutFullScreen();
			break;
		case GLUT_KEY_F11:
			glutReshapeWindow((*dv1p).std_win_width, (*dv1p).std_win_height);
			glutPositionWindow(0, 0);
			break;
		case GLUT_KEY_LEFT:
			if((*dv1p).load_MapMode())
			{
				if((*dv1p).row_slice)
				{
					(*dv1p).update_animate.store(true, std::memory_order_relaxed);
					(*dv1p).row_slice--;
				}
			}
			break;
		case GLUT_KEY_RIGHT:
			if((*dv1p).load_MapMode())
			{
				(*dv1p).update_animate.store(true, std::memory_order_relaxed);
				(*dv1p).row_slice++;
			}
			break;
		case GLUT_KEY_UP:
			if(!(*dv1p).load_MapMode())
			{
				if((*dv1p).mov_lvl + .1 <= 1.)
				{
					(*dv1p).update_animate.store(true, std::memory_order_relaxed);

					if((*dv1p).zoom <= (*dv1p).zoom_std)
						(*dv1p).mov_lvl += .1;
					else if((*dv1p).zoom >= (*dv1p).zoom_std * (1. + .1))
						(*dv1p).mov_lvl += .02;
					else if((*dv1p).zoom >= (*dv1p).zoom_std * (1. + .2))
						(*dv1p).mov_lvl += .01;
					else
						(*dv1p).mov_lvl += .005;
				}
			}
			else
			{
				(*dv1p).update_animate.store(true, std::memory_order_relaxed);
				(*dv1p).col_slice++;
			}
			break;
		case GLUT_KEY_DOWN:
			if(!(*dv1p).load_MapMode())
			{
				if((*dv1p).mov_lvl - .1 >= (*dv1p).min_norm)
				{
					(*dv1p).update_animate.store(true, std::memory_order_relaxed);

					if((*dv1p).zoom <= (*dv1p).zoom_std)
						(*dv1p).mov_lvl -= .1;
					else if((*dv1p).zoom >= (*dv1p).zoom_std * (1. + .1))
						(*dv1p).mov_lvl -= .02;
					else if((*dv1p).zoom >= (*dv1p).zoom_std * (1. + .2))
						(*dv1p).mov_lvl -= .01;
					else
						(*dv1p).mov_lvl -= .005;
				}
				else
					(*dv1p).mov_lvl = (*dv1p).min_norm;
			}
			else
			{
				if((*dv1p).col_slice)
				{
					(*dv1p).update_animate.store(true, std::memory_order_relaxed);
					(*dv1p).col_slice--;
				}
			}
			break;
		default:
			#ifndef IGYBA_NDEBUG
			iprint(stdout, "key %i, pos (%i, %i)\n", a_keys, x, y);
			#else
			break;
			#endif
	}
}

void viewer::TrackballHandler(const int mode, const int button, const int state, const int x, const int y)
{
	static double startMX = .0, startMY = .0;
	switch(mode)
	{
		case VIEWER_KNOWMOUSEBUTTON:
			if(!button && !state && !(*dv1p).rmb_down)
			{
				int vwprt[4], ny;
				double modlview[16], prjction[16];
				float wx = x, wy, wz;
				glGetIntegerv(GL_VIEWPORT, vwprt);
				wy = ny = vwprt[3] - y;
				glGetDoublev(GL_MODELVIEW_MATRIX, modlview);
				glGetDoublev(GL_PROJECTION_MATRIX, prjction);
				glReadPixels(x, ny, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &wz);
				gluUnProject(wx, wy, wz, modlview, prjction, vwprt,
								&(*dv1p).wc[0], &(*dv1p).wc[1], &(*dv1p).wc[2]);
				#ifndef IGYBA_NDEBUG
				iprint(stdout, "wx: %g, wy: %g, wz: %g\n",
						(*dv1p).wc[0], (*dv1p).wc[1], (*dv1p).wc[2]);
				#endif
				(*dv1p).update_animate.store(true, std::memory_order_relaxed);
			}
			if(!(*dv1p).load_MapMode())
			{
				if(!button && !state && !(*dv1p).move_box && !(*dv1p).rmb_down) /* rotate */
				{
					(*dv1p).lmb_down = true;
					startMY = x - (*dv1p).rot_y;
					startMX = y - (*dv1p).rot_x;
				}
				else if(!button && !state && (*dv1p).move_box && !(*dv1p).rmb_down) /* translate */
				{
					(*dv1p).lmb_down = true;
					startMY = y + (*dv1p).mov_y * (*dv1p).win_height / 4.;
					startMX = x - (*dv1p).mov_x * (*dv1p).win_width / 4.;
				}
				else if(!button && state)
					(*dv1p).lmb_down = false;
				else if(button == 2 && !state && !(*dv1p).lmb_down) /* zoom */
				{
					(*dv1p).rmb_down = true;
					startMY = y;
				}
				else if(button == 2 && state)
					(*dv1p).rmb_down = false;
				break;
			}
		case VIEWER_MOUSEMOTION:
			if(!(*dv1p).load_MapMode())
			{
				if((*dv1p).lmb_down && !(*dv1p).move_box && !(*dv1p).rmb_down) /* rotate */
				{
					(*dv1p).rot_x = y - startMX;
					(*dv1p).rot_y = x - startMY;
				}
				else if((*dv1p).lmb_down && (*dv1p).move_box && !(*dv1p).rmb_down) /* translate */
				{
					(*dv1p).mov_x = (x - startMX) / (*dv1p).win_width * 4.;
					(*dv1p).mov_y = -(y - startMY) / (*dv1p).win_height * 4.;
				}
				else if((*dv1p).rmb_down && !(*dv1p).lmb_down) /* zoom */
				{
					double temp, scl = (1. + (y - startMY) / (*dv1p).zoom_std);
					const double zoom_max = (*dv1p).zoom_std * (1. + 1.5),
					             zoom_min = (*dv1p).zoom_std * .2;
					if((*dv1p).zoom > zoom_max)
						temp = zoom_max;
					else if((*dv1p).zoom == zoom_max && scl > 1.)
						break;
					else if((*dv1p).zoom == zoom_min && scl < 1.)
						break;
					else if((*dv1p).zoom >= zoom_min)
						temp = (*dv1p).zoom * scl;
					else
						temp = zoom_min;

					(*dv1p).zoom = temp;
					#ifndef IGYBA_NDEBUG
					iprint(stdout, "zoom: %g\n", (*dv1p).zoom);
					#endif
					startMY = y;
				}
				break;
			}
		case VIEWER_RESET:
			startMX = startMY = 0.;
	}
}

void viewer::MotionHandler(const int x, const int y)
{
	#ifndef IGYBA_NDEBUG
	iprint(stdout, "mouse motion %d, %d\n", x, y);
	#endif
	TrackballHandler(VIEWER_MOUSEMOTION, 0, 0, x, y);
}

void viewer::MouseHandler(const int button, const int state, const int x, const int y)
{
	#ifndef IGYBA_NDEBUG
	iprint(stdout, "b = %d, s = %d, (%d, %d)\n", button, state, x, y);
	#endif
	TrackballHandler(VIEWER_KNOWMOUSEBUTTON, button, state, x, y);
}

void viewer::set_DisplayMainThread(void)
{
	if((*dv1p).load_NewDataAvailable()) /**< check for news. has to be true in the first run! */
	{
		/* now we can wait for the data to be swapped over */
		(*dv1p).event_SwapDataToViewer.wait();
		/* ...and we are ready to be stopped again! */
		(*dv1p).event_SwapDataToViewer.reset();
	}

	static uchar prev = 0xFF;

	if(!(*dv1p).load_MapMode())
	{
		if(prev || prev == 0xFF)
		{
			(*dv1p).update_animate.store(true, std::memory_order_relaxed);
			(*dv1p).display_3dView(true);
		}
		else
			(*dv1p).display_3dView(false);
	}
	else
	{
		if(!prev || prev == 0xFF)
		{
			(*dv1p).update_animate.store(true, std::memory_order_relaxed);
			(*dv1p).display_MapView(true);
		}
		else
			(*dv1p).display_MapView(false);
	}
	prev = (*dv1p).load_MapMode();
}

void viewer::animate_View(void)
{
	static uint avgtime = 0;
	if((*dv1p).update_animate.load(std::memory_order_relaxed) ||
		(*dv1p).lmb_down ||
		(*dv1p).rmb_down ||
		(*dv1p).load_RotateBox())
	{
		if(!(*dv1p).load_ConstantAnimation())
			(*dv1p).update_animate.store(false, std::memory_order_relaxed);
		constexpr uint avg = 20;
		static uint frams = 0;
		if(frams++ < avg) /**< collect enough averaging before displaying the fps */
			avgtime = glutGet(GLUT_ELAPSED_TIME);
		else
		{
			char title[64];
			static uint off = 0;
			snprintf(title, 64,
					"viewer - %.1f fps",
					1e3 * avg / (avgtime - off));
			glutSetWindowTitle(title);
			frams = 0;
			avgtime = off = glutGet(GLUT_ELAPSED_TIME);
		}

		glutSetWindow((*dv1p).win_main);
		glutPostRedisplay();

	}
	else
		glutSetWindowTitle("viewer - 0 fps");

	std::this_thread::sleep_for(std::chrono::milliseconds(10));

	if(!(*dv1p).lmb_down && (*dv1p).load_RotateBox())
		(*dv1p).rot_y += .02 * (glutGet(GLUT_ELAPSED_TIME) - avgtime);

	(*dv1p).rot_y = fmod((*dv1p).rot_y, 360.);
	(*dv1p).rot_x = fmod((*dv1p).rot_x, 360.);
}

void viewer::set_DisplayMain(void)
{
	static uchar prev = 0xFF;

	if(!(*dv1p).load_MapMode())
	{
		if(prev || prev == 0xFF)
		{
			(*dv1p).update_animate.store(true, std::memory_order_relaxed);
			(*dv1p).display_3dView(true);
		}
		else
			(*dv1p).display_3dView(false);
	}
	else
	{
		if(!prev || prev == 0xFF)
		{
			(*dv1p).update_animate.store(true, std::memory_order_relaxed);
			(*dv1p).display_MapView(true);
		}
		else
			(*dv1p).display_MapView(false);
	}
	prev = (*dv1p).load_MapMode();
}

void viewer::set_SubDisplay(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluOrtho2D(0., 1., 0., 1.);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	glBegin(GL_LINE_LOOP);
	glColor3dv(viewer::cwhite);
	glVertex2d(0., 0.);
	glVertex2d(0., 1.);
	glVertex2d(1., 1.);
	glVertex2d(1., 0.);
	glEnd();

	(*dv1p).print_BlockString2d("these keys may have the one or the other effect: "
								"F1, F11, F12, Esc, R, m, p, Mouse Buttons. "
								"this list might be incomplete.",
								23,
								.02, .9,
								0., -.08);
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glutSwapBuffers();
}

void viewer::reshape_View(const int width, const int height)
{
	if(width != (int)(*dv1p).win_width || height != (int)(*dv1p).win_height)
	{
		glViewport(0, 0, width, height);
		(*dv1p).win_width = width;
		(*dv1p).win_height = height;
	}
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	if(!(*dv1p).load_MapMode())
		glOrtho(-.5 * (*dv1p).win_width, .5 * (*dv1p).win_width,
				-.5 * (*dv1p).win_height, .5 * (*dv1p).win_height,
				viewer::near_clip,
				viewer::far_clip);
	else
		gluOrtho2D(-1., 1, -1., 1);
}

void viewer::close_All(void)
{
	static bool escaped = false;

	if(glutGetWindow() && (*dv1p).esc_down.load(std::memory_order_relaxed))
	{
		(*dv1p).esc_down.store(false, std::memory_order_relaxed);
		escaped = true;
		glutIdleFunc(NULL);
		iprint(stdout,
				"'%s' reached by 'Esc'. resetting variables . ", __func__);
		fflush(stdout);
		(*dv1p).reset_MemberVariables();
		iprint(stdout, "done\n");
		glutDestroyWindow((*dv1p).win_main);
		glutLeaveMainLoop();
	}
	else if(glutGetWindow() &&
			!(*dv1p).esc_down.load(std::memory_order_relaxed) &&
			!escaped)
	{
		iprint(stdout,
				"'%s' reached by closing window. resetting variables . ",
				__func__);
		fflush(stdout);
		(*dv1p).reset_MemberVariables();
		iprint(stdout, "done\n");
		glutLeaveMainLoop(); /**< calling this guy twice is OK */
	}
	else if(glutGetWindow() &&
			!(*dv1p).esc_down.load(std::memory_order_relaxed) &&
			escaped) /**< happens when closing the window by 'Esc' */
	{
		iprint(stdout, "'%s': I've been here before\n", __func__);
		escaped = false;
	}
	else
		warn_msg("this shouldn't happen and likely ends in at least traceable data", ERR_ARG);
}

void viewer::wait_UntilViewerPause(const std::string &text)
{
	iprint(stdout, "waiting for viewer thread to pause .");
	while(true)
	{
		if(event_SwapDataToViewer.check())
			break;
		std::this_thread::sleep_for(std::chrono::milliseconds(80));
		iprint(stdout, ".");
		fflush(stdout);
	}
	iprint(stdout, " done. %s\n", text.c_str());
}

void viewer::toggle_Idling(void)
{
	constant_animate.store(!constant_animate.load(std::memory_order_acquire),
							std::memory_order_release);
	if((*dv1p).load_ConstantAnimation())
		(*dv1p).update_animate.store(true, std::memory_order_relaxed);
}

void viewer::toggle_Map3DMode(void)
{
	map_mode.store(!map_mode.load(std::memory_order_acquire),
				std::memory_order_release);
}

void viewer::toggle_Rotation(void)
{
	rot_box.store(!rot_box.load(std::memory_order_acquire),
				std::memory_order_release);
}

/** \brief Calling this function does invoke an atomic store, but doesn't
allow for any reasoning of a sequential sanity outside of the viewer thread.
 *
 * \param b const bool
 * \return void
 *
 */
void viewer::store_MapMode(const bool b)
{
	map_mode.store(b, std::memory_order_relaxed);
}

bool viewer::load_MapMode(void)
{
	return map_mode.load(std::memory_order_relaxed);
}

void viewer::store_RotateBox(const bool b)
{
	rot_box.store(b, std::memory_order_relaxed);
}

bool viewer::load_RotateBox(void)
{
	return rot_box.load(std::memory_order_relaxed);
}

void viewer::store_AllocatedMemory(const bool b)
{
	allocd.store(b, std::memory_order_relaxed);
}

bool viewer::load_AllocatedMemory(void)
{
	return allocd.load(std::memory_order_relaxed);
}

void viewer::store_AllocatedMemoryRelease(const bool b)
{
	allocd.store(b, std::memory_order_release);
}

bool viewer::load_AllocatedMemoryAcquire(void)
{
	return allocd.load(std::memory_order_acquire);
}

void viewer::store_FilledMemory(const bool b)
{
	filled.store(b);
}

bool viewer::load_FilledMemory(void)
{
	return filled.load();
}

void viewer::store_NewDataAvailable(const bool b)
{
	new_data.store(b, std::memory_order_release);
}

bool viewer::load_NewDataAvailable(void)
{
	return new_data.load(std::memory_order_acquire);
}

void viewer::store_ConstantAnimation(const bool b)
{
	constant_animate.store(b, std::memory_order_relaxed);
}

bool viewer::load_ConstantAnimation(void)
{
	return constant_animate.load(std::memory_order_relaxed);
}

void viewer::store_IsRunning(const bool b)
{
	running.store(b, std::memory_order_release);
}

bool viewer::load_IsRunning(void)
{
	return running.load(std::memory_order_acquire);
}

void viewer::store_GlRows(const uint n)
{
	atmc_glrows.store(n);
}

uint viewer::load_GlRows(void)
{
	return atmc_glrows.load();
}

void viewer::store_GlCols(const uint n)
{
	atmc_glcols.store(n);
}

uint viewer::load_GlCols(void)
{
	return atmc_glcols.load();
}
