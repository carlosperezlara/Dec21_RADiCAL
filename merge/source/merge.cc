#include <iostream>
#include <x742RawReader.h>
#include <DRSGroupData.h>
#include <TString.h>
#include <TTree.h>
#include <TFile.h>

int main() {

  // =====================
  // DRS READER
  x742RawReader fileDRS(Form("../daq/output.dat"));
  DRSGroupData *group0 = new DRSGroupData();
  DRSGroupData *group1 = new DRSGroupData();
  fileDRS.SetGroupData( 0, group0 );
  fileDRS.SetGroupData( 1, group1 );
  group0->LoadCalibrations( "../x742/DRSpackage/x742_calib/Tables_gr0_cell.txt",
			    "../x742/DRSpackage/x742_calib/Tables_gr0_nsample.txt",
			    "../x742/DRSpackage/x742_calib/Tables_gr0_time.txt"  );
  group1->LoadCalibrations( "../x742/DRSpackage/x742_calib/Tables_gr1_cell.txt",
			    "../x742/DRSpackage/x742_calib/Tables_gr1_nsample.txt",
			    "../x742/DRSpackage/x742_calib/Tables_gr1_time.txt" );
  fileDRS.ReadHeader();

  // =====================
  // VARIABLES
  TTree *tEvent = new TTree("event","fnal tb dec 2021");
  // DRS Traces
  Double_t x0[1024];
  Double_t x1[1024];
  Double_t y0[9][1024];
  Double_t y1[9][1024];
  // Other detectors
  tEvent->Branch("gr0_voltage",&y0,"gr0_voltage[9][1024]/D");
  tEvent->Branch("gr0_time",&x0,"gr0_time[1024]/D");
  tEvent->Branch("gr1_voltage",&y1,"gr1_voltage[9][1024]/D");
  tEvent->Branch("gr1_time",&x1,"gr1_time[1024]/D");
    
  // =====================
  // MAIN LOOP
  int nev = 0;
  for(;;++nev) {
    if(!fileDRS.ReadEvent()) break; //all events
    if(nev%500==0)
      std::cout << "Events read so far: " << nev << std::endl;
    group0->GetX(x0);
    group1->GetX(x1);
    for(int ich=0; ich!=8; ++ich) {
      group0->GetY(ich,y0[ich]);
      group1->GetY(ich,y1[ich]);
    }
    group0->GetTRY(y0[8]);
    group1->GetTRY(y1[8]);

    // save
    tEvent->Fill();
  }

  // =====================
  // SAVE INTO ROOT FILE
  TFile *ofile = new TFile("output.root","RECREATE");
  tEvent->Write("event");
  ofile->Close();
  
  return 0;
}


