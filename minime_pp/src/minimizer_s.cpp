#include "minime_class.h"
#include "minimizer_s.h"
#include "random.h"
#include <assert.h>

constexpr static uint MAXEQ = 60, /**< used in gaussj */
                      MAXPARAM = 8;

constexpr static double SIM_TOLMIN = 10e-9,
                        SIM_TOLFACT = 3.,
                        SIM_LENFACT = 3.,
                        SIM_STARTLEN = .1,
                        SIM_STARTTOL = 10e-4,
                        QUICKOUT = 10e-7,
                        PUNISH_MIN = .1,
                        PUNISH_BETA = .1,
                        MAR_REPTOL = 10e-4,
                        MAR_QUICKOUT = 10e-6,
                        MAR_LAMMIN = -18.,
                        MAR_LAMMAX = 12.,
                        MAR_BRENTTOL = 1e-2,
                        MAR_DERIV_H = 1e-4, /**< decreasing this might cause
                        trouble when computing the Hessian */
                        SQRT_EPSILON = 1e-8;

struct pset
{
	double parm[MAXPARAM],
		   eval;
};

/* internal header */

static double punish(const double x);
static void swap_pset(pset *p1, pset *p2);
static double get_span(const pset *p);
static int check_equal(double x1, double x2);
static int ptcompare(const void *p1, const void *p2);
static void status(const pset *res_pt ptp, const char *res_pt meth,
				const char *res_pt text);
static void rndperm(int *p, const uint n);
static void startvectors(const uint n, pset x[MAXPARAM + 1],
						const uint irandom);
static int gaussj(double *res_pt a, const uint n, double *res_pt b);
static double ssq(pset *pp);
static double amotry(pset p[MAXPARAM], pset *psump, const uint ihi,
					const double fac);
static void amoeba(pset p[MAXPARAM], const double ftol, const bool noisy);
static void simplex(pset *p0, double startlen, double tol, const bool noisy);
static double fminbr(double a, double b, double (*f) (double x), double tol);
static double fmarq(double loglam);
static void marq(pset *p, const bool noisy);
static void manymarquardt(pset *ptp, const bool noisy);
static double hessian(pset *p);
static void covar(pset *p, void *extra);

/* Global but static definitions. */
struct lmar
{
	double a[MAXPARAM * MAXPARAM], /* Hessian for mar */
		   b[MAXPARAM]; /* Right hand side for mar */
	pset p0,
		 ptry;
};

struct mini_struct
{
	unsigned long nfunc;
	double paralpha[MAXPARAM],
		   logmin[MAXPARAM],
		   logmax[MAXPARAM];
	uint nparm_tot,
		 nparm,
		 pindex[MAXPARAM],
		 funcp_nout;
	parameter *pa_io;
	void (*funcp)(double *, double *); /* 1. parameter is input, 2. one is
	output per reference. */
};

static mini_struct min_s;
static lmar lmar_c;

#pragma omp threadprivate(min_s, lmar_c)

/* functions */

double punish(const double x)
{
	double ret;
	if(x > 3.)
		ret = 16.085536923187667741 * (x - 3.) + 12.585536923187667741;
	else
		ret = exp(x) - x - x * x * .5;
	return ret;
}

void swap_pset(pset *p1, pset *p2)
{
	pset temp;
	memcpy(&temp, p1, sizeof(pset));
	memcpy(p1, p2, sizeof(pset));
	memcpy(p2, &temp, sizeof(pset));
}

double get_span(const pset *p)
{
	double dist, max = -1.;
	for(uint i = 0; i <= min_s.nparm; i++)
		for(uint j = 0; j < i; j++)
		{
			dist = 0.;
			for(uint k = 0; k < min_s.nparm; k++)
				dist += (p[i].parm[k] - p[j].parm[k]) * (p[i].parm[k] - p[j].parm[k]);
			dist = sqrt(dist);
			if(dist > max)
				max = dist;
		}
	return max;
}

int check_equal(double x1, double x2)
{
	constexpr double thresh = 1e-10;
	double y1 = fabs(x1),
	       y2 = fabs(x2);
	if(y2 > y1)
		y1 = y2;
	if(y1 < thresh)
		y1 = thresh;
	return (fabs(x1 - x2) / y1 < thresh);
}

