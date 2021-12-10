#ifndef __DRSGROUPDATA__
#define __DRSGROUPDATA__

class DRSGroupData {
 public:
  DRSGroupData();
  ~DRSGroupData();
  //--------------
  void CalibrateTime(int freq=0);
  //--------------
  void SetTriggerCell(UInt_t val) {fTC = val;}
  void SetFrequency(UInt_t val) {fFR = val;}
  //--------------
  void SetChannelDatum(int ich, int isa, UInt_t val) {fChannel[ich][isa] = val;}
  void SetTRDatum(int isa, UInt_t val) {fTR[isa] = val;}
  //--------------
  void GetChannelData(int ich, UInt_t *y);
  //--------------
  void GetX(Double_t *x);
  void GetY(int ich, Double_t *y);
  //--------------
  void GetTRY(Double_t *y);
  //--------------
  UInt_t GetTC() {return fTC;}
  //--------------
  void SetCellOffsetConstant(int ich, int icell, Double_t val) {fCellOffset[ich][icell] = val;}
  //--------------
  void LoadCalibrations(TString file1, TString file2, TString file3);
  
 private:
  // basic data stream
  UInt_t fFR; // 0: 5 GS/s   1: 2.5 GS/s   2: 1 GS/s   3: 750 MS/s
  UInt_t fTC;
  UInt_t fChannel[8][1024];
  UInt_t fTR[1024];

  Double_t fDeltaTime[1024]; // *constants* bin width common to all gthe group
  
  // calibration constants
  Bool_t fApplyOfflineCalibration;
  Double_t fCellOffset[9][1024];
  
  ClassDef(DRSGroupData, 0);
};

#endif
