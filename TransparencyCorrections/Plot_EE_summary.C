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

Int_t PlotTransparency() {
  TFile* fin = new TFile("./TranspVar_EE_summary.root");
  fin->cd(); 
 
  TProfile** hEERing = new TProfile*[22];
  TProfile** hEERingWeek = new TProfile*[22];
  TProfile** hEERingTwoWeek = new TProfile*[22];

  hEERing[0] = (TProfile*)EERing_0;
  hEERing[1] = (TProfile*)EERing_1;
  hEERing[2] = (TProfile*)EERing_2;
  hEERing[3] = (TProfile*)EERing_3;
  hEERing[4] = (TProfile*)EERing_4;
  hEERing[5] = (TProfile*)EERing_5;
  hEERing[6] = (TProfile*)EERing_6;
  hEERing[7] = (TProfile*)EERing_7;
  hEERing[8] = (TProfile*)EERing_8;
  hEERing[9] = (TProfile*)EERing_9;
  hEERing[10] = (TProfile*)EERing_10;
  hEERing[11] = (TProfile*)EERing_11;
  hEERing[12] = (TProfile*)EERing_12;
  hEERing[13] = (TProfile*)EERing_13;
  hEERing[14] = (TProfile*)EERing_14;
  hEERing[15] = (TProfile*)EERing_15;
  hEERing[16] = (TProfile*)EERing_16;
  hEERing[17] = (TProfile*)EERing_17;
  hEERing[18] = (TProfile*)EERing_18;
  hEERing[19] = (TProfile*)EERing_19;
  hEERing[20] = (TProfile*)EERing_20;
  hEERing[21] = (TProfile*)EERing_21;

  hEERingWeek[0] = (TProfile*)EERingWeek_0;
  hEERingWeek[1] = (TProfile*)EERingWeek_1;
  hEERingWeek[2] = (TProfile*)EERingWeek_2;
  hEERingWeek[3] = (TProfile*)EERingWeek_3;
  hEERingWeek[4] = (TProfile*)EERingWeek_4;
  hEERingWeek[5] = (TProfile*)EERingWeek_5;
  hEERingWeek[6] = (TProfile*)EERingWeek_6;
  hEERingWeek[7] = (TProfile*)EERingWeek_7;
  hEERingWeek[8] = (TProfile*)EERingWeek_8;
  hEERingWeek[9] = (TProfile*)EERingWeek_9;
  hEERingWeek[10] = (TProfile*)EERingWeek_10;
  hEERingWeek[11] = (TProfile*)EERingWeek_11;
  hEERingWeek[12] = (TProfile*)EERingWeek_12;
  hEERingWeek[13] = (TProfile*)EERingWeek_13;
  hEERingWeek[14] = (TProfile*)EERingWeek_14;
  hEERingWeek[15] = (TProfile*)EERingWeek_15;
  hEERingWeek[16] = (TProfile*)EERingWeek_16;
  hEERingWeek[17] = (TProfile*)EERingWeek_17;
  hEERingWeek[18] = (TProfile*)EERingWeek_18;
  hEERingWeek[19] = (TProfile*)EERingWeek_19;
  hEERingWeek[20] = (TProfile*)EERingWeek_20;
  hEERingWeek[21] = (TProfile*)EERingWeek_21;
  /*
  hEERingTwoWeek[0] = (TProfile*)EERingTwoWeek_0;
  hEERingTwoWeek[1] = (TProfile*)EERingTwoWeek_1;
  hEERingTwoWeek[2] = (TProfile*)EERingTwoWeek_2;
  hEERingTwoWeek[3] = (TProfile*)EERingTwoWeek_3;
  hEERingTwoWeek[4] = (TProfile*)EERingTwoWeek_4;
  hEERingTwoWeek[5] = (TProfile*)EERingTwoWeek_5;
  hEERingTwoWeek[6] = (TProfile*)EERingTwoWeek_6;
  hEERingTwoWeek[7] = (TProfile*)EERingTwoWeek_7;
  hEERingTwoWeek[8] = (TProfile*)EERingTwoWeek_8;
  hEERingTwoWeek[9] = (TProfile*)EERingTwoWeek_9;
  hEERingTwoWeek[10] = (TProfile*)EERingTwoWeek_10;
  hEERingTwoWeek[11] = (TProfile*)EERingTwoWeek_11;
  hEERingTwoWeek[12] = (TProfile*)EERingTwoWeek_12;
  hEERingTwoWeek[13] = (TProfile*)EERingTwoWeek_13;
  hEERingTwoWeek[14] = (TProfile*)EERingTwoWeek_14;
  hEERingTwoWeek[15] = (TProfile*)EERingTwoWeek_15;
  hEERingTwoWeek[16] = (TProfile*)EERingTwoWeek_16;
  hEERingTwoWeek[17] = (TProfile*)EERingTwoWeek_17;
  hEERingTwoWeek[18] = (TProfile*)EERingTwoWeek_18;
  hEERingTwoWeek[19] = (TProfile*)EERingTwoWeek_19;
  hEERingTwoWeek[20] = (TProfile*)EERingTwoWeek_20;
  hEERingTwoWeek[21] = (TProfile*)EERingTwoWeek_21;
  */

  ifstream fFill;
  fFill.open("./FillReport.txt",ios::in);
  if(!fFill) {
    cout << "Error: file FillReport could not be opened" << endl;
    exit(1);
  }
  cout << " file FillReport opened" << endl;
  std::string line;
  getline (fFill, line);  // skip header line
  float InstLumi[350],IntLumi;
  std::string bdate, btime, edate, etime, type, scheme;
  int dd, mo, yy, hh, mn, ss;
  struct tm when = {0};
  time_t converted;
  long int fill_beg[350], fill_end[350];
  int fill_it = 0, fill_nb[350];
  while (!fFill.eof()) {
    fFill >> fill_nb[fill_it] >> InstLumi[fill_it] >> IntLumi >> bdate >> btime >> edate >> etime >> type >> scheme;

    sscanf(bdate.c_str(), "%d.%d.%d", &yy, &mo, &dd);
    sscanf(btime.c_str(), "%d:%d:%d", &hh, &mn, &ss);
    when.tm_year = yy - 1900;
    when.tm_mon = mo - 1;
    when.tm_mday = dd;
    when.tm_hour = hh + 1;
    when.tm_min = mn;
    when.tm_sec = ss;
    converted = mktime(&when);
    cout << "Time : " << converted << endl;
    fill_beg[fill_it] = static_cast<int> (converted);

    sscanf(edate.c_str(), "%d.%d.%d", &yy, &mo, &dd);
    sscanf(etime.c_str(), "%d:%d:%d", &hh, &mn, &ss);
    when.tm_year = yy - 1900;
    when.tm_mon = mo - 1;
    when.tm_mday = dd;
    when.tm_hour = hh + 1;
    when.tm_min = mn;
    when.tm_sec = ss;
    converted = mktime(&when);
    fill_end[fill_it] = static_cast<int> (converted);
    // check
    char buf[256];
    strftime(buf, sizeof(buf), "%F %R:%S", gmtime(&fill_beg[fill_it]));
    cout << fill_nb[fill_it] << " " << bdate << " " << btime << " " << edate << " " << etime << " ";
    printf("begins: %li (%s ", fill_beg[fill_it], buf);
    strftime(buf, sizeof(buf), "%F %R:%S", gmtime(&fill_end[fill_it]));
    printf("ends: %li (%s\n", fill_end[fill_it], buf);
    fill_it++;
  }
  cout << " nb of fills " << fill_it << endl;

  fFill.close();

  gStyle->SetOptStat(0);      
  //-----------------------------------------------------------------------------------------------------
  //  TDatime T1(2012,04,02,00, 00, 00);      // cf time1st
  //  gStyle->SetTimeOffset(T1.Convert());
  //------------------------------------------------------------------------------------------------------ 
  TCanvas cCan("cCan", "Transparency variation");
  //  cCan.Divide(3,2);
  cCan.Divide(2,3);
  cCan.SetCanvasSize(630,885);
      
  bool TwoWeek = false;
  //  bool TwoWeek = true;
   int cnt = 0, view = 0;
  for (Int_t ring = 0; ring < 22; ring++) {
    double min = 1.;
    for(int bin = 0; bin < 8000; bin++) {
      double x = hEERing[ring]->GetBinContent(bin);
      if(x > 0.) {
	double err = hEERing[ring]->GetBinError(bin);
	double val = x - err;
	if(val < min) min = val;
      }
    }
    int imin = min * 100;
    min = imin / 100.;
    hEERing[ring]->SetMinimum(min);
    hEERing[ring]->GetXaxis()->SetTimeDisplay(1);
    hEERing[ring]->GetXaxis()->SetTimeFormat("%d\/%m\/%y%F1970-01-01 00:00:00");	
    cout << " Ring " << ring << " Minimal value " << min << endl;
    cnt++;
    cCan.cd(cnt);
    hEERing[ring]->Draw();
  if(!TwoWeek) {
      hEERingWeek[ring]->Draw("PSAME");
      hEERingWeek[ring]->SetMarkerStyle(20);
      hEERingWeek[ring]->SetMarkerSize(0.5);
      hEERingWeek[ring]->SetMarkerColor(kRed);
      hEERingWeek[ring]->SetLineColor(kRed);
      if(cnt == 6 || (view%2 == 1 && cnt == 5)) {
	if(view < 2)
	  cCan.Print(Form("EE-_Transp_%i.gif",view));  // print canvas to gif file
	else
	  cCan.Print(Form("EE+_Transp_%i.gif",view%2));
	cCan.Clear();
	//    cCan.Divide(3,2);
	cCan.Divide(2,3);
	view++;
	cnt = 0;
        }
    }
  /* 
    else {
      hEERingTwoWeek[ring]->Draw("PSAME");
      hEERingTwoWeek[ring]->SetMarkerStyle(20);
      hEERingTwoWeek[ring]->SetMarkerSize(0.5);
      hEERingTwoWeek[ring]->SetMarkerColor(kRed);
      hEERingTwoWeek[ring]->SetLineColor(kRed);
      if(cnt == 6 || (view%2 == 1 && cnt == 5)) {
	if(view < 2)
	  cCan.Print(Form("EE-_TwoWeekTransp_%i.gif",view));  // print canvas to gif file
	else
	  cCan.Print(Form("EE+_TwoWeekTransp_%i.gif",view%2));
	cCan.Clear();
	//    cCan.Divide(3,2);
	cCan.Divide(2,3);
	view++;
	cnt = 0;
      }
    }
    */
  }  //  loop over rings
  // draw a zoom on one ring histo
    /******************************/
   /* other ring can be choosen  */
  /******************************/
  Int_t choosenRing = 8;
  // EE-18 to EE-28 equals  0 to 10
  // EE+18 to EE+28 equals  11 to 21
  TCanvas cCan2("cCan2", "Transparency variation");
  cCan2.cd(1);
  TAxis *xAxis = hEERing[choosenRing]->GetXaxis();
  Double_t x1 = xAxis->GetBinLowEdge(xAxis->GetFirst());
  Double_t x2 = xAxis->GetBinUpEdge(xAxis->GetLast());
  Int_t binNb =  xAxis->GetNbins();
  Double_t bin_width = (x2 - x1) / (Double_t)binNb;
  cout << " low edge " << x1 <<  " up edge " << x2  << " Nb of bins " << binNb 
       << " bin_width " << bin_width << endl;
  hEERing[choosenRing]->GetXaxis()->SetTimeDisplay(1);
  hEERing[choosenRing]->GetXaxis()->SetTimeFormat("%d\/%m\/%y%F1970-01-01 00:00:00");	
  /******************************************************************/
   /*  other period can be choosen (keep ~ 1 week between min max  */
  /****************************************************************/
  // long int xmin = 1351856900;   // from 3 November 2012
  // long int xmin = 1345680000;
   long int xmin = 1337856900;   // from 25 May 2012
  //  long int xmax = 1343260800;
  // long int twoweeks = 1209600;  // seconds in two weeks
  long int week = 604800;  // seconds in a week
  //long int half_week = 302400;  // seconds in 1/2 week
  // long int xmax = xmin + twoweeks;
   long int xmax = xmin + week;
  //long int xmax = xmin + half_week;
  int bin1st = (xmin - x1)/bin_width;
  int binlast = (xmax - x1)/bin_width;
  Double_t Y_offset = 1.5;
  Double_t X_offset = 0.015;
  cout << " bin1st " << bin1st << " binlast " << binlast   << endl;
  hEERing[choosenRing]->Draw();
  hEERing[choosenRing]->GetYaxis()->SetTitle("relative transparency (1.0 : Jan 2011)");
  hEERing[choosenRing]->GetYaxis()->SetTitleOffset(Y_offset);
  xAxis->SetRangeUser(xmin, xmax);   
  x1 = xAxis->GetBinLowEdge(xAxis->GetFirst());
  x2 = xAxis->GetBinUpEdge(xAxis->GetLast());
  xAxis->SetLabelOffset(X_offset);  
  cout << " low edge " << x1 <<  " up edge " << x2  << endl;
  Double_t c1st, e1st, ymax = 0, ymin = 1.;
  for(int ib = bin1st; ib < binlast + 1; ib++) {
    Double_t c1st = hEERing[choosenRing]->GetBinContent(ib);
    Double_t e1st = hEERing[choosenRing]->GetBinError(ib);
    //    cout << " c1st " << c1st << " e1st " << e1st << endl;
    if(c1st != 0) {
      Double_t y1 = c1st + e1st;
      if(ymax < y1) ymax = y1;
      y1 = c1st - e1st;
      if(ymin > y1) ymin = y1;
    }
  }
  cout << " low y " << ymin <<  " up y " << ymax  << endl;
  Int_t iy = ymin * 100 - 1;
  ymin = (Double_t)iy / 100.;
  iy = ymax * 100 + 1;
  ymax = (Double_t)iy / 100.;
  cout << " low y " << ymin <<  " up y " << ymax  << endl;
  hEERing[choosenRing]->SetMinimum(ymin);
  hEERing[choosenRing]->SetMaximum(ymax);

  hEERingWeek[choosenRing]->Draw("SAME");
  hEERingWeek[choosenRing]->GetXaxis()->SetRangeUser(xmin, xmax);  
  hEERingWeek[choosenRing]->SetMarkerStyle(20);
  hEERingWeek[choosenRing]->SetMarkerSize(0.5);
  hEERingWeek[choosenRing]->SetMarkerColor(kRed);
  hEERingWeek[choosenRing]->SetLineColor(kRed);
  int toggle = 0;
  TLatex lum;
  for(int if = 0; if < fill_it; if++) {
    //	TBox* b = new TBox(fill_beg[if], min, fill_end[if], 1.0);
    //	b->SetFillStyle(4020);
    //	b->SetFillColor(141);
    //	b->Draw();
    if(fill_beg[if] > xmin &&fill_beg[if] < xmax) {
      TBox* b = new TBox(fill_beg[if], ymin + (ymax - ymin)/20, fill_end[if], ymax - (ymax - ymin)/20);
      b->SetFillStyle(3005);
      b->SetFillColor(kGreen - 7);
      b->Draw();
      TLine* lb = new TLine(fill_beg[if], ymin + (ymax - ymin)/20, fill_beg[if], ymax - (ymax - ymin)/20);
      lb->SetLineColor(kGreen);
      lb->Draw();
      Double_t posy = ymin + (ymax - ymin)/10;
      if(toggle%2 == 1) posy += 0.005;
      Int_t lumI = InstLumi[if]/10.;
      Double_t val = (Double_t)lumI / 100;
      if(InstLumi[if] < 10.) {
	lumI = InstLumi[if];
	val = (Double_t)lumI / 1000;
      }
      else if(InstLumi[if] > 10000.) {
	lumI = InstLumi[if]/100;
	val = (Double_t)lumI / 10;
      }
      stringstream sts;
      sts << val;
      string test = sts.str();
      lum.DrawLatex(fill_beg[if], posy, test.c_str()); 
      lum.SetTextColor(kGreen + 2);
      toggle++;
    }
    lum.DrawLatex(xmax - (xmax -xmin) / 3., ymax - (ymax -ymin)/8., "Inst Lumi (/nb/s)"); 
    lum.SetTextColor(kGreen + 2);
    /*
    if(fill_end[if] > xmin &&fill_end[if] < xmax) {
      TLine* le = new TLine(fill_end[if], ymin, fill_end[if], ymax);
      le->SetLineColor(kBlue);
      le->Draw();
    }
    */
  }
  if(choosenRing < 11)
    cCan2.Print(Form("EE-%i_Transp.gif", choosenRing + 18));  // print canvas to gif file
  else
    cCan2.Print(Form("EE+%i_Transp.gif", choosenRing + 7));
  cout << " Well done!!!" << endl;
  
  return 0;
}
