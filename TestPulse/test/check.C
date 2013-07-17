#include <iostream>
#include <fstream>
#include <string>
#include "TProfile.h"

const Int_t kSample=10;
const Int_t kGains=3;

void check() {
  h_gain1  = new TH2F("h_gain1","Gain 1 correlation", 100, 10., 12., 100 ,10., 12.);
  h_gain6  = new TH2F("h_gain6","Gain 6 correlation", 100, 1.9, 2.0, 100, 1.9, 2.0);
  // Open test pulse ratio file
  ifstream fRatioInput;
  fRatioInput.open("TP_Gainratio_00027475.txt");
  if(!fRatioInput.is_open()) {
    cout  << "can't open first TestPulse Ratio file " << endl;
    return;
  }
  // Read ratios
  Double_t ratiotp1[1700];
  Double_t ratiotp6[1700];
  for(int i = 0; i < 1700; i++) {
    Int_t Xtal;
    fRatioInput >>  Xtal >> ratiotp1[i] >> ratiotp6[i];
  }
  fRatioInput.close();
  //  fRatioInput.open("/afs/cern.ch/cms/ECAL/testbeam/pedestal/2006/GAINRATIO/laboratory_gainratio_sm15.txt");
  fRatioInput.open("TP_Gainratio_00027591.txt");
  if(!fRatioInput.is_open()) {
    cout  << "can't open second TestPulse Ratio file " << endl;
    return;
  }
  /*
  Int_t sm;
  fRatioInput >>  sm;
  fRatioInput >>  sm;
  string st1,st2,st3,st4,st5;
  fRatioInput >>  st1;
  fRatioInput >>  st1 >> st2 >> st3 >> st4 >> st5;
  fRatioInput >>  sm;
  fRatioInput >>  st1 >> st2;
  */
  Double_t ratio1;
  Double_t ratio6;
  for(int i = 0; i < 1700; i++) {
    Int_t Xtal;
    //    string barcode;
    //    Int_t ch, val;
    //    fRatioInput >>  barcode >> ch >> Xtal >> ratio1 >> ratio6 >> val;
    fRatioInput >> Xtal >> ratio1 >> ratio6;
    //    cout << " crystal " << Xtal << " ratio6 " << ratio6 << endl;
    Double_t diff1 = ratio1 - ratiotp1[Xtal];
    Double_t diff6 = ratio6 - ratiotp6[Xtal];
    h_gain1->Fill(ratio1, ratiotp1[Xtal]);
    h_gain6->Fill(ratio6, ratiotp6[Xtal]);
    if(fabs(diff6) > 0.1) cout << " channel " << Xtal << " diff " << diff6 << endl;
    //cout << " channel " << Xtal << " diff " << diff6 << endl;
  }
  fRatioInput.close();
  TCanvas* cRatio = new TCanvas("cRatio","GainRatio");
  cRatio->SetCanvasSize(630,885);
  cRatio->Divide(1,2);
  cRatio->cd(1);
  h_gain1->Draw();
  cRatio->Update();
  cRatio->cd(2);
  h_gain6->Draw();
  cRatio->Update();
  cRatio->Print("GainRatio.ps");
  h_gain1->Delete();
  h_gain6->Delete();
  gROOT->ProcessLine(".q");
}
