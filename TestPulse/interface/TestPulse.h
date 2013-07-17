#include <memory>

// user include files
#include "FWCore/Framework/interface/ESHandle.h"

#include "Geometry/EcalMapping/interface/EcalElectronicsMapping.h"
#include "Geometry/EcalMapping/interface/EcalMappingRcd.h"

#include "CaloOnlineTools/EcalTools/interface/EcalFedMap.h"

class TestPulse : public edm::EDAnalyzer {
public:
  explicit TestPulse(const edm::ParameterSet&);
  ~TestPulse();

  virtual void beginRun(edm::Run const &, edm::EventSetup const &);
  virtual void endJob();
  virtual void analyze( const edm::Event&, const edm::EventSetup& );

  enum { kEBChannels = 61200, kEBEta = 85, kEEChannels = 14648, kEEEta = 25};
  enum { kGains = 3, kFirstGainId = 1, kSamples = 10};
private:
  // ----------member data ---------------------------
  const EcalElectronicsMapping* ecalElectronicsMap_;
  EcalFedMap* fedMap_;

  int runnumber_;
  string ECALType_; // EB or EE
	  
  bool doPedes;
  bool doWeights;
  bool useWeights;

  unsigned int cnt_evt_;
  int nevent[10];
  int nGains;
  int iFirstGain;
  int nSample;
  int iGainForWeights;

  std::vector<int> maskedChannels_;
  std::vector<int> gain0Channels_;

  bool bLogFile;
  bool bSaveProfiles;
  ofstream fTxtOutput;

  string digiProducer_;
  string EBdigiCollection_;
  string EEdigiCollection_; 
  vector<int> vNGainMesures;

  double weightsA[kEBChannels][kGains][kSamples];
  double weightsA6[kEBChannels][kGains][kSamples];
  double pedestal_[kEBChannels][kGains];
  double entries_[kEBChannels][kGains];

  double weightsEEA[kEEChannels][kGains][kSamples];
  double weightsEEA6[kEEChannels][kGains][kSamples];
  double pedestalEE_[kEEChannels][kGains];
  double entriesEE_[kEEChannels][kGains];

  TH1F *hPeak;
  TH1F *hRMS[3];
  TH1F *hXal325;
  TH1F *hXal3252;
  TH2F *hXal325b;
  TH1F*** hXtalAmplitude;
  TH1F*** hXtalAmplitudeDQM;
  TProfile*** pXtalPulse;

  TH1F *hPeakEE;
  TH1F *hRMSEE[3];
  TH1F*** hXtalAmplitudeEE;
  TH1F*** hXtalAmplitudeDQMEE;
  TProfile*** pXtalPulseEE;

};
