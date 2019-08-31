/**
 * clemens schaefermeier, 2014
 * clesch(at)fysik.dtu.dk
 *
 * minime
 *
 ** @todo
 *
 */

#include <iostream>
#include "minime_class.h"

int main(void)
{
	fp_error(ERR_ARG);
	/* return test(); */
/*
	double lambda = 1.064,
	       scl = 1.;
	minime_profile dm1;
    dm1.set_Wavelength(lambda);
    dm1.set_PixelToUm(scl);
    dm1.set_Plotting(true);
	dm1.fill_DataFromFile("../viewer/elohtyp.dat");
	dm1.fit_GaussEllip();
*/

	double lambda = 1.064,
	       z0_init = 0.,
	       z0_min = -2e3,
	       z0_max = -z0_min,
	       temp = 0.;
	minime_propagation pro1;
	pro1.set_Plotting(true);

	std::string input = "";
	std::stringstream instream;
	while(true)
	{
		std::cout << "First, I need to know the wavelength in um: ";
		std::getline(std::cin, input);
		instream << input;
		if(instream >> lambda)
			break;
		std::cout << "Invalid number, try again." << std::endl;
		instream.clear();
		instream.str(std::string());
	}

	std::cout << "Next, the waist position can be entered. If nothing is " \
	"entered, standard values will be taken. So: " << std::endl <<
	"What is the estimated waist position in mm: ";
	instream.clear();
	instream.str(std::string());
	std::getline(std::cin, input);
	instream << input;
	if(instream >> temp)
		z0_init = temp;
	else
		std::cout << "No number provided, taking " << z0_init << std::endl;

	std::cout << "Estimate the minimal (which is probably negative) position " \
	"of the waist in mm: ";
	instream.clear();
	instream.str(std::string());
	std::getline(std::cin, input);
	instream << input;
	if(instream >> temp)
		z0_min = temp;
	else
		std::cout << "No number provided, taking " << z0_min << std::endl;

	std::cout << "Estimate the maximum (which is larger than the initial guess) "
	"position of the waist in mm: ";
	instream.clear();
	instream.str(std::string());
	std::getline(std::cin, input);
	instream << input;
	if(instream >> temp)
		z0_max = temp;
	else
		std::cout << "No number provided, taking " << z0_max << std::endl;

	std::cout << "You entered: " << std::endl <<
	" Wavelength      : " << lambda << " um" << std::endl <<
	" Initial guess   : " << z0_init << " mm" << std::endl <<
	" Minimal position: " << z0_min << " mm" << std::endl <<
	" Maximal position: " << z0_max << " mm" << std::endl <<
	"Hit enter to continue." << std::endl;
	getchar();

	pro1.set_Wavelength(lambda);
	pro1.fill_DataFromFile("beampropagation.dat");
	pro1.fit_BeamProp(z0_init, z0_min, z0_max);

	/* Write streakyground file from multiple dark images. */
//	startracker("init/initavgdarkFINAL", 8, "bad/streakyground.dat", 256, 256);
	/* Write histogram. */
//	file_histo("bad/streakyground.dat", 16384);
	/* Write average of overexposed images. */
//	startracker("init/initavgbrightFINAL", 3, "bad/higherground.dat", 256, 256);
//	file_histo("bad/higherground.dat", 16384);
	/* Find bad pixels. */
//	find_bad("bad/higherground.dat", "bad/streakyground.dat", 0, 2.75, 1, 256, 256);

	return EXIT_SUCCESS;
}


int test(void)
{
	#include "minimizer_s.h"
	uint np = 2, noisy = 1;
	parameter *test = minime::alloc_Parameter(2);

	test[0].name = "P1";
	test[0].unit = "E";
	test[0].init = 0.;
	test[0].fit = 1;
	test[0].minv = -20.;
	test[0].maxv = 20.;
	test[0].log = 0;

	test[1].name = "P2";
	test[1].unit = "Z";
	test[1].init = 0.;
	test[1].fit = 1;
	test[1].minv = -20.;
	test[1].maxv = 20.;
	test[1].log = 0;

	minimizer(v_banane, np, test, 2, noisy, 0, 0);
	free(test);

	return EXIT_SUCCESS;
}
