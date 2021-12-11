#include <fstream>
#include <iostream>
#include <TString.h>
#include "x742RawReader.h"

//=======
x742RawReader::x742RawReader(TString filename) {
  std::cout << "x742RawReader :: " << filename.Data() << std::endl;
  fIFS.close();
  fIFS.open( filename.Data() );
  fGroupMask = 0b1111;
  for(int i=0; i!=4; ++i)
    fGroup[i] = 0;
}
//=======
x742RawReader::~x742RawReader() {
  fIFS.close();
}
//=======
void x742RawReader::ReadHeader() {
  fIFS.seekg(0);
}
//=======
bool x742RawReader::ReadEvent() {
  UInt_t  uint;
  //RUN HEADER
  fIFS.read((char*) &uint, 4);
  if(!fIFS.good()) return false;
  //std::cout << "First word is " << uint << std::endl;
  UInt_t INIT = uint>>28;
  UInt_t TOTAL_EVENT_SIZE = (uint << 4)>>4;
  //std::cout << " INIT: " << INIT << (INIT==0b1010?" GOOD":" OOPPS") << std::endl;
  //std::cout << " TOTAL EVENT SIZE: " << TOTAL_EVENT_SIZE << std::endl;

  fIFS.read((char*) &uint, 4);
  //std::cout << "Second word is " << uint << std::endl;
  UInt_t BOARDID = uint>>27;
  UInt_t BF = (uint>>26) & 0b1;
  UInt_t RES1 = (uint>>24) & 0b11;
  UInt_t PATTERN = ((uint<<8)>>16);
  UInt_t RES2 = ((uint<<24)>>28);
  UInt_t GROUPMASK = uint & 0b11;
  //std::cout << " BOARD ID: " << BOARDID << std::endl;
  //std::cout << " BF: " << BF << (BF!=0?"    W A R N I N G ! ! !    BOARD FAILED FLAG FIRED":"") << std::endl;
  //std::cout << " RESERVED: " << RES1 << std::endl;
  //std::cout << " PATTERN: " << PATTERN << std::endl;
  //std::cout << " RESERVED: " << RES2 << std::endl;
  //std::cout << " GROUP MASK: " << GROUPMASK << std::endl;
  fGroupMask = GROUPMASK;
  
  fIFS.read((char*) &uint, 4);
  //std::cout << "Third word is " << uint << std::endl;
  UInt_t RES3 = uint>>24;
  UInt_t EVENTCOUNTER = (uint<<8)>>8;
  //std::cout << " RESERVED: " << RES3 << std::endl;
  //std::cout << " EVENT COUNTER: " << EVENTCOUNTER << std::endl;

  fIFS.read((char*) &uint, 4);
  //std::cout << "Fourth word is " << uint << std::endl; // EVENT TIME

  bool allGood = true;
  if(fGroupMask & 0b0001)  allGood = allGood && ReadGroup(0);
  if(fGroupMask & 0b0010)  allGood = allGood && ReadGroup(1);
  return allGood;
}
//=======
bool x742RawReader::ReadGroup(int iGroup) {
  UInt_t  uint;
  fIFS.read((char*) &uint, 4);
  UInt_t CONTROL1 = uint>>30;
  UInt_t STARTINDEXCELL = (uint<<2)>>22;
  UInt_t CONTROL2 = (uint>>18) & 0b11;
  UInt_t CONTROL3 = (uint>>13) & 0b111;
  UInt_t FREQ = (uint>>16) & 0b11;
  UInt_t TR = (uint>>12) & 0b1;
  UInt_t SIZE = (uint<<20)>>20;
  //std::cout << std::endl;
  //std::cout << " READING GROUP " << iGroup << std::endl;
  //std::cout << " START INDEX CELL: " << STARTINDEXCELL << std::endl;
  //std::cout << " FREQ: " << FREQ << " { 5 GS/s, 2.5 GS/s, 1 GS/s, 750 MS/s } " << std::endl;
  //std::cout << " TR: " << TR << (TR==1?" (PRESENT)":"(NOT PRESENT)") << std::endl;
  //std::cout << " SIZE: " << SIZE << std::endl;
  //std::cout << " CONTROL1: " << CONTROL1 << std::endl;
  //std::cout << " CONTROL2: " << CONTROL2 << std::endl;
  //std::cout << " CONTROL3: " << CONTROL3 << std::endl;
  if((CONTROL1+CONTROL2+CONTROL3)!=0) {
    return false;
  }
  UInt_t buffer[3]; // 4bytes*3 = 12bytes = 96bits = 12bits * 8 channels
  for(int isa=0; isa!=1024; ++isa) {
    fIFS.read((char*) &buffer[2], 4);
    fIFS.read((char*) &buffer[1], 4);
    fIFS.read((char*) &buffer[0], 4);
    UInt_t adc[8];
    adc[0] = buffer[2] & 0xfff;       //first 12 bits
    adc[1] = (buffer[2]>>12) & 0xfff; // next 12 bits
    adc[2] = (buffer[2]>>24) & 0xff;  // last  8 bits
    adc[2] |= ((buffer[1] & 0xf)<<8); //first  4 bits
    adc[3] = (buffer[1]>>4) & 0xfff;  // next 12 bits
    adc[4] = (buffer[1]>>16) & 0xfff; // next 12 bits
    adc[5] = (buffer[1]>>28) & 0xf;   // last  4 bits
    adc[5] |= ((buffer[0] & 0xff)<<4);//first  8 bits
    adc[6] = (buffer[0]>>8) & 0xfff;  // next 12 bits
    adc[7] = (buffer[0]>>20) & 0xfff; // last 12 bits
    //cout << "ISA " << isa << endl;
    if(fGroup[iGroup]) { // STORE DATA
      for(int ich=0; ich!=8; ++ich) {
	fGroup[iGroup] -> SetChannelDatum( ich, isa, adc[ich]  );
      }
    }
  }
  if(TR) {
    for(int ichunk=0; ichunk!=128; ++ichunk) { //128*8 = 1024
      fIFS.read((char*) &buffer[2], 4);
      fIFS.read((char*) &buffer[1], 4);
      fIFS.read((char*) &buffer[0], 4);
      UInt_t adc[8];
      adc[0] = buffer[2] & 0xfff;       //first 12 bits
      adc[1] = (buffer[2]>>12) & 0xfff; // next 12 bits
      adc[2] = (buffer[2]>>24) & 0xff;  // last  8 bits
      adc[2] |= ((buffer[1] & 0xf)<<8); //first  4 bits
      adc[3] = (buffer[1]>>4) & 0xfff;  // next 12 bits
      adc[4] = (buffer[1]>>16) & 0xfff; // next 12 bits
      adc[5] = (buffer[1]>>28) & 0xf;   // last  4 bits
      adc[5] |=(( buffer[0] & 0xff)<<4);//first  8 bits
      adc[6] = (buffer[0]>>8) & 0xfff;  // next 12 bits
      adc[7] = (buffer[0]>>20) & 0xfff; // last 12 bits
      //cout << "CHUNK " << ichunk << endl;
      for(int isa=0; isa!=8; ++isa) {
	//cout << " " << isa << ": " << adc << endl;
	int real_isa=ichunk*8+isa;
	if(fGroup[iGroup]) { // STORE DATA
	  fGroup[iGroup] -> SetTRDatum( real_isa, adc[isa]  );
	}
      }
    }
    if(fGroup[iGroup]) { // STORE DATA
      fGroup[iGroup]->SetTriggerCell( STARTINDEXCELL );
      fGroup[iGroup]->SetFrequency( FREQ );
    }
  }
  
  fIFS.read((char*) &uint, 4);
  //std::cout << " GROUP TRIGGER TIME TAG: " << uint << std::endl;

  return true;
}
