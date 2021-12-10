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
    fDeltaTime[icell] = 0;
  }
  fApplyOfflineCalibration = kTRUE;
  //fApplyOfflineCalibration = kFALSE;
  for(int icell=0; icell!=1024; ++icell) {
    for(int ich=0; ich!=8; ++ich) {
      fCellOffset[ich][icell] = 0;
    }
    fCellTimeOffset[icell] = 0;
  }
  CalibrateTime(0);
}
//====================================
DRSGroupData::~DRSGroupData() {
}
//====================================
void DRSGroupData::LoadCalibrations(TString fileCell, TString filePhase, TString fileTime) {
  int ch, ce;
  Double_t offset;
  //===========
  ifstream icelloffset( fileCell.Data() );
  for(int ich=0; ich!=9; ++ich) {
    for(int icell=0; icell!=1024; ++icell) {
      icelloffset >> ch >> ce >> offset;
      SetCellOffsetConstant( ch, ce, offset );
    }
  }
  icelloffset.close();
  //===========
  ifstream icelltime( fileTime.Data() );
  Double_t dt = 0.2;// 0.2ns    <== 5.0 GS/s
  for(int icell=0; icell!=1024; ++icell) {
    icelltime >> ce >> offset;
    SetCellTimeOffsetConstant( ce, dt*ce - offset );
  }
  icelltime.close();

  CalibrateTime(0);
}
//====================================
void DRSGroupData::CalibrateTime(int freq) {
  Double_t dt = 0.2;       // 0.2ns    <== 5.0 GS/s
  if(freq==1) dt = 0.4;    // 0.4ns    <== 2.5 GS/s
  if(freq==2) dt = 1.0;    // 1.0ns    <== 1.0 GS/s
  if(freq==3) dt = 1.3333; // 1.3333ns <== 750 MS/s
  // baseline
  for(int icell=0; icell!=1024; ++icell) {
    fDeltaTime[icell] = dt;
  }
  //----------------
  if(fApplyOfflineCalibration) {
    for(int icell=0; icell!=1024; ++icell) {
      fDeltaTime[icell] -= fCellTimeOffset[icell]; // subtracting measured cell dt
    }
  }
}
//====================================
void DRSGroupData::GetX(Double_t *x) {
  x[0] = 0;
  for(int isa=0; isa!=1023; ++isa) {
    int icell = ( isa + fTC ) % 1024;
    x[isa+1] = x[isa] + fDeltaTime[icell];
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
void DRSGroupData::GetChannelData(int ich, UInt_t *y) {
  for(int isa=0; isa!=1024; ++isa) {
    y[isa] = fChannel[ich][isa];
  }
}
