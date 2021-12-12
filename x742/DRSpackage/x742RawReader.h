#ifndef __X742RAWREADER__
#define __X742RAWREADER__

#include <fstream>
#include <TString.h>
#include "DRSGroupData.h"

/*=====================
Raw reader for x742 series based on the DRS4 chipset
keep in mind that DRS4 stores in one file as
many channels as there were saved. So the unpacking
is a bit tricky. I addapted the reader to accomodate
for buffering several channels at the same time.
=====================*/

class x742RawReader {
public:
  x742RawReader(TString filename);
  ~x742RawReader();
  void  ReadHeader();
  bool  ReadEvent();
  void  SetGroupData(int igrp, DRSGroupData *val) {fGroup[igrp] = val;}
  UInt_t GetEventNumber() {return fEventCounter;}
  UInt_t GetTimeTag() {return fTimeTag;}
  
private:
  bool ReadGroup(int group);
  std::ifstream fIFS;
  UInt_t fGroupMask;
  UInt_t fEventCounter;
  UInt_t fTimeTag;
  DRSGroupData *fGroup[4];
  
  ClassDef(x742RawReader,0);
};

#endif
