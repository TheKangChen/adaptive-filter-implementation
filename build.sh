gcc -o lms.out main.c filter.c \
    -Wall \
	-I/opt/homebrew/include \
	-L/opt/homebrew/lib -lsndfile -lportaudio -lncurses