static int ptcompare(const void *p1, const void *p2)
{
	const pset *x1 = static_cast<const pset *>(p1),
	           *x2 = static_cast<const pset *>(p2);
	if((*x1).eval < (*x2).eval)
		return -1;
	else if((*x1).eval > (*x2).eval)
		return 1;
	return 0;
}

void status(const pset *res_pt ptp, const char *res_pt meth, const char *res_pt text)
{
	const double sq = (*ptp).eval;
	iprint(stdout, "%5s %8lu %12g ", meth, min_s.nfunc, sq);
	for(uint i = 0; i < min_s.nparm; i++)
	{
		double ep = (*ptp).parm[i];
		parameter *pap = min_s.pa_io + min_s.pindex[i];
		if((*pap).log)
			ep = exp(ep);
		iprint(stdout, "%10.3f ", ep);
	}
	iprint(stdout, "%s\n", text);
}

void rndperm(int *p, const uint n)
{
	uint *left = (uint *)malloc(n * sizeof(uint));
	for(uint i = 0; i < n; i++)
		left[i] = i;
	for(int nleft = n; nleft > 0; --nleft)
	{
		const uint sel = irand(nleft);
		p[n - nleft] = left[sel];
		for(int i = sel; i < nleft - 1; i++)
			left[i] = left[i + 1];
	}
	free(left);
}

/**
 * Erzeugen von (n+1) Einheitsvektoren des n-dim. Raumes, die Eckpunkte
 * eines gleichmaessigen Simplex darstellen (mit dem Nullpunkt im
 * Zentrum). x[i][j] ist die j-te Komponente des i-ten Vektors. Alle
 * Vektoren haben die Laenge 1, das Skalarprodukt zwischen je 2
 * verschiedenen Vektoren hat den Wert -1/n, und der Abstand zwischen je
 * 2 Vektoren betraegt sqrt(2+2/n). Falls irandom==1,
 * werden die Koordinaten mit einer Zufallspermutation vertauscht, und
 * mit 50% Wahrscheinlichkeit der ganze Simplex invertiert, um im statistischen
 * Mittel wirklich alle Raumrichtungen gleich zu behandeln.
 */
void startvectors(const uint n, pset x[MAXPARAM + 1], const uint irandom)
{
	int *rp = (int *)malloc(n * sizeof(int));
	uint invert = 0;
	double a;
	if(irandom)
	{
		rndperm(rp, n);
		invert = irand(2);
	}
	for(uint j = 0; j < n; j++) /**< assert(n <= MAXPARAM); */
	{
		/* spaltenweise berechnen (j zaehlt Komponenten) */
		int jp = (irandom == 1) ? rp[j] : (int) j;
		for(uint i = 0; i < j; i++)
			/* i zaehlt Vektoren; Ueberdiagonalelemente sind 0 */
			x[i].parm[jp] = 0.;
		if(!j) /* erste Komponente einfacher */
			a = 1.; /* Diagonalelement */
		else /* weitere Komponenten */
			a = sqrt((double)((n + 1) * (n - j)) / ((double)(n * (n - j + 1)))); /* Diagonalelement */
		if(irandom && invert)
			a *= -1.;
		x[j].parm[jp] = a;
		a /= (j - n); /* negativ */
		for(uint i = j + 1; i <= n; i++)
			x[i].parm[jp] = a; /* Unterdiagonalelemente */
	}
	free(rp);
}

int gaussj(double *res_pt a, const uint n, double *res_pt b)
{
	#define SWAP(a,b) {swapx = a; a = b; b = swapx;}
	uint indxc[MAXEQ], indxr[MAXEQ], ipiv[MAXEQ];
	uint icol = 0, irow = 0;
	double dum, swapx;
	assert(n <= MAXEQ);
	for(uint j = 0; j < n; j++)
		ipiv[j] = 0;
	for(uint i = 0; i < n; i++)
	{
		double big = 0.;
		for(uint j = 0; j < n; j++)
			if(ipiv[j] != 1)
				for(uint k = 0; k < n; k++)
				{
					if(!ipiv[k])
					{
						if(fabs(a[j * n + k]) >= big)
						{
							big = fabs(a[j * n + k]);
							irow = j;
							icol = k;
						}
						fflush(stdout);
					}
					else if(ipiv[k] > 1)
					{
						iprint(stdout, " gaussj failed (1)\n");
						return 1;
					}
				}
		++(ipiv[icol]);
		if(irow != icol)
		{
			for(uint l = 0; l < n; l++)
				SWAP(a[irow * n + l], a[icol * n + l])
			SWAP(b[irow], b[icol])
		}
		indxr[i] = irow;
		indxc[i] = icol;
		if(a[icol * n + icol] == 0.)
			/* iprint(stdout, " gaussj failed (2)\n"); */
			return 1;

		double pivinv = 1. / a[icol * n + icol];
		a[icol * n + icol] = 1.;
		for(uint l = 0; l < n; l++)
			a[icol * n + l] *= pivinv;
		b[icol] *= pivinv;
		for(uint ll = 0; ll < n; ll++)
			if(ll != icol)
			{
				dum = a[ll * n + icol];
				a[ll * n + icol] = 0.;
				for(uint l = 0; l < n; l++)
					a[ll * n + l] -= a[icol * n + l] * dum;
				b[ll] -= b[icol] * dum;
			}
	}
	for(int l = n - 1; l >= 0; l--)
		if(indxr[l] != indxc[l])
			for(uint k = 0; k < n; k++)
				SWAP(a[k * n + indxr[l]], a[k * n + indxc[l]])

	return 0;
}

