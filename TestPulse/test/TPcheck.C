#include <iostream>
#include <fstream>
#include <string>
#include <iomanip>
#include "TProfile.h"

const Int_t kGains=3;
const Int_t kChannels=1700;
//const Int_t kRuns = 234;
const Int_t kRuns = 224;
TGraphErrors *gXtalAmplitude_12;
TGraphErrors *gXtalAmplitude_6;
TGraphErrors *gXtalAmplitude_1;

void TPcheck() {
  //  Int_t runn[kRuns] = {15817,15846,15854,15861,15868,15880,15881,15891,15896,15904,
  Int_t runn[kRuns] = {15846,15854,15861,15868,15880,15881,15891,15896,15904,
		       15908,15911,15921,15932,15936,15939,15945,15952,15957,15963,
		       15969,15976,15988,16021,
		       //		       16024,16025,16026,16027,16028,16029,16031,16032,16033,
		       16042,16043,16044,16086,16096,16106,16114,16119,16123,
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
  TH1F** hR126 = new TH1F*[kChannels];
  TH1F** hR121 = new TH1F*[kChannels];
  TH1F** hR126p = new TH1F*[kChannels];
  TH1F** hR121p = new TH1F*[kChannels];
  for (int iChannel = 0; iChannel < kChannels; iChannel++) {
    hXtal[iChannel] = new TH1F*[kGains];
    if(iChannel == 1295)
      hR126[1295] = new TH1F("hR126_1296","Ratio126 1296",200,1.90,2.00);
    else
      hR126[iChannel] = new TH1F(Form("hR126_%d",iChannel + 1),Form("Ratio126 %d",iChannel + 1),200,1.88,2.08);
    hR121[iChannel] = new TH1F(Form("hR121_%d",iChannel + 1),Form("Ratio121 %d",iChannel + 1),200,10.5.,11.5);
    hR126p[iChannel] = new TH1F(Form("hR126p_%d",iChannel + 1),Form("Ratio126 %d",iChannel + 1),kRuns,0,kRuns);
    hR121p[iChannel] = new TH1F(Form("hR121p_%d",iChannel + 1),Form("Ratio121 %d",iChannel + 1),kRuns,0,kRuns);
    for (int gainId = 0; gainId < kGains; gainId++) {
      hXtal[iChannel][gainId] = new TH1F(Form("hXtal_%d_%d",iChannel + 1,gainValues[gainId]),"hXtal",150,0,1.5);
    }
  }
  TH1F* hRMS_12 = new TH1F("hRMS_12","RMS 12",100,0,0.001);
  TH1F* hRMS_6 = new TH1F("hRMS_6","RMS 6",100,0,0.001);
  TH1F* hRMS_1 = new TH1F("hRMS_1","RMS 1",100,0,0.001);
  /*
  TH1F* hXl300_126 = new TH1F("hXtal_300_126","hXtal_300_126",kRuns,0,kRuns);
  TH1F* hXl300_126p = new TH1F("hXtal_300_126p","Xtal_300_126",100,1.94,1.95);
  TH1F* hXl300_121 = new TH1F("hXtal_300_121","hXtal_300_121",kRuns,0,kRuns);
  TH1F* hXl300_121p = new TH1F("hXtal_300_121p","Xtal_300_121",100,10.95,11.05);
  TH1F* hXl873_126 = new TH1F("hXtal_873_126","hXtal_873_126",kRuns,0,kRuns);
  TH1F* hXl873_126p = new TH1F("hXtal_873_126p","Xtal_873_126",100,1.94,1.95);
  TH1F* hXl873_121p = new TH1F("hXtal_873_121p","Xtal_873_121",100,10.95,11.05);
  TH1F* hXl873_121 = new TH1F("hXtal_873_121","hXtal_873_121",kRuns,0,kRuns);
  TH1F* hXl811_126 = new TH1F("hXtal_811_126","hXtal_811_126",kRuns,0,kRuns);
  TH1F* hXl811_126p = new TH1F("hXtal_811_126p","Xtal_811_126",100,1.99,2.00);
  TH1F* hXl893_126 = new TH1F("hXtal_893_126","hXtal_893_126",kRuns,0,kRuns);
  TH1F* hXl893_126p = new TH1F("hXtal_893_126p","Xtal_893_126",100,1.94,1.95);
  TH1F* hXl813_121 = new TH1F("hXtal_813_121","hXtal_813_121",kRuns,0,kRuns);
  TH1F* hXl813_121p = new TH1F("hXtal_813_121p","Xtal_813_121",100,10.95,11.05);
  TH1F* hXl812_121 = new TH1F("hXtal_812_121","hXtal_812_121",kRuns,0,kRuns);
  TH1F* hXl812_121p = new TH1F("hXtal_812_121p","Xtal_812_121",100,10.87,10.97);
  */

  TH1F* hChi2NDF_12 = new TH1F("hChi2NDF_12","hChi2NDF 12",100,0,10.);
  TH1F* hChi2NDF_6 = new TH1F("hChi2NDF_6","hChi2NDF 6",100,0,10.);
  TH1F* hChi2NDF_1 = new TH1F("hChi2NDF_1","hChi2NDF 1",100,0,10.);
  TH1F* hChi2NDF_12_OK = new TH1F("hChi2NDF_12_OK","hChi2NDF 12 after cut",100,0,10.);
  TH1F* hChi2NDF_6_OK = new TH1F("hChi2NDF_6_OK","hChi2NDF 6 after cut",100,0,10.);
  TH1F* hChi2NDF_1_OK = new TH1F("hChi2NDF_1_OK","hChi2NDF 1 after cut",100,0,10.);

  string  rootfile;
  Int_t usedrun126[kChannels];
  Int_t usedrun121[kChannels];
  Double_t Rat126[kChannels][kRuns];
  Double_t Rat121[kChannels][kRuns];
  for (int iChannel = 0; iChannel < kChannels; iChannel++) {
    usedrun126[iChannel] = 0;
    usedrun121[iChannel] = 0;
  }
  for (int irun = 0; irun < kRuns; irun++) {
    //    rootfile = Form("/home/fay/TestPulse_000%d_SM06.root",runn[irun]);
    rootfile = Form("/home/fay/TestPulse_000%d.root",runn[irun]);
    cout << " Opening file " << irun << " " << rootfile << endl;
    TFile file(rootfile.c_str());

    gXtalAmplitude_12 = (TGraphErrors*) file.Get("gXtalAmplitude_12");
    gXtalAmplitude_6 = (TGraphErrors*) file.Get("gXtalAmplitude_6");
    gXtalAmplitude_1 = (TGraphErrors*) file.Get("gXtalAmplitude_1");

    gXtalChi2_12 = (TGraphErrors*) file.Get("gXtalChi2_12");
    gXtalChi2_6 = (TGraphErrors*) file.Get("gXtalChi2_6");
    gXtalChi2_1 = (TGraphErrors*) file.Get("gXtalChi2_1");

    gXtalEntry_12 = (TGraph*) file.Get("gXtalEntry_12");
    gXtalEntry_6 = (TGraph*) file.Get("gXtalEntry_6");
    gXtalEntry_1 = (TGraph*) file.Get("gXtalEntry_1");
    
    g_Chi2overNdf_12 = (TGraph*) file.Get("gChi2overNdf_12");
    g_Chi2overNdf_6 = (TGraph*) file.Get("gChi2overNdf_6");
    g_Chi2overNdf_1 = (TGraph*) file.Get("gChi2overNdf_1");
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
    /*
    if(runn[irun] == 16267) {
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
    }
    */

    Double_t x12,y12,x6,y6,x1,y1;
    Double_t rms12,rms6,rms1;
    Double_t chi2_12,chi2_6,chi2_1;
    Double_t entry_12,entry_6,entry_1;
    Double_t Chi2NDF_12,Chi2NDF_6,Chi2NDF_1;
    Int_t Xtalmiss[kGains] = {0,0,0};
    for (int iChannel = 0; iChannel < kChannels; iChannel++) {
      y12 = y6 = y1 = 0;
      gXtalAmplitude_12->GetPoint(iChannel+1,x12,y12);
      gXtalAmplitude_6->GetPoint(iChannel+1,x6,y6);
      gXtalAmplitude_1->GetPoint(iChannel+1,x1,y1);

      rms12 = gXtalAmplitude_12->GetErrorY(iChannel+1);
      rms6 = gXtalAmplitude_6->GetErrorY(iChannel+1);
      rms1 = gXtalAmplitude_1->GetErrorY(iChannel+1);

      gXtalChi2_12->GetPoint(iChannel+1,x12,chi2_12);
      gXtalChi2_6->GetPoint(iChannel+1,x12,chi2_6);
      gXtalChi2_1->GetPoint(iChannel+1,x12,chi2_1);

      gXtalEntry_12->GetPoint(iChannel+1,x12,entry_12);
      gXtalEntry_6->GetPoint(iChannel+1,x12,entry_6);
      gXtalEntry_1->GetPoint(iChannel+1,x12,entry_1);

      g_Chi2overNdf_12->GetPoint(iChannel+1,x12,Chi2NDF_12);
      g_Chi2overNdf_6->GetPoint(iChannel+1,x12,Chi2NDF_6);
      g_Chi2overNdf_1->GetPoint(iChannel+1,x12,Chi2NDF_1);
      hChi2NDF_12->Fill(Chi2NDF_12);
      hChi2NDF_6->Fill(Chi2NDF_6);
      hChi2NDF_1->Fill(Chi2NDF_1);
      if(entry_12 > 100) {
	hChi2NDF_12_OK->Fill(Chi2NDF_12);
	hChi2NDF_6_OK->Fill(Chi2NDF_6);
	hChi2NDF_1_OK->Fill(Chi2NDF_1);

	hRMS_12->Fill(rms12);
	hRMS_6->Fill(rms6);
	hRMS_1->Fill(rms1);
      }

      if(y12 == 0) {
	Xtalmiss[0] ++;
	//	if(iChannel < 1675) cout << iChannel << " y12 = 0 " << endl;
      }
      else if(entry_12 > 100 && rms12 < 0.001) {
	hXtal[iChannel][0]->Fill(y12);
	if(entry_6 > 100 && y6 != 0 && rms6 < 0.001) {
	  //	  Double_t Rat126 = y12/y6;
	  //	  hR126[iChannel]->Fill(Rat126);
	  //	  hR126p[iChannel]->Fill(usedrun126[iChannel],Rat126);
	  Rat126[iChannel][usedrun126[iChannel]] = y12/y6;
	  if(usedrun126[iChannel] > 0 &&
	     fabs(Rat126[iChannel][usedrun126[iChannel]] - Rat126[iChannel][usedrun126[iChannel] - 1]) > 0.001)
	    cout << " channel " << iChannel + 1 << " run " << runn[irun] << " used " << usedrun126[iChannel]
		 << " ratio 12/6 " << Rat126[iChannel][usedrun126[iChannel]]
		 << " previous " << Rat126[iChannel][usedrun126[iChannel] - 1] << endl;
	  hR126[iChannel]->Fill(Rat126[iChannel][usedrun126[iChannel]]);
	  //	  hR126p[iChannel]->Fill(irun,Rat126);
	  hR126p[iChannel]->Fill(usedrun126[iChannel],Rat126[iChannel][usedrun126[iChannel]]);
	  usedrun126[iChannel]++;
	  /*
	  if(iChannel == 300) {
	    hXl300_126->Fill(irun,Rat126);
	    hXl300_126p->Fill(Rat126);
	    if(Rat126>1.9455) cout << "ch 300 run " << runn[irun] << " 12/6 " << Rat126
				   << " 12 " << y12 << " 6 " << y6
				   << " chi2_12 " << chi2_12 << " chi2_6 " << chi2_6 
				   << " Chi2NDF_12 " << Chi2NDF_12 << " Chi2NDF_6 " << Chi2NDF_6
				   << " RMS 12 " << rms12 << " RMS 6 " << rms6
				   << " entry_12 " << entry_12 << " entry_6 " << entry_6 << endl;
	  }
	  if(iChannel == 873) {
	    hXl873_126->Fill(irun,Rat126);
	    hXl873_126p->Fill(Rat126);
	    if(Rat126>1.944 || Rat126<1.940) cout << "ch 873 run " << runn[irun] << " 12/6 " << Rat126
				   << " 12 " << y12 << " 6 " << y6
				   << " chi2_12 " << chi2_12 << " chi2_6 " << chi2_6
				   << " Chi2NDF_12 " << Chi2NDF_12 << " Chi2NDF_6 " << Chi2NDF_6
				   << " RMS 12 " << rms12 << " RMS 6 " << rms6
				   << " entry_12 " << entry_12 << " entry_6 " << entry_6 << endl;
	  }
	  if(iChannel == 811) {
	    hXl811_126->Fill(irun,Rat126);
	    hXl811_126p->Fill(Rat126);
	    if(Rat126>1.997 || Rat126<1.993) cout << "ch 811 run " << runn[irun] << " 12/6 " << Rat126
				   << " 12 " << y12 << " 6 " << y6
				   << " chi2_12 " << chi2_12 << " chi2_6 " << chi2_6
				   << " Chi2NDF_12 " << Chi2NDF_12 << " Chi2NDF_6 " << Chi2NDF_6
				   << " RMS 12 " << rms12 << " RMS 6 " << rms6
				   << " entry_12 " << entry_12 << " entry_6 " << entry_6 << endl;
	  }
	  if(iChannel == 893) {
	    hXl893_126->Fill(irun,Rat126);
	    hXl893_126p->Fill(Rat126);
	    if(Rat126>1.949 || Rat126<1.946) cout << "ch 893 run " << runn[irun] << " 12/6 " << Rat126
				   << " 12 " << y12 << " 6 " << y6
				   << " chi2_12 " << chi2_12 << " chi2_6 " << chi2_6
				   << " Chi2NDF_12 " << Chi2NDF_12 << " Chi2NDF_6 " << Chi2NDF_6
				   << " RMS 12 " << rms12 << " RMS 6 " << rms6
				   << " entry_12 " << entry_12 << " entry_6 " << entry_6 << endl;
	  }
	  */
	} // test on gain 6 data correctness
	if(entry_1 > 100 && y1 != 0 && rms1 < 0.001) {
	  //	  Double_t Rat121 = y12/y1;
	  //	  hR121[iChannel]->Fill(Rat121);
	  //	  hR121p[iChannel]->Fill(usedrun121[iChannel],Rat121);
	  Rat121[iChannel][usedrun121[iChannel]] = y12/y1;
	  if(usedrun121[iChannel] > 0 &&
	     fabs(Rat121[iChannel][usedrun121[iChannel]] - Rat121[iChannel][usedrun121[iChannel] - 1]) > 0.02)
	    cout << " channel " << iChannel + 1 << " run " << runn[irun] << " used " << usedrun121[iChannel]
		 << " ratio 12/1 " << Rat121[iChannel][usedrun121[iChannel]]
		 << " previous " << Rat121[iChannel][usedrun121[iChannel] - 1] << endl;
	  hR121[iChannel]->Fill(Rat121[iChannel][usedrun121[iChannel]]);
	  //	  hR121p[iChannel]->Fill(irun,Rat121);
	  hR121p[iChannel]->Fill(usedrun121[iChannel],Rat121[iChannel][usedrun121[iChannel]]);
	  usedrun121[iChannel]++;
	  /*
	  if(iChannel == 300) {
	    hXl300_121->Fill(irun,Rat121);
	    hXl300_121p->Fill(Rat121);
	    if(Rat121 > 11.025 || Rat121 < 11.015) cout << "ch 300 run " << runn[irun] << " 12/1 " << Rat121
				   << " 12 " << y12 << " 1 " << y1
				   << " chi2_12 " << chi2_12 << " chi2_1 " << chi2_1
				   << " Chi2NDF_12 " << Chi2NDF_12 << " Chi2NDF_1 " << Chi2NDF_1
				   << " RMS 12 " << rms12 << " RMS 1 " << rms1
				   << " entry_12 " << entry_12 << " entry_1 " << entry_1 << endl;
	  }
	  if(iChannel == 873) {
	    hXl873_121->Fill(irun,Rat121);
	    hXl873_121p->Fill(Rat121);
	    if(Rat121 > 11.01 || Rat121 < 10.98) cout << "ch 873 run " << runn[irun] << " 12/1 " << Rat121
				   << " 12 " << y12 << " 1 " << y1
				   << " chi2_12 " << chi2_12 << " chi2_1 " << chi2_1
				   << " Chi2NDF_12 " << Chi2NDF_12 << " Chi2NDF_1 " << Chi2NDF_1
				   << " RMS 12 " << rms12 << " RMS 1 " << rms1
				   << " entry_12 " << entry_12 << " entry_1 " << entry_1 << endl;
	  }
	  if(iChannel == 812) {
	    hXl812_121->Fill(irun,Rat121);
	    hXl812_121p->Fill(Rat121);
	    if(Rat121 > 10.96 || Rat121 < 10.87) cout << "ch 812 run " << runn[irun] << " 12/1 " << Rat121
				   << " 12 " << y12 << " 1 " << y1
				   << " chi2_12 " << chi2_12 << " chi2_1 " << chi2_1
				   << " Chi2NDF_12 " << Chi2NDF_12 << " Chi2NDF_1 " << Chi2NDF_1
				   << " RMS 12 " << rms12 << " RMS 1 " << rms1
				   << " entry_12 " << entry_12 << " entry_1 " << entry_1 << endl;
	  }
	  if(iChannel == 813) {
	    hXl813_121->Fill(irun,Rat121);
	    hXl813_121p->Fill(Rat121);
	    if(Rat121 > 10.99 || Rat121 < 10.97) cout << "ch 813 run " << runn[irun] << " 12/1 " << Rat121
				   << " 12 " << y12 << " 1 " << y1
				   << " chi2_12 " << chi2_12 << " chi2_1 " << chi2_1
				   << " Chi2NDF_12 " << Chi2NDF_12 << " Chi2NDF_1 " << Chi2NDF_1
				   << " RMS 12 " << rms12 << " RMS 1 " << rms1
				   << " entry_12 " << entry_12 << " entry_1 " << entry_1 << endl;
	  }
	  */
	} // test on gain 1 data correctness
      } // test on gain 12 data correctness
      if(y6 == 0) {
	Xtalmiss[1] ++;
	//	if(iChannel < 1675) cout << iChannel << " y6 = 0 " << endl;
      }
      else if(entry_6 > 100 && rms6 < 0.001) {
	hXtal[iChannel][1]->Fill(y6);
      }
      if(y1 == 0) {
	Xtalmiss[2] ++;
	//	if(iChannel < 1675) cout << iChannel << " y1 = 0 " << endl;
      }
      else if(entry_1 > 100 && rms1 < 0.001) {
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
      //      if(Xtalmiss[gainId] > 25) 
      //	cout << "gain " << gainValues[gainId] << " missing " << Xtalmiss[gainId]
      //	     << endl;
    }
  } // end loop on runs
  /*
  TCanvas *c1 = new TCanvas("c1","c1",800,1000);
  c1->Divide(1,3);
  c1->cd(1);
  hChi2NDF_12->Draw();
  c1->SetLogy(1);
  c1->cd(2);
  hChi2NDF_6->Draw();
  c1->cd(3);
  hChi2NDF_1->Draw();
  c1->Update();
  c1->Print("TPcheck.ps(");
  cout << "Chi2NDF "<< endl;
  int i;
  cin >> i;

  c1->cd(1);
  hChi2NDF_12_OK->Draw();
  c1->cd(2);
  hChi2NDF_6_OK->Draw();
  c1->cd(3);
  hChi2NDF_1_OK->Draw();
  c1->Update();
  c1->Print("TPcheck.ps");
  cout << "Chi2NDF_OK "<< endl;
  int i;
  cin >> i;

  c1->cd(1);
  hRMS_12->Draw();
  c1->cd(2);
  hRMS_6->Draw();
  c1->cd(3);
  hRMS_1->Draw();
  c1->Update();
  c1->Print("TPcheck.ps");
  cout << "RMS "<< endl;
  int i;
  cin >> i;
  c1->cd(0);
  c1->Clear();

  c1->Divide(2,2);
  c1->cd(1);
  hXl300_126->SetMaximum(1.95);
  hXl300_126->SetMinimum(1.94);
  hXl300_126->Draw();
  c1->cd(2);
  hXl300_126p->Draw();
  c1->cd(3);
  hXl300_121->SetMaximum(11.05);
  hXl300_121->SetMinimum(10.95);
  hXl300_121->Draw();
  c1->cd(4);
  hXl300_121p->Draw();
  c1->Update();
  c1->Print("TPcheck.ps");
  cout << "channel 300 "<< endl;
  int i;
  cin >> i;
  c1->cd(1);
  hXl873_126->SetMaximum(1.95);
  hXl873_126->SetMinimum(1.94);
  hXl873_126->Draw();
  c1->cd(2);
  hXl873_126p->Draw();
  c1->cd(3);
  hXl873_121->Draw();
  hXl873_121->SetMaximum(11.05);
  hXl873_121->SetMinimum(10.95);
  c1->cd(4);
  hXl873_121p->Draw();
  c1->Update();
  c1->Print("TPcheck.ps");
  cout << "channel 873 "<< endl;
  int i;
  cin >> i;
  c1->cd(1);
  hXl811_126->SetMaximum(2.00);
  hXl811_126->SetMinimum(1.99);
  hXl811_126->Draw();
  c1->cd(2);
  hXl811_126p->Draw();
  c1->cd(3);
  hXl893_126->SetMaximum(1.95);
  hXl893_126->SetMinimum(1.94);
  hXl893_126->Draw();
  c1->cd(4);
  hXl893_126p->Draw();
  c1->Update();
  c1->Print("TPcheck.ps");
  cout << "channel 893 "<< endl;
  int i;
  cin >> i;
  c1->cd(1);
  hXl812_121->Draw();
  hXl812_121->SetMaximum(10.97);
  hXl812_121->SetMinimum(10.87);
  c1->cd(2);
  hXl812_121p->Draw();
  c1->cd(3);
  hXl813_121->Draw();
  hXl813_121->SetMaximum(11.05);
  hXl813_121->SetMinimum(10.95);
  c1->cd(4);
  hXl813_121p->Draw();
  c1->Update();
  c1->Print("TPcheck.ps");
  cout << "channel 813 "<< endl;
  int i;
  cin >> i;
  */

  TCanvas *c2 = new TCanvas("c2","c2",1000,500);
  TH1F** hRMS = new TH1F*[kGains];
  for (int gainId = 0; gainId < kGains; gainId++) {
    hRMS[gainId] = new TH1F(Form("RMS_%d",gainValues[gainId]),
			    Form("RMS gain %d",gainValues[gainId]),1700,0,1700);
    for (int iChannel = 0; iChannel < kChannels; iChannel++) {
      Double_t rms = hXtal[iChannel][gainId]->GetRMS();
      hRMS[gainId]->Fill(iChannel,rms);
      /*
      if(iChannel%100 == 0) {
	Double_t mean = hXtal[iChannel][gainId]->GetMean();
 	hXtal[iChannel][gainId]->Draw();
	c2->Update();
	cout << "channel " << iChannel << " gain " << gainValues[gainId] 
	     << " mean " << mean << " rms " << rms << endl;
      }
      */
      hXtal[iChannel][gainId]->Delete();
    }
  }
  hRMS126 = new TH1F("RMS126","RMS ratio12 6",1700,0,1700);
  hRMS121 = new TH1F("RMS121","RMS ratio12 1",1700,0,1700);
  for (int iChannel = 0; iChannel < kChannels; iChannel++) {
    Int_t nbinx =  hR126[iChannel]->GetNbinsX();
    Double_t under = hR126[iChannel]->GetBinContent(0);
    if(under != 0.) cout << "12/6 channel " << iChannel << " underflow " << under << endl;
    Double_t over = hR126[iChannel]->GetBinContent(nbinx+1);
    if(over != 0.) cout << "12/6 channel " << iChannel << " overflow " << over << endl;
    Double_t rms = hR126[iChannel]->GetRMS();
    hRMS126->Fill(iChannel,rms);
    if(rms>0.002) cout << " channel " << iChannel << " rms12/6 " << rms << endl;
    nbinx =  hR121[iChannel]->GetNbinsX();
    under = hR121[iChannel]->GetBinContent(0);
    if(under != 0.) cout << "12/1 channel " << iChannel << " underflow " << under << endl;
    over = hR121[iChannel]->GetBinContent(nbinx+1);
    if(over != 0.) cout << "12/1 channel " << iChannel << " overflow " << over << endl;
    rms = hR121[iChannel]->GetRMS();
    hRMS121->Fill(iChannel,rms);
    if(rms>0.02) cout << "channel " << iChannel << " rms12/1 " << rms << endl;
  }

  hRMS[0]->Draw();
  hRMS[0]->GetXaxis()->SetTitle("Channels");
  hRMS[1]->SetLineColor(2);
  hRMS[1]->Draw("SAME");
  hRMS[2]->SetLineColor(4);
  hRMS[2]->Draw("SAME");
  c2->Update();
  c2->Print("TPcheck.ps(");
  cout << "ratio 12/6 "<< endl;
  int i;
  cin >> i;
  hRMS126->Draw();
  c2->Update();
  c2->Print("TPcheck.ps");
  cout << "ratio 12/1 "<< endl;
  cin >> i;
  hRMS121->Draw();
  c2->Update();
  c2->Print("TPcheck.ps)");

  // Output root file
  TFile* fout = new TFile("TPcheck.root","recreate");
  fout->cd();
  hRMS126->Write();
  hRMS121->Write();
  for (int iChannel = 0; iChannel < kChannels; iChannel++) {
    hR126[iChannel]->Write(); 
    hR126[iChannel]->Delete();
    hR121[iChannel]->Write(); 
    hR121[iChannel]->Delete();
    hR126p[iChannel]->SetAxisRange(0.,usedrun126[iChannel] - 1,"X");
    hR126p[iChannel]->Write();
    hR126p[iChannel]->Delete();
    hR121p[iChannel]->SetAxisRange(0.,usedrun121[iChannel] - 1,"X");
    hR121p[iChannel]->Write();
    hR121p[iChannel]->Delete();
  }
  fout->Close();

  //  gROOT->ProcessLine(".q");
}
