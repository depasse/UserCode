#include <iostream>
#include <fstream>
#include <string>
#include <iomanip>
#include "TProfile.h"
const Int_t kGains=3;
const Int_t kEBChannels=61200;
const Int_t kEEChannels=14648;

void TPCompare() {
  //  Int_t runn = 112511;
  Int_t runn = 64028;
  Int_t gainValues[kGains] = {12, 6, 1};
  TH1F* hRat126_1 = new TH1F("hRap126_1","My Ratio 12/6",100,1.86,2.06);
  TH1F* hRat121 = new TH1F("hRap121","Ratio 12/1",100,10.5,11.5);
  TH1F* hRat126_2 = new TH1F("hRap126_2","His Ratio 12/6",100,1.86,2.06);

  TH2F* hCorr126 = new TH2F("hCorr126","Gain ratio x12/x6 correlation", 
			50,1.86, 2.06,50, 1.86, 2.06);
  TH1F* hDiff126 = new TH1F("hDiff126","Gain ratio x12/x6 difference", 
			50, -0.025, 0.025);

  TCanvas cPrint("cPrint");
 
  Int_t answ;
  ifstream fRatio1, fRatio2;
  fRatio1.open(Form("TP_GainratioEB_%i.txt",runn));
  fRatio2.open("barrelGain12to6-64028.txt");
  //  fRatio2.open("TP_GainratioEB_64028.txt");
  Int_t ixtal;
  Double_t Rat126_1, Rat126_2, Rat121;
  for (int iChannel = 0; iChannel < kEBChannels; iChannel++) {
    fRatio1 >> ixtal  >> Rat121 >> Rat126_1;
    fRatio2 >> Rat126_2;
    if(iChannel%1000 == 0) cout << " Channel " << ixtal << " Rat 12/6 " << Rat126_1 
				<< " his " << Rat126_2  << " Rat 12/1 " << Rat121 << endl;
    hRat126_1->Fill(Rat126_1);
    hRat121->Fill(Rat121);
    hRat126_2->Fill(Rat126_2);
    if(Rat126_1 != 0 && Rat126_2 != -2 && Rat126_2 != 0) {
      hCorr126->Fill(Rat126_1, Rat126_2);
      hDiff126->Fill((Rat126_1-Rat126_2)/Rat126_1);
    }
    else if(Rat126_1 == 0 && Rat126_2 != -2 && Rat126_2 != 0)
      cout <<  " Channel " << ixtal << " my ratio = 0 when his = " << Rat126_2<< endl;
  }
  fRatio1.close();
  fRatio2.close();
  TCanvas* cRatio = new TCanvas("cRatio", "cRatio");
  cRatio->SetCanvasSize(630,885);
  cRatio->Divide(1,2);
  cRatio->cd(1);
  hRat126_1->Draw();
  cRatio->cd(2);
  hRat126_2->Draw();
  //  cout << "continue? "<< endl;
  //  cin >> answ;
  //  c1->cd(3);
  //  hRat121->Draw();
  gPad->Update();
  gPad->Print("Ratio126.gif");

  TCanvas* cRatioM1 = new TCanvas("cRatioM1","Gain Ratio");
  cRatioM1->SetCanvasSize(630,885);
  cRatioM1->Divide(1,2);
  cRatioM1->cd(1);
  hCorr126->SetMarkerStyle(20);
  hCorr126->Draw();
  //  hCorr126M1->GetXaxis()->SetTitle("Weights 6");
  //  hCorr126M1->GetYaxis()->SetTitle("Weights 12");
  hCorr126->GetXaxis()->SetTitle("Mine");
  hCorr126->GetYaxis()->SetTitle("Yours");
  TLine* l = new TLine(1.86,1.86,2.06,2.06);
  l->SetLineColor(2);
  l->SetLineWidth(3);
  l->Draw();
  cRatioM1->Update();

  cRatioM1->cd(2);
  hDiff126->Draw();
  //  hDiff126M1->GetXaxis()->SetTitle("(Weights 6 - Weights 12)/Weights 6");
  hDiff126->GetXaxis()->SetTitle("(Mine - Yours)/Mine");
  hDiff126->GetYaxis()->SetTitle("# Xtals");
  hDiff126->SetFillColor(5);
  gPad->Print("hCorr126.gif");
}
