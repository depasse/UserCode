// -*- C++ -*-
//
// Package:    TestPulseWeightUncalibRecHitProducer
// Class:      TestPulseWeightUncalibRecHitProducer
// 
/**\class TestPulseWeightUncalibRecHitProducer /TestPulse/src/TestPulseWeightUncalibRecHitProducer.cc

 Description: <one line class summary>

 Implementation:
     <Notes on implementation>
*/
//
// Original Author:  Jean-Laurent Agram
//         Created:  Fri Jun 16 10:09:22 CEST 2006
// $Id$
//
//


// system include files
// user include files
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/EDAnalyzer.h"

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"

#include "FWCore/ParameterSet/interface/ParameterSet.h"

#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include <DataFormats/EcalRawData/interface/EcalRawDataCollections.h>
#include "DataFormats/EcalDigi/interface/EcalDigiCollections.h"
#include "DataFormats/EcalDetId/interface/EBDetId.h"
#include "DataFormats/EcalRecHit/interface/EcalUncalibratedRecHit.h"
#include "DataFormats/EcalRecHit/interface/EcalRecHitCollections.h"


#include "TFile.h" 
#include "TH1.h"
#include "TH2.h"
#include "TProfile.h"
#include "TGraphErrors.h"
#include "TF1.h"

#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>

#include <vector>
#include <map>

using namespace edm;
using namespace std;

#include "Pit/TestPulse/interface/TestPulse.h"

//
// constants, enums and typedefs
//

//
// static data member definitions
//

//
// constructors and destructor
//
TestPulse::TestPulse(const edm::ParameterSet& iConfig) {  
  // parameter initialisation
  doPedes           = iConfig.getParameter<bool>("doPedes");   
  doWeights         = iConfig.getParameter<bool>("doWeights");   
  useWeights        = iConfig.getParameter<bool>("useWeights");
  bLogFile          = iConfig.getUntrackedParameter<bool>("LogFile",1);
  bSaveProfiles     = iConfig.getUntrackedParameter<bool>("SaveProfiles",1);
  iGainForWeights   = iConfig.getUntrackedParameter<int>("GainForWeights");
  digiProducer_     = iConfig.getParameter<string>("digiProducer");
  EBdigiCollection_ = iConfig.getParameter<string>("EBdigiCollection");
  EEdigiCollection_ = iConfig.getParameter<string>("EEdigiCollection");
  runnumber_        = iConfig.getUntrackedParameter<int>("runnumber",-1);
  ECALType_         = iConfig.getParameter<string>("ECALType");
  
  vector<int> listDefaults;
  listDefaults.push_back(-1);  
  maskedChannels_   = iConfig.getUntrackedParameter<vector<int> >("maskedChannels", listDefaults);
  int NbOfmaskedChannels =  maskedChannels_.size();
  cout << " Nb masked channels " << NbOfmaskedChannels << endl;
  for (vector<int>::iterator iter = maskedChannels_.begin(); iter != maskedChannels_.end(); ++iter)
    cout<< " : masked channel " << *(iter) << endl;
  gain0Channels_    = iConfig.getUntrackedParameter<vector<int> >("gain0Channels", listDefaults);
  cout << " Nb gain 0 channels " << gain0Channels_.size() << endl;
  for (vector<int>::iterator iter = gain0Channels_.begin(); iter != gain0Channels_.end(); ++iter)
    cout<< "  gain 0 channel " << *(iter) << endl;
   
  // Gain values
  int gainValues[kGains] = {12, 6, 1};
  vNGainMesures.resize(kGains,0);
   
  if(doWeights)  cout << "Will compute weights." << endl;
  if(useWeights) {
    cout << "Will compute pulse amplitude from weights computed with gain " 
	 << gainValues[iGainForWeights] << endl;
    bSaveProfiles = 0;  // do not write individual profiles at this stage
  }
  // Output files
  if(bLogFile) fTxtOutput.open(Form("TxtOutput%d.txt",runnumber_), ios::out); 
 
  // int initialisation
  iFirstGain=1;
  nSample=10;
   
  // weights
  for(int ixtal=0; ixtal < kEBChannels; ixtal++)
    for(int igain=0; igain < kGains; igain++)
      for(int iSample=0; iSample < nSample; iSample++) {
	weightsA[ixtal][igain][iSample] = 0;
	weightsA6[ixtal][igain][iSample] = 0;
      }
   
   
  // Histos
  //--------

  double Ampmin[kGains] = {0.96, 0.48, 0.06};
  double Ampmax[kGains] = {1.04, 0.56, 0.14};
  double AmpminDQM[kGains] = {2000.,  500., 100.};
  double AmpmaxDQM[kGains] = {2400., 1500., 600.};
  if(ECALType_ == "EB" || ECALType_ == "EA") {
    hPeak = new TH1F("hPeak", "Peak", 10, 0, 10);
    hXal325 = new TH1F("hXal325", "Xal325", 100, 2350, 2450.);
    hXal3252 = new TH1F("hXal3252", "Xal3252", 100, 2200, 2300.);
    hXal325b = new TH2F("hXal325b", "Xal325 s3 vs 5", 100, 2230, 2280., 100, 1200, 1350.);

    pXtalPulse = new TProfile**[kEBChannels];
    hXtalAmplitude = new TH1F**[kEBChannels];
    hXtalAmplitudeDQM = new TH1F**[kEBChannels];
    for(int ixtal=0; ixtal < kEBChannels; ixtal++) {
      pXtalPulse[ixtal] = new TProfile*[kGains];
      hXtalAmplitude[ixtal] = new TH1F*[kGains];
      hXtalAmplitudeDQM[ixtal] = new TH1F*[kGains];
      for(int igain=0; igain < kGains; igain++) {
	entries_[ixtal][igain] = 0.;
	if (ixtal == 22139 && igain == 1)
	  hXtalAmplitude[22139][1] = new TH1F("hXtalAmplitude_22139_1",
		   "UncalRecHit amplitude Crystal 22139 gain 6", 200, 0.40, 0.48);
	else if (ixtal == 22139 && igain == 2)
	  hXtalAmplitude[22139][2] = new TH1F("hXtalAmplitude_22139_2",
		   "UncalRecHit amplitude Crystal 22139 gain 1", 200, -0.2, 0.06);
	if (ixtal == 59791 && igain == 1)
	  hXtalAmplitude[59791][1] = new TH1F("hXtalAmplitude_59791_1",
		   "UncalRecHit amplitude Crystal 59791 gain 6", 200, 0.0, 0.8);
	else if (ixtal == 59791 && igain == 2)
	  hXtalAmplitude[59791][2] = new TH1F("hXtalAmplitude_59791_2",
		   "UncalRecHit amplitude Crystal 59791 gain 1", 200, 0.0, 0.2);
	else
	  hXtalAmplitude[ixtal][igain] =
	    new TH1F(Form("hXtalAmplitude_%i_%i",ixtal,igain),
		     Form("UncalRecHit amplitude Crystal %i gain %i",ixtal,gainValues[igain]),
		     200, Ampmin[igain], Ampmax[igain]);
	hXtalAmplitudeDQM[ixtal][igain] =
	    new TH1F(Form("hXtalAmplitudeDQM_%i_%i",ixtal,igain),
		     Form("UncalRecHit amplitude Crystal %i gain %i",ixtal,gainValues[igain]),
		     200, AmpminDQM[igain], AmpmaxDQM[igain]);
	pXtalPulse[ixtal][igain]= 
	  new TProfile(Form("pXtalPulse_%i_%i",ixtal,igain),
		       Form("Pulse Crystal %i gain %i",ixtal,gainValues[igain]),
		       10, 0., 10.);
      /*
      ptemp = new TProfile(Form("pXtalGainsForSample_%i_%i",ixtal,gainValues[igain]),
			   "Gains for each sample", 12, -1, 11);
      pXtalGainsForSample[ixtal][igain]=ptemp;
      */

      }  // loop over gains
    } // loop over crystals
    for(int igain=0; igain < kGains; igain++) {
      hRMS[igain] = new TH1F(Form("hRMS_%i",gainValues[igain]),"RMS", 100, 0.0, 0.001);
    }
  }  //  barrel

  if(ECALType_ == "EE" || ECALType_ == "EA") {
    double AmpminDQMEE[kGains] = {2300., 1000., 150.};
    double AmpmaxDQMEE[kGains] = {3300., 2000., 450.};
    hPeakEE = new TH1F("hPeakEE", "EE Peak", 10, 0, 10);
    pXtalPulseEE = new TProfile**[kEEChannels];
    hXtalAmplitudeEE = new TH1F**[kEEChannels];
    hXtalAmplitudeDQMEE = new TH1F**[kEEChannels];
    for(int ixtal=0; ixtal < kEEChannels; ixtal++) {
      pXtalPulseEE[ixtal] = new TProfile*[kGains];
      hXtalAmplitudeEE[ixtal] = new TH1F*[kGains];
      hXtalAmplitudeDQMEE[ixtal] = new TH1F*[kGains];
      //    cout << " Crystal " << ixtal << endl;
      for(int igain=0; igain < kGains; igain++) {
	//      cout << " gain " << igain << endl;
	entriesEE_[ixtal][igain] = 0.;
	/*
	if(ixtal == 674 && igain == 1)  // special cases
	  hXtalAmplitudeEE[674][1] = new TH1F("hXtalAmplitudeEE_674_1",
		   "EE UncalRecHit amplitude Crystal 674 gain 6", 200, 0.38, 0.46);
	else if (ixtal == 8430 && igain == 2)
	  hXtalAmplitudeEE[8430][2] = new TH1F("hXtalAmplitudeEE_8430_2",
		   "EE UncalRecHit amplitude Crystal 8430 gain 1", 200, 0.04, 0.12);
	else if (ixtal == 9399 && igain == 2)
	  hXtalAmplitudeEE[9399][2] = new TH1F("hXtalAmplitudeEE_9399_2",
		   "EE UncalRecHit amplitude Crystal 9399 gain 1", 200, -0.2, 0.06);
	*/
	if (ixtal == 3598 && igain == 1)
	  hXtalAmplitudeEE[3598][1] = new TH1F("hXtalAmplitudeEE_3598_1",
		   "EE UncalRecHit amplitude Crystal 3598 gain 6", 200, 0.40, 0.48);
	else
	  hXtalAmplitudeEE[ixtal][igain] =
	    new TH1F(Form("hXtalAmplitudeEE_%i_%i",ixtal,igain),
		     Form("EE UncalRecHit amplitude Crystal %i gain %i",ixtal,gainValues[igain]),
		     200, Ampmin[igain], Ampmax[igain]);
	hXtalAmplitudeDQMEE[ixtal][igain] =
	    new TH1F(Form("hXtalAmplitudeDQMEE_%i_%i",ixtal,igain),
		     Form("EE UncalRecHit amplitude Crystal %i gain %i",ixtal,gainValues[igain]),
		     200, AmpminDQMEE[igain], AmpmaxDQMEE[igain]);
	pXtalPulseEE[ixtal][igain] = 
	  new TProfile(Form("pXtalPulseEE_%i_%i",ixtal,igain),
		       Form("EE Pulse Crystal %i gain %i",ixtal,gainValues[igain]), 
		       10, 0., 10.);
      }  // loop over gains
    } // loop over crystals
    for(int igain=0; igain < kGains; igain++) {
      hRMSEE[igain] = new TH1F(Form("hRMSEE_%i",gainValues[igain]),"EE RMS", 100, 0.0, 0.001);
    }
  } // endcaps

  if(doPedes) {
    cout << "Pedestal computed from presample " << endl;
    if(ECALType_ == "EB" || ECALType_ == "EA") {
      for(int iChannel = 0; iChannel < kEBChannels; iChannel++) {
	for(int igain=0; igain < kGains; igain++)
	  pedestal_[iChannel][igain] = 0.;
      }
    }
    if(ECALType_ == "EE" || ECALType_ == "EA") {
      for(int iChannel = 0; iChannel < kEEChannels; iChannel++) {
	for(int igain=0; igain < kGains; igain++)
	  pedestalEE_[iChannel][igain] = 0.;
      }
    }
    return;
  }

  // Read weights
  //---------------
  if(useWeights) {
    ifstream fPedestal;
    int crystal = 0;
    int gain = 0;
    /*
    // Get SM name
    // Case where the same weights for a same SM
    string sSM = sFilename;
    if(sFilename.length() > 0) { 
      if(sSM.find("-", 0) != string::npos) sSM.erase(0, sSM.find("-", 0)+1); 
      if(sSM.find("-", 0) != string::npos) sSM.erase(0, sSM.find("-", 0)+1); 
      if(sSM.rfind("-", sSM.length()) != string::npos) sSM.erase(sSM.rfind("-", sSM.length()), sSM.length()); 
      if(sSM.rfind("-", sSM.length()) != string::npos) sSM.erase(sSM.rfind("-", sSM.length()), sSM.length()); 
    }
    */  
    // Open weights file
    // Barrel
    if(ECALType_ == "EB" || ECALType_ == "EA") {
      ifstream fWeights;
      fWeights.open(Form("TestPulseEB_%i.wgt",runnumber_));
      if(!fWeights.is_open()) {
	cout << "ERROR : can't open file TestPulse.wgt'" << endl;
	exit (1);
      }
  
      cout << "Read weights file" << endl;
      while (!fWeights.eof()) {
	//070411      getline(fWeightsInput, buffer);
	//070411      sscanf(buffer.c_str(), "%i %i", &crystal, &gain);
	fWeights >> crystal >> gain;
	//      cout << crystal << " " << gain << endl;

	if(fWeights.eof()) break;

	for(int iSample=0; iSample < nSample; iSample++) {
	//070411	getline(fWeightsInput, buffer);
	//070411	sscanf(buffer.c_str(), "%lf %lf", &weightsA[crystal-1][gain-1][iSample], &weightsP[crystal-1][gain-1][iSample]);
	  fWeights >> weightsA[crystal][gain-1][iSample];
	//cout << weightsA[crystal-1][gain-1][iSample/2]<<" "<<weightsP[crystal-1][gain-1][iSample/2] << endl;
	//	if(crystal < 10) cout << weightsA[crystal-1][gain-1][iSample] << " " ;
	}
      //      if(crystal < 10) cout  << endl;

	for(int iSample=0; iSample < nSample; iSample++) {
	  fWeights >> weightsA6[crystal][gain-1][iSample];
	}
      }
      fWeights.close();
      // gain 6 special method read pesdestal file
      fPedestal.open(Form("pedestal_%i",runnumber_));
      cout << "Reading pedestal file for gains 1 and 6" << endl;
      for(int xt=0; xt < kEBChannels; ++xt) {
	int iChannel = 0;
	double mean12 = 0., mean6 = 0., mean1 = 0.;
	//	double mean12 = 0.,sig12 = 0.,mean6 = 0.,sig6 = 0.,mean1 = 0.,sig1 = 0.;
	//	double siglf12 = 0.,siglf6 = 0.,siglf1 = 0.,sighf12 = 0.,sighf6 = 0.,sighf1 = 0.;
	//    pedesin >> iChannel >> ieta >> iphi >> mean12 >> sig12 >> mean6 >> sig6 >> mean1 >> sig1;
	//	fPedestal >> iChannel >> mean12 >> sig12  >> siglf12  >> sighf12 
	//		  >> mean6 >> sig6 >> siglf6  >> sighf6 
	//		  >> mean1 >> sig1 >> siglf1 >> sighf1;
	fPedestal >> iChannel >> mean12 >> mean6>> mean1;
	pedestal_[iChannel][1] = mean6;
	pedestal_[iChannel][2] = mean1;
	//    cout << iChannel << " " << pedestal6_[iChannel-1] << endl;
      }
      fPedestal.close();
    }  // barrel

    // Now endcaps
    if(ECALType_ == "EE" || ECALType_ == "EA") {
      ifstream fWeightsEE;
      // Open weights file
      fWeightsEE.open(Form("TestPulseEE_%i.wgt",runnumber_));
      if(!fWeightsEE.is_open()) {
	cout << "ERROR : can't open file TestPulseEE.wgt'" << endl;
	exit (1);
      }
      cout << " file TestPulseEE.wgt opened" << endl;

      while (!fWeightsEE.eof()) {
	fWeightsEE >> crystal >> gain;
	if(fWeightsEE.eof()) break;

	for(int iSample=0; iSample < kSamples; iSample++) {
	  fWeightsEE >> weightsEEA[crystal][gain-1][iSample];
	}

	for(int iSample=0; iSample < kSamples; iSample++) {
	  fWeightsEE >> weightsEEA6[crystal][gain-1][iSample];
	}
      }
      fWeightsEE.close();
      cout << "Read EE weights file" << endl;
      // gain 6 special method read pesdestal file
      fPedestal.open(Form("pedestalEE_%i",runnumber_));
      cout << "Reading EE pedestal file for gains 1 and 6" << endl;
      for(int xt=0; xt < 14648; ++xt) {
	int iChannel = 0;
	double mean12 = 0., mean6 = 0., mean1 = 0.;
	//	double mean12 = 0.,sig12 = 0.,mean6 = 0.,sig6 = 0.,mean1 = 0.,sig1 = 0.;
	//	double siglf12 = 0.,siglf6 = 0.,siglf1 = 0.,sighf12 = 0.,sighf6 = 0.,sighf1 = 0.;
	//	fPedestal >> iChannel >> mean12 >> sig12  >> siglf12  >> sighf12 
	//		  >> mean6 >> sig6 >> siglf6  >> sighf6 
	//		  >> mean1 >> sig1 >> siglf1 >> sighf1;
	fPedestal >> iChannel >> mean12 >> mean6 >> mean1;
	pedestalEE_[iChannel][1] = mean6;
	pedestalEE_[iChannel][2] = mean1;
      }
      fPedestal.close();
    }  // endcaps
    cout << " EE weights" << endl;
  } // if(useWeights)
  //  cout << "leaving constructor" << endl;
}


