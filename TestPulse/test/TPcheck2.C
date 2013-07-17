#include <iostream>
#include <fstream>
#include <string>
#include <iomanip>
#include "TProfile.h"

const Int_t kGains=3;
const Int_t kChannels=1700;
const Int_t kRuns = 234;
TGraphErrors *gXtalAmplitude_12;
TGraphErrors *gXtalAmplitude_6;
TGraphErrors *gXtalAmplitude_1;

void TPcheck2() {
  Int_t runn[kRuns] = {15817,15846,15854,15861,15868,15880,15881,15891,15896,15904,
		       15908,15911,15921,15932,15936,15939,15945,15952,15957,15963,
		       15969,15976,15988,16021,16024,16025,16026,16027,16028,
		       16029,16031,16032,16033,16042,16043,16044,
		       16086,16096,16106,16114,16119,16123,
		       16128,16135,16156,16174,16205,16218,16224,16228,16235,
		       16243,16246,16254,16264,16267,16268,16270,16303,16311,16314,
		       16322,16328,16334,16339,16345,16352,16358,16364,16374,16380,
		       16402,16407,16410,16417,16422,16427,16433,16440,16446,16469,
		       16476,16479,16483,16501,16507,16509,16524,16536,16540,16545,
		       16550,16554,16564,16569,16574,16582,16584,16591,16596,16611,
		       16624,16630,16652,16658,16673,16683,16689,16693,16707,16711,
		       16732,16737,16748,16754,16759,16766,16790,16797,16803,16810,
		       16821,16826,16838,16843,16856,16863,16871,16878,16886,16890,
		       16894,16899,16918,16926,16935,16941,16961,16975,16992,17018,
		       17032,17046,17066,17067,17073,17092,17103,17108,17118,17123,
		       17128,17135,17141,17149,17156,17161,17176,17191,17208,17219,
		       17224,17241,17251,17258,17266,17270,17273,17286,17297,17314,
		       17318,17322,17351,17358,17368,17389,17406,17409,17415,17421,
		       17434,17441,17455,17460,17461,17469,17473,17474,17476,
		       17486,17490,17495,17501,17508,17515,17524,17527,
		       17577,17591,17599,17618,17638,17647,17676,17709,17723,
		       17732,17746,17752,17756,17764,17768,17776,17786,17790,17794,
		       17798,17802,17806,17811,17817,17822,17828,17834,17845,17853,
		       17861,17875,17890,17907,17925,17936,17974};
  Int_t gainValues[kGains] = {12, 6, 1};

  TH1F*** hXtal = new TH1F**[kChannels];
  for (int iChannel = 0; iChannel < kChannels; iChannel++) {
    hXtal[iChannel] = new TH1F*[kGains];
    for (int gainId = 0; gainId < kGains; gainId++) {
      hXtal[iChannel][gainId] = new TH1F(Form("hXtal_%d_%d",iChannel,gainValues[gainId]),Form("Xtal_%d_%d",iChannel,gainValues[gainId]),150,0,1.5);
    }
  }

  string  rootfile;
  //  for (int irun = 0; irun < kRuns; irun++) {
  for (int irun = 0; irun < 10; irun++) {
    rootfile = Form("/home/fay/TestPulse_000%d_SM06.root",runn[irun]);
    cout << " Opening file " << rootfile << endl;
    TFile file(rootfile.c_str());

    gXtalAmplitude_12 = (TGraphErrors*) file.Get("gXtalAmplitude_12");
    gXtalAmplitude_6 = (TGraphErrors*) file.Get("gXtalAmplitude_6");
    gXtalAmplitude_1 = (TGraphErrors*) file.Get("gXtalAmplitude_1");

    /*
    TCanvas *c1 = new TCanvas("c1","c1",0,0,1500,500);
    //    TPad *pad = new TPad("pad", "", 0, 0.05, 1, 1);
    //    pad->Draw();
    c1->cd();
    TH1F *hf = c1->DrawFrame(0,0,1750,gXtalAmplitude_12->GetYaxis()->GetXmax());
    gXtalAmplitude_12->SetHistogram(hf);
    gXtalAmplitude_6->SetLineColor(2);
    gXtalAmplitude_1->SetLineColor(4);
    hf->GetXaxis()->SetRangeUser(0,1750);
    gXtalAmplitude_12->Draw("AP");
    gXtalAmplitude_6->Draw("P");
    gXtalAmplitude_1->Draw("P");
    */

    Double_t x12,y12,x6,y6,x1,y1;
    Int_t Xtalmiss[kGains] = {0,0,0};
    for (int iChannel = 0; iChannel < kChannels; iChannel++) {
      y12 = y6 = y1 =0;
      gXtalAmplitude_12->GetPoint(iChannel+1,x12,y12);
      gXtalAmplitude_6->GetPoint(iChannel+1,x6,y6);
      gXtalAmplitude_1->GetPoint(iChannel+1,x1,y1);
      if(y12 == 0) {
	Xtalmiss[0] ++;
	//	if(iChannel < 1675) cout << iChannel << " y12 = 0 " << endl;
      }
      else {
	hXtal[iChannel][0]->Fill(y12);
      }
      if(y6 == 0) {
	Xtalmiss[1] ++;
	//	if(iChannel < 1675) cout << iChannel << " y6 = 0 " << endl;
      }
      else {
	hXtal[iChannel][1]->Fill(y6);
      }
      if(y1 == 0) {
	Xtalmiss[2] ++;
	//	if(iChannel < 1675) cout << iChannel << " y1 = 0 " << endl;
      }
      else {
	hXtal[iChannel][2]->Fill(y1);
      }
    }
    /*
    TH1F*** hXtalAmplitude = new TH1F**[kChannels];
    Int_t Xtalmiss[kGains] = {0,0,0};
    for (int iChannel = 0; iChannel < kChannels; iChannel++) {
      hXtalAmplitude[iChannel] = new TH1F*[kGains];
      // Loop over different gains
      for (int gainId = 0; gainId < kGains; gainId++) {
	hXtalAmplitude[iChannel][gainId] = (TH1F*) file.Get(Form("hXtalAmplitude_%d_%d", iChannel+1,gainValues[gainId]));
	if(!hXtalAmplitude[iChannel][gainId]) {
	  //	  cout << " no histo for channel " << iChannel+1 << " gain " << gainValues[gainId] << endl;
	  Xtalmiss[gainId] ++;
	  continue ;
	}
	int nEntries = hXtalAmplitude[iChannel][gainId]->GetEntries();
	if (nEntries) {
	  //	  Double_t Totmean = hPedestal[iChannel][gainId]->GetMean();
	  //	  Double_t Totsig = hPedestal[iChannel][gainId]->GetRMS();
	  //
	  //	  hTotNoise[gainId]->Fill(Totsig);
	}  // nEntries not 0
	else {
	  std::cout << " No entry for channel " <<  iChannel 
		    << std::endl;
	}
      }  // End loop over gains
    }  // End loop over channels
    */
    for (int gainId = 0; gainId < kGains; gainId++) {
      //      if(Xtalmiss[gainId] > 0) 
      if(Xtalmiss[gainId] > 25) 
	cout << "gain " << gainValues[gainId] << " missing " << Xtalmiss[gainId]
	     << endl;
    }
  }
  TH1F** hRMS = new TH1F*[kGains];
  for (int gainId = 0; gainId < kGains; gainId++) {
    hRMS[gainId] = new TH1F(Form("RMS_%d",gainValues[gainId]),
    			    Form("RMS gain %d",gainValues[gainId]),1700,0,1700);
    for (int iChannel = 0; iChannel < kChannels; iChannel++) {
      Double_t rms = hXtal[iChannel][gainId]->GetRMS();
      Int_t entries = hXtal[iChannel][gainId]->GetEntries();
      hRMS[gainId]->Fill(iChannel,rms);
      hXtal[iChannel][gainId]->Delete();
    }
  }
 
  TCanvas *c2 = new TCanvas("c2","c2",1000,500);
  //  c2->cd();
  //  TH1F *hf = c2->DrawFrame(0,0,1750,1.5);
  //  hRMS[0]->SetHistogram(hf);
  // hRMS[1]->SetLineColor(2);
  // hRMS[2]->SetLineColor(4);
  //  hf->GetXaxis()->SetRangeUser(0,1750);
  //  hRMS[0]->Draw();
  //  hRMS[0]->GetXaxis()->SetTitle("Channels");
  hRMS[0]->Draw();
  hRMS[0]->GetXaxis()->SetTitle("Channels");
  c2->Update();
  hRMS[1]->SetLineColor(2);
  hRMS[1]->Draw("SAME");
  c2->Update();
  hRMS[2]->SetLineColor(4);
  hRMS[2]->Draw("SAME");
  c2->Update();
  c2->Print("TPcheck.ps");
  cout << "new gain " << gainId << endl;
  int i;
  cin >> i;
 
  //  gROOT->ProcessLine(".q");
}
