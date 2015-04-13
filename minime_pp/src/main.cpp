/**
 * clemens schaefermeier, 2014
 * clesch(at)fysik.dtu.dk
 *
 * minime
 *
 ** @todo
 *
 */

#include "minime_class.h"

int main(void)
{
	fp_error(ERR_ARG);
	/* return test(); */
	double lambda = 1.064e-3,
	       scl = 1.;

	minime dm1;
    dm1.set_Wavelength(lambda);
    dm1.set_PixelToUm(scl);
    dm1.set_Plotting(false);
	dm1.fill_DataFromFile("../viewer/elohtyp.dat");

	dm1.fit_GaussEllip();

	/* write streakyground file from multiple dark images */
//	startracker("init/initavgdarkFINAL", 8, "bad/streakyground.dat", 256, 256);
	/* Write histogram */
//	file_histo("bad/streakyground.dat", 16384);
	/* write average of overexposed images */
//	startracker("init/initavgbrightFINAL", 3, "bad/higherground.dat", 256, 256);
//	file_histo("bad/higherground.dat", 16384);
	/* find bad pixels */
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
	test[0].min = -20.;
	test[0].max = 20.;
	test[0].log = 0;

	test[1].name = "P2";
	test[1].unit = "Z";
	test[1].init = 0.;
	test[1].fit = 1;
	test[1].min = -20.;
	test[1].max = 20.;
	test[1].log = 0;

	minimizer(v_banane, np, test, 2, noisy, 0);
	free(test);

	return EXIT_SUCCESS;
}
