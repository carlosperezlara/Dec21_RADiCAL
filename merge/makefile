all: merge

clean: 
	rm -f merge
	rm Dict*

merge: source/merge.cc
	rootcint -f Dict.cpp -c `root-config --cflags` -p DRSpackage/DRSGroupData.h  DRSpackage/x742RawReader.h DRSpackage/LinkDef.h
	g++ -fPIC -g -O2 -o merge source/merge.cc DRSpackage/DRSGroupData.cxx  DRSpackage/x742RawReader.cxx Dict.cpp -L/usr/lib/ -IDRSpackage -lCAENComm -lCAENDigitizer -lm -lpthread `root-config --cflags --libs`