TestPulse::~TestPulse() {
}


//
// member functions
//

//========================================================================
void  TestPulse::beginRun(edm::Run const &, edm::EventSetup const & c) {
//========================================================================

  cout << "Entering beginRun" << endl;
  cnt_evt_ = 0;
  for(int i = 0; i < 10; i++)
    nevent[i] = 0; 
  edm::ESHandle< EcalElectronicsMapping > handle;
  c.get< EcalMappingRcd >().get(handle);
  ecalElectronicsMap_ = handle.product();
 
  fedMap_ = new EcalFedMap();
}

//========================================================================
void TestPulse::endJob() {
//========================================================================
 
  cout << " End job Number of events: " << cnt_evt_ << endl;

  cout << " Nb EB events         " << nevent[0] << endl
       << " Nb EB events gain 12 " << nevent[1] << endl
       << " Nb EB events gain 6  " << nevent[2] << endl
       << " Nb EB events gain 1  " << nevent[3] << endl
       << " Nb EE events         " << nevent[5] << endl
       << " Nb EE events gain 12 " << nevent[6] << endl
       << " Nb EE events gain 6  " << nevent[7] << endl
       << " Nb EE events gain 1  " << nevent[8] << endl;

  TFile fRootOutput(Form("TestPulse_%d.root",runnumber_),"RECREATE");
 
  // Compute weights
  //-----------------

  double baseline=0;
  double f=0;
  double sumf=0;
  double sumf2=0;
  double weightA=0;
  int gainValues[kGains] = {12, 6, 1};

  cout << " End job Number of events: " << cnt_evt_ << endl;

  TH1F *hEntries[3];
  TH1F *hEntriesEE[3];
  for(int igain=0; igain < kGains; igain++) {
    hEntries[igain] = new TH1F(Form("hEntries_%i",igain),Form("Entries_%i",gainValues[igain]), 100, 0., 1000.);
    hEntriesEE[igain] = new TH1F(Form("hEntriesEE_%i",igain),Form("EE Entries_%i",gainValues[igain]), 100, 0., 1000.);
  }
  ofstream fPedestal;
  //  float sig = 1.;
  if((ECALType_ == "EB" || ECALType_ == "EA") && nevent[0] > 0) {    // Barrel data present
    fTxtOutput << "N channels : gain 1:"<<vNGainMesures[2]/kEBChannels
	       <<"  gain 6:"<<vNGainMesures[1]/kEBChannels
	       <<"  gain 12:"<<vNGainMesures[0]/kEBChannels<< endl;

    int Noentry[36][68][3];
    int NoentryChannel[36][68][3][25];
    int NoentrySM[36][3];
    for (int gainId = 0; gainId < kGains; gainId++) {
      for (int SM = 0; SM <36; SM++) {
	NoentrySM[SM][gainId] = 0;
	for (int tower = 0; tower < 68; tower++) {
	  Noentry[SM][tower][gainId] = 0;
	}
      }
    }
    if(doPedes) {
      cout << "Pedestal computed from presample " << endl;
      fPedestal.open(Form("pedestal_%d",runnumber_));
    }
    for(int iChannel = 0; iChannel < kEBChannels; iChannel++) {
      EBDetId myEBDetId = EBDetId::unhashIndex(iChannel);
      int SM = myEBDetId.ism();  // 1:36
      int towerID = myEBDetId.tower().iTT(); // 1:68
      int SMm1 = SM - 1;  // 0:35 for arrays
      for(int gainId=0; gainId < kGains; gainId++) {
	hEntries[gainId]->Fill(entries_[iChannel][gainId]);
	if(entries_[iChannel][gainId] != 0.) {
	  if(doPedes)
	    pedestal_[iChannel][gainId] /= entries_[iChannel][gainId];
	}
	else {
	  int index = Noentry[SMm1][towerID - 1][gainId];
	  NoentryChannel[SMm1][towerID - 1][gainId][index] = iChannel;
	  Noentry[SMm1][towerID - 1][gainId]++;
	  NoentrySM[SMm1][gainId]++;
	  if(doPedes)
	    pedestal_[iChannel][gainId]  = -999.;
	}
      }   // loop on gainId
      if(doPedes) {
	fPedestal << setw(5) << iChannel << " " 
		  << setw(7) << setprecision(5) << pedestal_[iChannel][0] << " " 
	  //		  << setw(7) << sig << " " << setw(7) << sig  << " " << setw(7) << sig  << " " 
		  << setw(7) << setprecision(5) << pedestal_[iChannel][1]  << " " 
	  //		  << setw(7) << sig << " " << setw(7) << sig  << " " << setw(7) << sig  << " " 
	  //		  << setw(7) << setprecision(5) << pedestal1_[iChannel] << " "  
	  //		  << setw(7) << sig << " " << setw(7) << sig  << " " << setw(7) << sig  << endl;
 		  << setw(7) << setprecision(5) << pedestal_[iChannel][2] << endl;
     }
    } // loop on channel

    for(int gainId=0; gainId < kGains; gainId++) 
      hEntries[gainId]->Write();
    // print missing SM and towers
    for (int gainId = 0; gainId < kGains; gainId++) { 
      for (int SM = 0; SM <36; SM++) {
	if(NoentrySM[SM][gainId] == 1700)
	  cout << " No entry for SM " << SM + 1 << " gain " <<  gainValues[gainId]
	       << endl;
	else {
	  for (int tower = 0; tower < 68; tower++) {
	    if(Noentry[SM][tower][gainId] == 25)
	      cout << " No entry for " << Noentry[SM][tower][gainId] 
		   << " channels in tower " << tower + 1
		   << " SM " << SM + 1 << " gain " <<  gainValues[gainId]
		   << endl;
	    else if(Noentry[SM][tower][gainId] > 0) {
	      int index = Noentry[SM][tower][gainId];
	      cout << " No entry for " << index
		   << " channels in tower " << tower + 1
		   << " SM " << SM + 1 << " gain " <<  gainValues[gainId] << " channels";
	      for (int ich = 0; ich < index; ich++)
		cout << " " << NoentryChannel[SM][tower][gainId][ich];
	      cout << endl;
	    } // Entries on this tower
	  }  // loop on tower
	}  // Entries on this SM
      }  // loop on SM
    }  // loop on gainId

    if(doPedes) {
      fPedestal.close();
    }  // doPedes
    /*
    else {
      double Sample[kSamples];
      for(int ixtal=0; ixtal < kEBChannels; ixtal++) {
	for(int igain=0; igain < kGains; igain++) {
	  baseline=0;
	  bool badPulse = false;
	  for(int iSample = 0; iSample < kSamples; iSample++)
	    Sample[iSample] = pXtalPulse[ixtal][gainId]->GetBinContent(iSample+1);

	  for(int iSample=0; iSample < 3; iSample++)
	    baseline += Sample[iSample];
	  baseline /= 3.;

	  float Amax = 0.;
	  int Smax = 0;
	  for(int iSample = 3; iSample < 9; iSample++) {
	    f = Sample[iSample]) - baseline;
	    if(f > Amax) {
	      Amax = f;
	      Smax = iSample;
	    }
	    sumf += f;
	    sumf2 += f*f;
	  }
	  if(entries_[ixtal][igain] != 0) {
	    float  err = pXtalPulse[ixtal][igain]->GetBinError(Smax+1);
	    if(Smax != 6) {
	      cout << " crystal " << ixtal << " gain " << gainValues[igain]
		   << " Pb sample max " << Smax << " Amp " << Amax << endl;
	      badPulse = true;
	    }
	    if(igain == 0) {  // gain 12
	      if(Amax < 2000. || Amax > 2400.) {
		cout << " crystal " << ixtal << " gain 12  Pb amplitude " << Amax << endl;
		badPulse = true;
	      }
	      if(err > 40.) {
		cout << " crystal " << ixtal << " gain 12 Pb width " << err 
		     << " Amp " << Amax << endl;
		badPulse = true;
	      }
	    }
	    if(igain == 1) {  // gain 6
	      if(Amax < 1000. || Amax > 1200.) {
		cout << " crystal " << ixtal << " gain 6  Pb amplitude " << Amax << endl;
		badPulse = true;
	      }
	      if(err > 20.) {
		cout << " crystal " << ixtal << " gain 6 Pb width " << err 
		     << " Amp " << Amax << endl;
		badPulse = true;
	      }
	    }
	    if(igain == 2) {  // gain 1
	      if(Amax < 150. || Amax > 250.) {
		cout << " crystal " << ixtal << " gain 1  Pb amplitude " << Amax << endl;
		badPulse = true;
	      }
	      if(err > 10.) {
		cout << " crystal " << ixtal << " gain 1 Pb width " << err 
		     << " Amp " << Amax << endl;
		badPulse = true;
	      }
	    }
	  }
	  if(badPulse) pXtalPulse[ixtal][igain]->Write();
	}
      }
    }  // !doPedes
    */
  }  // barrel data present

  // End cap
  if((ECALType_ == "EE" || ECALType_ == "EA") && nevent[5] > 0) {    // End cap data present
    Int_t NoentryEE[2][316][3];
    int NoentryDCCEE[2][9][3];
    int NoentryChannelEE[2][316][3][25];
    for (int gainId = 0; gainId < kGains; gainId++)
      for (int Zside = 0; Zside <2; Zside++) {
	for (int iSC = 0; iSC < 316; iSC++)
	  NoentryEE[Zside][iSC][gainId] = 0;
	for (int DCC = 0; DCC <9; DCC++)
	  NoentryDCCEE[Zside][DCC][gainId] = 0;
      }

    ofstream fPedestal;
    if(doPedes) {
      cout << "EE Pedestal computed from presample " << endl;
      fPedestal.open(Form("pedestalEE_%d",runnumber_));
    }
    for(int iChannel = 0; iChannel < kEEChannels; iChannel++) {
      EEDetId checkEEDetId = EEDetId();
      if(checkEEDetId.validHashIndex(iChannel)) {
	EEDetId myEEDetId = EEDetId::unhashIndex(iChannel);
	int iz = myEEDetId.zside();  // 
	//	int ix = myEEDetId.ix();  // 
	//	int iy = myEEDetId.iy();  // 
	int iSC = myEEDetId.isc();
	//	int iquad = myEEDetId.iquadrant();  // 
	int izz = iz;
	if(iz == -1) izz = 0;
	EcalElectronicsId elecId = ecalElectronicsMap_->getElectronicsId(myEEDetId);
	int DCCid = elecId.dccId() - 1;  // 0-8
	if(DCCid > 44) {
	  if(iz == -1) cout << " Endcap strange EE+ for DCCId " << DCCid + 1 << endl;
	  DCCid -= 45;
	}
	else if(iz == 1) cout << " Endcap strange EE- for DCCId " << DCCid + 1 << endl;
	for(int gainId=0; gainId < kGains; gainId++) {
	  hEntriesEE[gainId]->Fill(entriesEE_[iChannel][gainId]);
	  if(entriesEE_[iChannel][gainId] != 0.) {
	    NoentryDCCEE[izz][DCCid][gainId]++;
	    if(doPedes) {
	    //	    double xmean = pedestalStandardEE_[arrayId] / double(nEntriesStandardEE_[arrayId]);
	    //	    Totmean = xmean;
	    //	    double sigmean = pedestal2StandardEE_[arrayId] / double(nEntriesStandardEE_[arrayId]);
	    //	    double Totsig2 = sigmean - xmean * xmean;
	    //	    Totsig = sqrt(Totsig2);
	      pedestalEE_[iChannel][gainId] /= entriesEE_[iChannel][gainId];
	    }
	  }
	  else {
	    int index = NoentryEE[izz][iSC - 1][gainId];
	    NoentryChannelEE[izz][iSC - 1][gainId][index] = iChannel;
	    if(iSC > 316)
	      cout << " EE Channel " <<  iChannel << " Super Crystal " << iSC << endl;
	    NoentryEE[izz][iSC - 1][gainId]++;   // iSC runs from 1 to 316
	    if(doPedes)
	      pedestalEE_[iChannel][gainId] = -999.;
	  }
	}   // end loop on gainId
	if(doPedes) {
	  fPedestal << setw(5) << iChannel << " " 
		    << setw(7) << setprecision(5) << pedestalEE_[iChannel][0] << " " 
	    //		    << setw(7) << sig << " " << setw(7) << sig  << " " << setw(7) << sig  << " " 
		    << setw(7) << setprecision(5) << pedestalEE_[iChannel][1]  << " " 
	    //		    << setw(7) << sig << " " << setw(7) << sig  << " " << setw(7) << sig  << " " 
	    //		    << setw(7) << setprecision(5) << pedestalEE1_[iChannel] << " "  
	    //		    << setw(7) << sig << " " << setw(7) << sig  << " " << setw(7) << sig  << endl;
		    << setw(7) << setprecision(5) << pedestalEE_[iChannel][2] << endl; 
	}
      }  // valid Crystal number
    } // End loop over endcap channels
    for(int gainId=0; gainId < kGains; gainId++) {
      hEntriesEE[gainId]->Write();
      // print missing channel, SC, slice
      for (int Zside = 0; Zside <2; Zside++) {
	std::string side = "-";
	int Fedid = 601;
	if(Zside == 1) {
	  side = "+";
	  Fedid = 646;
	}
	for (int DCC = 0; DCC < 9; DCC++)
	  if(NoentryDCCEE[Zside][DCC][gainId] == 0) {
	    string sliceName = fedMap_->getSliceFromFed(Fedid + DCC);
	    cout << " No entry for " << sliceName  << " gain " <<  gainValues[gainId] << endl;
	  }
	for (int iSC = 0; iSC < 316; iSC++) {
	  // check if it is not part of am empty slice (already printed)
	  if(NoentryEE[Zside][iSC][gainId] > 0) {
	    int ich = NoentryChannelEE[Zside][iSC][gainId][0];
	    EEDetId myEEDetId = EEDetId::unhashIndex(ich);
	    EcalElectronicsId elecId = ecalElectronicsMap_->getElectronicsId(myEEDetId);
	    int DCCid = elecId.dccId() - 1;  // 0-8
	    if(DCCid > 44)  DCCid -= 45;
	    if(NoentryDCCEE[Zside][DCCid][gainId] != 0) { // not yet printed
	      if(NoentryEE[Zside][iSC][gainId] == 25)
		cout << " No entry for " << NoentryEE[Zside][iSC][gainId] 
		     << " channels in SC " << iSC + 1 // iSC runs from 1 to 316
		     << " Z side " << Zside
		     << " gain " <<  gainValues[gainId]
		     << endl;
	      else if(NoentryEE[Zside][iSC][gainId] > 0) {
		int index = NoentryEE[Zside][iSC][gainId];
		cout << " No entry for " << index
		     << " channels in SC " << iSC + 1
		     << " Z side " << Zside
		     << " gain " <<  gainValues[gainId] << " channels";
		for (int ich = 0; ich < index; ich++)
		  cout << " " << NoentryChannelEE[Zside][iSC][gainId][ich];
		cout << endl;
	      }
	    } // NoentryDCCEE > 0
	  } // NoentryEE > 0
	} // loop on SC
      }  // loop on Zside
    }  // loop on gain
    if(doPedes) {
      fPedestal.close();
    }  // doPedes
  }  // End cap data present

  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  if(doWeights) {
    //       LogInfo("") << "Compute weights.";
    cout << "Compute weights." << endl;
    ofstream fWeights;
 
    double Sample[kSamples];
    if(ECALType_ == "EB" || ECALType_ == "EA") {  // barrel
      fWeights.open(Form("TestPulseEB_%i.wgt",runnumber_));
      for(int ixtal=0; ixtal < kEBChannels; ixtal++) {
	for(int gainId=0; gainId < kGains; gainId++) {
	  baseline=0; sumf=0; sumf2=0; weightA=0;
	  fWeights << ixtal << " " << gainId+1 << endl;
 
	  for(int iSample = 0; iSample < kSamples; iSample++)
	    Sample[iSample] = pXtalPulse[ixtal][gainId]->GetBinContent(iSample+1);

	  for(int iSample=0; iSample < 3; iSample++)
	    baseline += Sample[iSample];
	  baseline /= 3.;

	// 070411	 for(int iSample=3; iSample < nSample; iSample++) {
	// 070411 3 + 5 sample method 
	// 070417 use 3rd sample	for(int iSample = 4; iSample < 9; iSample++) {
	  for(int iSample = 4; iSample < 9; iSample++) {
	    f = Sample[iSample] - baseline;
	    sumf += f;
	    sumf2 += f*f;
	  }

	  double sumweightA=0;
	  double sumweightF=0;
	  //  3 + 5 sample method 
	  if(sumf == 0) {
	    if(entries_[ixtal][gainId] != 0) {
	      cout << "EB Xtal " << ixtal << " gain " << gainId << " sumf = 0" << endl;
	      pXtalPulse[ixtal][gainId]->Write();
	    }
	    for(int method = 0; method < 2; method++) {
	      for(int iSample=0; iSample < nSample; iSample++) {
		weightA = 0.;
		fWeights << weightA << " ";
	      } // loop over iSample
	      fWeights << endl;
	    }  // loop over 2 methods
	  }
	  else {
	    for(int iSample=0; iSample < nSample; iSample++) {
	      //	 070417 use 3rd sample    if(iSample == 3 || iSample == 9)
	      if(iSample == 3 || iSample == 9)
		weightA = 0.;
	      else {
		f = 0;
	      // 070417 use 3rd sample	      if(iSample > 3) f = Sample[iSample] - baseline;
		if(iSample > 3) f = Sample[iSample] - baseline;
		weightA = (8 * f - sumf) / (8 * sumf2 - sumf*sumf);
		sumweightA+=weightA;
		sumweightF+=weightA * f;
	      }
	      fWeights << weightA << " ";
	    } // loop over iSample
	    fWeights << endl;
	    // check
	    if(sumweightA < -0.0001 || sumweightA > 0.0001 || sumweightF < 0.9999 || sumweightF > 1.0001)
	      cout << " EB channel " << ixtal
		   << " Weight 3 + 5 problem sumweightA " << sumweightA 
		   << " sumweightF " << sumweightF << endl;
	    // 070411 end 3 + 5 sample method 
	    // 070411 5 sample method 
	    sumweightF = 0.;
	    for(int iSample=0; iSample < nSample; iSample++) {
	      // 070417 use 3rd sample	    if(iSample <= 3 || iSample == 9)
	      // 091019	      if(iSample <= 2 || iSample > 7)
	      if(iSample <= 3 || iSample == 9)
		weightA = 0.;
	      else {
		f = Sample[iSample] - baseline;	 	 
		weightA = f / sumf2;
		sumweightF+=weightA * f;
	      }
	      fWeights << weightA << " ";
	    } // loop over iSample
	    fWeights << endl;
	    // check
	    if(sumweightF < 0.9999 || sumweightF> 1.0001)
	      cout << " EB channel " << ixtal
		   << " Weight 5 problem sumweightF " << sumweightF << endl;
	    // 070411 end 5 sample method
	  }  //  check sumf == 0  
	} // for(int igain=0; igain < kGains; igain++)
      } // for(int ixtal=0; ixtal < kEBChannels; ixtal++)
      fWeights.close();
    } // barrel

    if(ECALType_ == "EE" || ECALType_ == "EA") {  // endcaps
      fWeights.open(Form("TestPulseEE_%i.wgt",runnumber_));
      EEDetId checkEEDetId = EEDetId();
      for(int ixtal=0; ixtal < kEEChannels; ixtal++) {
	if(checkEEDetId.validHashIndex(ixtal)) {
	  for(int gainId=0; gainId < kGains; gainId++) {
	    baseline=0; sumf=0; sumf2=0; weightA=0;
	    fWeights << ixtal << " " << gainId+1 << endl;
	    for(int iSample = 0; iSample < kSamples; iSample++)
	      Sample[iSample] = pXtalPulseEE[ixtal][gainId]->GetBinContent(iSample+1);

	    for(int iSample=0; iSample < 3; iSample++)
	      baseline += Sample[iSample];
	    baseline /= 3.;

	    // 091013	      for(int iSample = 3; iSample < 8; iSample++) {
	    for(int iSample = 4; iSample < 9; iSample++) {
	      f = Sample[iSample] - baseline;
	      sumf += f;
	      sumf2 += f*f;
	    }

	    double sumweightA=0;
	    double sumweightF=0;
	    // 070411 3 + 5 sample method 
	    if(sumf == 0) {
	      if(entriesEE_[ixtal][gainId] != 0) {
		cout << "EE Xtal " << ixtal << " gain " << gainValues[gainId] << " sumf = 0" << endl;
		pXtalPulseEE[ixtal][gainId]->Write();
	      }
	      for(int method = 0; method < 2; method++) {
		for(int iSample=0; iSample < kSamples; iSample++) {
		  weightA = 0.;
		  fWeights << weightA << " ";
		} // loop over iSample
		fWeights << endl;
	      }  // loop over 2 methods
	    }
	    else {
	      for(int iSample=0; iSample < kSamples; iSample++) {
		  //	 070417 use 3rd sample    if(iSample == 3 || iSample == 9)
		  // 091013	     		  if(iSample > 7)
		if(iSample == 3 || iSample == 9)
		  weightA = 0.;
		else {
		  f = 0;
		  if(iSample > 3) f = Sample[iSample] - baseline;
		  weightA = (8 * f - sumf) / (8 * sumf2 - sumf*sumf);
		  sumweightA+=weightA;
		  sumweightF+=weightA * f;
		}
		fWeights << weightA << " ";
	      } // loop over iSample
	      fWeights << endl;
	      // check
	      if(sumweightA < -0.0001 || sumweightA > 0.0001 || sumweightF < 0.9999 || sumweightF > 1.0001)
		cout << " EE channel " << ixtal
		     << " Weight 3 + 5 problem sumweightA " << sumweightA 
		     << " sumweightF " << sumweightF << endl;
	      // 070411 end 3 + 5 sample method 
	      // 070411 5 sample method 
	      sumweightF = 0.;
	      for(int iSample=0; iSample < kSamples; iSample++) {
		  // 070417 use 3rd sample	    if(iSample <= 3 || iSample == 9)
		  // 091013		  if(iSample <= 2 || iSample > 7)
		if(iSample <= 3 || iSample == 9)
		  weightA = 0.;
		else {
		  f = Sample[iSample] - baseline;	 	 
		  weightA = f / sumf2;
		  sumweightF+=weightA * f;
		}
		fWeights << weightA << " ";
	      } // loop over iSample
	      fWeights << endl;
	      // check
	      if(sumweightF < 0.9999 || sumweightF> 1.0001)
		cout << " EE channel " << ixtal
		     << " Weight 5 problem sumweightF " << sumweightF << endl;
	      // 070411 end 5 sample method
	    }  //  check sumf == 0
	  }  // for(int gainId=0; gainId < kGains; gainId++)
	}  // validHashedIndex
      }  // loop over channels
      fWeights.close();
    }  // end caps
  } // doWeights
  std::ofstream fDQM;
  fDQM.open(Form("DQMlike_%d",runnumber_));
  if(ECALType_ == "EB" || ECALType_ == "EA") {  // barrel
    for(int ixtal=0; ixtal < kEBChannels; ixtal++) {
      EBDetId myEBDetId = EBDetId::unhashIndex(ixtal);
      int iChperSM = myEBDetId.ic(); // 1:1700
      int SM = myEBDetId.ism();  // 1:36
      bool DQMindex = false;
      for (int gainId = 2; gainId > -1; gainId--) { 
	Double_t nEntries = hXtalAmplitudeDQM[ixtal][gainId]->GetEntries();
	if(nEntries != 0) {
	  Double_t mean = hXtalAmplitudeDQM[ixtal][gainId]->GetMean(1);
	  if(!DQMindex) {
	    if(SM <10) fDQM << "55555  10110" << SM;
	    else fDQM << "55555  1011" << SM;
	    if(iChperSM < 10) fDQM << "000" << iChperSM;
	    else if(iChperSM < 100) fDQM << "00" << iChperSM;
	    else if(iChperSM < 1000) fDQM << "0" << iChperSM;
	    else fDQM  << iChperSM;
	    DQMindex = true;
	    if(gainId == 1) fDQM << " -1.0    ";
	    else if(gainId == 0) fDQM << " -1.0    -1.0    ";
	  }
	  fDQM << " " << std::setw(7) << std::setprecision(5) << mean << " ";
	}  // only if read out
      }   // End loop over gains
      // now RMS
      DQMindex = false;
      for (int gainId = 2; gainId > -1; gainId--) { 
	Double_t nEntries = hXtalAmplitudeDQM[ixtal][gainId]->GetEntries();
	if(nEntries != 0) {
	  Double_t rms = hXtalAmplitudeDQM[ixtal][gainId]->GetRMS(1);
	  if(!DQMindex) {
	    DQMindex = true;
	    if(gainId == 1) fDQM << " -1.0    ";
	    else if(gainId == 0) fDQM << " -1.0    -1.0    ";
	  }
	  fDQM << std::setw(7) << std::setprecision(4) << rms << " ";
	  if(gainId == 0) fDQM << " 0" << std::endl;
	  if(ixtal%1000 == 0) hXtalAmplitudeDQM[ixtal][gainId]->Write();  // only few ones
	}  // only if read out
      }   // End loop over gains
    }    // End loop over channels
  }  // barrel

  if(ECALType_ == "EE" || ECALType_ == "EA") {  // endcaps
    for(int ixtal=0; ixtal < kEEChannels; ixtal++) {
      EEDetId myEEDetId = EEDetId::unhashIndex(ixtal);
      int iz = myEEDetId.zside();
      int ix = myEEDetId.ix();
      int iy = myEEDetId.iy();
      bool DQMindex = false;
      for (int gainId = 2; gainId > -1; gainId--) { 
	Double_t nEntries = hXtalAmplitudeDQMEE[ixtal][gainId]->GetEntries();
	if(nEntries != 0) {
	  Double_t mean = hXtalAmplitudeDQMEE[ixtal][gainId]->GetMean(1);
	  if(!DQMindex) {
	    if(iz == -1) fDQM << "55555  2010";
	    else fDQM << "55555  2012";
	    if(ix < 10) fDQM << "00" << ix;
	    else if(ix < 100) fDQM << "0" << ix;
	    else fDQM  << ix;
	    if(iy < 10) fDQM << "00" << iy;
	    else if(iy < 100) fDQM << "0" << iy;
	    else fDQM  << iy;
	    DQMindex = true;
	    if(gainId == 1) fDQM << " -1.0   ";
	    else if(gainId == 0) fDQM << " -1.0    -1.0     ";
	  }
	  fDQM << std::setw(7) << std::setprecision(5) << mean << " ";
	}  // only if read out
      }   // End loop over gains
      // now RMS
      DQMindex = false;
      for (int gainId = 2; gainId > -1; gainId--) { 
	Double_t nEntries = hXtalAmplitudeDQMEE[ixtal][gainId]->GetEntries();
	if(nEntries != 0) {
	  Double_t rms = hXtalAmplitudeDQMEE[ixtal][gainId]->GetRMS(1);
	  if(!DQMindex) {
	    DQMindex = true;
	    if(gainId == 1) fDQM << " -1.0    ";
	    else if(gainId == 0) fDQM << " -1.0    -1.0    ";
	  }
	  fDQM << std::setw(7) << std::setprecision(4) << rms << " ";
	  if(gainId == 0) fDQM << " 0" << std::endl;
	  if(ixtal%1000 == 0) hXtalAmplitudeDQMEE[ixtal][gainId]->Write();  // only few ones
	}  // only if read out
      }   // End loop over gains
    }    // End loop over channels
  }  // endcaps    
  fDQM.close();

  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  if(useWeights) {
    //    vector<TGraphErrors*> gXtalAmplitude;
    //    vector<TGraph*> gXtalChi2;
    TGraphErrors* gXtalAmplitude[kGains];
    TGraph* gXtalChi2[kGains];

    TF1* fgaus = new TF1("fgaus", "gaus", 0, 1.5);

    int ifit = 0;

    if(ECALType_ == "EB" || ECALType_ == "EA") {  // barrel
      Double_t Amplitude[kEBChannels][kGains];
      for(int gainId=0; gainId < kGains; gainId++) {
       //	TGraphErrors* gGraphTemp = new TGraphErrors(kEBChannels);
       //	gGraphTemp->SetName(Form("gXtalAmplitude_%i", gainValues[igain]));
       //	gXtalAmplitude.push_back(gGraphTemp);
	gXtalAmplitude[gainId] = new TGraphErrors(kEBChannels);
	gXtalAmplitude[gainId]->SetName(Form("gXtalAmplitude_%i", gainValues[gainId]));
	gXtalAmplitude[gainId]->SetTitle(Form("EB Amplitude gain %i", gainValues[gainId]));
	//	TGraph* gGraphTemp2 = new TGraph(kEBChannels);
	//	gGraphTemp2->SetName(Form("gXtalChi2_%i", gainValues[igain]));
	//	gXtalChi2.push_back(gGraphTemp2);
	gXtalChi2[gainId] = new TGraph(kEBChannels);
	gXtalChi2[gainId]->SetName(Form("gXtalChi2_%i", gainValues[gainId]));
	gXtalChi2[gainId]->SetTitle(Form("EB Chi2 gain %i", gainValues[gainId]));

	for(int ixtal=0; ixtal < kEBChannels; ixtal++) {
	  Double_t nEntries = hXtalAmplitude[ixtal][gainId]->GetEntries();
	  if(nEntries != 0) {
	    //	  if(nEntries != 600. && nEntries != 150. && nEntries != 50.)
	    if(nEntries != 350.)
	      cout << "chan " << ixtal << " gain " << gainId << " N entries " << nEntries << endl;
	    Double_t mean = hXtalAmplitude[ixtal][gainId]->GetMean(1);
	    Double_t rms = hXtalAmplitude[ixtal][gainId]->GetRMS(1);
	    hRMS[gainId]->Fill(rms);
	    fgaus->SetParameter(0,nEntries/10);
	    fgaus->SetParameter(1,mean);
	    fgaus->SetParameter(2,rms);
	    ifit = hXtalAmplitude[ixtal][gainId]->Fit("fgaus","Q+");
	    //  Check the fit...
	    Double_t Chi2 = fgaus->GetChisquare();
	    Int_t NDF = fgaus->GetNDF();
	    Chi2 = Chi2/(Float_t)NDF;
	    Double_t fpeak = fgaus->GetParameter(0);
	    Double_t fmean = fgaus->GetParameter(1);
	    Double_t frms = fgaus->GetParameter(2);
	    if(frms > 2*rms || abs(fmean - mean) > rms || fpeak < nEntries/10 || fpeak > nEntries) {
	      cout << "Bad fit for channel " << ixtal << " gain " << gainValues[gainId];
	      if(abs(fmean - mean) > rms)
		cout << " fit mean " << fmean << " mean " << mean << " rms " << rms;
	      if(frms > 2*rms )
		cout <<  " fit rms " << frms;
	      if(fpeak < nEntries/10 || fpeak > nEntries)
		cout  << " fit max " << fpeak;
	      cout << " try to rebin"  << endl;
	      ifit = 1;
	    }

	    // try a new fit after rebinning
	    if(ifit) {
	      hXtalAmplitude[ixtal][gainId]->Rebin(2);
	      fgaus->SetParameter(0,nEntries/10);
	      fgaus->SetParameter(1,mean);
	      fgaus->SetParameter(2,rms);
	      ifit = hXtalAmplitude[ixtal][gainId]->Fit("fgaus","Q+");
	      Chi2 = fgaus->GetChisquare();
	      NDF = fgaus->GetNDF();
	      Chi2 = Chi2/(Float_t)NDF;
	      fpeak = fgaus->GetParameter(0);
	      fmean = fgaus->GetParameter(1);
	      frms = fgaus->GetParameter(2);
	      if(frms > 2*rms || abs(fmean - mean) > rms || fpeak < nEntries/10 || fpeak > nEntries) {
		ifit = 1;
	      }
	    }
	    if(ifit) {
	      cout<<"Bad fit for channel "<<ixtal <<" gain "<<gainValues[gainId] << endl;
	      fTxtOutput<<"Bad fit for channel "<<ixtal <<" gain "<<gainValues[gainId] << endl;
	      cout << " use mean " << mean << " instead of fit " 
		   << fgaus->GetParameter(1) << endl;
	      hXtalAmplitude[ixtal][gainId]->Write();
	      gXtalAmplitude[gainId]->SetPoint(ixtal+1,ixtal+1,mean);
	      gXtalAmplitude[gainId]->SetPointError(ixtal+1, 0,rms);
	      Amplitude[ixtal][gainId] = mean;
	    }
	    else {
	      gXtalAmplitude[gainId]->SetPoint(ixtal+1,ixtal+1,fgaus->GetParameter(1));
	      gXtalAmplitude[gainId]->SetPointError(ixtal+1, 0,fgaus->GetParameter(2));
	      Amplitude[ixtal][gainId] = fmean;
	    }
	    gXtalChi2[gainId]->SetPoint(ixtal+1,ixtal+1,fgaus->GetChisquare());
	  }  // nEntries not 0
	  else {
	    gXtalAmplitude[gainId]->SetPoint(ixtal+1,ixtal+1,0.);
	    Amplitude[ixtal][gainId] = 0.;
	  }  // nEntries = 0
	}  // loop over crystals
      } // loop over gains

      // compute the gain ratios
      cout << " Computing the gain ratios" << endl;
      ofstream fRatioOutput;
      fRatioOutput.open(Form("TP_GainratioEB_%i.txt",runnumber_));
      for(int ixtal=0; ixtal < kEBChannels; ixtal++) {
	fRatioOutput <<  ixtal;
	if(Amplitude[ixtal][0] == 0. && entries_[ixtal][0] != 0) 
	  cout << "EB Channel " << ixtal << " amp gain 12 =0!" << endl; 
	if(Amplitude[ixtal][2] == 0.) {
	  if(entries_[ixtal][2] != 0) cout << "EB Channel " << ixtal << " amp gain 1 =0!" << endl; 
	  fRatioOutput <<  " 0.0";
	}
	else
	  fRatioOutput << " " << Amplitude[ixtal][0]/Amplitude[ixtal][2];
	if(Amplitude[ixtal][1] == 0.) {
	  if(entries_[ixtal][1] != 0) cout << "EB Channel " << ixtal << " amp gain 6 =0!" << endl; 
	  fRatioOutput <<  " 0.0";
	}
	else
	  fRatioOutput << " " << Amplitude[ixtal][0]/Amplitude[ixtal][1];
	fRatioOutput << endl;
      } // loop over crystals
      fRatioOutput.close();

      for(int ixtal=0; ixtal < kEBChannels; ixtal += 1000)
	for(int gainId=0; gainId < kGains; gainId++) {
	  if(hXtalAmplitude[ixtal][gainId]->GetEntries() != 0)
	    hXtalAmplitude[ixtal][gainId]->Write();  // only few ones
	}
      for(int gainId=0; gainId < kGains; gainId++) {
	if(hXtalAmplitude[22139l][gainId]->GetEntries() != 0)
	  hXtalAmplitude[22139][gainId]->Write();  // special ones
	if(hXtalAmplitude[59791][gainId]->GetEntries() != 0)
	  hXtalAmplitude[59791][gainId]->Write();
	gXtalAmplitude[gainId]->Write();
	gXtalChi2[gainId]->Write();
	hRMS[gainId]->Write();
      }

      if(bLogFile) fTxtOutput << "Number of events : " << cnt_evt_  << endl;
      if(bLogFile) fTxtOutput << "Gains 1: "<< vNGainMesures[2]/(float)kEBChannels <<" 6: "<<  vNGainMesures[1]/(float)kEBChannels <<" 12: "<< vNGainMesures[0]/(float)kEBChannels << endl;

    }   // barrel


    if(ECALType_ == "EE" || ECALType_ == "EA") {  // endcaps
      Double_t Amplitude[kEEChannels][kGains];
      EEDetId checkEEDetId = EEDetId();
      for(int gainId=0; gainId < kGains; gainId++) {
	gXtalAmplitude[gainId] = new TGraphErrors(kEEChannels);
	gXtalAmplitude[gainId]->SetName(Form("gXtalAmplitudeEE_%i", gainValues[gainId]));
	gXtalAmplitude[gainId]->SetTitle(Form("EE Amplitude gain %i", gainValues[gainId]));
	gXtalChi2[gainId] = new TGraph(kEEChannels);
	gXtalChi2[gainId]->SetName(Form("gXtalChi2EE_%i", gainValues[gainId]));
	gXtalChi2[gainId]->SetTitle(Form("EE Chi2 gain %i", gainValues[gainId]));

	for(int ixtal=0; ixtal < kEEChannels; ixtal++) {
	  if(checkEEDetId.validHashIndex(ixtal)) {
	    Double_t nEntries = hXtalAmplitudeEE[ixtal][gainId]->GetEntries();
	    if(nEntries != 0.) {
	      if(nEntries != 350.)
		cout << "EE chan " << ixtal << " gain " << gainId << " N entries " << nEntries << endl;
	      Double_t mean = hXtalAmplitudeEE[ixtal][gainId]->GetMean(1);
	      Double_t rms = hXtalAmplitudeEE[ixtal][gainId]->GetRMS(1);
	      hRMSEE[gainId]->Fill(rms);
	      fgaus->SetParameter(0,nEntries/10);
	      fgaus->SetParameter(1,mean);
	      fgaus->SetParameter(2,rms);
	      ifit = hXtalAmplitudeEE[ixtal][gainId]->Fit("fgaus","Q+");
	      //  Check the fit...
	      Double_t Chi2 = fgaus->GetChisquare();
	      Int_t NDF = fgaus->GetNDF();
	      Chi2 = Chi2/(Float_t)NDF;
	      Double_t fpeak = fgaus->GetParameter(0);
	      Double_t fmean = fgaus->GetParameter(1);
	      Double_t frms = fgaus->GetParameter(2);
	      if(frms > 2*rms || abs(fmean - mean) > rms || fpeak < nEntries/10 || fpeak > nEntries) {
		cout << "Bad fit for EE channel " << ixtal << " gain " << gainValues[gainId];
		if(abs(fmean - mean) > rms)
		  cout << " fit mean " << fmean << " mean " << mean << " rms " << rms;
		if(frms > 2*rms )
		  cout <<  " fit rms " << frms;
		if(fpeak < nEntries/10 || fpeak > nEntries)
		  cout  << " fit max " << fpeak;
		cout << " try to rebin"  << endl;
		ifit = 1;
	      }

	      // try a new fit after rebinning
	      if(ifit) {
		hXtalAmplitudeEE[ixtal][gainId]->Rebin(2);
		fgaus->SetParameter(0,nEntries/10);
		fgaus->SetParameter(1,mean);
		fgaus->SetParameter(2,rms);
		ifit = hXtalAmplitudeEE[ixtal][gainId]->Fit("fgaus","Q+");
		Chi2 = fgaus->GetChisquare();
		NDF = fgaus->GetNDF();
		Chi2 = Chi2/(Float_t)NDF;
		fpeak = fgaus->GetParameter(0);
		fmean = fgaus->GetParameter(1);
		frms = fgaus->GetParameter(2);
		if(frms > 2*rms || abs(fmean - mean) > rms || fpeak < nEntries/10 || fpeak > nEntries) {
		  ifit = 1;
		}
	      }
	      if(ifit) {
		cout<<"Bad fit for EE channel "<<ixtal <<" gain "<<gainValues[gainId] << endl;
		fTxtOutput<<"Bad fit for EE channel "<<ixtal <<" gain "<<gainValues[gainId] << endl;
		cout << " use mean " << mean << " instead of fit " 
		     << fgaus->GetParameter(1) << endl;
		hXtalAmplitudeEE[ixtal][gainId]->Write();
		gXtalAmplitude[gainId]->SetPoint(ixtal+1,ixtal+1,mean);
		gXtalAmplitude[gainId]->SetPointError(ixtal+1, 0,rms);
		Amplitude[ixtal][gainId] = mean;
	      }
	      else {
		gXtalAmplitude[gainId]->SetPoint(ixtal+1,ixtal+1,fgaus->GetParameter(1));
		gXtalAmplitude[gainId]->SetPointError(ixtal+1, 0,fgaus->GetParameter(2));
		Amplitude[ixtal][gainId] = fmean;
	      }
	      gXtalChi2[gainId]->SetPoint(ixtal+1,ixtal+1,fgaus->GetChisquare());
	    }  // nEntries not 0
	    else {
	      gXtalAmplitude[gainId]->SetPoint(ixtal+1,ixtal+1,0.);
	      Amplitude[ixtal][gainId] = 0.;
	    }  // nEntries = 0
	  }  //  validHashedIndex
	}  // loop over crystals
      } // loop over gains

      // compute the gain ratios
      cout << " Computing the gain ratios for endcaps" << endl;
      ofstream fRatioOutput;
      fRatioOutput.open(Form("TP_GainratioEE_%i.txt",runnumber_));
      for(int ixtal=0; ixtal < kEEChannels; ixtal++) {
	if(checkEEDetId.validHashIndex(ixtal)) {
	  fRatioOutput <<  ixtal;
	  if(Amplitude[ixtal][0] == 0. && entriesEE_[ixtal][0] != 0) 
	    cout << "EE Channel " << ixtal << " amp gain 12 =0!" << endl; 
	  if(Amplitude[ixtal][2] == 0.) {
	    if(entriesEE_[ixtal][2] != 0) cout << "EE Channel " << ixtal << " amp gain 1 =0!" << endl; 
	    fRatioOutput <<  " 0.0";
	  }
	  else
	    fRatioOutput << " " << Amplitude[ixtal][0]/Amplitude[ixtal][2];
	  if(Amplitude[ixtal][1] == 0.) {
	    if(entriesEE_[ixtal][1] != 0) cout << "EE Channel " << ixtal << " amp gain 6 =0!" << endl; 
	    fRatioOutput <<  " 0.0";
	  }
	  else
	    fRatioOutput << " " << Amplitude[ixtal][0]/Amplitude[ixtal][1];
	  fRatioOutput << endl;
	}  //  validHashedIndex
      }  // loop over crystals
      fRatioOutput.close();

      for(int ixtal=0; ixtal < kEEChannels; ixtal += 1000)
	for(int gainId=0; gainId < kGains; gainId++) {
	  if(hXtalAmplitudeEE[ixtal][gainId]->GetEntries() != 0)
	    hXtalAmplitudeEE[ixtal][gainId]->Write();  // only few ones
	}
      for(int gainId=0; gainId < kGains; gainId++) {
	hXtalAmplitudeEE[674][gainId]->Write();  // special ones
	hXtalAmplitudeEE[3598][gainId]->Write();
	gXtalAmplitude[gainId]->Write();
	gXtalChi2[gainId]->Write();
	hRMSEE[gainId]->Write();
      }

      if(bLogFile) fTxtOutput << "Number of events : " << cnt_evt_  << endl;
      if(bLogFile) fTxtOutput << " Gains 1: " << vNGainMesures[2]/(float)kEEChannels 
			      << "       6: " << vNGainMesures[1]/(float)kEEChannels 
			      << "      12: " << vNGainMesures[0]/(float)kEEChannels << endl;
    }  // endcaps
  } // useWeights
  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

  //  fRootOutput->cd();
  if(ECALType_ == "EB" || ECALType_ == "EA") {
    hPeak->Write();
    hXal325->Write();
    hXal3252->Write();
    hXal325b->Write();

    if(bSaveProfiles) {
      for(int ixtal = 0; ixtal < kEBChannels; ixtal += 10000) { // only few ones
	for(int gain = 0; gain < kGains; gain++)
	  if(pXtalPulse[ixtal][gain]->GetEntries() > 0.) pXtalPulse[ixtal][gain]->Write();
      }
    }
  }  // barrel
  if(ECALType_ == "EE" || ECALType_ == "EA") {
    hPeakEE->Write();
    if(bSaveProfiles) {
      for(int ixtal = 0; ixtal < kEEChannels; ixtal += 1000)  // only few ones
	for(int gain = 0; gain < kGains; gain++)
	  if(pXtalPulseEE[ixtal][gain]->GetEntries() > 0.) pXtalPulseEE[ixtal][gain]->Write();
    }
  }  // endcaps
  fRootOutput.Close();

  // Close files
  if(bLogFile) fTxtOutput.close();

  cout << "End of code" << endl;

}


