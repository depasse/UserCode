TGraphErrors *gXtalAmplitude_12;
TGraphErrors *gXtalAmplitude_6;
TGraphErrors *gXtalAmplitude_1;
TGraph *gXtalChi2_12;
TGraph *gXtalChi2_6;
TGraph *gXtalChi2_1;
ofstream fRatioOutput;

int ComputeRatio(TGraphErrors *g, TH1F *h, int ixtal, double ysup, double yinf, double eysup=0, double eyinf=0) {
  //  cout << "channel " << ixtal << endl;
  if(yinf > 0 && ysup > 0) g->SetPoint(ixtal+1, ixtal+1, ysup/yinf);
  else g->SetPoint(ixtal+1, ixtal+1, 0);
  if(yinf > 0 && ysup > 0) g->SetPointError(ixtal+1, 0, yinf/ysup * (eysup/ysup + eyinf/yinf));
  else g->SetPointError(ixtal+1, 0, 0);
  if(yinf <= 0 || ysup <=0) {
    cout<<"Bad channel (ampl<=0) : "<<ixtal+1<<"  gain = "<<ysup<<"/"<<yinf<<endl;
    return ixtal+1;
  }
  else
    if(ysup/yinf < 1.7 || ysup/yinf > 13) {
      cout<<"Bad channel (g<1.7, g>13) : "<<ixtal+1<<" gain "<<ysup<<"/"<<yinf<<" = "<<ysup/yinf<<endl;
      return ixtal+1;
    }
    else h->Fill(ysup/yinf);
	
  return 0;
}

TH1F* ProfileToHisto(TProfile *p) {
  TH1F *h =new TH1F( Form("h%s",p->GetName()), p->GetTitle(), p->GetNbinsX(), p->GetXaxis()->GetXmin(), p->GetXaxis()->GetXmax());
  for(int ibin=0; ibin < p->GetNbinsX()+2; ibin++) {
    h->SetBinContent(ibin, p->GetBinContent(ibin));
    h->SetBinError(ibin, p->GetBinError(ibin));
    //cout<<"Error : "<<p->GetBinError(ibin)<<endl;
  }
  return h;
}

TPaveStats* GetPaveStats(TH1 *h1) {
  //h1->GetListOfFunctions()->Print();
  TObject *obj;
  TPaveStats *stats = 0;
  TIter next(h1->GetListOfFunctions());
  while ((obj = next())) {
    if(obj->InheritsFrom("TPaveStats")) stats = (TPaveStats*) obj;
  }
  if(stats) cout<<"stats : "<<stats->GetName()<<endl;
  return stats;
}

void DrawPaveStats(TH1 *h1, float x1, float y1, float x2, float y2) {
  TPaveStats* stats = GetPaveStats(h1);
  stats->SetX1NDC(x1);
  stats->SetY1NDC(y1);
  stats->SetX2NDC(x2);
  stats->SetY2NDC(y2);
}

void DrawPulse(TFile *file, int channel=1) {

  TProfile *p12 = (TProfile*) file->Get(Form("pXtalPulse_%i_12", channel));
  TH1F* h12 = ProfileToHisto( p12 );
  TProfile *p6 = (TProfile*) file->Get(Form("pXtalPulse_%i_6", channel));
  TH1F* h6 = ProfileToHisto( p6 );
  TProfile *p1 = (TProfile*) file->Get(Form("pXtalPulse_%i_1", channel));
  TH1F* h1 = ProfileToHisto( p1 );
  
  float max=0;
  max = h12->GetMaximum();
  if(h6->GetMaximum() > max ) max=h6->GetMaximum();
  if(h1->GetMaximum() > max ) max=h1->GetMaximum();
  h12->GetYaxis()->SetRangeUser(0, max*1.1);
  
  h12->Draw("E");
  TPaveText *text = new TPaveText();
  text->SetX1NDC(0.4);
  text->SetY1NDC(0.92);
  text->SetX2NDC(0.7);
  text->SetY2NDC(0.98);
  text->AddText("Bad Channel");
  text->SetTextAlign(22);
  text->SetTextColor(2);
  text->Draw();
  h6->SetLineColor(2);
  h6->Draw("E:same");
  h1->SetLineColor(4);
  h1->Draw("E:same");
}

