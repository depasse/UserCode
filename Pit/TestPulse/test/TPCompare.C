#include <iostream>
#include <fstream>
#include <string>
#include <iomanip>
#include "TProfile.h"
const Int_t kGains=3;
const Int_t kEBChannels=61200;
const Int_t kEEChannels=14648;

void TPCompare() {
  Int_t runn = 204292;
 

  Int_t answ;
  Int_t gainValues[kGains] = {12, 6, 1};
  TH1F* hRat126_1 = new TH1F("hRap126_1",Form("EE Ratio 12/6 run %i", runn),100,1.86,2.06);
  TH1F* hRat121_1 = new TH1F("hRap121_1",Form("EE Ratio 12/1 run %i", runn),100,10.5,11.5);
  TH1F* hRat126_2 = new TH1F("hRap126_2","EE Ratio 12/6 Reference : run 192043",100,1.86,2.06);
  TH1F* hRat121_2 = new TH1F("hRap121_2","EE Ratio 12/1 Reference : run 192043",100,10.5,11.5);

  TH2F* hCorr126 = new TH2F("hCorr126",Form("EE Gain ratio x12/x6 correlation _ run %i", runn), 
			50,1.86, 2.06,50, 1.86, 2.06);
  TH1F* hDiff126 = new TH1F("hDiff126",Form("EE Gain ratio x12/x6 difference _ run %i", runn), 
			50, -0.025, 0.025);
  TH2F* hCorr121 = new TH2F("hCorr121",Form("EE Gain ratio x12/x1 correlation _ run %i", runn), 
			50,10.5, 11.5,50, 10.5, 11.5);
  TH1F* hDiff121 = new TH1F("hDiff121",Form("EE Gain ratio x12/x1 difference _ run %i", runn), 
			50, -0.025, 0.025);

  TCanvas cPrint("cPrint");
 
 
  ifstream fRatio1, fRatio2;
  fRatio1.open(Form("TP_GainratioEE_%i.txt",runn));
  fRatio2.open("TP_GainratioEE_192043.txt");
  Int_t ixtal;
  Double_t Rat126_1, Rat126_2, Rat121_1, Rat121_2;
  for (int iChannel = 0; iChannel < kEEChannels; iChannel++) {
    fRatio1 >> ixtal  >> Rat121_1 >> Rat126_1;
    fRatio2 >> ixtal  >> Rat121_2 >> Rat126_2;
    if(iChannel%1000 == 0) cout << " Channel " << ixtal << " Rat 12/6 " << Rat126_1 
				<< " Ref " << Rat126_2  << " Rat 12/1 " << Rat121_1
				<< " Ref " << Rat121_2  << endl;
    hRat126_1->Fill(Rat126_1);
    hRat121_1->Fill(Rat121_1);
    hRat126_2->Fill(Rat126_2);
    hRat121_2->Fill(Rat121_2);
   if(Rat126_1 != 0 && Rat126_2 != 0) {
      hCorr126->Fill(Rat126_2, Rat126_1);
      hDiff126->Fill((Rat126_1-Rat126_2)/Rat126_2);
    }
    else if(Rat126_1 == 0 || Rat126_2 == 0)
      cout <<  " Channel " << ixtal << " Ratio12/6 = 0  " << endl;
   if(Rat121_1 != 0 && Rat121_2 != 0) {
      hCorr121->Fill(Rat121_2, Rat121_1);
      hDiff121->Fill((Rat121_1-Rat121_2)/Rat121_2);
    }
    else if(Rat121_1 == 0 || Rat121_2 == 0)
      cout <<  " Channel " << ixtal << " Ratio12/1 = 0  " << endl;
  }
  fRatio1.close();
  fRatio2.close();


  TCanvas* cRatio126 = new TCanvas("cRatio126", "Ratio 12/6");
  // cRatio126->SetCanvasSize(630,885);
  cRatio126->Divide(1,2);
  cRatio126->cd(1);
  hRat126_1->Draw();

  cRatio126->cd(2);
  hRat126_2->Draw();
  cRatio126->Print(Form("EE_Ratio126_%i.gif", runn));

  TCanvas* cRatio121 = new TCanvas("cRatio121", "Ratio 12/1");
  // cRatio121->SetCanvasSize(630,885);
  cRatio121->Divide(1,2);
  cRatio121->cd(1);
  hRat126_1->Draw();

  cRatio121->cd(2);
  hRat126_2->Draw();
  cRatio121->Print(Form("EE_Ratio121_%i.gif", runn));

  //  cout << "continue? "<< endl;
  // cin >> answ;

  TCanvas* cRatioCorr126 = new TCanvas("cRatioCorr126","Corr Gain Ratio 12/6");
  // cRatioCorr126->SetCanvasSize(630,885);
  //cRatioCorr126->Divide(1,1);
  //cRatioCorr126->cd(1);
  hCorr126->SetMarkerStyle(20);
  hCorr126->Draw();
  hCorr126->GetXaxis()->SetTitle("run 192043");
  hCorr126->GetYaxis()->SetTitle(Form("run %i", runn));
  TLine* l = new TLine(1.86,1.86,2.06,2.06);
  l->SetLineColor(2);
  l->SetLineWidth(3);
  l->Draw();
  cRatioCorr126->Print(Form("EE_hCorr126_%i.gif", runn));

  // cout << "continue? "<< endl;
  // cin >> answ;

  TCanvas* cRatioCorr121 = new TCanvas("cRatioCorr121","Corr Gain Ratio 12/1");
  //  cRatioCorr121->SetCanvasSize(630,885);
  //cRatioCorr121->Divide(1,1);
  //cRatioCorr121->cd(1);
  hCorr121->SetMarkerStyle(20);
  hCorr121->Draw();
  hCorr121->GetXaxis()->SetTitle("run 192043");
  hCorr121->GetYaxis()->SetTitle(Form("run %i", runn));
  TLine* l = new TLine(10.5,10.5,11.5,11.5);
  l->SetLineColor(3);
  l->SetLineWidth(3);
  l->Draw();
  cRatioCorr121->Print(Form("EE_hCorr121_%i.gif", runn));

  // cout << "continue? "<< endl;
  // cin >> answ;



  TCanvas* cRatioDiff126 = new TCanvas("cRatioDiff126","Diff Gain Ratio 12/6");
  //  cRatioDiff126->SetCanvasSize(630,885);
  // cRatioDiff126->Divide(1,1);
  // cRatioDiff126->cd(1);
  cRatioDiff126->SetLogy(1);
  hDiff126->SetMarkerStyle(20);
  hDiff126->Draw();
  hDiff126->GetXaxis()->SetTitle(Form("(run %i - run 192043)/run 192043", runn));
  hDiff126->GetYaxis()->SetTitle("# Xtals");
  hDiff126->SetFillColor(5);
  cRatioDiff126->Print(Form("EE_hDiff126_%i.gif", runn));

  TCanvas* cRatioDiff121 = new TCanvas("cRatioDiff121","Diff Gain Ratio 12/1");
  // cRatioDiff121->SetCanvasSize(630,885);
  // cRatioDiff121->Divide(1,1);
  // cRatioDiff121->cd(1);
  cRatioDiff121->SetLogy(1);
  hDiff121->SetMarkerStyle(20);
  hDiff121->Draw();
  hDiff121->GetXaxis()->SetTitle(Form("(run %i - run 192043)/run 192043", runn));
  hDiff121->GetYaxis()->SetTitle("# Xtals");
  hDiff121->SetFillColor(5);
  cRatioDiff121->Print(Form("EE_hDiff121_%i.gif", runn));

}
