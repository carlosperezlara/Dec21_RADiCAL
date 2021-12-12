#include "package.cc"

void style( TH1 *puntero, int col ) {
  puntero->SetLineColor( col );
  puntero->SetFillColor( col );
  puntero->GetYaxis()->SetLabelSize(0.06);
  puntero->GetXaxis()->SetLabelSize(0.06);
}

int quickread() {
  x742RawReader file(Form("../output.dat"));

  DRSGroupData *group0 = new DRSGroupData();
  DRSGroupData *group1 = new DRSGroupData();
  file.SetGroupData( 0, group0 );
  file.SetGroupData( 1, group1 );
  group0->LoadCalibrations( "../../DRSpackage/x742_calib/Tables_gr0_cell.txt",
			    "../../DRSpackage/x742_calib/Tables_gr0_nsample.txt",
			    "../../DRSpackage/x742_calib/Tables_gr0_time.txt"  );
  group1->LoadCalibrations( "../../DRSpackage/x742_calib/Tables_gr1_cell.txt",
			    "../../DRSpackage/x742_calib/Tables_gr1_nsample.txt",
			    "../../DRSpackage/x742_calib/Tables_gr1_time.txt" );
  
  TGraph *timestamp = new TGraph();
  TH2D *prof[18];
  TH1D *min[18];
  TH1D *max[18];
  for(int i=0; i!=18; ++i) {
    prof[i] = new TH2D( Form("prof_%02d",i), Form("Channel %d;Time (ns)",i), 1024, -0.1, -0.1+0.2*1024, 100, -510.0, +510.0 );
    min[i] = new TH1D( Form("min_%02d",i), Form("min_%02d",i), 100, -510, +510 );
    max[i] = new TH1D( Form("max_%02d",i), Form("max_%02d",i), 100, -510, +510 );
    style( min[i], kBlue-3 );
    style( max[i], kRed-3 );
  }
  TH2D *corr = new TH2D( "corr", "corr", 100, -510, +510, 100, -510.0, +510.0 );

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

    UInt_t ts_gr0 = group0->GetTimeStamp();
    UInt_t ts_gr1 = group1->GetTimeStamp();
    if(ts_gr0 != ts_gr1) {
      cout << "WARNING!" << endl;
    }
    timestamp->SetPoint(nev+1,ts_gr0*8.5e-9,nev);
    
    Double_t x0[1024];
    Double_t y0[1024];
    Double_t x1[1024];
    Double_t y1[1024];
    UInt_t adc0units[1024];
    UInt_t adc1units[1024];

    group0->GetX(x0);
    group1->GetX(x1);

    double dmin[18];
    double dmax[18];
    for(int ich=0; ich!=8; ++ich) {
      group0->GetY(ich,y0);
      group1->GetY(ich,y1);
      dmin[ich+0] = 1e12;
      dmin[ich+8] = 1e12;
      dmax[ich+0] = -1e12;
      dmax[ich+8] = -1e12;
      for(int isa=0; isa!=1024; ++isa) {
	prof[ich + 0]->Fill( x0[isa], y0[isa] );
	prof[ich + 8]->Fill( x1[isa], y1[isa] );
	if(dmin[ich + 0]>y0[isa]) dmin[ich + 0] = y0[isa];
	if(dmin[ich + 8]>y1[isa]) dmin[ich + 8] = y1[isa];
	if(dmax[ich + 0]<y0[isa]) dmax[ich + 0] = y0[isa];
	if(dmax[ich + 8]<y1[isa]) dmax[ich + 8] = y1[isa];
      }
      min[ich + 0]->Fill( dmin[ich + 0] );
      min[ich + 8]->Fill( dmin[ich + 8] );
      max[ich + 0]->Fill( dmax[ich + 0] );
      max[ich + 8]->Fill( dmax[ich + 8] );
    }

    corr->Fill( dmin[1], dmin[2] );

    group0->GetTRY(y0);
    group1->GetTRY(y1);
    dmin[16] = 1e12;
    dmin[17] = 1e12;
    dmax[16] = -1e12;
    dmax[17] = -1e12;
    for(int isa=0; isa!=1024; ++isa) {
      prof[16]->Fill( x0[isa], y0[isa] );
      prof[17]->Fill( x1[isa], y1[isa] );
      if(dmin[16]>y0[isa]) dmin[16] = y0[isa];
      if(dmin[17]>y1[isa]) dmin[17] = y1[isa];
      if(dmax[16]<y0[isa]) dmax[16] = y0[isa];
      if(dmax[17]<y1[isa]) dmax[17] = y1[isa];
    }
    min[16]->Fill( dmin[16] );
    min[17]->Fill( dmin[17] );
    max[16]->Fill( dmax[16] );
    max[17]->Fill( dmax[17] );
  }
  
  TCanvas *main1 = new TCanvas("main1","ALL EVENTS",100,0,800,600);
  main1->Divide(5,4);
  for(int ich = 0; ich!=18; ++ich) {
    main1->cd(ich+1);
    prof[ich]->Draw("colz");
  }

  TCanvas *main2 = new TCanvas("main2","FLOOR == CEILING",900,0,800,600);
  main2->Divide(5,4);
  for(int ich = 0; ich!=18; ++ich) {
    main2->cd(ich+1);
    if( min[ich]->GetMaximum() > max[ich]->GetMaximum() ) {
      min[ich]->Draw();
      max[ich]->Draw("same");
      min[ich]->SetTitle(Form("CHN  %d",ich));
    } else {
      max[ich]->Draw();
      min[ich]->Draw("same");
      max[ich]->SetTitle(Form("CHN  %d",ich));
    }
  }

  TCanvas *main3 = new TCanvas("main3","CORRELATION",900,600,800,600);
  corr->Draw("colz");

  TCanvas *main4 = new TCanvas("main4","EVENT TIME",100,600,800,600);
  timestamp->Draw("A*");
  timestamp->GetYaxis()->SetTitle("triggers");
  timestamp->GetXaxis()->SetTitle("seconds");
  
  return 0;
}