void DrawAmplitude(TFile *file, int channel=1, int gain=1) {
  TH1F *hbadchi2 = (TH1F*) file->Get(Form("hXtalAmplitude_%i_%i", channel, gain));
  float mean = hbadchi2->GetMean();
  float rms = hbadchi2->GetRMS();
  hbadchi2->GetXaxis()->SetRangeUser(mean-10*rms, mean+10*rms);
  hbadchi2->Draw();
  TPaveText *text = new TPaveText();
  text->SetX1NDC(0.4);
  text->SetY1NDC(0.92);
  text->SetX2NDC(0.7);
  text->SetY2NDC(0.98);
  text->AddText("Bad Channel");
  text->SetTextAlign(22);
  text->SetTextColor(2);
  text->Draw();
}

void CopyGraph(TGraphErrors *g, TGraphErrors *gnew) {
  gnew->Set(g->GetN());
  double x, y, ex, ey;
  for(int ipt=0; ipt < g->GetN(); ipt++) {
    g->GetPoint(ipt, x, y);
    ex = g->GetErrorX(ipt);
    ey = g->GetErrorY(ipt);
    gnew->SetPoint(ipt, x, y);
    gnew->SetPointError(ipt, ex, ey);
  }
}

//void GainRatioForOneRun(float *gainratios, char* filename="GainRatio-H4-000018025-SM5-TEST_PULSE-MGPA.root", int DAC=0) {
void GainRatioForOneRun(float *gainratios, char* filename="TestPulse_208624.root", int DAC=0) {

  TString sfilename(filename);
  sfilename.Remove(0, sfilename.Index("/")+1);
  sfilename.Remove(0, sfilename.Index("/")+1); // do nothing if files in local directory
  sfilename.Remove(0, sfilename.Index("-")+1);
  sfilename.Remove(sfilename.Index("."), sfilename.Length());
  cout <<"File : "<< sfilename << endl;
  TString sSM;
  sSM+=sfilename;
  sSM.Remove(0, sSM.Index("SM"));
  //  sSM.Remove(sSM.Index("-"), sSM.Length());
  cout<<"SM : "<<sSM<<endl;
  TString sMethod;
  sMethod+=sfilename;
  sMethod.Remove(0, sMethod.Index("MGPA")+5);
  cout<<"Method : "<<sMethod<<endl;
  cout<<"DAC : "<<DAC<<endl;

  TFile *file = TFile::Open(filename);
  TFile *foutput = new TFile(Form("GainRatio-%s.root", sSM.Data()), "UPDATE");
  gXtalAmplitude_12 = (TGraphErrors*) file->Get("gXtalAmplitude_12");
  gXtalAmplitude_6 = (TGraphErrors*) file->Get("gXtalAmplitude_6");
  gXtalAmplitude_1 = (TGraphErrors*) file->Get("gXtalAmplitude_1");
  gXtalChi2_12 = (TGraph*) file->Get("gXtalChi2_12");
  gXtalChi2_6 = (TGraph*) file->Get("gXtalChi2_6");
  gXtalChi2_1 = (TGraph*) file->Get("gXtalChi2_1");

  TCanvas *c1 = new TCanvas("c1","c1",0,0,1500,500);
  TPad *pad = new TPad("pad", "", 0, 0.05, 1, 1);
  //pad->SetFillColor(18);
  pad->Draw();

  c1->cd();
  TSlider *slider = new TSlider("slider", "xaxis", 0.1, 0.005, 0.9, 0.045, 16);
  slider->SetFillColor(14);
  slider->SetRange(0.1,0.5);

  pad->cd();
  TH1F *hf = c1->DrawFrame(0,0,1750,gXtalAmplitude_12->GetYaxis()->GetXmax());
  //TH1F *hf = c1->DrawFrame(0,0,1750,1.5);
  gXtalAmplitude_12->SetHistogram(hf);
  gXtalAmplitude_6->SetLineColor(2);
  gXtalAmplitude_1->SetLineColor(4);

  bool scan = 0;
  int nrange = 40;
  if(scan)
    for(int irange=0; irange < nrange; irange++) {
      hf->GetXaxis()->SetRangeUser(irange*(1750./nrange)-5,(irange+1)*(1750./nrange));
      gXtalAmplitude_12->Draw("AP");
      gXtalAmplitude_6->Draw("P");
      gXtalAmplitude_1->Draw("P");
      slider->SetRange(irange*(1./nrange), (irange+1)*(1./nrange));
      pad->Modified();
      pad->Update();
      gSystem->Sleep(50);
      //getchar();
    }
  hf->GetXaxis()->SetRangeUser(0,1750);
  gXtalAmplitude_12->Draw("AP");
  gXtalAmplitude_6->Draw("P");
  gXtalAmplitude_1->Draw("P");

  TCanvas *c2 = new TCanvas("c2","c2",0,350,700,500);

  TGraphErrors *g12_6 = new TGraphErrors(gXtalAmplitude_12->GetN());
  TGraphErrors *g12_1 = new TGraphErrors(gXtalAmplitude_12->GetN());
  TH1F *h12_6 = new TH1F(Form("h12_6_%s_DAC%i_%s", sSM.Data(), DAC, sMethod.Data()),"Gain Ratio 12/6",300,0,3);
  TH1F *h12_1 = new TH1F(Form("h12_1_%s_DAC%i_%s", sSM.Data(), DAC, sMethod.Data()),"Gain Ratio 12/1",300,0,15);
  double x12, y12, x6, y6, x1, y1;
  double xc12, yc12, xc6, yc6, xc1, yc1;
  int badchannel=0;
  for(int ixtal=0; ixtal < 1700; ixtal++) {
    gXtalAmplitude_12->GetPoint(ixtal+1, x12, y12);
    gXtalAmplitude_6->GetPoint(ixtal+1, x6, y6);
    gXtalAmplitude_1->GetPoint(ixtal+1, x1, y1);
    if( y12 == 0 || y6 == 0 || y1 == 0) {
      cout << "Channel " << ixtal+1 << " ampl 12 " << y12 << " 6 " << y6 << " 1 " << y1 << endl;
      continue;
    }
    ComputeRatio(g12_6, h12_6, ixtal, y12, y6, gXtalAmplitude_12->GetErrorY(ixtal), gXtalAmplitude_6->GetErrorY(ixtal));
    badchannel = ComputeRatio(g12_1, h12_1, ixtal, y12, y1, gXtalAmplitude_12->GetErrorY(ixtal), gXtalAmplitude_1->GetErrorY(ixtal));
    /*
    if(badchannel) {
      DrawPulse(file, badchannel);
      c2->Modified();
      c2->Update();
      getchar();
    }
    */
    gXtalChi2_12->GetPoint(ixtal+1, xc12, yc12);
    gXtalChi2_6->GetPoint(ixtal+1, xc6, yc6);
    gXtalChi2_1->GetPoint(ixtal+1, xc1, yc1);
	
    int badchi2limit=150;
    /*
    if(!badchannel)
      if(yc12 > badchi2limit || yc6 > badchi2limit || yc1 > badchi2limit) {  
	cout<<"Bad channel (chi2>"<<badchi2limit<<") : "<<ixtal+1<<"  gain x12/x6 = "<<y12<<"/"<<y6<<" = "<<y12/y6<<endl;
	cout<<"                         gain x12/x1 = "<<y12<<"/"<<y1<<" = "<<y12/y1<<endl;
	c2->SetLogy();
	if(yc12 > badchi2limit) DrawAmplitude(file, ixtal+1, 12);
	if(yc6 > badchi2limit) DrawAmplitude(file, ixtal+1, 6);
	if(yc1 > badchi2limit) DrawAmplitude(file, ixtal+1, 1);
	c2->Print("badchannel.eps");
	c2->Modified();
	c2->Update();
	getchar();
	c2->SetLogy(0);
      }
    */
  }

  TH1F *g2h12_6 = new TH1F(Form("g2h12_6_%s_DAC%i_%s", sSM.Data(), DAC, sMethod.Data()),"Gain Ratio 12/6",1700,0,1700);
  TH1F *g2h12_1 = new TH1F(Form("g2h12_1_%s_DAC%i_%s", sSM.Data(), DAC, sMethod.Data()),"Gain Ratio 12/1",1700,0,1700);
  for(int ixtal=0; ixtal < 1700; ixtal++) {
    g12_6->GetPoint(ixtal+1, x6, y6);
    g2h12_6->Fill(x6, y6);
    g12_1->GetPoint(ixtal+1, x1, y1);
    g2h12_1->Fill(x1, y1);
  }
  for(int ixtal = 1; ixtal < 1701; ixtal++) {
    g12_1->GetPoint(ixtal, x1, y1);
    g12_6->GetPoint(ixtal, x6, y6);
    fRatioOutput <<  ixtal << " " << y1 << " " << y6 << endl;
  }

  double mean = 0;
  double sigma = 0;

  h12_6->Draw();
  h12_6->Fit("gaus","+");
  c2->SetLogy();
  gStyle->SetOptStat(111111);
  gStyle->SetOptFit();
  DrawPaveStats(h12_6, 0.75, 0.65, 0.98, 0.98);
  mean = h12_6->GetFunction("gaus")->GetParameter(1);
  sigma = h12_6->GetFunction("gaus")->GetParameter(2);
  h12_6->GetXaxis()->SetRangeUser(mean-10*sigma, mean+10*sigma);
  c2->Print("gainratio12_6.eps");

  //c2->Clear();
  //g12_6->Draw("AP");

  TCanvas *c3 = new TCanvas("c3","c3",500,350,700,500);
  h12_1->Draw();
  h12_1->Fit("gaus","+");
  c3->SetLogy();
  mean = h12_1->GetFunction("gaus")->GetParameter(1);
  sigma = h12_1->GetFunction("gaus")->GetParameter(2);
  h12_1->GetXaxis()->SetRangeUser(mean-10*sigma, mean+10*sigma);
  c3->Print("gainratio12_1.eps");
  DrawPaveStats(h12_1, 0.75, 0.65, 0.98, 0.98);
  c3->Print("gainratio12_1.eps");

  //gainratios = new float[4];
  gainratios[0]=h12_6->GetFunction("gaus")->GetParameter(1);
  gainratios[1]=h12_6->GetFunction("gaus")->GetParameter(2);
  gainratios[2]=h12_1->GetFunction("gaus")->GetParameter(1);
  gainratios[3]=h12_1->GetFunction("gaus")->GetParameter(2);
  gainratios[4]=h12_6->GetFunction("gaus")->GetParError(1);
  gainratios[5]=h12_1->GetFunction("gaus")->GetParError(1);

  cout<<"GAINRATIO 12/6 : "<<gainratios[0]<<endl;

  c1->Update();
  c2->Update();
  c3->Update();
  getchar();

  foutput->cd();

  //	TGraphErrors *g_12 = new TGraphErrors();
  //	CopyGraph(gXtalAmplitude_12, g_12);
  //	g_12->SetName(Form("g_12_%s_DAC%i_%s", sSM.Data(), DAC, sMethod.Data()));
  //	g_12->Write();
  gXtalAmplitude_12->SetName(Form("gAmplitude_12_%s_DAC%i_%s", sSM.Data(), DAC, sMethod.Data()));
  gXtalAmplitude_12->Write();
  gXtalAmplitude_6->SetName(Form("gAmplitude_6_%s_DAC%i_%s", sSM.Data(), DAC, sMethod.Data()));
  gXtalAmplitude_6->Write();
  gXtalAmplitude_1->SetName(Form("gAmplitude_1_%s_DAC%i_%s", sSM.Data(), DAC, sMethod.Data()));
  gXtalAmplitude_1->Write();

  TH1F *g2h12 = new TH1F(Form("g2hAmplitude_12_%s_DAC%i_%s", sSM.Data(), DAC, sMethod.Data()),"Reconstructed Amplitude, gain x12",1700,0,1700);
  TH1F *g2h6 = new TH1F(Form("g2hAmplitude_6_%s_DAC%i_%s", sSM.Data(), DAC, sMethod.Data()),"Reconstructed Amplitude, gain x6",1700,0,1700);
  TH1F *g2h1 = new TH1F(Form("g2hAmplitude_1_%s_DAC%i_%s", sSM.Data(), DAC, sMethod.Data()),"Reconstructed Amplitude, gain x1",1700,0,1700);
  for(int ixtal=0; ixtal < 1700; ixtal++) {
    gXtalAmplitude_12->GetPoint(ixtal+1, x12, y12);
    g2h12->Fill(x12, y12);
    gXtalAmplitude_6->GetPoint(ixtal+1, x6, y6);
    g2h6->Fill(x6, y6);
    gXtalAmplitude_1->GetPoint(ixtal+1, x1, y1);
    g2h1->Fill(x1, y1);
  }
  g2h12->Write();
  g2h6->Write();
  g2h1->Write();

  /*	gXtalChi2_12->SetName(Form("gChi2_12_%s_DAC%i_%s", sSM.Data(), DAC, sMethod.Data()));
	gXtalChi2_12->Write();
	gXtalChi2_6->SetName(Form("gChi2_6_%s_DAC%i_%s", sSM.Data(), DAC, sMethod.Data()));
	gXtalChi2_6->Write();
	gXtalChi2_1->SetName(Form("gChi2_1_%s_DAC%i_%s", sSM.Data(), DAC, sMethod.Data()));
	gXtalChi2_1->Write();
  */	
  g12_6->SetName(Form("g12_6_%s_DAC%i_%s", sSM.Data(), DAC, sMethod.Data()));
  g12_6->Write();
  g12_1->SetName(Form("g12_1_%s_DAC%i_%s", sSM.Data(), DAC, sMethod.Data()));
  g12_1->Write();
  h12_6->Write();
  cout<<h12_6->GetName()<<endl;
  h12_1->Write();
  g2h12_6->Write();
  g2h12_1->Write();

  foutput->Close();
  file->Close();
}


