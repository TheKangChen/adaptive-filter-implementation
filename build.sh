gcc -o lms.out main.c \
    -Wall \
	-I/opt/homebrew/include \
	-L/opt/homebrew/lib -lsndfile -lportaudio -lncurses