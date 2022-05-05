#include <math.h>
#include "filter.h"

void fir(double *x, double *y, double *w, int xlen, int idx) {
    for (int k=0; k<FILT_ORDER; ++k) {
        if ((idx-k) >=0 && (idx-k) <= xlen-1) {
            y[idx] += x[idx-k] * w[k];
        } else {
            y[idx] += 0.0 * w[k];
        }
    }
}

void lms(double *x, double *d, double *y, double *w, double *e, float miu, int idx) {
    // calculate error
    e[idx] = d[idx] - y[idx];

    // update filter coefficients
    for (int k=0; k<FILT_ORDER-1; ++k) {
        w[k+1] = w[k] + (miu * e[idx] * x[k]);
    }
}

void normalize(double *x, double *y, int numSamples) {
    // get max value of array x & y
    double max_x = 0.0;
    double max_y = 0.0;
    for (int i=0; i<numSamples; ++i) {
        max_x = fabs(x[i]) > max_x ? fabs(x[i]) : max_x;
        max_y = fabs(y[i]) > max_y ? fabs(y[i]) : max_y;
    }

    // normalize
    for (int i=0; i<numSamples; ++i) {
        y[i] = y[i] * ((double)max_x / (double)max_y);
    }
}