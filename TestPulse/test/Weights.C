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

using namespace std;

Int_t Weights() {
  TFile* fin = new TFile("./GainRatio_116514_1.root");
  double Sample[10];
  for(int iSample = 0; iSample < 10; iSample++) {
    Sample[iSample] = pXtalPulseEE_0_0_1->GetBinContent(iSample+1);
    cout << " sample " << iSample << " Amp " << Sample[iSample] << endl;
  }
  double baseline=0;
  double f=0;
  double sumf=0;
  double sumf2=0;
  double weightA=0;
  for(int iSample=0; iSample < 3; iSample++) {
    baseline += Sample[iSample];
  }
  baseline /= 3.;
  cout << " Pedestal " << baseline << endl;

  for(int iSample = 4; iSample < 9; iSample++) {
    f = Sample[iSample] - baseline;
    sumf += f;
    sumf2 += f*f;
  }
  cout << " sumf " << sumf << " sumf2 " << sumf2 << endl;

  double sumweightA=0;
  double sumweightF=0;
  if(sumf == 0) {
    cout << " sumf = 0" << endl;
  }
  else {
    for(int iSample=0; iSample < 10; iSample++) {
      if(iSample == 3 || iSample == 9)
	weightA = 0.;
      else {
	f = 0;
	if(iSample > 3) f = Sample[iSample] - baseline;
	weightA = (8 * f - sumf) / (8 * sumf2 - sumf*sumf);
	sumweightA+=weightA;
	sumweightF+=weightA * f;
      }
      cout << weightA << " ";
    } // loop over iSample
    cout << endl;
    cout << " Weight 3 + 5  sumweightA " << sumweightA 
	 << " sumweightF " << sumweightF << endl;
    sumweightF = 0.;
    for(int iSample=0; iSample < 10; iSample++) {
      if(iSample <= 3 || iSample == 9)
	weightA = 0.;
      else {
	f = Sample[iSample] - baseline;	 	 
	weightA = f / sumf2;
	sumweightF+=weightA * f;
      }
      cout << weightA << " ";
    } // loop over iSample
    cout << endl;
    cout << " Weight 5 sumweightF " << sumweightF << endl;
  }  //  check sumf == 0
}