double ssq(pset *pp)
{
	double ret,
	       xx,
	       *x = (double *)malloc(min_s.nparm_tot * sizeof(double)),
	       *y = (double *)malloc(min_s.funcp_nout * sizeof(double));
	uchar ssq_lift;
	for(uint i = 0; i < min_s.nparm; i++)
	{
		int i0 = min_s.pindex[i]; /* min_s.pindex in original parameter */
		if(min_s.pa_io[i0].log)
			min_s.pa_io[i0].val = exp(pp->parm[i]);
		else
			min_s.pa_io[i0].val = pp->parm[i];
	}

	for(uint i = 0; i < min_s.nparm_tot; i++)
		x[i] = min_s.pa_io[i].val;

	min_s.funcp(x, y);
	ret = y[0];
	if(ret < PUNISH_MIN)
	{
		ret += PUNISH_MIN;
		ssq_lift = 1;
	}
	else
		ssq_lift = 0;
	for(uint i = 0; i < min_s.nparm; i++)
	{
		const double p = pp->parm[i];
		if(p < min_s.logmin[i])
		{
			xx = min_s.logmin[i] - p;
			ret *= punish(xx * min_s.paralpha[i]);
		}
		else if(p > min_s.logmax[i])
		{
			xx = p - min_s.logmax[i];
			ret *= punish(xx * min_s.paralpha[i]);
		}
	}
	if(ssq_lift)
		ret -= PUNISH_MIN;
	pp->eval = ret;
	++min_s.nfunc;
	free(x);
	free(y);
	return ret;
}

double amotry(pset p[MAXPARAM], pset *psump, const uint ihi, const double fac)
{
	uint j;
	double fac1, fac2, ytry;
	pset ptest;
	fac1 = (1. - fac) / min_s.nparm;
	fac2 = fac1 - fac;
	for(j = 0; j < min_s.nparm; j++)
		ptest.parm[j] = psump->parm[j] * fac1 - p[ihi].parm[j] * fac2;
	ssq(&ptest);
	ytry = ptest.eval;
	if(ytry < p[ihi].eval)
	{
		for(j = 0; j < min_s.nparm; j++)
			psump->parm[j] += ptest.parm[j] - p[ihi].parm[j];

		memcpy(&(p[ihi]), &ptest, sizeof(pset));
	}
	return ytry;
}

