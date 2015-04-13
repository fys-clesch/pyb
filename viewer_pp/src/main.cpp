/**
 * clemens schaefermeier, 2014
 * clesch(at)fysik.dtu.dk
 *
 * TODO:
 *
 */

#include "viewer_class.h"
#define NO_OMP 1

int main(int argc, char **argv)
{
	viewer dv1;

	std::string fname = "../viewer/elohtyp.dat";

	dv1.alloc_DataFromFile(fname);
	dv1.fill_DataFromFile(fname);

	#if NO_OMP
	viewer::launch_Freeglut(argc, argv);
	/*
	std::this_thread::sleep_for(std::chrono::seconds(1));
	dv1.alloc_DataFromFile(fname);
	dv1.fill_DataFromFile(fname);
	viewer::launch_Freeglut(argc, argv);
	*/
	#else
	#pragma omp parallel
	{
		#pragma omp sections nowait
		{
			#pragma omp section
			{
				viewer::launch_Freeglut(argc, argv);
			}
			#pragma omp section
			{
				printf("This is a test of OpenMP\n");
				for(uint i = 0; i < 3; i++)
				{
					std::this_thread::sleep_for(std::chrono::seconds(1));
					printf("%u\n", i);
				}
			}
		}
		printf("Almost done! ;-)\n");
	}

	std::this_thread::sleep_for(std::chrono::seconds(1));
	dv1.alloc_DataFromFile(fname);
	dv1.fill_DataFromFile(fname);
	viewer::launch_Freeglut(argc, argv);
	#endif

	return EXIT_SUCCESS;
}
