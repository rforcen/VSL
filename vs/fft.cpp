//
//  fft.cpp
//  Transformada rapida de Fourier
//

#include "fft.h"
#include <math.h>
#include <stdlib.h>
#include <cfloat>
#include <cstring>
namespace fft {
//
// 	fft.c -> Vector de entrada de tamaño n+2!!
//	transformada rapida de Fourier FFT
// 	y = vector de entrada de long=n+2 simetria Hermitiana
//       [Re(0),Re(1), ..., Re(N-1)] (direct transform)
//       [Re(0),Im(0),Re(1),Im(1),...,Re(N/2),Im(N/2)] (inverse)
//			da la salida en el mismo vector.
// 	kind = tipo=0 directa, 1 inversa
//	n = numero de valores
//   m = log(n)/log(2) ; 2^m=n

static const int smoothIters = 1;

// max fft items
#define L2MAX_LEN 19
#define MAX_LEN (1 << L2MAX_LEN)
#define PI M_PI
#define TWOPI (M_PI * 2)
#define SQRT2 M_SQRT2

//   									m= 1 2 3  4  5  6   7   8   9   10
//   11...
// funciona con potencias de 2:  2,4,8,16,32,64,128,256,512,1024,2048...
// m=potencia de 2 de n
// y[n+2] len important !!
//
void fft(double *y, int m) {
  int j = 1, is, id, k, n1, i, i0, n2, n4, n8, i1, i2, i3, i4, i5, i6, i7, i8;
  int n;
  double xt, r1, t1, t2, t3, t4, t5, t6, cc1, ss1, cc3, ss3;
  double e, a, a3;
  double *x;
  // static double x1 [MAX_LEN+2];
  double *x1;
  static double *ccc1[L2MAX_LEN + 1], *sss1[L2MAX_LEN + 1],
      *ccc3[L2MAX_LEN + 1], *sss3[L2MAX_LEN + 1];
  static int last_n = 0, last_m = 0;

  // calc n=2^m
  n = 1 << m;

  /******* Check transform length *********************************************/
  if (n > MAX_LEN || m > L2MAX_LEN) return;

  /******* Compute table of sines/cosines *************************************/
  if (n != last_n) /* Transform size has changed */
  {
    if (last_n != 0) /* Correct , it does not work !!! */
      for (k = 3; k <= last_m; k++) {
        delete[] sss1[k];
        delete[] sss3[k];
        delete[] ccc1[k];
        delete[] ccc3[k];
      }
    n2 = 4;
    for (k = 3; k <= m; k++) /* Get space for new ones */
    {
      n2 <<= 1;
      n8 = n2 >> 3;
      e = a = TWOPI / n2;
      sss1[k] = new double[n8 + 1];
      sss3[k] = new double[n8 + 1];
      ccc1[k] = new double[n8 + 1];
      ccc3[k] = new double[n8 + 1];
      if (sss1[k] == NULL || sss3[k] == NULL || ccc1[k] == NULL ||
          ccc3[k] == NULL)
        return;
      for (j = 2; j <= n8; j++) {
        a3 = a + a + a;
        ccc1[k][j] = cos(a);
        sss1[k][j] = sin(a);
        ccc3[k][j] = cos(a3);
        sss3[k][j] = sin(a3);
        a += e;
      }
    }
    last_n = n; /* Update last size */
    last_m = m;
  }

  /********************* Copy items to temp. array ****************************/

  x1 = new double[MAX_LEN + 2];
  memcpy(x1, y, (n + 2) * sizeof(double));

  x = x1 - 1; /*** Dirty trick to simulate that indices begin at 1 ***/

  /* Direct transform */
  /*******************  Digit reverse  counter  *******************************/
  {
    j = 1;
    n1 = n - 1;
    for (i = 1; i <= n1; i++) {
      if (i < j) {
        xt = x[j];
        x[j] = x[i];
        x[i] = xt;
      }
      k = n >> 1; /* n/2; */
      while (k < j) {
        j -= k;
        k >>= 1; /* k/=2 */
      }
      j += k;
    }

    /******************  Length two butterflies
     * *********************************/

    is = 1;
    id = 4;
    do {
      for (i0 = is; i0 <= n; i0 += id) {
        i1 = i0 + 1;
        r1 = x[i0];
        x[i0] = r1 + x[i1];
        x[i1] = r1 - x[i1];
      }
      is = (id << 1) - 1;
      id <<= 2; /* id *= 4; */
    } while (is < n);

    /*************** L-shaped butterflies
     * **************************************/

    n2 = 2;
    for (k = 2; k <= m; k++) {
      n2 <<= 1;     /* n2 *= 2; */
      n4 = n2 >> 2; /* n2/4;    */
      n8 = n2 >> 3; /* n2/8;    */
      is = 0;
      id = n2 << 1;
      do {
        for (i = is; i < n; i += id) {
          i1 = i + 1;
          i2 = i1 + n4;
          i3 = i2 + n4;
          i4 = i3 + n4;
          t1 = x[i4] + x[i3];
          x[i4] -= x[i3];
          x[i3] = x[i1] - t1;
          x[i1] += t1;
          if (n4 != 1) {
            i1 += n8;
            i2 += n8;
            i3 += n8;
            i4 += n8;
            t1 = (x[i3] + x[i4]) / SQRT2;
            t2 = (x[i3] - x[i4]) / SQRT2;
            x[i4] = x[i2] - t1;
            x[i3] = -x[i2] - t1;
            x[i2] = x[i1] - t2;
            x[i1] += t2;
          }
        }
        is = (id << 1) - n2;
        id <<= 2; /* id *= 4; */
      } while (is < n);
      for (j = 2; j <= n8; j++) {
        cc1 = ccc1[k][j];
        ss1 = sss1[k][j];
        cc3 = ccc3[k][j];
        ss3 = sss3[k][j];
        is = 0;
        id = n2 << 1;
        do {
          for (i = is; i < n; i += id) {
            i1 = i + j;
            i2 = i1 + n4;
            i3 = i2 + n4;
            i4 = i3 + n4;
            i5 = i + n4 - j + 2;
            i6 = i5 + n4;
            i7 = i6 + n4;
            i8 = i7 + n4;
            t1 = x[i3] * cc1 + x[i7] * ss1;
            t2 = x[i7] * cc1 - x[i3] * ss1;
            t3 = x[i4] * cc3 + x[i8] * ss3;
            t4 = x[i8] * cc3 - x[i4] * ss3;
            t5 = t1 + t3;
            t6 = t2 + t4;
            t3 = t1 - t3;
            t4 = t2 - t4;
            t2 = x[i6] + t6;
            x[i3] = t6 - x[i6];
            x[i8] = t2;
            t2 = x[i2] - t3;
            x[i7] = -x[i2] - t3;
            x[i4] = t2;
            t1 = x[i1] + t5;
            x[i6] = x[i1] - t5;
            x[i1] = t1;
            t1 = x[i5] + t4;
            x[i5] -= t4;
            x[i2] = t1;
          }
          is = (id << 1) - n2;
          id <<= 2; /* id *=4;*/
        } while (is < n);
      }
    }
    /******* Rerrange data as in program header, copy to y
     * **********************/
    j = n >> 1; /* n/2 */
    y[0] = x1[0];
    y[1] = 0.0;
    y[n] = x1[j];
    y[n + 1] = 0.0;
    for (i = 1, k = 2; i < j; i++, k += 2) {
      y[k] = x1[i];
      y[k + 1] = x1[n - i];
    }
  }
  delete[] x1;
}

//************************
/*** Inverse transform **/
//************************
void fft_inv(double *y, int m) {
  int j = 1, is, id, k, n1, i, i0, n2, n4, n8, i1, i2, i3, i4, i5, i6, i7, i8;
  int n;
  double xt, r1, t1, t2, t3, t4, t5, cc1, ss1, cc3, ss3, fn;
  double e, a, a3;
  double *x;
  // static double x1 [MAX_LEN+2];
  double *x1;
  static double *ccc1[L2MAX_LEN + 1], *sss1[L2MAX_LEN + 1],
      *ccc3[L2MAX_LEN + 1], *sss3[L2MAX_LEN + 1];
  static int last_n = 0, last_m = 0;

  // calc n=2^m
  n = 1 << m;

  /******* Check transform length *********************************************/
  if (n > MAX_LEN || m > L2MAX_LEN) return;

  /******* Compute table of sines/cosines *************************************/
  if (n != last_n) /* Transform size has changed */
  {
    if (last_n != 0) /* Correct , it does not work !!! */
      for (k = 3; k <= last_m; k++) {
        delete[] sss1[k];
        delete[] sss3[k];
        delete[] ccc1[k];
        delete[] ccc3[k];
      }
    n2 = 4;
    for (k = 3; k <= m; k++) /* Get space for new ones */
    {
      n2 <<= 1;
      n8 = n2 >> 3;
      e = a = TWOPI / n2;
      sss1[k] = new double[n8 + 1];
      sss3[k] = new double[n8 + 1];
      ccc1[k] = new double[n8 + 1];
      ccc3[k] = new double[n8 + 1];
      if (sss1[k] == NULL || sss3[k] == NULL || ccc1[k] == NULL ||
          ccc3[k] == NULL)
        return;
      for (j = 2; j <= n8; j++) {
        a3 = a + a + a;
        ccc1[k][j] = cos(a);
        sss1[k][j] = sin(a);
        ccc3[k][j] = cos(a3);
        sss3[k][j] = sin(a3);
        a += e;
      }
    }
    last_n = n; /* Update last size */
    last_m = m;
  }

  /********************* Copy items to temp. array ****************************/

  x1 = new double[MAX_LEN + 2];
  memcpy(x1, y, (n + 2) * sizeof(double));

  {
    j = n >> 1; /* n/2 */
    y[0] = x1[0];
    y[j] = x1[n];
    for (i = 1, k = 2; i < j; i++, k += 2) {
      y[i] = x1[k];
      y[n - i] = x1[k + 1];
    }
    x = y - 1; /*** Operate with reversed data ***/
  }

  {
    n2 = n << 1;
    for (k = 1; k < m; k++) {
      is = 0;
      id = n2;
      n2 >>= 1;
      n4 = n2 >> 2;
      n8 = n4 >> 1;
      do {
        for (i = is; i < n; i += id) {
          i1 = i + 1;
          i2 = i1 + n4;
          i3 = i2 + n4;
          i4 = i3 + n4;
          t1 = x[i1] - x[i3];
          x[i1] += x[i3];
          x[i2] *= 2.0;
          x[i3] = t1 - 2.0 * x[i4];
          x[i4] = t1 + 2.0 * x[i4];
          if (n4 != 1) {
            i1 += n8;
            i2 += n8;
            i3 += n8;
            i4 += n8;
            t1 = (x[i2] - x[i1]) / SQRT2;
            t2 = (x[i4] + x[i3]) / SQRT2;
            x[i1] += x[i2];
            x[i2] = x[i4] - x[i3];
            x[i3] = 2.0 * (-t2 - t1);
            x[i4] = 2.0 * (-t2 + t1);
          }
        }
        is = (id << 1) - n2;
        id <<= 2;
      } while (is < n - 1);
      for (j = 2; j <= n8; j++) {
        cc1 = ccc1[m - k + 1][j]; /*** Note that cosine table is inverted]] ***/
        ss1 = sss1[m - k + 1][j];
        cc3 = ccc3[m - k + 1][j];
        ss3 = sss3[m - k + 1][j];
        is = 0;
        id = n2 << 1;
        do {
          for (i = is; i < n; i += id) {
            i1 = i + j;
            i2 = i1 + n4;
            i3 = i2 + n4;
            i4 = i3 + n4;
            i5 = i + n4 - j + 2;
            i6 = i5 + n4;
            i7 = i6 + n4;
            i8 = i7 + n4;
            t1 = x[i1] - x[i6];
            x[i1] += x[i6];
            t2 = x[i5] - x[i2];
            x[i5] += x[i2];
            t3 = x[i8] + x[i3];
            x[i6] = x[i8] - x[i3];
            t4 = x[i4] + x[i7];
            x[i2] = x[i4] - x[i7];
            t5 = t1 - t4;
            t1 += t4;
            t4 = t2 - t3;
            t2 += t3;
            x[i3] = t5 * cc1 + t4 * ss1;
            x[i7] = -t4 * cc1 + t5 * ss1;
            x[i4] = t1 * cc3 - t2 * ss3;
            x[i8] = t2 * cc3 + t1 * ss3;
          }
          is = (id << 1) - n2;
          id <<= 2;
        } while (is < n - 1);
      }
    }

    /*******************  Length two butterflies
     * *******************************/

    is = 1;
    id = 4;
    do {
      for (i0 = is; i0 <= n; i0 += id) {
        i1 = i0 + 1;
        r1 = x[i0];
        x[i0] = r1 + x[i1];
        x[i1] = r1 - x[i1];
      }
      is = (id << 1) - 1;
      id <<= 2;
    } while (is < n);

    /*******************  Digit reverse counter
     * ********************************/

    j = 1;
    n1 = n - 1;
    for (i = 1; i <= n1; i++) {
      if (i < j) {
        xt = x[j];
        x[j] = x[i];
        x[i] = xt;
      }
      k = n >> 1;
      while (k < j) {
        j -= k;
        k >>= 1;
      }
      j += k;
    }

    /*********** Re-scale output
     * ************************************************/
    fn = n;
    for (i = 1; i <= n; i++) x[i] /= fn;
  }
  delete[] x1;
}

//---------------------------------------------------------------------------
// fft formant calc.
//---------------------------------------------------------------------------

// descendant order of power
static int _sort_pm(const void *p1, const void *p2) {
  if (((Tpm *)p1)->v > ((Tpm *)p2)->v)
    return -1;
  else
    return +1;
}

//---------------------------------------------------------------------------
// calculates 'n' formants of a fft value
// needs a smooth fft vector.
//---------------------------------------------------------------------------
void Formants(double *vfft, double *HzFrm, double *PwrFrm, double *phFrm, int n,
              int &nform) {
#define forall(n) for (i = 3; i < n; i++)
  Tpm *pm;
  int i;
  int p, ip = 0;

  if (!nform) return;

  pm = new Tpm[nform];

  // after smooth we've got: 0..0..local_max1..0..0..local_max2....
  forall(n - 3) {
    while (vfft[i] == 0 && i < n - 3) i++;  // skip ceros
    double max;
    for (max = -DBL_MAX, p = -1; vfft[i] != 0 && i < n - 3; i++)  // local max
      if (vfft[i] > max) {
        max = vfft[i];
        p = i;
      }
    if (p != -1) {  // found something...?
      pm[ip].p = p;
      pm[ip].v = max;
      if (phFrm) pm[ip].ph = phFrm[p];  // save local max
      if (++ip >= nform) ip--;
    }
  }
  // find 'nform' greater max (formants)
  qsort(pm, ip, sizeof(*pm), _sort_pm);  // sort desc. and retrieve 3 first

  if (ip < nform) nform = ip;  // at least 'nform' values.
  for (i = 0; i < nform; i++) {
    HzFrm[i] = pm[i].p;   // pos of fft to convert to Hz.
    PwrFrm[i] = pm[i].v;  // already in % max power
    if (phFrm) phFrm[i] = pm[i].ph;
  }
  delete[] pm;
}

void Formants(double *vfft, double *HzFrm, double *PwrFrm, int n, int &nform) {
  Formants(vfft, HzFrm, PwrFrm, NULL, n, nform);
}

//---------------------------------------------------------------------------
// ProcessFFT
// input: FFT vector.
// 1.modulus, 2.max, 3. smooth, 4. log scale, 5. scale if != 0
// 6.Hz conversion
//---------------------------------------------------------------------------
void ProcessFFT(
    double *vfft, double *vx, int n,  // i/o vfft i/o vx in Hz (if !NULL)
    double &max, double &HzMax,       // scaled values in power & Hz
    bool smooth, bool scalelog,       // smooth?, log scale?
    double scale, int samples,        // scale==0 don't scale
    double &coherence,                // calc coherence
    int &nform,                       // number of formants (must smooth!)
    double *HzFrm, double *PowFrm,    // vector containing formants
    double *phFrm)  // scaled vfft values and samples for Hz conversion.
{
  int pm;
  double sfx = (double)samples / n / 2., sfy;
  double s = 0, s2 = 0, stddev;  // for coherence

  vfft[0] = vfft[1] = vfft[2] = 0;  // first three values=0
  max = -DBL_MAX;
  pm = -1;
  // max, modulus, log, x vaues in Hz
  for (int i = 3; i < n; i++) {  // ignore 3 first values
    double &va = vfft[i];
    if (va < 0) va = -va;  // modulus fabs
    {
      s += va;
      s2 += va * va;
    }  // for stddev in coherence calc.
    if (scalelog) va = (va) ? log(va) : 0;
    if (va > max) {
      max = va;
      pm = i;
    }                      // max
    if (vx) vx[i] *= sfx;  // Hz in x if not NULL
  }

  {
    double N = n - 3;                                 // coherence calc.
    stddev = sqrt((N * s2 - s * s) / (N * (N - 1)));  // stddev calc
    coherence =
        (stddev) ? 1 - (s / N) / stddev : 0;  // coherence calc. mean/stddev.
  }

  HzMax = pm * sfx;  // convert to Hz

  // scale
  if (scale && max) {
    sfy = scale / max;
    max *= sfy;
    for (int i = 3; i < n; i++) vfft[i] *= sfy;
  }

  // smooth fft.
  if (smooth) {
    double max10 = 0.05 * max;
    for (int i = 3; i < n - 1; i++)
      if (vfft[i] < max10) vfft[i] = 0;
    for (int c = 0; c < smoothIters; c++) {
      for (int i = 4; i < n - 1; i++) {
        if (vfft[i - 1] > vfft[i] && vfft[i] < vfft[i + 1])
          vfft[i] = (vfft[i - 1] + vfft[i + 1]) / 2;
      }
    }

    if (nform) {
      // get formants
      Formants(vfft, HzFrm, PowFrm, phFrm, n, nform);
      // convert HzFrm to Hz
      for (int i = 0; i < nform; i++) HzFrm[i] *= (double)samples / n / 2.;
    }
  }
}

//---------------------------------------------------------------------------
// ClosestPhiRel. find closest relation to PHI in a series of numbers.
// 0....... phi ... 1
//---------------------------------------------------------------------------
double ClosestPhiRel(double *Frm, int n) {
  double min = 1, rel;
  for (int i = 0; i < n; i++) {
    for (int j = i + 1; j < n; j++) {
      if (Frm[i] > Frm[j])
        if (Frm[i] != 0)
          rel = fabs(Frm[j] / Frm[i] - PHI);
        else
          rel = min;
      else if (Frm[j] != 0)
        rel = fabs(Frm[i] / Frm[j] - PHI);
      else
        rel = min;
      if (rel < min) min = rel;
    }
  }
  // min distance to phi
  return min;
}

//------------------------------------------------------------------------------
//                          ^
// Find 'nform' freq peaks / \, returning converted (samples/nFFT) Hz
// fc = SAMPLES/nFFT/2.
//  HzFrm[0]=0
// Input comes from adding several fft sound samples
//------------------------------------------------------------------------------
int GetPeaks(double *vfft, int nFFT, double *HzFrm, double *PwFrm, int &nform,
             double &fMax, double &fMin, double fc) {
  int i, c, pmax, pmin;

  HzFrm[0] = PwFrm[0] = 0;  // initial value is cero for PHI process
  // find max value and a left cero value
  double max = -DBL_MAX;
  int pm = -1;
  for (i = 0; i < nFFT; i++)
    if (vfft[i] > max) {
      max = vfft[i];
      pm = i;
    }
  for (i = pm; vfft[i] != 0 && i != 0; i--)
    ;   // find a left cero value
  i++;  // here's the first significant value.
  fMin = DBL_MAX;
  fMax = -DBL_MAX;
  for (c = 1; i < nFFT; i++) {
    double va = vfft[i];
    if (va > fMax) {
      pmax = i;
      fMax = va;
    }
    if (va < fMin) {
      pmin = i;
      fMin = va;
    }                                                      // Max, Min.
    if (vfft[i - 1] < vfft[i] && vfft[i] > vfft[i + 1]) {  // peak?
      HzFrm[c] = (double)i * fc;                           // save, cnv to Hz
      PwFrm[c] = vfft[i];
      if (++c >= nform) c--;
    }
  }
  fMax = (double)pmax * fc;
  fMin = (double)pmin * fc;
  nform = c;
  return c;
}

//------------------------------------------------------------------------------
// GetPHIRels. get all possible Phi rels in a Hz vector
//------------------------------------------------------------------------------
int GetPHIRels(double *HzFrm, int nform, TPHIRels *PHIRels) {
#define phiREL(x1, x2, x3, x4) \
  ((HzFrm[x1] - HzFrm[x2]) /   \
   (((HzFrm[x3] - HzFrm[x4])) ? (HzFrm[x3] - HzFrm[x4]) : 100))
#define inPHI(x) ((x) >= (PHI - DeltaPhi) && (x) <= (PHI + DeltaPhi))
#define savePhi(i, j, k, r) \
  {                         \
    PHIRels[c].i = i;       \
    PHIRels[c].j = j;       \
    PHIRels[c].k = k;       \
    c++;                    \
  }  // i,j,k are in close to PHI relation r~=PHI.

  int c = 0;
  double r;
  for (int i = 0; i < nform; i++)
    for (int j = i + 1; j < nform; j++)
      for (int k = j + 1; k < nform; k++) {
        r = phiREL(j, i, k, i);
        if (inPHI(r)) savePhi(i, j, k, r);
        r = phiREL(j, i, k, j);
        if (inPHI(r)) savePhi(i, j, k, r);
        r = phiREL(k, i, j, i);
        if (inPHI(r)) savePhi(i, j, k, r);
        r = phiREL(k, i, k, j);
        if (inPHI(r)) savePhi(i, j, k, r);
        r = phiREL(k, j, j, i);
        if (inPHI(r)) savePhi(i, j, k, r);
        r = phiREL(k, j, k, i);
        if (inPHI(r)) savePhi(i, j, k, r);
      }
  return c;
}

int GetRels(double *HzFrm, int nform, double *rels, double from, double to,
            double factor) {
#define RelREL(x1, x2, x3, x4) \
  ((HzFrm[x1] - HzFrm[x2]) /   \
   (((HzFrm[x3] - HzFrm[x4])) ? (HzFrm[x3] - HzFrm[x4]) : 100))
#define inRel(x) ((x) >= (from) && (x) <= (to))
#define saveRel(i) \
  { rels[(int)(r * factor)]++; }

  int c = 0;
  double r;
  for (int i = 0; i < nform; i++)
    for (int j = i + 1; j < nform; j++)
      for (int k = j + 1; k < nform; k++) {
        r = RelREL(j, i, k, i);
        if (inRel(r)) saveRel(r);
        r = RelREL(j, i, k, j);
        if (inRel(r)) saveRel(r);
        r = RelREL(k, i, j, i);
        if (inRel(r)) saveRel(r);
        r = RelREL(k, i, k, j);
        if (inRel(r)) saveRel(r);
        r = RelREL(k, j, j, i);
        if (inRel(r)) saveRel(r);
        r = RelREL(k, j, k, i);
        if (inRel(r)) saveRel(r);
      }
  return c;
}
// is 'x' in r+-delta
/*
static inline bool inRel(double x,double REL,double delta)
{ return (x>=(REL-delta)) && (x<=(REL+delta)); }
// calc difference relation.
static inline double calcRel(double *HzFrm, int x1,int x2,int x3,int x4)
{ return
(HzFrm[x1]-HzFrm[x2])/(((HzFrm[x3]-HzFrm[x4]))?(HzFrm[x3]-HzFrm[x4]):100); }
*/
#undef inRel
#undef calcRel
#define inRel(r, Rel, delta) (r >= (Rel - delta)) && (r <= (Rel + delta))
#define calcRel(HzFrm, x1, x2, x3, x4) \
  (HzFrm[x1] - HzFrm[x2]) /            \
      (((HzFrm[x3] - HzFrm[x4])) ? (HzFrm[x3] - HzFrm[x4]) : 100)
//
int CountRels(double *HzFrm, int nform, double Rel, double delta) {
  int c = 0;
  double r;
  for (int i = 0; i < nform; i++)
    for (int j = i + 1; j < nform; j++)
      for (int k = j + 1; k < nform; k++) {
        r = calcRel(HzFrm, j, i, k, i);
        if (inRel(r, Rel, delta)) c++;
        r = calcRel(HzFrm, j, i, k, j);
        if (inRel(r, Rel, delta)) c++;
        r = calcRel(HzFrm, k, i, j, i);
        if (inRel(r, Rel, delta)) c++;
        r = calcRel(HzFrm, k, i, k, j);
        if (inRel(r, Rel, delta)) c++;
        r = calcRel(HzFrm, k, j, j, i);
        if (inRel(r, Rel, delta)) c++;
        r = calcRel(HzFrm, k, j, k, i);
        if (inRel(r, Rel, delta)) c++;
      }
  return c;
}
//------------------------------------------------------------------------------
// CountPhiRels. count all possible Phi rels in a Hz vector
//------------------------------------------------------------------------------
int CountPhiRels(double *HzFrm, int nform) {
  return CountRels(HzFrm, nform, PHI, DeltaRel);
}
int CountOctRels(double *HzFrm, int nform) {
  return CountRels(HzFrm, nform, 0.5, DeltaRel);
}
int CountFourthRels(double *HzFrm, int nform) {
  return CountRels(HzFrm, nform, 0.75, DeltaRel);
}
int CountFifthRels(double *HzFrm, int nform) {
  return CountRels(HzFrm, nform, 2 / 3., DeltaRel);
}

// absulute values & scale to
double AbsScale(double *vfft, int n) { return AbsScale(vfft, n, 100.); }
double AbsScale(double *vfft, int n, double scale) {
  double max = -1;
  for (int i = 0; i < n; i++) {
    double &v = vfft[i];
    if (v < 0) v = -v;
    if (v > max) max = v;
  }
  if (max == 0) return 1;
  for (int i = 0; i < n; i++) {
    vfft[i] = scale * vfft[i] / max;
  }
  return max;  //??
}

//------------------------------------------------------------------------------
// Vector Min, Max, find in v and return values in vHz
//------------------------------------------------------------------------------
double v_MinMax(double &fMin, double &fMax, double *vHz, double *v, int n) {
  int i, pmax, pmin;
  for (fMax = -DBL_MAX, fMin = DBL_MAX, i = 0; i < n; i++) {
    double &va = v[i];
    if (va > fMax) {
      fMax = va;
      pmax = i;
    }
    if (va < fMin) {
      fMin = va;
      pmin = i;
    }
  }
  fMax = vHz[pmax];
  fMin = vHz[pmin];
  return fabs(fMax - fMin);
}

//------------------------------------------------------------------------------
// Zero vector
//------------------------------------------------------------------------------
void v_Zero(double *v, int n) { memset(v, 0, n * sizeof(double)); }
//------------------------------------------------------------------------------
// index <--> freq conversions
//------------------------------------------------------------------------------
double Index2Freq(int i, double samples, int nFFT) {
  return (double)i * (samples / nFFT / 2.);
}

int Freq2Index(double freq, double samples, int nFFT) {
  return (int)freq / (samples / nFFT / 2.);
}

//------------------------------------------------------------------------------
// find range of freq. with sum percentage (%) of freq.
//------------------------------------------------------------------------------
int FreqRange(double *v, int n, double sump, int &_fMin, int &_fMax) {
  int i, j;
  // sum all freq.
  double s = 0;
  for (i = 0; i < n; i++) s += v[i];
  int d = 0, mind = n;
  for (i = 0; i < n - 1; i++) {
    // find sump% of s
    double sp = 0, ps = sump * s / 100.;
    for (j = i; j < n - 1; j++) {
      sp += v[j];
      if (sp >= ps) {
        d = j - i;
        break;
      }
    }
    if (d < mind) {
      mind = d;
      _fMin = i;
      _fMax = j;
    }
  }
  return mind;
}

//---------------------------------------------------------------------------
// MirrorVector. v[i]=max-v[i]
//---------------------------------------------------------------------------
double MirrorVector(double *v, double *vo, int n) {
  double max = -DBL_MAX;
  for (int i = 0; i < n; i++)
    if (v[i] > max) max = v[i];
  for (int i = 0; i < n; i++) vo[i] = max - v[i];
  return max;
}

//---------------------------------------------------------------------------
// SortTwo. based on y keeping order in x
//---------------------------------------------------------------------------
static int _cmp2dbl(const void *p1, const void *p2) {
  typedef struct { double y, x; } T2double;
  return (int)(((T2double *)p1)->y - ((T2double *)p2)->y);
}
static int _cmp2dblAsc(const void *p1, const void *p2) {
  typedef struct { double y, x; } T2double;
  return (int)(((T2double *)p2)->y - ((T2double *)p1)->y);
}
void SortTwo(double *y, double *x, int n) {
  typedef struct { double y, x; } T2double;
  T2double *yx;
  yx = new T2double[n];
  for (int i = 0; i < n; i++) {
    yx[i].y = y[i];
    yx[i].x = x[i];
  }
  qsort(yx, n, sizeof(T2double), _cmp2dbl);
  for (int i = 0; i < n; i++) {
    y[i] = yx[i].y;
    x[i] = yx[i].x;
  }

  delete[] yx;
}
void SortTwoAsc(double *y, double *x, int n) {
  typedef struct { double y, x; } T2double;
  T2double *yx;
  yx = new T2double[n];
  for (int i = 0; i < n; i++) {
    yx[i].y = y[i];
    yx[i].x = x[i];
  }
  qsort(yx, n, sizeof(T2double), _cmp2dblAsc);
  for (int i = 0; i < n; i++) {
    y[i] = yx[i].y;
    x[i] = yx[i].x;
  }

  delete[] yx;
}

//
// tmp code
//
int CountPhiRels_ant(double *HzFrm, int nform) {
#undef savePhi
#define savePhi(i, j, k, r) c++  // i,j,k are in close to PHI relation r~=PHI.

  int c = 0;
  double r;
  for (int i = 0; i < nform; i++)
    for (int j = i + 1; j < nform; j++)
      for (int k = j + 1; k < nform; k++) {
        r = phiREL(j, i, k, i);
        if (inPHI(r)) savePhi(i, j, k, r);
        r = phiREL(j, i, k, j);
        if (inPHI(r)) savePhi(i, j, k, r);
        r = phiREL(k, i, j, i);
        if (inPHI(r)) savePhi(i, j, k, r);
        r = phiREL(k, i, k, j);
        if (inPHI(r)) savePhi(i, j, k, r);
        r = phiREL(k, j, j, i);
        if (inPHI(r)) savePhi(i, j, k, r);
        r = phiREL(k, j, k, i);
        if (inPHI(r)) savePhi(i, j, k, r);
      }
  return c;
}

//
// calc jitter from filtered spectrum
//
double Jitter(double *vfft, int n) {
  // find max and pos max (px)
  double mx = -999;
  int px = 0;
  for (int i = 0; i < n; i++)
    if (vfft[i] > mx) {
      mx = vfft[i];
      px = i;
    }
  // from 'px' find left and right margin to '0'
  int lm;
  for (lm = px; lm && vfft[lm] != 0; lm--)
    ;
  int rm;
  for (rm = px; rm < n && vfft[rm] != 0; rm++)
    ;
  int dst = rm - lm;
  // jitter is the rel between px and dst
  if (px)
    mx = (double)dst / (double)px;
  else
    mx = 0;
  return mx;
}

//
// Find min note (weak note) in HzFrm as min(frm 0..max..0)
//
double weakNote(double *musGrid, int n) {
  // find max and pos max (px)
  double mx = -1e55;
  int px = 0;
  for (int i = 0; i < n; i++)
    if (musGrid[i] > mx) {
      mx = musGrid[i];
      px = i;
    }
  // from 'px' find left and right margin to '0'
  int lm;
  for (lm = px; lm && musGrid[lm] != 0; lm--)
    ;
  if (musGrid[lm] == 0) lm++;
  int rm;
  for (rm = px; rm < n && musGrid[rm] != 0; rm++)
    ;
  if (musGrid[rm] == 0) rm--;
  // min of lm..rm interval
  double mn = 1e55;
  if (lm == rm) mn = 0;
  for (int i = lm; i < rm; i++)
    if (musGrid[i] < mn) mn = musGrid[i];
  return mn;
}
double excessNote(double *musGrid, int n) {
  // find max and pos max (px)
  double mx = -1e55;
  for (int i = 0; i < n; i++)
    if (musGrid[i] > mx) {
      mx = musGrid[i];
    }
  return mx;
}

//---------------------------------------------------------------------------
// Comparision Sorting support
//---------------------------------------------------------------------------
int _cmp_int(const void *p1, const void *p2) { return *(int *)p1 - *(int *)p2; }
int _cmp_dbl(const void *p1, const void *p2) {
  if (*(double *)p1 > *(double *)p2)
    return +1;
  else if (*(double *)p1 < *(double *)p2)
    return -1;
  else
    return 0;
}
int _cmp_flt(const void *p1, const void *p2) {
  if (*(float *)p1 > *(float *)p2)
    return +1;
  else if (*(float *)p1 < *(float *)p2)
    return -1;
  else
    return 0;
}

void MinMax(double *v, int n, double &Min, double &Max) {
  if (n == 0) {
    Min = Max = 0;
    return;
  }
  double mi = *v, mx = *v;
  for (int i = 0; i < n; i++) {
    if (v[i] > mx) mx = v[i];
    if (v[i] < mi) mi = v[i];
  }
}

void Sort(double *v, int n) { qsort(v, n, sizeof(double), _cmp_dbl); }

// decibel calc
double db(double range, double val) {
  if (val == 0) return -80;
  return 20. * log10(val / range);
}

// for 16bit sound
double db(double val) {
  if (val == 0) return -80;
  return 20. * log10(val / 32767.);
}

double db100(double val) {
  if (val == 0) return -80;
  return 20. * log10(val / 100.);
}

}  // namespace fft
