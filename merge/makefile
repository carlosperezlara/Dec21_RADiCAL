all: merge

clean: 
	rm -f merge
	rm Dict*

merge: source/merge.cc
	rootcint -f Dict.cpp -c `root-config --cflags` -p ../x742/DRSpackage/DRSGroupData.h  ../x742/DRSpackage/x742RawReader.h ../x742/DRSpackage/LinkDef.h
	g++ -fPIC -g -O2 -o merge source/merge.cc -I../x742/DRSpackage ../x742/DRSpackage/DRSGroupData.cxx  ../x742/DRSpackage/x742RawReader.cxx Dict.cpp -L/usr/lib/ -IDRSpackage -lCAENComm -lCAENDigitizer -lm -lpthread `root-config --cflags --libs`

