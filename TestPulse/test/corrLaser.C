//
// ROOT macro corrBeam.C
//
// Author : J. Blaha, 21/11/2006
//

void corrLaser()
{
  const Int_t kXtals = 1700;
  gStyle->SetTitleFontSize(0.06);
  //gStyle->SetStatFontSize(0.04);    
  gStyle->SetStatColor(10);
  gStyle->SetTitleFillColor(10);
  gStyle->SetCanvasColor(10);
  //  gStyle->SetOptStat("e");
  gStyle->SetOptStat(111111);
  gStyle->SetOptFit(1001);
  gStyle->SetPaperSize(20,26);
  //gStyle->SetStatH(0.10);  
  //gStyle->SetStatW(0.08);
  gStyle->SetStatH(0.15);  
  gStyle->SetStatW(0.25);

  TFile* fout = new TFile("corrBeam.root","recreate");

  hRatio126laser  = new TH1F("hRatio126laser","SM6, Gain ratio x12/x6", 
			      50, 1.85, 2.05);
  hRatio126TP = new TH1F("hRatio126TP","SM6, Gain ratio x12/x6", 
			   50, 1.85, 2.05);

  hCorr126 = new TH2F("hCorr126","SM6, Gain ratio x12/x6 corr.", 
			50,1.9, 2.02,50, 1.9, 2.02);
  
  hDiff126 = new TH1F("hDiff126","SM6, Gain ratio x12/x6 diff.", 
			50, -0.015,0.025);

  TCanvas cPrint("cPrint");
 
  //  Double_t ratio121TP[kXtals];
  Double_t ratio126laser[kXtals];
  Double_t ratio126TP[kXtals];
  Double_t ratio121TP[kXtals];
  for(Int_t iXtal = 0; iXtal < kXtals; iXtal++) {
    ratio126laser[iXtal] = 0.;
    ratio126TP[iXtal] = 0.;
  }
  // Open laser ratio file
  ifstream fRatioInput;
  fRatioInput.open("relgain_laser.out");
  if(!fRatioInput.is_open()) {
    cout  << "can't open laser Ratio file " << endl;
    return;
  }

  for(Int_t i = 0; !fRatioInput.eof(); i++) {
    Int_t Xtal,iXtal;
    Double_t ratio126, error;
    fRatioInput >> Xtal >> iXtal >> ratio126 >> error;
    ratio126laser[iXtal - 1] = ratio126;
    //    cout << iXtal << " " << ratio126laser[iXtal - 1] << endl;
  }
  fRatioInput.close();

  // Open Test Pulse ratio file
  ifstream fRatioInput;
  fRatioInput.open("TestPulse_gainratio_SM06.txt");
  if(!fRatioInput.is_open()) {
    cout  << "can't open old Ratio file " << endl;
    return;
  }
  
  for(Int_t iXtal = 0; !fRatioInput.eof(); iXtal++) {
    Int_t Xtal;
    fRatioInput >>  Xtal >> ratio121TP[iXtal] >> ratio126TP[iXtal];
    if(Xtal != iXtal + 1) cout << " pb Xtal " << Xtal << " " << iXtal << endl;
    cout << iXtal << " " << ratio126TP[iXtal] << endl;
  }
  fRatioInput.close();

  for(Int_t iXtal = 0; iXtal < kXtals; iXtal++) {
    if (ratio126laser[iXtal] != 0) {
      hRatio126laser->Fill(ratio126laser[iXtal]);
      hRatio126TP->Fill(ratio126TP[iXtal]);
      hCorr126->Fill(ratio126laser[iXtal], ratio126TP[iXtal]);
      hDiff126->
	  Fill((ratio126laser[iXtal]-ratio126TP[iXtal])/ratio126laser[iXtal]);
      //      cout << iXtal << " " << ratio126laser[iXtal] << " " << ratio126TP[iXtal]
      //	   << endl;
    }
  }

  TLegend* legend1 = new TLegend(0.15,0.75,0.35,0.85);
  legend1->SetBorderSize(1); 
  legend1->SetFillColor(0); 
  legend1->SetTextSize(0.03); 
  legend1->SetTextFont(42);
 
  TCanvas* cRatio = new TCanvas("cRatio", "cRatio");
  cRatio->SetCanvasSize(630,885);
  cRatio->Divide(1,2);
  cRatio->cd(1);
  hRatio126laser->Draw();
  gPad->Update();
  TPaveStats* st1 = 
    (TPaveStats*)hRatio126laser->GetListOfFunctions()->FindObject("stats");
  st1->SetLineColor(3);
  hRatio126laser->SetFillColor(3);
  hRatio126laser->GetYaxis()->SetTitle("# Xtals");
  legend1->AddEntry(hRatio126laser, "laser","f");
  hRatio126laser->Write();

  hRatio126TP->Draw("sames");
  gPad->Update();
  TPaveStats* st2 = 
    (TPaveStats*)hRatio126TP->GetListOfFunctions()->FindObject("stats");
  st2->SetLineColor(4);
  hRatio126TP->SetFillColor(4);
  hRatio126TP->GetYaxis()->SetTitle("# Xtals");
  legend1->AddEntry(hRatio126TP, "TestPulse","f");
  legend1->Draw();
  cRatio->Modified();
  hRatio126TP->Write();
  gPad->Print("hRatio126TP.eps");

  cRatio->Write("cRatio");
  // cRatio->Print("corrBeam.ps(");

  TCanvas* cRatio = new TCanvas("cRatio","Gain Ratio");
  cRatio->SetCanvasSize(630,885);
  cRatio->Divide(1,2);
  cRatio->cd(1);
  hCorr126->SetMarkerStyle(20);
  hCorr126->Draw();
  hCorr126->GetXaxis()->SetTitle("laser");
  hCorr126->GetYaxis()->SetTitle("TestPulse");
  TLine* l = new TLine(1.9,1.9,2.02,2.02);
  l->SetLineColor(2);
  l->SetLineWidth(3);
  l->Draw();
  cRatio->Update();
  gPad->Print("hCorr126.eps");

  cRatio->cd(2);
  hDiff126->Draw();
  //  hDiff126->GetXaxis()->SetTitle("(Weights 6 - Weights 12)/Weights 6");
  hDiff126->GetXaxis()->SetTitle("(laser - TestPulse)/laser");
  hDiff126->GetYaxis()->SetTitle("# Xtals");
  hDiff126->SetFillColor(5);
  gPad->Print("hDiff126.eps");
  cRatio->Print("corrBeam.ps");

  fout->Write();
  fout->Close();
 
  gROOT->ProcessLine(".q");
}