// ------------ method called to analyze the data  ------------
//========================================================================
void TestPulse::analyze( const edm::Event& iEvent, const edm::EventSetup& iSetup ) {
//========================================================================

  int ievt = iEvent.id().event();
  int run = iEvent.id().run();
  if(run != runnumber_)
    cout << " Pb on event " << ievt << " run " << run << " instead of " << runnumber_<< endl;
  cnt_evt_++;
  //  if(cnt_evt_%50 == 0) cout << cnt_evt_ << " Running on event " << ievt << endl;

  //Get the headers
  Handle<EcalRawDataCollection> DCCHeaders;
  try{ 
    iEvent.getByLabel("ecalDigis", DCCHeaders);
  } catch ( exception& ex ) {
    cout << "Error! can't get the product DCCHeaders for EcalRawDataCollection " << endl;
    return;
  }

  int gainIdRef = -1;
  short DACCAL = -1;

  if(cnt_evt_ == 1) cout << " DCCid ";
  for ( EcalRawDataCollection::const_iterator headerItr= DCCHeaders->begin();headerItr != DCCHeaders->end(); 
	++headerItr ) {
    int FEDid = headerItr->id();
    if(cnt_evt_ == 1) cout << FEDid << " ";
    int gainId = headerItr->getMgpaGain();
    short dccCAL = headerItr->getEventSettings().mgpa_content;
    if(headerItr == DCCHeaders->begin()) {
      gainIdRef = gainId;
      DACCAL = dccCAL;
    }
    else {
      if(gainId != gainIdRef) cout << " Pb gainId " << gainId << " ";
      if (DACCAL != dccCAL) cout << " Pb DACCAL  " << dccCAL << " on FED " << FEDid << endl;
    }
  }
  if(cnt_evt_ == 1) cout << endl;
  cout << " Running on event " << ievt << " at " << cnt_evt_ << " gainId " << gainIdRef 
       << " DACCAL " << DACCAL << endl;

  int nebd = 0;
  int need = 0;
  const EBDigiCollection* EBDigis = 0;
  const EEDigiCollection* EEDigis = 0;
  // Get the EB Digis
  if(ECALType_ == "EB" || ECALType_ == "EA") {
    Handle<EBDigiCollection> pEBDigis;
    try  {
      iEvent.getByLabel(digiProducer_, EBdigiCollection_, pEBDigis);
      EBDigis = pEBDigis.product();
      nebd = EBDigis->size();
    }
    catch(exception& ex) {
      cout << "Can't get " << digiProducer_ << " " << EBdigiCollection_ << " Will skip this event. " << endl;
      return;
    }
  } // barrel

  // endcaps Get the EE Digis
  if(ECALType_ == "EE" || ECALType_ == "EA") {
    Handle< EEDigiCollection > pEEDigis;
    try {
      iEvent.getByLabel( digiProducer_, EEdigiCollection_, pEEDigis);
      EEDigis = pEEDigis.product(); // get a ptr to the product
      need = EEDigis->size();
    } catch ( exception& ex ) {
      cout <<" EcalPedestal " << EEdigiCollection_ << " not available" << endl;
    } //getting endcap digis
  }  // endcaps

  // Loop over xtals
  //------------------
  
  int gainId=0;                                
  int SM = -999;
  int SMold = -999;
  int CrystalinSM = 0;
  int ChannelOK = 0;
  int gainPb = 0;
  vector<int>::iterator result;
  if(nebd != 0) {
    nevent[0]++;
    if(gainIdRef == 1) nevent[1]++;  // gain 12
    if(gainIdRef == 2) nevent[2]++;  // gain 6
    if(gainIdRef == 3) nevent[3]++;  // gain 1
    for (EBDigiCollection::const_iterator digiItr = EBDigis->begin(); digiItr != EBDigis->end(); ++digiItr) {
      EBDataFrame df( *digiItr );
      int ieta = df.id().ieta();
      int iphi = df.id().iphi();
      SM = df.id().ism();      // Get SM number (from 1 to 36)
      int iChannel = df.id().hashedIndex();      // here iChannel runs from 0 to 61200
      if(SM != SMold) {
	if(cnt_evt_ == 1) {
	  if(SMold != -999) cout << " SM " << SMold << " Number of read channels " << CrystalinSM << endl;
	  cout << " new SM " << SM << " channel " << iChannel
	       << " eta = " << ieta << " phi = " << iphi << endl;
	}
	SMold = SM;
	CrystalinSM = 0;
      }
      CrystalinSM++;
      gainId = df.sample(0).gainId();
    //    cout << " gain " << gainId << endl;
      if(gainId != gainIdRef) {
	gainPb++;
	if(gainPb == 1) cout << " gain problem for channel " << iChannel
			     << " gainIdRef " << gainIdRef << " gainId " << gainId << endl;
      }

      result = find(gain0Channels_.begin(), gain0Channels_.end(), iChannel);
      if (result == gain0Channels_.end()) {
	if(gainId <= 0 || gainId > 3) {
	  cout << "crystal "<< iChannel  << " eta " << ieta << " phi " << iphi
	       << " gain " << gainId  << " for event " << ievt << endl;
	  continue;
	}
      }
      else {  // known channel with gain problem
	if (gainId == 0) continue;
	else {
	  cout << "EcalCosmics::analyze: Strange: channel " << iChannel
	       << " has a right gainId " << gainId
	       << " eta = " << ieta
	       << " phi = " << iphi
	       << " for event " << cnt_evt_ << endl;
	}
      }

      //no less than 10 samples readout
      nSample = digiItr->size();
      //    cout << " nsample " << nSample << endl;
      if (nSample != 10)   {
	cout << " produce: Warning: N samples " << nSample
	     << " at eta = " << ieta << " and phi = " << iphi
	     << " for event " << ievt << endl;
	continue;
      }

      bool GainIsOK = 1;
      double prepulse = 0;
      double adc[10];
      int gainIdSample[10];
      int adcmin = 5000;
      double smax = 0;
      int adcmax = -1;
      for(int iSample = 0; iSample < nSample; iSample++) {
	int myadc = df.sample(iSample).adc();
	adc[iSample] = (double) myadc;
	if(adcmin > myadc) adcmin = myadc;
	if(adcmax < myadc) {
	  smax = iSample;
	  adcmax = myadc;
	}
	if(iSample < 3) prepulse += adc[iSample];
	//      cout << " sample " <<iSample << " adc " << adc << " prepulse " << prepulse << endl;
	gainIdSample[iSample] = df.sample(iSample).gainId();

      //      if(!iEvent.id().event()%100) // pour ne pas avoir le message pour tous les evts
      //	if(!detId.ic()%100) // pour ne pas avoir le message pour tous les cristaux
	if(gainIdSample[iSample] != gainId) GainIsOK = 0;
      }  // loop over iSample
      //      if(iChannel%10000 == 0) {
      //	int minmax = adcmax - adcmin;
      //	cout << " Channel " << iChannel << " minMax " << minmax << endl;
      //      }
      if(cnt_evt_ == 1 && !GainIsOK) {
      // LogWarning("Gain") << "Gain has changed in signal : event not used for gain ratios.";
      //	  cout << "Gain has changed in signal : event not used for gain ratios." << endl;
	cout << " gain change for EB channel " << iChannel << " " ;
	for(int iSample = 0; iSample < nSample; iSample++) 
	  cout << gainIdSample[iSample] << " " << adc[iSample] << " ";
	cout << endl;
     //	  fTxtOutput << "WARNING : Gain has changed in signal : event not used for gain ratios." << endl;
      }
      prepulse /= 3.;
      hPeak->Fill(smax);
      double ampliDQM = adcmax - prepulse;
      hXtalAmplitudeDQM[iChannel][gainId-1]->Fill(ampliDQM);
    //    cout << " prepulse " << prepulse << endl;
      if(GainIsOK) {
	ChannelOK++;
	if(gainId>0 && gainId < kGains+1) vNGainMesures[gainId-1] += 1;
	for(int iSample=0; iSample < nSample; iSample++) {
	  pXtalPulse[iChannel][gainId-1]->Fill(iSample,adc[iSample]);
	  //	pXtalPulse[detId.ic()-1][gainIdSample-1]->Fill(iSample, (*digiItr).sample(iSample).adc() - prepulse);
	}
	entries_[iChannel][gainId-1]++;
      }
      else continue;

      if(doPedes) {
	pedestal_[iChannel][gainId - 1] += prepulse;
	if(iChannel == 0) {
	  cout << "EB gainId " << gainId ;
	  for(int iSample = 0; iSample < nSample; iSample++)
	    cout << " " << adc[iSample];
	  cout << " pedestal " << prepulse << endl;
	}
      }

      // Reconstruction avec methode des poids
      //---------------------------------------
      //070411    double pedestal=0;
      double amplitude = 0;
      //    double adc = 0;
      //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
      if(useWeights && GainIsOK) {
	for(int iSample=0; iSample < nSample; iSample++) {
	  //	adc = df.sample(iSample).adc();
	  //070411	pedestal += weightsP[detId.ic()-1][iGainForWeights][iSample]*(adc-prepulse);
	  //fTxtOutput <<pedestal<<"+="<<weightsP[detId.ic()-1][gainId-1][iSample]<<"*"<<adc<<endl;
	  if(gainId == 1)
	    amplitude += weightsA[iChannel][iGainForWeights][iSample]*(adc[iSample] - prepulse);
	  else
	    amplitude += weightsA6[iChannel][iGainForWeights][iSample]*
	      (adc[iSample] - pedestal_[iChannel][gainId - 1]);
	  //fTxtOutput <<amplitude<<"+="<<weightsA[detId.ic()-1][gainId-1][iSample]<<"*"<<adc<<endl;
	  //	if(detId.ic() == 1) cout << weightsA[detId.ic()-1][gainId-1][iSample] << " " << adc << endl;
	}  // loop over samples
	if(gainId == 1) {
	  if(iChannel == 325) {
	    //	    cout << adc << " " << weightsA[iChannel][iGainForWeights][iSample] << " ";
	    hXal325->Fill(adc[4]);
	    hXal3252->Fill(adc[5]);
	    hXal325b->Fill(adc[5], adc[3]);
	  }
	}
	//      if(detId.ic() == 100) cout  << "gain " << gainId << " amp " << amplitude << endl;
	//      if(gainId == 1 && detId.ic() == 325) cout << amplitude << endl;
	//	if(detId.ic() == 1) cout << " amp " << amplitude << endl;
	//fTxtOutput <<"Xtal "<<detId.ic()<<" "<<gainId<<" "<<prepulse<<"="<<pedestal<<" "<<amplitude<<endl;
 
	//      EcalUncalibratedRecHit aHit(detId, amplitude, pedestal, 0, 0);
	//      EBuncalibRechits->push_back( aHit );
	hXtalAmplitude[iChannel][gainId-1]->Fill(amplitude);
	if(iChannel == 22139 || iChannel == 59791) {
	  cout << "EB Channel "<< iChannel << " gainId " << gainId ;
	  for(int iSample = 0; iSample < nSample; iSample++)
	    cout << " " << adc[iSample];
	  cout << " amp " << amplitude << endl;
	}
	if(iChannel == 0) {
	  cout << "EB gainId " << gainId ;
	  for(int iSample = 0; iSample < nSample; iSample++)
	    cout << " " << adc[iSample];
	  cout << " amp " << amplitude << endl;
	}
      } //if(useWeights && GainIsOK)
   
    } // loop over digis
    if(cnt_evt_ == 1)
      cout << " SM " << SM << " Number of read channels " << CrystalinSM << endl;
    cout << " number of good channels " << ChannelOK << " last gain " << gainId  << endl;
    if(gainPb != 0) cout << " Nb of gain problems : " << gainPb << endl;
  }  //  Barrel digis present

  /*********************************************************************************************************/
  // Loop over Ecal endcap digis
  if(need != 0) {
    nevent[5]++;
    if(gainIdRef == 1) nevent[6]++;  // gain 12
    if(gainIdRef == 2) nevent[7]++;  // gain 6
    if(gainIdRef == 3) nevent[8]++;  // gain 1
    gainPb = 0;
    ChannelOK = 0;
    if(cnt_evt_ == 1) cout << " Gain changed on channels : ";
    for (EEDigiCollection::const_iterator digiItr = EEDigis->begin(); digiItr != EEDigis->end(); ++digiItr) {
      int iChannel = EEDetId((*digiItr).id()).hashedIndex();      // here iChannel runs from 0 to 14647
      if(iChannel >= kEEChannels) {
	cout << " ****** Error ****** EE channel " << iChannel << endl;
	continue;
      }
      int iz = EEDetId((*digiItr).id()).zside();
      int ix = EEDetId((*digiItr).id()).ix();
      int iy = EEDetId((*digiItr).id()).iy();
      EEDataFrame df(*digiItr);
      gainId = df.sample(0).gainId();
      //    cout << " gain " << gainId << endl;
      if(gainId <= 0 || gainId > 3) {
	cout << "crystal "<< iChannel  << " side " << iz << " x " << ix << " y " << iy
	     << " gain " << gainId  << " for event " << ievt << endl;
	continue;
      }
      if (gainId != gainIdRef) {
	gainPb++;
	if(gainPb == 1) cout << " gain problem for EE channel " << iChannel
			     << " gainIdRef " << gainIdRef << " gainId " << gainId << endl;
      }

      //no less than 10 samples readout
      int nSample = digiItr->size();
      //    cout << " nsample " << nSample << endl;
      if (nSample != kSamples)   {
	cout << " produce: Warning: N samples " << nSample
	     << " at side " << iz << " x " << ix << " y " << iy
	     << " for event " << ievt << endl;
	continue;
      }

      bool GainIsOK = 1;
      double prepulse = 0;
      double adc[10];
      int gainIdSample[10];
      int adcmin = 5000;
      int adcmax = -1;
      double smax = 0;
      for(int iSample = 0; iSample < nSample; iSample++) {
	int myadc = df.sample(iSample).adc();
	adc[iSample] = (double) myadc;
	if(adcmin > myadc) adcmin = myadc;
	if(adcmax < myadc) {
	  adcmax = myadc;
	  smax = iSample;
	}
	if(iSample < 3) prepulse += adc[iSample];
	gainIdSample[iSample] = df.sample(iSample).gainId();

	if(gainIdSample[iSample] != gainId) GainIsOK = 0;
      }  // loop over iSample

      if(cnt_evt_ == 1 && !GainIsOK) {
	cout << " gain change for EE channel " << iChannel << " " ;
	for(int iSample = 0; iSample < nSample; iSample++) 
	  cout << gainIdSample[iSample] << " " << adc[iSample] << " ";
	cout << endl;
      }
      prepulse /= 3.;
      double ampliDQM = adcmax - prepulse;
      hXtalAmplitudeDQMEE[iChannel][gainId-1]->Fill(ampliDQM);
      hPeakEE->Fill(smax);
      //    cout << " prepulse " << prepulse << endl;
      if(GainIsOK) {
	ChannelOK++;
	if(gainId>0 && gainId < kGains+1) vNGainMesures[gainId - 1] += 1;
	for(int iSample=0; iSample < nSample; iSample++) {
	  pXtalPulseEE[iChannel][gainId - 1]->Fill(iSample,adc[iSample]);
	}
	entriesEE_[iChannel][gainId - 1]++;
      }
      else continue;

      if(doPedes) {
	pedestalEE_[iChannel][gainId - 1] += prepulse;
	if(iChannel == 0) {
	  cout << "EE gainId " << gainId ;
	  for(int iSample = 0; iSample < nSample; iSample++)
	    cout << " " << adc[iSample];
	  cout << " pedestal " << prepulse << endl;
	}
      }

      // Reconstruction avec methode des poids
      //---------------------------------------
      //070411    double pedestal=0;
      double amplitude = 0;
      //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
      if(useWeights && GainIsOK) {
	for(int iSample=0; iSample < nSample; iSample++) {
	  //	adc = df.sample(iSample).adc();
	  //070411	pedestal += weightsP[detId.ic()-1][iGainForWeights][iSample]*(adc-prepulse);
	  //fTxtOutput <<pedestal<<"+="<<weightsP[detId.ic()-1][gainId-1][iSample]<<"*"<<adc<<endl;
	  if(gainId == 1)
	    amplitude += weightsEEA[iChannel][iGainForWeights][iSample]*(adc[iSample] - prepulse);
	  else
	    amplitude += weightsEEA6[iChannel][iGainForWeights][iSample]*
	      (adc[iSample] - pedestalEE_[iChannel][gainId - 1]);
	  //fTxtOutput <<amplitude<<"+="<<weightsA[detId.ic()-1][gainId-1][iSample]<<"*"<<adc<<endl;
	  //	if(detId.ic() == 1) cout << weightsA[detId.ic()-1][gainId-1][iSample] << " " << adc << endl;
	}  // loop over samples
	//      if(detId.ic() == 100) cout  << "gain " << gainId << " amp " << amplitude << endl;
	//      if(gainId == 1 && detId.ic() == 325) cout << amplitude << endl;
	//	if(detId.ic() == 1) cout << " amp " << amplitude << endl;
	//fTxtOutput <<"Xtal "<<detId.ic()<<" "<<gainId<<" "<<prepulse<<"="<<pedestal<<" "<<amplitude<<endl;
 
	//      EcalUncalibratedRecHit aHit(detId, amplitude, pedestal, 0, 0);
	//      EBuncalibRechits->push_back( aHit );
	hXtalAmplitudeEE[iChannel][gainId-1]->Fill(amplitude);
	if(iChannel == 674 || iChannel == 13079 ||  iChannel == 3598) {
	  cout << "EE Channel "<< iChannel << " gainId " << gainId ;
	  for(int iSample = 0; iSample < nSample; iSample++)
	    cout << " " << adc[iSample];
	  cout << " amp " << amplitude << endl;
	}
      } //if(useWeights && GainIsOK)
   
    }  // loop over digis
    if(cnt_evt_ == 1) cout << endl;
    cout << " number of good EE channels " << ChannelOK << " gain " << gainIdRef  << endl;
    if(gainPb != 0) cout << " Nb of gain problems : " << gainPb << endl;
  }  //  End cap digis present

}
//define this as a plug-in
DEFINE_FWK_MODULE(TestPulse);
