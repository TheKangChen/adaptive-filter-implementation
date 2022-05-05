#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <sndfile.h>


#define FILT_ORDER 128
#define MAX_CHN 1
#define debug 1

typedef struct {
	unsigned int channels;
	unsigned int samplerate;
	double *x; // input signal
    double *d; // desired signal
    double *y; // filtered signal
    double *e; // error = d - y
    double *w; // filter coefficients
} Buf;


int main(int argc, char *argv[]) {
    char *xfile, *dfile, *ofile;
    float miu = 0.01f;
    SNDFILE *xsndfile, *dsndfile, *osndfile;
	SF_INFO xsfinfo, dsfinfo, osfinfo;
    Buf buf, *p = &buf;
    // FILE *xfp, *rfp, *ofp;

    memset(&xsfinfo, 0, sizeof(xsfinfo));
    memset(&dsfinfo, 0, sizeof(dsfinfo));
    // memset(&osfinfo, 0, sizeof(osfinfo));

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
    p->y = (double *)calloc(xsfinfo.frames * xsfinfo.channels, xsfinfo.frames * xsfinfo.channels * sizeof(double));
    p->e = (double *)malloc(xsfinfo.frames * xsfinfo.channels * sizeof(double));
    p->w = (double *)calloc(FILT_ORDER, FILT_ORDER * sizeof(double));

    if (p->x == NULL) return -1;
    if (p->d == NULL) return -1;
    if (p->y == NULL) return -1;
    if (p->e == NULL) return -1;
    if (p->w == NULL) return -1;

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
    for (int n=0; n<xlen; ++n) {
        p->y[n] = 0.0;
		// FIR filter
		for (int k=0; k<FILT_ORDER; ++k) {
			if ((n-k) >=0 && (n-k) <= xlen-1) {
				p->y[n] += p->x[n-k] * p->w[k];
			} else {
				p->y[n] += 0.0 * p->w[k];
            }
		}
    
        // calculate error
        p->e[n] = p->d[n] - p->y[n];

        // update filter coefficients
        for (int k=0; k<FILT_ORDER-1; ++k) {
            p->w[k+1] = p->w[k] + (miu * p->e[n] * p->x[k]);
        }
        if (debug) printf("y: %f\n", p->y[n]);
    }

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

    int count = sf_write_double(osndfile, p->y, xsfinfo.frames * xsfinfo.channels);
    if (count != xsfinfo.frames) {
		fprintf(stderr, "ERROR: wrong num frames for %s: %d, xsfinfo: %lld\n", argv[3], count, xsfinfo.frames);
		return -1;
	}

    sf_close(xsndfile);
    sf_close(dsndfile);
    sf_close(osndfile);
    free(p->x);
	free(p->d);
	free(p->y);
	free(p->e);
	free(p->w);

    return 0;
}

/* What do we need?
1. FIR filter Order 128
y = sum(ax + ax-1 + ax-2..... ax-127)
nested for loop O(M * N)

2. weights (filter coefficients) initialize to random

3. compute error signal (output signal)
error = desired signal - y

4. Update filter coefficients
h[k+1] = h[k] + miu * error[k] * x[k]

output y sig

repeat step 3 and 4
*/