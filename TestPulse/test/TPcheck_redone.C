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

void TPcheck_redone() {
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

  string  rootfile;
  //  for (int irun = 0; irun < kRuns; irun++) {
  for (int irun = 0; irun < 1; irun++) {
    rootfile = Form("/home/fay/TestPulse_000%d.root",runn[irun]);
    cout << " Opening file " << irun << " " << rootfile << endl;
    TFile file(rootfile.c_str());

    gXtalAmplitude_12 = (TGraphErrors*) file.Get("gXtalAmplitude_12");
    gXtalAmplitude_6 = (TGraphErrors*) file.Get("gXtalAmplitude_6");
    gXtalAmplitude_1 = (TGraphErrors*) file.Get("gXtalAmplitude_1");

    gXtalChi2_12 = (TGraph*) file.Get("gXtalChi2_12");
    gXtalChi2_6 = (TGraph*) file.Get("gXtalChi2_6");
    gXtalChi2_1 = (TGraph*) file.Get("gXtalChi2_1");

    gXtalEntry_12 = (TGraph*) file.Get("gXtalEntry_12");
    gXtalEntry_6 = (TGraph*) file.Get("gXtalEntry_6");
    gXtalEntry_1 = (TGraph*) file.Get("gXtalEntry_1");
    
    TCanvas *cEntry = new TCanvas("cEntry","cEntry",0,0,1000,500);
    TH1F *hf = cEntry->DrawFrame(0,0,1700,gXtalEntry_12->GetYaxis()->GetXmax());
    gXtalEntry_12->SetHistogram(hf);
    gXtalEntry_6->SetMarkerColor(2);
    gXtalEntry_1->SetMarkerColor(4);
    hf->GetXaxis()->SetRangeUser(0,1700);
    gXtalEntry_12->Draw("AP");
    gXtalEntry_6->Draw("P");
    gXtalEntry_1->Draw("P");
    cEntry->Update();
    cout << "continue? "<< endl;
    int i;
    cin >> i;

    TCanvas *c1 = new TCanvas("c1","c1",0,0,1500,500);
    c1->cd();
    TH1F *hf = c1->DrawFrame(0,0,1750,gXtalAmplitude_12->GetYaxis()->GetXmax());
    gXtalAmplitude_12->SetHistogram(hf);
    gXtalAmplitude_6->SetLineColor(2);
    gXtalAmplitude_1->SetLineColor(4);
    hf->GetXaxis()->SetRangeUser(0,1750);
    gXtalAmplitude_12->Draw("AP");
    gXtalAmplitude_6->Draw("P");
    gXtalAmplitude_1->Draw("P");
    cout << "continue? "<< endl;
    int i;
    cin >> i;

    TCanvas *chi2 = new TCanvas("chi2","chi2",0,0,1000,500);
    TH1F *hf = chi2->DrawFrame(0,0,1750,gXtalChi2_12->GetYaxis()->GetXmax());
    gXtalChi2_12->SetHistogram(hf);
    gXtalChi2_6->SetMarkerColor(2);
    gXtalChi2_1->SetMarkerColor(4);
    hf->GetXaxis()->SetRangeUser(0,1750);
    gXtalChi2_12->Draw("AP");
    gXtalChi2_6->Draw("P");
    gXtalChi2_1->Draw("P");
    chi2->Update();
    cout << "continue? "<< endl;
    int i;
    cin >> i;
   
  }   // end loop on runs
}
