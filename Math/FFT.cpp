#include <math.h>
#include <complex>
using namespace std;

#ifndef M_PI
#define M_PI        3.14159265358979323846
#endif

#define REV_FFT 1

void FastSplitFourierTransform(float * x,float * y,int m,int d)
{
	int n,l,e,f,i,j,o,k;

	float u,v,z,c,s,p,q,r,t,w,a;


	n=1 << m;


	for (l = 0; l < m; l++)
	{

		u = float(1.);
		v = float(0.);

		f=1<<(m - l - 1);
		e=f<<1;

		z=float(M_PI/f);

		c=cos(z);
		s=sin(z);

		if (! (d & REV_FFT)) s =- s;

		for (j = 0; j < f; j++)
		{
			for(i = j; i < n; i+=e)
			{
				o =i + f;
				p = x[i] + x[o];
				r = x[i] - x[o];
				q = y[i] + y[o];
				t = y[i] - y[o];
				x[o] = r * u - t * v;
				y[o] = t * u + r * v;
				x[i] = p;
				y[i] = q;
			} // i-loop
			w = u * c - v * s;
			v = v * c + u * s;
			u = w;
		}  // j - loop
	} // l - loop

	j = 0;

	for (i = 0; i < n - 1; i++)
	{
		if (i < j)
		{
			p = x[j];
			q = y[j];
			x[j] = x[i];
			y[j] = y[i];
			x[i] = p;
			y[i] = q;
		}
		k=n/2;
		while(k<=j)
		{
			j=j-k;
			k=k/2;
		}
		j+=k;
	}

	if(! (d & REV_FFT)) return;

	a=float(1.0/n);
	for(k=0;k<n;k++)
	{
		x[k]*=a;
		y[k]*=a;
	}

	return;
}

template<class T>  void FastFourierTransform(complex<T> * x, int order_power,
											int direction)
{
	int n, l, e, f, i, j, o, k;

	T z, a;
	complex<T> rt, uv, cs, pq;

	n=1 << order_power;


	for (l = 0; l < order_power; l++)
	{
		uv = 1.;

		f=1<<(order_power - l - 1);
		e=f<<1;

		z=T(M_PI/f);

		cs = complex<T>(cos(z), sin(z));

		if (! (direction & REV_FFT)) cs.imag(-cs.imag());

		for(j = 0; j < f; j++)
		{
			for(i = j; i < n; i += e)
			{
				o=i+f;
				pq = x[i] + x[o];
				rt = x[i] - x[o];
				x[o] = rt * uv;
				x[i] = pq;
			} // i-loop
			uv *= cs;
		}  // j - loop
	} // l - loop

	j = 0;

	for (i = 0; i < n-1; i++)
	{
		if (i < j)
		{
			pq = x[j];
			x[j] = x[i];
			x[i] = pq;
		}
		k = n / 2;
		while(k <= j)
		{
			j = j - k;
			k = k / 2;
		}
		j += k;
	}

	if (! (direction & REV_FFT)) return;

	a=T(1.0/n);
	for (k = 0; k < n; k++)
	{
		x[k] *= a;
	}

	return;
}

// C++ draft does not require type specification after 'template'
// keyword in the explicit instantiation, but MSC does
//template<float> void FastFourierTransform(complex<float> *, int, int);
//template<double> void FastFourierTransform(complex<double> *, int, int);
static void (*f1)(complex<float> *, int, int) = FastFourierTransform;
static void (*f2)(complex<double> *, int, int) = FastFourierTransform;
