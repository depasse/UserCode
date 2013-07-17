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

Int_t WeightsCh() {
  int crystal, gain, DAC;
  double weightsEEA;
  ifstream fWeights;
  fWeights.open("GainRatioEE_116514.wgt");
  ofstream fWeightsNew;
  fWeightsNew.open("GainRatioEE_116514_new.wgt");
  while (!fWeights.eof()) {
    fWeights >> crystal >> gain >> DAC;
    if(crystal%1000 == 1) cout << crystal - 1 << endl;
    fWeightsNew << crystal - 1 << " " << gain << " " << DAC << endl;
    if(fWeights.eof()) break;
    for(int iSample=0; iSample < 10; iSample++) {
      fWeights >> weightsEEA;
      fWeightsNew << weightsEEA << " ";
    }
    fWeightsNew<< endl;
    for(int iSample=0; iSample < 10; iSample++) {
      fWeights >> weightsEEA;
      fWeightsNew << weightsEEA << " ";
    }
    fWeightsNew<< endl;
  }
  fWeights.close();
  fWeightsNew.close();
}