void amoeba(pset p[MAXPARAM], const double ftol, const bool noisy)
{
	uint nsame = 0;
	pset psum; /* holds only coordinates, no ssq */
	double ymin, oldlow, oldsum, span = 1.;

	for(uint j = 0; j < MAXPARAM; j++)
		psum.parm[j] = 0.;

	for(uint i = 0; i < min_s.nparm + 1; i++)
		for(uint j = 0; j < min_s.nparm; j++)
			psum.parm[j] += p[i].parm[j];

	ymin = oldlow = oldsum = 9e99;
	for(;;)
	{
		qsort(p, min_s.nparm + 1, sizeof(pset), ptcompare);
		double sum = 0.;
		for(uint i = 0; i <= min_s.nparm; i++)
			sum += p[i].eval;
		if(check_equal(p[0].eval, oldlow) && check_equal(sum, oldsum))
		{
			/* no progress */
			++nsame;
			if(nsame >= min_s.nparm)
				break;
		}
		else
		{
			oldlow = p[0].eval;
			oldsum = sum;
			nsame = 0;
		}
		uint ilo = 0,
		     ihi = min_s.nparm,
		     inhi = min_s.nparm - 1;
		double rtol = (fabs(p[ihi].eval) + fabs(p[ilo].eval));
		if(rtol < SIM_TOLMIN)
			rtol = SIM_TOLMIN;
		rtol = 2 * fabs(p[ihi].eval - p[ilo].eval) / rtol;
		if(rtol < ftol || span < 1e-13)
		{
			if(ilo != 0)
				swap_pset(&(p[0]), &(p[ilo]));
			break;
		}
		span = get_span(p);
		if(p[ilo].eval < ymin)
		{
			/* nur Verbesserungen drucken */
			ymin = .9 * p[ilo].eval;
			if(noisy)
				status(&(p[0]), "splx", "");
		}
		double ytry = amotry(p, &psum, ihi, -1.);

		if(ytry <= p[ilo].eval)
			ytry = amotry(p, &psum, ihi, 2.);
		else if(ytry >= p[inhi].eval)
		{
			double ysave = p[ihi].eval;
			ytry = amotry(p, &psum, ihi, .5);
			if(ytry >= ysave)
			{
				/* shrinking */
				for(uint i = 0; i < min_s.nparm + 1; i++)
					if(i != ilo)
					{
						for(uint j = 0; j < min_s.nparm; j++)
							p[i].parm[j] = .5 * (p[i].parm[j] + p[ilo].parm[j]);

						ssq(&(p[i]));
					}

				for(uint j = 0; j < min_s.nparm; j++)
					psum.parm[j] = 0.;
				for(uint i = 0; i < min_s.nparm + 1; i++)
					for(uint j = 0; j < min_s.nparm; j++)
						psum.parm[j] += p[i].parm[j];
				ymin = 9e99;
			}
		}
	}
}

void simplex(pset *p0, double startlen, double tol, const bool noisy)
{
	pset p[MAXPARAM + 1],
	     psave;
	memcpy(&psave, p0, sizeof(pset));
	startvectors(min_s.nparm, p, 1);
	for(uint i = 0; i <= min_s.nparm; i++)
	{
		for(uint j = 0; j < min_s.nparm; j++)
			p[i].parm[j] = startlen * p[i].parm[j] + p0->parm[j];
		ssq(&(p[i]));
	}
	amoeba(p, tol, noisy);
	if(p[0].eval < p0->eval)
		memcpy(p0, &(p[0]), sizeof(pset));
	assert(p0->eval <= psave.eval);
	if(noisy)
		status(p0, "Sout", " ");
}

/* find minimum of 1-D function, from NETLIB */
double fminbr(double a, double b, double(*f)(double x), double tol)
{
	double x, v, w, /* abscissas */
	       fx,      /* f(x) */
	       fv,      /* f(v) */
	       fw;      /* f(w) */
	const double r = (3. - sqrt(5.)) / 2; /* gold section ratio */
	assert(tol > 0. && b > a);
	v = a + r * (b - a);
	fv = (*f)(v); /* first step - always gold section */
	x = v;
	w = v;
	fx = fv;
	fw = fv;
	for(;;) /* main iteration loop */
	{
		double range = b - a, /* range over which the minimum */
		                      /* is searched for */
		       middle_range = (a + b) / 2.,
		       tol_act = /* actual tolerance */
		       SQRT_EPSILON * fabs(x) + tol / 3.,
		       new_step; /* step at this iteration */

		if(fabs(x - middle_range) + range / 2 <= 2 * tol_act)
			return x; /* acceptable approx. is found */
		/* obtain the gold section step */
		new_step = r * (x < middle_range ? b - x : a - x);
		/* decide if the interpolation can be tried */
		if(fabs(x - w) >= tol_act)	/*if x and w are distinct */
		{
			/* interpolation may be tried */
			double p, /* Interpolation step is calcula- */
			       q, /* ted as p/q; division operation is delayed until last moment */
			       t;
			t = (x - w) * (fx - fv);
			q = (x - v) * (fx - fw);
			p = (x - v) * q - (x - w) * t;
			q = 2 * (q - t);
			if(q > 0.) /* q was calculated with the op- */
				p = -p; /* positive sign; make q positive */
			else /* and assign possible minus to */
				q = -q; /* p */
			if(fabs(p) < fabs(new_step * q) && /* if x+p/q falls in [a,b] */
			   p > q * (a - x + 2 * tol_act) && /* not too close to a and */
			   p < q * (b - x - 2 * tol_act)) /* b, and isn't too large */
				new_step = p / q; /* it is accepted*/
			/*
			 * if p/q is too large then the
			 * gold section procedure can
			 * reduce [a,b] range to more
			 * extent */
		}
		if(fabs(new_step) < tol_act)
		{
			/* adjust the step to be not less */
			if(new_step > 0.) /* than tolerance */
				new_step = tol_act;
			else
				new_step = -tol_act;
		}
		/* obtain the next approximation to min */
		{ /**> and reduce the enveloping range */
			double t = x + new_step, /* tentative point for the min */
			       ft = (*f)(t);
			if(ft <= fx)
			{
				/* t is a better approximation */
				if(t < x) /* reduce the range so that */
					b = x; /* t would fall within it */
				else
					a = x;
				v = w;
				w = x;
				x = t; /* assign the best approx to x  */
				fv = fw;
				fw = fx;
				fx = ft;
			}
			else /* x remains the better approx  */
			{
				if(t < x) /* reduce the range enclosing x */
					a = t;
				else
					b = t;
				if(ft <= fw || w == x)
				{
					v = w;
					w = t;
					fv = fw;
					fw = ft;
				}
				else if(ft <= fv || v == x || v == w)
				{
					v = t;
					fv = ft;
				}
			}
		} /**< end of block */
	} /* end of loop */
}

