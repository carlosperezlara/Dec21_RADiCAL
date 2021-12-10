#include "package.cc"

int quickread() {
  x742RawReader file(Form("../output.dat"));

  DRSGroupData *group0 = new DRSGroupData();
  DRSGroupData *group1 = new DRSGroupData();
  file.SetGroupData( 0, group0 );
  file.SetGroupData( 1, group1 );

  TH2D *prof[18];
  TH1D *min[18];
  TH1D *max[18];
  for(int i=0; i!=18; ++i) {
    prof[i] = new TH2D( Form("prof_%02d",i), Form("Channel %d;Time (ns)",i), 1024, -0.1, -0.1+0.2*1024, 100, -500.0, +500.0 );
    min[i] = new TH1D( Form("min_%02d",i), Form("min_%02d",i), 1000, -1000, +1000 );
    max[i] = new TH1D( Form("max_%02d",i), Form("max_%02d",i), 1000, -1000, +1000 );
  }

  // DRS READER
  file.ReadHeader();
  const int n =1024;
  double sx[18][n];
  
  // =====================
  // main loop
  int nev = 0;
  for(;;++nev) {
    if(!file.ReadEvent()) break; //all events
    if(nev%500==0)
      cout << "Events read so far: " << nev << endl;
    Double_t x0[1024];
    Double_t y0[1024];
    Double_t x1[1024];
    Double_t y1[1024];
    UInt_t adc0units[1024];
    UInt_t adc1units[1024];
    for(int ich=0; ich!=8; ++ich) {
      group0->GetX(ich,x0);
      group0->GetY(ich,y0);
      group1->GetX(ich,x1);
      group1->GetY(ich,y1);
      double dmin0 = 1e12;
      double dmin1 = 1e12;
      double dmax0 = -1e12;
      double dmax1 = -1e12;
      for(int isa=0; isa!=1024; ++isa) {
	prof[ich + 0]->Fill( x0[isa], y0[isa] );
	prof[ich + 8]->Fill( x1[isa], y1[isa] );
	if(dmin0>y0[isa]) dmin0 = y0[isa];
	if(dmin1>y1[isa]) dmin1 = y1[isa];

	if(dmax0<y0[isa]) dmax0 = y0[isa];
	if(dmax1<y1[isa]) dmax1 = y1[isa];
      }
      min[ich + 0]->Fill( dmin0 );
      min[ich + 8]->Fill( dmin1 );
      max[ich + 0]->Fill( dmax0 );
      max[ich + 8]->Fill( dmax1 );
    }


    group0->GetTRX(x0);
    group0->GetTRY(y0);
    group1->GetTRX(x1);
    group1->GetTRY(y1);
    double dmin0 = 1e12;
    double dmin1 = 1e12;
    double dmax0 = -1e12;
    double dmax1 = -1e12;
    for(int isa=0; isa!=1024; ++isa) {
      prof[16]->Fill( x0[isa], y0[isa] );
      prof[17]->Fill( x1[isa], y1[isa] );
      if(dmin0>y0[isa]) dmin0 = y0[isa];
      if(dmin1>y1[isa]) dmin1 = y1[isa];
      if(dmax0<y0[isa]) dmax0 = y0[isa];
      if(dmax1<y1[isa]) dmax1 = y1[isa];
    }
    min[16]->Fill( dmin0 );
    min[17]->Fill( dmin1 );
    max[16]->Fill( dmax0 );
    max[17]->Fill( dmax1 );
  }
  
  TCanvas *main1 = new TCanvas("main1");
  main1->Divide(5,4);
  for(int ich = 0; ich!=18; ++ich) {
    main1->cd(ich+1);
    prof[ich]->Draw("colz");
  }

  TCanvas *main2 = new TCanvas("main2");
  main2->Divide(5,4);
  for(int ich = 0; ich!=18; ++ich) {
    main2->cd(ich+1);
    min[ich]->Draw();
  }

  TCanvas *main3 = new TCanvas("main3");
  main3->Divide(5,4);
  for(int ich = 0; ich!=18; ++ich) {
    main3->cd(ich+1);
    max[ich]->Draw();
  }
  
  return 0;
}

