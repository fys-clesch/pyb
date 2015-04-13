/**
 * clemens schaefermeier, 2014
 * clesch(at)fysik.dtu.dk
 */

#include "grabber_class.h"
#include "test.h"

int main(void)
{
	test_pic2();
//	test_pic3();
	test_vid();

	fprintf(stdout, "accessing camera.. ");
	fflush(stdout);
	VideoCapture cap;
	cap.open(0);

	if(!cap.isOpened())
	{
		error_msg("no access to camera", ERR_ARG);
		return EXIT_FAILURE;
	}
	else if(!cap.grab())
	{
		error_msg("no initial frame to grab", ERR_ARG);
		return EXIT_FAILURE;
	}

	fprintf(stdout, "done\n");

	namedWindow("Camera", CV_WINDOW_AUTOSIZE);

	grabber run;
	run.set_MainWindowName("Camera");
	run.set_Pix2UmScale(1.);
	cap.retrieve(run.in);
	run.update_Mats_RgbAndFp();
	setMouseCallback("Camera", run.cast_static_set_MouseEvent, &run);

	uint i = 1;
	while(i)
	{
		if(!cap.read(run.in))
		{
			fprintf(stderr, "no frame to grab\n");
			break;
		}
		else if(i == 114) /* r */
		{
			run.set_Background();
		}
		else if(i == 65618) /* shift + r */
		{
			run.unset_Background();
		}
		else
		{
			run.update_Mats_RgbAndFp();
			run.get_Moments();
			run.draw_Moments();
			run.show_Im_RGB();
		}
		i = (waitKey(30) == 27) ? false : true;
	}

	cap.release();
	destroyAllWindows();

	return EXIT_SUCCESS;
}