/* ssq for mar */
double fmarq(const double loglam)
{
	double alam[MAXPARAM * MAXPARAM],
	       delta[MAXPARAM];

	memcpy(alam, lmar_c.a, MAXPARAM * MAXPARAM * sizeof(double));
	memcpy(delta, lmar_c.b, MAXPARAM * sizeof(double));

	for(uint i = 0; i < min_s.nparm; i++)
		alam[i * min_s.nparm + i] *= 1. + exp(loglam);

	if(gaussj(alam, min_s.nparm, delta))
		return 1e30;
	memcpy(&lmar_c.ptry, &lmar_c.p0, sizeof(pset));

	for(uint i = 0; i < min_s.nparm; i++)
	{
		lmar_c.ptry.parm[i] += delta[i];
		if(lmar_c.ptry.parm[i] < min_s.logmin[i])
			lmar_c.ptry.parm[i] = min_s.logmin[i];
		if(lmar_c.ptry.parm[i] > min_s.logmax[i])
			lmar_c.ptry.parm[i] = min_s.logmax[i];
	}
	ssq(&lmar_c.ptry);
	return lmar_c.ptry.eval;
}

void marq(pset *p, const bool noisy)
{
	const double ssq0 = hessian(p);
	/* double lambest = */ fminbr(MAR_LAMMIN, MAR_LAMMAX, fmarq, MAR_BRENTTOL);
	if(lmar_c.ptry.eval < ssq0) /* improvement */
	{
		memcpy(p, &lmar_c.ptry, sizeof(pset));
		if(noisy)
			status(p, "marq", " ");
	}
	else
		memcpy(p, &lmar_c.p0, sizeof(pset)); /* restore starting point */
}

void manymarquardt(pset *ptp, const bool noisy)
{
	for(;;)
	{
		const double sqold = ptp->eval;
		marq(ptp, noisy);
		if((sqold - ptp->eval) / sqold < MAR_REPTOL)
			break;
		if(ptp->eval < MAR_QUICKOUT)
			break;
	}
}

/** \brief Computes thee Hessian matrix
 *
 * \param p pset*
 * \return double
 *
 */
