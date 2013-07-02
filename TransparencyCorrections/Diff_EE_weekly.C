#include <string.h>
#include "TFile.h"
#include "TH1.h"
#include "Riostream.h"
#include "TPaveStats.h"
#include <cstdlib> 
#include <fstream>
#include <iostream>
#include <iomanip.h>   

gROOT->Reset();

Int_t Diff_EE_weekly() {
  gStyle->SetOptStat(0);
  TFile* fin = new TFile("./TranspVar_EE_weekly_49.root");
  fin->cd(); 
 
  TProfile** hEERing = new TProfile*[2];

  hEERing[0] = (TProfile*)EERingSide_0;
  hEERing[1] = (TProfile*)EERingSide_1;

  TCanvas cCan("cCan", "Transparency variation");
  //  cCan.Divide(3,2);
  cCan.Divide(1,2);
  //  cCan.SetCanvasSize(630,885);
      
  double x[2][11];
  for (Int_t side = 0; side < 2; side++) {
    double min = 1.;
    for (Int_t ring = 0; ring < 11; ring++) {
      //    for(int bin = 0; bin < 8000; bin++) {
      x[side][ring] = hEERing[side]->GetBinContent(ring + 1);
      if(x > 0.) {
	double err = hEERing[side]->GetBinError(ring + 1);
	double val = x - err;
	if(val < min) min = val;
      }
    }
  }
  cCan.cd(1);
  hEERing[0]->Draw("");
  cCan.cd(2);
  hEERing[1]->Draw("");
  cCan.Print("TranspVar_EE_week_49.gif");  // print canvas to gif file

  hEERing[0]->Delete();
  hEERing[1]->Delete();
  fin->Close();

  fin = new TFile("./TranspVar_EE_weekly_48.root");
  fin->cd(); 
  hEERing[0] = (TProfile*)EERingSide_0;
  hEERing[1] = (TProfile*)EERingSide_1;
  TProfile** hEEDiffSide = new TProfile*[2];
  for (int side = 0; side < 2; side++) {
    string EE = "EE-";
    if(side == 1) EE = "EE+";
    hEEDiffSide[side] = new TProfile(Form("EEDiffSide_%i",side),Form("Difference %s",EE.c_str()),11, 0., 11.);

    for (Int_t ring = 0; ring < 11; ring++) {
	double xold = hEERing[side]->GetBinContent(ring + 1);
	double diff =  x[side][ring] - xold;
	cout << " side " << side << " ring " << ring << " val " << xold << " diff " << diff << endl;
	hEEDiffSide[side]->Fill(ring, diff);
    }
  }
  //    double min = 1.;
  TCanvas cCanDiff("cCanDiff", "Transparency difference");
  //  cCan.Divide(3,2);
  cCanDiff.Divide(1,2);
  for (Int_t side = 0; side < 2; side++) {
    //   int imin = min * 100;
    //    min = imin / 100.;
    //    hEERing[side]->SetMinimum(min);
    //    cout << " Ring " << ring << " Minimal value " << min << endl;
    cCanDiff.cd(side + 1);
    hEEDiffSide[side]->Draw("P");
    hEEDiffSide[side]->SetMarkerStyle(20);
    hEEDiffSide[side]->SetMarkerSize(0.5);
    hEEDiffSide[side]->SetMarkerColor(kRed);
    //    hEEDiffSide[side]->SetLineColor(kRed);
  }  //  loop over sides

  cCanDiff.Print("TranspVar_EE_Diff_49_48.gif");  // print canvas to gif file
  cCanDiff.Clear();
  cout << " Well done!!!" << endl;
  
  return 0;
}
