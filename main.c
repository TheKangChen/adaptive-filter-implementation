#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <sndfile.h>


#define FILT_ORDER 128
#define MAX_CHN 1
#define debug 0

typedef struct {
	unsigned int channels;
	unsigned int samplerate;
	double *x; // input signal
    double *d; // desired signal
} Buf;

typedef struct {
    float miu; // step size
    double *y; // filtered signal
    double *e; // error = d - y
    double *w; // filter coefficients
} LMS;

void normalize(double *x, double *y, int numSamples);

int main(int argc, char *argv[]) {
    char *xfile, *dfile, *ofile;
    SNDFILE *xsndfile, *dsndfile, *osndfile;
	SF_INFO xsfinfo, dsfinfo, osfinfo;
    Buf buf, *p = &buf;
    LMS lms, *lp = &lms;

    memset(&xsfinfo, 0, sizeof(xsfinfo));
    memset(&dsfinfo, 0, sizeof(dsfinfo));

    // parse commond line args
    if (argc != 4) {
        fprintf(stderr, "Usage: %s masked_signal.wav desired_signal.wav output_signal.wav", argv[0]);
        return -1;
    }
    
    xfile = argv[1];
    dfile = argv[2];
    ofile = argv[3];

    // open files
    if ((xsndfile = sf_open(xfile, SFM_READ, &xsfinfo)) == NULL) {
        fprintf(stderr, "ERROR: failed to open file %s\n", xfile);
        return -1;
    }
    if ((dsndfile = sf_open(dfile, SFM_READ, &dsfinfo)) == NULL) {
        fprintf(stderr, "ERROR: failed to open file %s\n", dfile);
        return -1;
    }

    printf("Frames: %lld, Channels: %d, Samplerate: %d, %s\n", xsfinfo.frames, xsfinfo.channels, xsfinfo.samplerate, xfile);
    printf("Frames: %lld, Channels: %d, Samplerate: %d, %s\n", dsfinfo.frames, dsfinfo.channels, dsfinfo.samplerate, dfile);

    p->channels = xsfinfo.channels;
    p->samplerate = xsfinfo.samplerate;

    // check channel & sample rate of both files match
    if (xsfinfo.channels > MAX_CHN) {
        fprintf(stderr, "ERROR: %s channel more than max allowed channel 1\n", xfile);
        return -1;
    }
    if (dsfinfo.channels != p->channels) {
        fprintf(stderr, "ERROR: %s channel number %d does not match file %s: %d\n", dfile, dsfinfo.channels, xfile, p->channels);
        return -1;
    }
    if (dsfinfo.samplerate != p->samplerate) {
        fprintf(stderr, "ERROR: %s sample rate %d does not match file %s: %d\n", dfile, dsfinfo.samplerate, xfile, p->samplerate);
        return -1;
    }

    // allocate memory for all datas
    p->x = (double *)malloc(xsfinfo.frames * xsfinfo.channels * sizeof(double));
    p->d = (double *)malloc(dsfinfo.frames * dsfinfo.channels * sizeof(double));

    lp->y = (double *)calloc(xsfinfo.frames * xsfinfo.channels, xsfinfo.frames * xsfinfo.channels * sizeof(double));
    lp->w = (double *)calloc(FILT_ORDER, FILT_ORDER * sizeof(double));
    lp->e = (double *)malloc(xsfinfo.frames * xsfinfo.channels * sizeof(double));

    if (p->x == NULL) return -1;
    if (p->d == NULL) return -1;
    if (lp->y == NULL) return -1;
    if (lp->w == NULL) return -1;
    if (lp->e == NULL) return -1;

    // read input signal
    int xcount = sf_readf_double(xsndfile, p->x, xsfinfo.frames);
    if (xcount != xsfinfo.frames) {
        fprintf(stderr, "ERROR: num frames for %s: %d, sfinfo: %lld\n", xfile, xcount, xsfinfo.frames);
        return -1;
    }
    // read desired signal
    int dcount = sf_readf_double(dsndfile, p->d, dsfinfo.frames);
    if (dcount != dsfinfo.frames) {
        fprintf(stderr, "ERROR: num frames for %s: %d, sfinfo: %lld\n", dfile, dcount, dsfinfo.frames);
        return -1;
    }

    // get total num of samples in signal
    int xlen = xsfinfo.frames * xsfinfo.channels;
    int dlen = dsfinfo.frames * dsfinfo.channels;
    if (xlen != dlen) {
        fprintf(stderr, "ERROR: different number of samples in %s & %s", xfile, dfile);
        return -1;
    }

    /* filter signal */
    lp->miu = 0.01f; // recommend start with 0.01f
    for (int n=0; n<xlen; ++n) {
        lp->y[n] = 0.0;
		// FIR filter
		for (int k=0; k<FILT_ORDER; ++k) {
			if ((n-k) >=0 && (n-k) <= xlen-1) {
				lp->y[n] += p->x[n-k] * lp->w[k];
			} else {
				lp->y[n] += 0.0 * lp->w[k];
            }
		}
    
        // calculate error
        lp->e[n] = p->d[n] - lp->y[n];

        // update filter coefficients
        for (int k=0; k<FILT_ORDER-1; ++k) {
            lp->w[k+1] = lp->w[k] + (lp->miu * lp->e[n] * p->x[k]);
        }
        if (debug) printf("error: %f\n", lp->e[n]);
    }

    /* normalize output */
    // normalize(p->x, p->y, xsfinfo.frames * xsfinfo.channels);

    /* write to file */
    osfinfo.samplerate = 48000;
    osfinfo.channels = 1;
    osfinfo.format = xsfinfo.format;
    
    if (debug) printf("%d\n", xsfinfo.format);
    if (debug) printf("%d\n", osfinfo.format);

    if ((osndfile = sf_open(ofile, SFM_WRITE, &osfinfo)) == NULL) {
        fprintf(stderr, "ERROR: failed to open file %s\n", ofile);
        return -1;
    }

    int count = sf_write_double(osndfile, lp->e, xsfinfo.frames * xsfinfo.channels);
    if (count != xsfinfo.frames) {
		fprintf(stderr, "ERROR: wrong num frames for %s: %d, xsfinfo: %lld\n", argv[3], count, xsfinfo.frames);
		return -1;
	}

    sf_close(xsndfile);
    sf_close(dsndfile);
    sf_close(osndfile);

    free(p->x);
	free(p->d);
	free(lp->y);
	free(lp->w);
	free(lp->e);

    return 0;
}

void normalize(double *x, double *y, int numSamples) {
    // get max value of array x & y
    double max_x = 0.0;
    double max_y = 0.0;
    for (int i=0; i<numSamples; ++i) {
        max_x = fabs(x[i]) > max_x ? fabs(x[i]) : max_x;
        max_y = fabs(y[i]) > max_y ? fabs(y[i]) : max_y;
    }
    printf("x max value: %f\n", max_x);
    printf("y max value: %f\n", max_y);

    // normalize
    for (int i=0; i<numSamples; ++i) {
        y[i] = y[i] * ((double)max_x / (double)max_y);
    }
}