double hessian(pset *p)
{
	constexpr double h = MAR_DERIV_H;
	const double sq0 = ssq(p);
	memcpy(&lmar_c.p0, p, sizeof(pset));
	for(uint i = 0; i < min_s.nparm; ++i)
	{
		const double p_i = p->parm[i];
		p->parm[i] = p_i + h;
		const double sq1 = ssq(p) - sq0;
		p->parm[i] = p_i - h;
		const double sq2 = ssq(p) - sq0;
		p->parm[i] = p_i;
		lmar_c.a[i * min_s.nparm + i] = (sq1 + sq2) / (2. * h * h); /* alpha = deriv/2 */
		lmar_c.b[i] = (sq1 - sq2) / (-4. * h); /* beta = deriv/(-2) */
	}
	/* off-diagonal elements */
	for(uint i = 0; i < min_s.nparm; ++i)
		for(uint j = 0; j < i; ++j)
		{
			const double p_i = p->parm[i],
			             p_j = p->parm[j];
			p->parm[i] = p_i + h;
			p->parm[j] = p_j + h;
			double sq1 = ssq(p);
			p->parm[i] = p_i - h;
			p->parm[j] = p_j + h;
			sq1 -= ssq(p);
			p->parm[i] = p_i + h;
			p->parm[j] = p_j - h;
			sq1 -= ssq(p);
			p->parm[i] = p_i - h;
			p->parm[j] = p_j - h;
			sq1 += ssq(p);
			p->parm[i] = p_i;
			p->parm[j] = p_j;
			lmar_c.a[i * min_s.nparm + j] =
			lmar_c.a[j * min_s.nparm + i] = sq1 / (8. * h * h); /* alpha = deriv / 2 */
		}
	return sq0;
}

double minimizer(void (*f)(double *, double *), const uint np, parameter *ppa,
				const uint f_nout, const bool noisy, const bool tofile)
{
	pset p;
	min_s.pa_io = ppa;
	min_s.funcp = f;
	assert(np <= MAXPARAM);
	min_s.nparm_tot = np;
	min_s.nfunc = 0;
	min_s.nparm = 0;
	min_s.funcp_nout = f_nout;
	frandseed(time(0));

	for(uint i = 0; i < np; i++)
	{
		if(min_s.pa_io[i].fit)
		{
			if(min_s.pa_io[i].log)
			{
				if(exp(min_s.pa_io[i].maxv) / exp(min_s.pa_io[i].minv) > 2.)
					min_s.paralpha[min_s.nparm] = 1.57 / PUNISH_BETA;
				else
					min_s.paralpha[min_s.nparm] =
					    1.57 / (PUNISH_BETA *
					    (exp(min_s.pa_io[i].maxv) / exp(min_s.pa_io[i].minv) - 1.));
				p.parm[min_s.nparm] = log(min_s.pa_io[i].init);
				min_s.logmin[min_s.nparm] = log(min_s.pa_io[i].minv);
				min_s.logmax[min_s.nparm] = log(min_s.pa_io[i].maxv);
			}
			else
			{
				min_s.paralpha[min_s.nparm] =
				    1.57 / ((min_s.pa_io[i].maxv - min_s.pa_io[i].minv) * PUNISH_BETA);
				p.parm[min_s.nparm] = min_s.pa_io[i].init;
				min_s.logmin[min_s.nparm] = min_s.pa_io[i].minv;
				min_s.logmax[min_s.nparm] = min_s.pa_io[i].maxv;
			}
			min_s.pindex[min_s.nparm] = i;
			++min_s.nparm;
		}
		else
			min_s.pa_io[i].val = min_s.pa_io[i].init;
	}
	if(noisy)
	{
		parameter *pap;
		iprint(stdout, "%5s %8s %12s", " ", " ", " ");
		for(uint i = 0; i < min_s.nparm; i++)
		{
			pap = min_s.pa_io + min_s.pindex[i];
			iprint(stdout, "%10s ", (*pap).name.c_str());
		}
		iprint(stdout, "\n%5s %8s %12s", " ", " ", " ");
		for(uint i = 0; i < min_s.nparm; i++)
		{
			pap = min_s.pa_io + min_s.pindex[i];
			iprint(stdout, "[%9s]", (*pap).unit.c_str());
		}
		iprint(stdout, "\n");
	}
	ssq(&p);
	if(noisy)
		status(&p, "entr", "");

	manymarquardt(&p, noisy);
	if(p.eval > QUICKOUT)
	{
		double span = SIM_STARTLEN, tol = SIM_STARTTOL;
		do
		{
			simplex(&p, span, tol, noisy);
			tol /= SIM_TOLFACT;
			span /= SIM_LENFACT;
			manymarquardt(&p, noisy);
			if(p.eval < QUICKOUT)
				break;
		}
		while(tol > SIM_TOLMIN);
	}
	ssq(&p);
	if(noisy)
	{
		status(&p, "final", "");

		for(uint i = 0; i < np; i++)
			iprint(stdout, "%s = %.8g %s %s\n",
					min_s.pa_io[i].name.c_str(),
					min_s.pa_io[i].val,
					min_s.pa_io[i].unit.c_str(),
					(min_s.pa_io[i].fit) ? " fitted" : "");
	}
	if(tofile)
	{
		std::string timebuf;
		get_DateAndTime(timebuf);

		FILE *fp = fopen("minime_output.txt", "a");
		if(fp == NULL)
			perror("error opening file");
		fprintf(fp, "\n%s\n", timebuf.c_str());
		for(uint i = 0; i < np; i++)
			fprintf(fp, "{'%s', '%s', %g, %g, %g, %d, %d, 0}\n",
					min_s.pa_io[i].name.c_str(),
					min_s.pa_io[i].unit.c_str(),
					min_s.pa_io[i].val,
					min_s.pa_io[i].minv,
					min_s.pa_io[i].maxv,
					min_s.pa_io[i].log,
					min_s.pa_io[i].fit);
		fclose(fp);
	}
	return p.eval;
}

