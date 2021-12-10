#include "DRSGroupData.h"

//====================================
DRSGroupData::DRSGroupData() {
  fFR = 0;
  fTC = 0;
  for(int isa=0; isa!=1024; ++isa) {
    for(int ich=0; ich!=8; ++ich) {
      fChannel[ich][isa] = 0;
    }
    fTR[isa] = 0;
  }
  for(int icell=0; icell!=1024; ++icell) {
    for(int itch=0; itch!=9; ++itch) {
      fDeltaTime[itch][icell] = 0;
    }
  }
  fApplyOfflineCalibration = kTRUE;
  //fApplyOfflineCalibration = kFALSE;
  for(int icell=0; icell!=1024; ++icell) {
    for(int ich=0; ich!=8; ++ich) {
      fCellOffset[ich][icell] = 0;
      fCellTimeOffset[ich][icell] = 0;
    }
  }
  CalibrateTime(0);
}
//====================================
DRSGroupData::~DRSGroupData() {
}
//====================================
void DRSGroupData::CalibrateTime(int freq) {
  Double_t dt = 0.2;       // 0.2ns    <== 5.0 GS/s
  if(freq==1) dt = 0.4;    // 0.4ns    <== 2.5 GS/s
  if(freq==2) dt = 1.0;    // 1.0ns    <== 1.0 GS/s
  if(freq==3) dt = 1.3333; // 1.3333ns <== 750 MS/s
  // baseline
  for(int itch=0; itch!=9; ++itch) {
    for(int icell=0; icell!=1024; ++icell) {
      fDeltaTime[itch][icell] = dt;
    }
  }
  //----------------
  if(fApplyOfflineCalibration) {
    for(int ich=0; ich!=8; ++ich) {
      for(int icell=0; icell!=1024; ++icell) {
	fDeltaTime[ich][icell] -= fCellTimeOffset[ich][icell]; // subtracting measured cell dt
      }
    }
  }
  //----------------
  // special case for tch=8 (TR) since it requires no calibration
  fDeltaTime[8][0] = 0;
  for(int icell=1; icell!=1024; ++icell) {
    fDeltaTime[8][icell] += fDeltaTime[8][icell-1];
  }
}
//====================================
void DRSGroupData::GetX(int ich, Double_t *x) {
  x[0] = 0;
  for(int isa=0; isa!=1023; ++isa) {
    int icell = ( isa + fTC ) % 1024;
    x[isa+1] = x[isa] + fDeltaTime[ich][icell];
  }
}
//====================================
void DRSGroupData::GetY(int ich, Double_t *y) {
  for(int isa=0; isa!=1024; ++isa) {
    Double_t adc = fChannel[ich][isa];
    //----------------
    if(fApplyOfflineCalibration) {
      int icell = ( isa + fTC ) % 1024;
      adc -= fCellOffset[ich][icell]; // subtracting measured cell offset
    }
    //----------------
    y[isa] = 1000 * ( adc / 4095. - 0.5 );
  }
}
//====================================
void DRSGroupData::GetTRY(Double_t *y) {
  for(int isa=0; isa!=1024; ++isa) {
    Double_t adc = fTR[isa];
    y[isa] = 1000 * ( adc / 4095. - 0.5 );
  }
}
//====================================
void DRSGroupData::GetTRX(Double_t *x) {
  // tch=8 contains already summed axis
  for(int isa=0; isa!=1024; ++isa) {
    x[isa] = fDeltaTime[8][isa];
  }
}
//====================================
void DRSGroupData::GetChannelData(int ich, UInt_t *y) {
  for(int isa=0; isa!=1024; ++isa) {
    y[isa] = fChannel[ich][isa];
  }
}
