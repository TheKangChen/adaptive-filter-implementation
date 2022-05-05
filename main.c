#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sndfile.h>
#include "main.h"
#include "filter.h"

#define debug 0

int main(int argc, char *argv[]) {
    char *xfile, *dfile, *ofile;
    SNDFILE *xsndfile, *dsndfile, *osndfile = NULL;
	SF_INFO xsfinfo, dsfinfo, osfinfo;
    struct Buf buf, *p = &buf;
    struct LMS lms_cal, *lp = &lms_cal;

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

		// adaptive filter
        fir(p->x, lp->y, lp->w, xlen, n);
        lms(p->x, p->d, lp->y, lp->w, lp->e, lp->miu, n);

        if (debug) printf("error: %f\n", lp->e[n]);
    }

    /* write to file */
    osfinfo.samplerate = 48000;
    osfinfo.channels = 1;
    osfinfo.format = xsfinfo.format;
    
    if (debug) {
        printf("%d\n", xsfinfo.format);
        printf("%d\n", osfinfo.format);
    }

    // write to wav file
    if ((osndfile = sf_open(ofile, SFM_WRITE, &osfinfo)) == NULL) {
        fprintf(stderr, "ERROR: failed to open file %s\n", ofile);
        return -1;
    }

    int count = sf_write_double(osndfile, lp->e, xsfinfo.frames * xsfinfo.channels);

    if (count != xsfinfo.frames) {
		fprintf(stderr, "ERROR: num of frames in osfinfo: %d does not match isfinfo: %lld\n", count, xsfinfo.frames);
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
