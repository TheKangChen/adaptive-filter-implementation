gcc -o lms main.c \
    -Wall \
	-I/opt/homebrew/include \
	-L/opt/homebrew/lib -lsndfile -lportaudio -lncurses