#include <string.h>
#include "TFile.h"
#include "TH1.h"
#include "TProfile.h"
#include "TPaveStats.h"
#include "TCanvas.h"
#include "Riostream.h"
#include <cstdlib> 
#include <fstream>
#include <iostream>

gROOT->Reset();

Int_t Plot_EBEE_summary() {
  TProfile*** hRing = new TProfile**[28];
  TProfile*** hRingWeek = new TProfile**[28];
  for (int ring = 0; ring < 28; ring++) {
    hRing[ring] = new TProfile*[2];
    hRingWeek[ring] = new TProfile*[2];
  }

  TFile* fin = new TFile("./TranspVar_EBEE_summary.root");
  // loop over all keys in this directory
  TIter nextkey(gDirectory->GetListOfKeys() );
  TKey *key;

  while ((key = (TKey*)nextkey())) {      // read object from file
    fin->cd();
    TObject *obj = key->ReadObj();
    if (obj->IsA()->InheritsFrom("TProfile")) { 
      string tit = obj->GetName();
      //      TProfile* h = (TProfile*) obj;
      if(tit.substr(0,5) == "Ring_") {
	string bid, bid2;
	int ring, side;
	if(tit.size() == 8) {
	  bid = tit.substr(5,1);
	  bid2 = tit.substr(7,1);
	}
	else {
	  bid = tit.substr(5,2);
	  bid2 = tit.substr(8,1);
	}
	std::istringstream ii(bid);
	ii >> ring;
	std::istringstream ii(bid2);
	ii >> side;
	//	cout << " title " << tit << " ring " << ring << " side " << side << endl;
	hRing[ring][side] = (TProfile*)obj->Clone(tit.c_str());
      }    //  Ring_
      // overdraw Weekly values
      else if(tit.substr(0,9) == "RingWeek_") {
	string bid, bid2;
	int ring, side;
	if(tit.size() == 12) {
	  bid = tit.substr(9,1);
	  bid2 = tit.substr(11,1);
	}
	else {
	  bid = tit.substr(9,2);
	  bid2 = tit.substr(12,1);
	}
	std::istringstream ii(bid);
	ii >> ring;
	std::istringstream ii(bid2);
	ii >> side;
	//	cout << " title " << tit << " ring " << ring << " side " << side << endl;
	hRingWeek[ring][side] = (TProfile*)obj->Clone(tit.c_str());
      }    //  Ring_Week
    }     // TProfile
  }      //  loop over all histos
  TCanvas cCan("cCan", "Transparency variations");
  cCan.Divide(2,3);
  cCan.SetCanvasSize(630,885);
  gStyle->SetOptStat(0);

  for (int side = 0; side < 2; side++) {
    int cnt = 0, view = 0;
    for (int ring = 0; ring < 28; ring++) {
      double min = 1.;
      for(int bin = 1; bin < 10000; bin++) {
	double x = hRing[ring][side]->GetBinContent(bin);
	if(x > 0.) {
	  double err = hRing[ring][side]->GetBinError(bin);
	  double val = x - err;
	  if(val < min) min = val;
	}
      }   //  loop over all bins
      int imin = min * 100;
      min = imin / 100.;
      hRing[ring][side]->SetMinimum(min);
      hRing[ring][side]->GetXaxis()->SetTimeDisplay(1);
      hRing[ring][side]->GetXaxis()->SetTimeFormat("%d\/%m\/%y%F1970-01-01 00:00:00");
      if(ring < 17)
	if(side == 0)
	  hRing[ring][side]->SetTitle(Form("EB- ring %i", ring +1));
	else
	  hRing[ring][side]->SetTitle(Form("EB+ ring %i", ring +1));
      else
	if(side == 0)
	  hRing[ring][side]->SetTitle(Form("EE- ring %i", ring +1));
	else
	  hRing[ring][side]->SetTitle(Form("EE+ ring %i", ring +1));
      cnt++;
      cCan.cd(cnt);
      hRing[ring][side]->Draw();
      hRingWeek[ring][side]->Draw("PSAME");
      hRingWeek[ring][side]->SetMarkerStyle(20);
      hRingWeek[ring][side]->SetMarkerSize(0.5);
      hRingWeek[ring][side]->SetMarkerColor(kRed);
      hRingWeek[ring][side]->SetLineColor(kRed);
      cout << " ring " << ring << " side " << side 
	   << " cnt " << cnt << " view " << view << endl;
      if(cnt%6 == 0 || (view == 2 && cnt == 5) || (view == 4 && cnt == 5)) {
	if(side == 0)
	  cCan.Print(Form("EBEE-_Transp_%i.gif",view));  // print canvas to gif file
	else
	  cCan.Print(Form("EBEE+_Transp_%i.gif",view));
	cCan.Clear();
	cCan.Divide(2,3);
	view++;
	cnt = 0;
      }   //  make a gif file */
    }  // loop over side
  }   //  loop over ring
  fin->Close();
 
  cout << " Well done!!!" << endl;
  
  return 0;
}