void GainRatioFromRecHit() {
  // Lecture fichier d'entree
  FILE *config = fopen("GainRatioFromRecHit.in", "r");
  if (config==NULL) cout<<"No file 'GainRatioFromRecHit.in'"<<endl;

  char FILENAME[100];
  char DAC[4];
  int DACSETTING;
  int NFILES=0;

  const int nmaxfiles = 10;
  char files[nmaxfiles][100];
  float dac[nmaxfiles];

  while(!feof(config)) {
    fscanf(config, "%s %s %i", FILENAME, DAC, &DACSETTING);
    //cout<<NFILES<<" "<<FILENAME<<" "<<DACSETTING<<endl;
    strcpy(files[NFILES],FILENAME);
    dac[NFILES]=DACSETTING;
    NFILES++;
  }
  fclose(config);

  // Open output ratio file
  fRatioOutput.open("TestPulse_gainratio.txt");

  const int nfiles = NFILES-1; // derniere ligne lue 2x
  // charge, index is DAC
  float charge[257];
  charge[0]=0;
  charge[1]=0.6;
  charge[2]=1.7;
  charge[3]=0.7;
  charge[4]=4.1;
  charge[5]=0.8;
  charge[6]=2.1;
  charge[7]=0.8;
  charge[8]=6.4;
  charge[9]=0.9;
  charge[10]=2.3;
  charge[17]=1.2;
  charge[18]=3.0;
  charge[64]=20.4;

  for(int ifile=0; ifile < nfiles; ifile++) {
    cout<<files[ifile]<<" DAC "<<dac[ifile]<<endl;
  }
  cout<<endl;

  float *gainratios = new float[6];
  TGraphErrors *GainRatio12_6vsDAC = new TGraphErrors(nfiles);
  TGraphErrors *GainRatio12_6vsCharge = new TGraphErrors(nfiles);
  TGraph *GainRatioError12_6vsCharge = new TGraph(nfiles);

  for(int ifile=0; ifile < nfiles; ifile++) {
    DACSETTING=dac[ifile];
    GainRatioForOneRun(gainratios, files[ifile], dac[ifile]);
    GainRatio12_6vsDAC->SetPoint(ifile, dac[ifile], gainratios[0]); 
    GainRatio12_6vsDAC->SetPointError(ifile, 0, gainratios[1]);
    GainRatio12_6vsCharge->SetPoint(ifile, charge[DACSETTING], gainratios[0]);
    //GainRatio12_6vsCharge->SetPointError(ifile, 0, gainratios[1]);
    GainRatio12_6vsCharge->SetPointError(ifile, 0, gainratios[4]);
    GainRatioError12_6vsCharge->SetPoint(ifile, charge[DACSETTING], gainratios[1]);
    cout<<"ratio : "<<gainratios[0]<<endl;
  }
  fRatioOutput.close();

  /*
  TCanvas *c4 = new TCanvas("c4","c4",0,0,700,500);
  GainRatio12_6vsDAC->Draw("AP");
  GainRatio12_6vsDAC->SetMarkerStyle(20);
  GainRatio12_6vsDAC->SetTitle("Gain ratio average vs DAC");
  GainRatio12_6vsDAC->GetXaxis()->SetTitle("DAC");
  GainRatio12_6vsDAC->GetYaxis()->SetRangeUser(1.95, 1.98);
  gStyle->SetOptFit(1);
  GainRatio12_6vsDAC->Fit("pol1");
  c4->Print("GainRatiovsDAC.eps");
  c4->Modified();
  c4->Update();
  getchar();

  c4->Clear();
  GainRatio12_6vsCharge->Draw("AP");
  GainRatio12_6vsCharge->SetMarkerStyle(20);
  GainRatio12_6vsCharge->SetTitle("Gain ratio average vs charge");
  GainRatio12_6vsCharge->GetXaxis()->SetTitle("charge [pC]");
  GainRatio12_6vsCharge->GetYaxis()->SetRangeUser(1.95, 1.98);
  GainRatio12_6vsCharge->Fit("pol1");
  c4->Print("GainRatiovsCharge.eps");
  GainRatio12_6vsCharge->Print();
  GainRatio12_6vsCharge->Print("all");
  c4->Modified();
  c4->Update();
  getchar();
  */

  /*	c4->Clear();
	GainRatioError12_6vsCharge->Draw("AP");
	GainRatioError12_6vsCharge->SetMarkerStyle(20);
	GainRatioError12_6vsCharge->SetTitle("Gain ratio error vs charge");
	GainRatioError12_6vsCharge->GetXaxis()->SetTitle("charge [pC]");
	c4->Print("GainRatioErrorvsCharge.eps");*/
}
