#!/bin/bash

# parse command line
./lms.out
./lms.out ./Samples/Sine440+Noise.wav ./Samples/Sine_440.wav filteredSine_N.wav 0.01 1234
./lms.out ./Samples/Sine440+Noise.wav ./Samples/Sine_440.wav filteredSine_N.wav abc

# Gaussian Noise
./lms.out ./Samples/Sine440+Noise.wav ./Samples/Sine_440.wav filteredSine_N.wav 0.01
./lms.out ./Samples/Woman+Noise.wav ./Samples/WomanPoem.wav filteredWoman_N.wav 0.01
./lms.out ./Samples/Man+Noise.wav ./Samples/ManPoem.wav filteredMan_N.wav 0.01
./lms.out ./Samples/Music1+Noise.wav ./Samples/Music1.wav filteredMusic1_N.wav 0.01
./lms.out ./Samples/Music2+Noise.wav ./Samples/Music2.wav filteredMusic2_N.wav 0.01

# Cityscape
./lms.out ./Samples/Sine440+Cityscape.wav ./Samples/Sine_440.wav filteredSine_C.wav 0.0001
./lms.out ./Samples/Woman+Cityscape.wav ./Samples/WomanPoem.wav filteredWoman_C.wav 0.0001
./lms.out ./Samples/Man+Cityscape.wav ./Samples/ManPoem.wav filteredMan_C.wav 0.0001
./lms.out ./Samples/Music1+Cityscape.wav ./Samples/Music1.wav filteredMusic1_C.wav 0.0001
./lms.out ./Samples/Music2+Cityscape.wav ./Samples/Music2.wav filteredMusic2_C.wav 0.0001

# Subway
./lms.out ./Samples/Sine440+Subway.wav ./Samples/Sine_440.wav filteredSine_S.wav 0.0001
./lms.out ./Samples/Woman+Subway.wav ./Samples/WomanPoem.wav filteredWoman_S.wav 0.0001
./lms.out ./Samples/Man+Subway.wav ./Samples/ManPoem.wav filteredMan_S.wav 0.0001
./lms.out ./Samples/Music1+Subway.wav ./Samples/Music1.wav filteredMusic1_S.wav 0.0001
./lms.out ./Samples/Music2+Subway.wav ./Samples/Music2.wav filteredMusic2_S.wav 0.0001

# Music
./lms.out ./Samples/Woman+Music2.wav ./Samples/WomanPoem.wav filteredWoman_M.wav 0.01
./lms.out ./Samples/Man+Music2.wav ./Samples/ManPoem.wav filteredMan_M.wav 0.01