void v_banane(double *res_pt x, double *res_pt y)
{
	y[0] = (100. * POW2(x[1] - POW2(x[0])) + POW2(1. - x[0]));
	y[1] = -1.; /**< just a test */
}

void covar(pset *p, void *extra)
{
	mini_struct fitd = min_s;
	FILE *logfp = fopen("minime_output_covar.txt", "a");
	double *ainv = (double *)malloc(fitd.nparm * fitd.nparm * sizeof(double)),
	       *delta = (double *)malloc(fitd.nparm * sizeof(double));
	char msg[256];

	hessian(p);
	memcpy(ainv, lmar_c.a, fitd.nparm * fitd.nparm * sizeof (double));
	for(uint i = 0; i < fitd.nparm; i++)
		delta[i] = 0;
	if(gaussj(ainv, fitd.nparm, delta))
	{
		snprintf(msg, 255, "Hessian is singular. Skipped evaluation of covar.");
		status(p, "", msg);
		free(ainv);
		free(delta);
		fclose(logfp);
		return;
	}
	fprintf(logfp, "\nCorrelation matrix:\n         ");
	for(uint i = 0; i < fitd.nparm; i++)
		fprintf(logfp, "%s ", fitd.pa_io[fitd.pindex[i]].name.c_str());
	fprintf(logfp, "\n");
	for(uint i = 0; i < fitd.nparm; i++)
	{
		fprintf(logfp, "%s ", fitd.pa_io[fitd.pindex[i]].name.c_str());
		for(uint j = 0; j <= i; j++)
		{
			double temp = ainv[i * fitd.nparm + i] * ainv[j * fitd.nparm + j];
			if(temp > 0.)
			{
				double corr = ainv[i * fitd.nparm + j] / sqrt (temp);
				fprintf(logfp, "%g ", corr);
			}
			else
				fprintf(logfp, " ******* ");
		}
		putc('\n', logfp);
	}
	double cfact = p->eval / (50. - fitd.nparm); // ghh
	fprintf(logfp, "\nBest parameter estimates:\n");
	for(uint i = 0; i < fitd.nparm; i++)
	{
		double ep, rvar;
		if(ainv[i * fitd.nparm + i] < 0.)
			status(p, "", "newcovar[i, i] < 0.");
		if(fitd.pa_io[fitd.pindex[i]].log)
		{
			ep = exp(p->parm[i]);
			rvar = (exp(sqrt(cfact * (fabs(ainv[i * fitd.nparm + i])))) - 1.);
		}
		else
		{
			ep = p->parm[i];
			rvar = (sqrt(cfact * (fabs(ainv[i * fitd.nparm + i])))) / ep;
		}
		if(rvar < .5)
			fprintf(logfp, "%s = %30.20g +- %g (%.3g%%) ",
					fitd.pa_io[fitd.pindex[i]].name.c_str(), ep,
					rvar * ep, 100. * rvar);
		else
			fprintf(logfp, "%s = %g */ %g ",
					fitd.pa_io[fitd.pindex[i]].name.c_str(), ep, rvar + 1.);
		if(ep < fitd.pa_io[fitd.pindex[i]].minv)
			fprintf(logfp, "< MIN!\n");
		else if(ep > fitd.pa_io[fitd.pindex[i]].maxv)
			fprintf(logfp, "> MAX!\n");
		else
			fprintf(logfp, "\n");
	}
	free(ainv);
	free(delta);
	fclose(logfp);
}
