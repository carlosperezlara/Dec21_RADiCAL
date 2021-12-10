all: daq calib

clean: 
	rm -f daq calib

daq: source/daq.cc
	g++ -fPIC -g -O2 -o daq source/daq.cc -L/usr/lib/ -L. -lCAENComm -lCAENDigitizer -lm -lpthread

calib: source/calib.cc
	g++ -fPIC -g -O2 -o calib source/calib.cc -L/usr/lib/ -L. -lCAENComm -lCAENDigitizer -lm -lpthread
