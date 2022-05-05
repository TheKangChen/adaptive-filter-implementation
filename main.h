struct Buf {
	unsigned int channels;
	unsigned int samplerate;
	double *x; // input signal
    double *d; // desired signal
};

struct LMS {
    float miu; // step size
    double *y; // filtered signal
    double *e; // error = d - y
    double *w; // filter coefficients
};