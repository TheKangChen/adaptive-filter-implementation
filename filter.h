#define FILT_ORDER 128
#define MAX_CHN 1

void fir(double *x, double *y, double *w, int xlen, int idx);
void lms(double *x, double *d, double *y, double *w, double *e, float miu, int idx);
void normalize(double *x, double *y, int numSamples);