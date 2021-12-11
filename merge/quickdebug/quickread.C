void style( TH1 *puntero, int col ) {
  puntero->SetLineColor( col );
  puntero->SetFillColor( col );
  puntero->GetYaxis()->SetLabelSize(0.06);
  puntero->GetXaxis()->SetLabelSize(0.06);
}

int quickread() {
  TFile *ifile = new TFile("../output.root");
  TTree *tree = (TTree*) ifile->Get("event");
  Double_t gr0_voltage[9][1024];
  Double_t gr1_voltage[9][1024];
  Double_t gr0_time[1024];
  Double_t gr1_time[1024];
  tree->SetBranchAddress("gr0_voltage",&gr0_voltage);
  tree->SetBranchAddress("gr1_voltage",&gr1_voltage);
  tree->SetBranchAddress("gr0_time",&gr0_time);
  tree->SetBranchAddress("gr1_time",&gr1_time);
  
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

  
  
  // =====================
  // main loop
  Long64_t entries = tree->GetEntries();
  for(Long64_t nev = 0; nev!= entries ; ++nev) {
    tree->GetEntry( nev );
    if(nev%500==0)
      cout << "Events read so far: " << nev << endl;

    double dmin[18];
    double dmax[18];
    for(int ich=0; ich!=8; ++ich) {
      dmin[ich+0] = 1e12;
      dmin[ich+8] = 1e12;
      dmax[ich+0] = -1e12;
      dmax[ich+8] = -1e12;
      for(int isa=0; isa!=1024; ++isa) {
	prof[ich + 0]->Fill( gr0_time[isa], gr0_voltage[ich][isa] );
	prof[ich + 8]->Fill( gr1_time[isa], gr1_voltage[ich][isa] );
	if(dmin[ich + 0]>gr0_voltage[ich][isa]) dmin[ich + 0] = gr0_voltage[ich][isa];
	if(dmin[ich + 8]>gr1_voltage[ich][isa]) dmin[ich + 8] = gr1_voltage[ich][isa];
	if(dmax[ich + 0]<gr0_voltage[ich][isa]) dmax[ich + 0] = gr0_voltage[ich][isa];
	if(dmax[ich + 8]<gr1_voltage[ich][isa]) dmax[ich + 8] = gr1_voltage[ich][isa];
      }
      min[ich + 0]->Fill( dmin[ich + 0] );
      min[ich + 8]->Fill( dmin[ich + 8] );
      max[ich + 0]->Fill( dmax[ich + 0] );
      max[ich + 8]->Fill( dmax[ich + 8] );
    }

    corr->Fill( dmin[1], dmin[2] );

    dmin[16] = 1e12;
    dmin[17] = 1e12;
    dmax[16] = -1e12;
    dmax[17] = -1e12;
    for(int isa=0; isa!=1024; ++isa) {
      prof[16]->Fill( gr0_time[isa], gr0_voltage[8][isa] );
      prof[17]->Fill( gr1_time[isa], gr1_voltage[8][isa] );
      if(dmin[16]>gr0_voltage[8][isa]) dmin[16] = gr0_voltage[8][isa];
      if(dmin[17]>gr1_voltage[8][isa]) dmin[17] = gr1_voltage[8][isa];
      if(dmax[16]<gr0_voltage[8][isa]) dmax[16] = gr0_voltage[8][isa];
      if(dmax[17]<gr1_voltage[8][isa]) dmax[17] = gr1_voltage[8][isa];
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

  return 0;
}


