#include "CondCore/Utilities/interface/Utilities.h"

#include "CondCore/DBCommon/interface/DbConnection.h"
#include "CondCore/DBCommon/interface/DbScopedTransaction.h"
#include "CondCore/DBCommon/interface/DbTransaction.h"
#include "CondCore/DBCommon/interface/Exception.h"
#include "CondCore/MetaDataService/interface/MetaData.h"

#include "CondCore/DBCommon/interface/Time.h"
#include "CondFormats/Common/interface/TimeConversions.h"

#include "CondCore/IOVService/interface/IOVProxy.h"
#include "CondFormats/EcalObjects/interface/EcalLaserAPDPNRatios.h"
#include "CondFormats/DataRecord/interface/EcalLaserAPDPNRatiosRcd.h"
#include "CondFormats/EcalObjects/interface/EcalLaserAPDPNRatiosRef.h"
#include "CondFormats/DataRecord/interface/EcalLaserAPDPNRatiosRefRcd.h"

#include "OnlineDB/EcalCondDB/interface/MonLaserBlueDat.h"
#include "OnlineDB/EcalCondDB/interface/MonPNBlueDat.h"
#include "OnlineDB/EcalCondDB/interface/MonTimingLaserBlueCrystalDat.h"
#include "OnlineDB/EcalCondDB/interface/RunCrystalErrorsDat.h"
#include "OnlineDB/EcalCondDB/interface/RunTTErrorsDat.h"
#include "OnlineDB/EcalCondDB/interface/all_od_types.h"
#include "OnlineDB/EcalCondDB/interface/all_fe_config_types.h"
#include "OnlineDB/EcalCondDB/interface/all_monitoring_types.h"
#include "OnlineDB/Oracle/interface/Oracle.h"
#include "OnlineDB/EcalCondDB/interface/EcalCondDBInterface.h"

#include <boost/program_options.hpp>
#include <vector>
#include <iterator>
#include <iostream>
#include <fstream>

#include "TFile.h"
#include "TProfile.h"
#include "TH2.h"

using namespace std;

namespace cond {
  class LaserValidation : public Utilities {
  public:
    typedef EcalLaserAPDPNRatios A;
    typedef EcalLaserAPDPNRatios::EcalLaserAPDPNRatiosMap AMap;
    typedef EcalLaserAPDPNRatios::EcalLaserAPDPNRatiosMap::const_iterator AMapCit;
    typedef EcalLaserAPDPNRatios::EcalLaserAPDPNpair AP;
    typedef EcalLaserAPDPNRatios::EcalLaserTimeStamp AT;

    std::string getToken(cond::DbSession & s, std::string & tag) {
      s = openDbSession("connect", true);
      cond::MetaData metadata_svc(s);
      cond::DbScopedTransaction transaction(s);
      transaction.start(true);
      std::string token = metadata_svc.getToken(tag);
      transaction.commit();
      std::cout << "Source iov token: " << token << "\n";
      return token;
    }

    LaserValidation();
    ~LaserValidation();
    int execute();

  private:
    enum { 
      kChannels = 75848, kEBChannels = 61200,  kEEChannels = 14648,
    };
    std::vector<int>ringEE_;
    std::vector<int> EB_status_, EE_status_;

  };
}

cond::LaserValidation::LaserValidation():Utilities("cmscond_list_iov") {
        addConnectOption();
        addAuthenticationOptions();
        addOption<bool>("summary","s","stprint also the summary for each payload");
        addOption<bool>("doPlot","p","Produce some plots for the selected interval (optional)");
        addOption<std::string>("tag","t","list info of the specified tag");
        addOption<std::string>("geom","g","geometry file (default: detid_geom.dat)");
        addOption<std::string>("output","o","output file (default: ecallaserplotter.root)");
        addOption<int>("nbweek","n","number of weeks before work");
}

cond::LaserValidation::~LaserValidation(){
}


int cond::LaserValidation::execute() {
  //  int nbweek = 0;
  //  if (hasOptionValue("nbweek")) nbweek = getOptionValue<int>("nbweek");

  /**************************************************************************************************************/
  std::cout << "Retrieving run list from ONLINE DB ... " << std::endl;
  std::string m_sid = "cms_omds_adg";
  std::string m_user = "cms_ecal_r";
  std::string m_pass = "3c4l_r34d3r";
  EcalCondDBInterface* econn;
  econn = new EcalCondDBInterface( m_sid, m_user, m_pass );
  std::cout << "Connection done" << std::endl;
  
  if (!econn) { 
    std::cout << " Problem with OMDS: connection parameters " << m_sid << "/" << m_user << "/" << m_pass << std::endl;
    throw cms::Exception("OMDS not available");
  } 
  // these are the online conditions DB classes 
  RunList my_runlist ;
  RunTag  my_runtag;
  LocationDef my_locdef;
  RunTypeDef my_rundef;
  my_locdef.setLocation("P5_Co");
  my_rundef.setRunType("PHYSICS");  
  my_runtag.setLocationDef(my_locdef);
  my_runtag.setRunTypeDef(my_rundef);
  my_runtag.setGeneralTag("GLOBAL");   
  
  // range of validity
  unsigned int min_run = 183000;
  unsigned int max_run = 300000;
  std::cout << " min_run : "  << min_run << " max_run : "  << max_run << std::endl;

  
  // here we retrieve the Monitoring run records  
  MonVersionDef monverdef;
  monverdef.setMonitoringVersion("test01");
  MonRunTag mon_tag;
  mon_tag.setGeneralTag("CMSSW-online");        
  mon_tag.setMonVersionDef(monverdef);
  RunList my_list; 
  my_list = econn->fetchRunListByLocation(my_runtag, min_run, max_run, my_locdef);
  std::vector<RunIOV> run_vec = my_list.getRuns();
  int mon_runs = run_vec.size();
  std::cout << "number of Mon runs is " << mon_runs << std::endl;
  /**************************************************************************************************************/

  initializePluginManager();

  cond::DbSession session  = openDbSession("connect", true);
  cond::DbScopedTransaction pretransaction(session);
  pretransaction.start(true);

  std::string tag = getOptionValue<std::string>("tag");
  std::cout << " tag " << tag <<std::endl;
  //  EcalLaserAPDPNRatios_TL500_IL1E34_mc
  string runNb = tag.substr(21,13);
  std::string output = hasOptionValue("output") ? getOptionValue<std::string>("output") : "ecallaserplotter.root";

  cond::MetaData metadata_svc(session);
  std::string token;
  cond::DbScopedTransaction transaction(session);
  transaction.start(true);
  transaction.commit();
  token = metadata_svc.getToken(tag);
  std::cout << " token " << token <<std::endl;

  cond::IOVProxy iov(session, token);

  const std::set<std::string> payloadClasses = iov.payloadClasses();
  std::cout << "Tag " << tag << endl;;

  typedef unsigned int LuminosityBlockNumber_t;
  typedef unsigned int RunNumber_t;

  std::ifstream fCrystal;
  fCrystal.open("Crystal");
  if(!fCrystal.is_open()) {
    std::cout << "ERROR : cannot open file Crystal" << std::endl;
    exit (1);
  }
  for (int iChannel = 0; iChannel < kEEChannels; iChannel++) {
    int r;
    fCrystal >> r;
    ringEE_.push_back(r);
  }
  fCrystal.close();

  std::ofstream fWeek;

  TProfile** hRingSide = new TProfile*[2];
  double RingWeekMean[28][2],RingWeekRMS[28][2];
  int NbValWeek[28][2];
  for (int side = 0; side < 2; side++) {
    string EBEE = "EBEE-";
    if(side == 1) EBEE = "EBEE+";
    hRingSide[side] = new TProfile(Form("RingSide_%i",side),Form("%s",EBEE.c_str()),28, 0., 28.);
    for (int ring = 0; ring < 28; ring++) {
      RingWeekMean[ring][side] = 0.;
      RingWeekRMS[ring][side] = 0.;
      NbValWeek[ring][side] = 0;
    }
  }

  EB_status_.resize(kEBChannels, 0);
  EE_status_.resize(kEEChannels, 0);

  for (cond::IOVProxy::const_iterator ita = iov.begin(); ita != iov.end(); ++ita) {
    boost::shared_ptr<A> pa = session.getTypedObject<A>(ita->token());
    for (int iChannel = 0; iChannel < kEEChannels; iChannel++) {
      EEDetId eeId = EEDetId::unhashIndex(iChannel);
      int iz = eeId.zside();
      int izz = iz;
      if(iz == -1) izz = 0;
      int ring = abs(ringEE_[iChannel]) - 1;
      if(ring < 17 || ring > 27) std::cout << " channel " << iChannel << " ring " << ring << "\n";

      EcalLaserAPDPNRatios::EcalLaserAPDPNRatiosMap::const_iterator itAPDPN;
      itAPDPN = pa->getLaserMap().find(eeId);
      float p2 = (*itAPDPN).p2;
      if(iChannel%5000 == 0) cout << "EE channel " << iChannel 
				  << " p1 " << (*itAPDPN).p1 << " p2 " << p2 << " p3 " << (*itAPDPN).p3 << endl;
      if(p2 == 1. || p2 == 0. || p2 > 1.1) {
	EE_status_[iChannel]++;
      }
      RingWeekMean[ring][izz] += p2;
      RingWeekRMS[ring][izz] += p2 * p2;
      NbValWeek[ring][izz]++;
    }  // loop over EE channels
    for (int iChannel = 0; iChannel < kEBChannels; iChannel++) {
      EBDetId ebId = EBDetId::unhashIndex(iChannel);
      int ieta = ebId.ieta();  // -85:-1,1:85
      int izz = 1;
      if(ieta < 0) izz = 0;
      int ring = (abs(ieta) - 1)/5;
      if(ring < 0 || ring > 16) std::cout << " EB channel " << iChannel << " ring " << ring << "\n";

      EcalLaserAPDPNRatios::EcalLaserAPDPNRatiosMap::const_iterator itAPDPN;
      itAPDPN = pa->getLaserMap().find(ebId);
      float p2 = (*itAPDPN).p2;
      if(iChannel%5000 == 0) cout << "EB channel " << iChannel
				  << " p1 " << (*itAPDPN).p1 << " p2 " << p2 << " p3 " << (*itAPDPN).p3 << endl;
      if(p2 == 1. || p2 == 0. || p2 > 1.1) {
	EB_status_[iChannel]++;
      }
      RingWeekMean[ring][izz] += p2;
      RingWeekRMS[ring][izz] += p2 * p2;
      NbValWeek[ring][izz]++;
    }  // loop over EB channels
  }  // end loop over iovs
  string fname = "weekly_EBEE_mc" + runNb;
  fWeek.open(fname.c_str());
  for (int ring = 0; ring < 28; ring++) {
    for (int side = 0; side < 2; side++) {
      if(NbValWeek[ring][side] <0) {
	std::cout << " No entry for ring " << ring << "\n";
	exit(-1);
      }
      RingWeekMean[ring][side] /= (double)NbValWeek[ring][side];
      double x = RingWeekMean[ring][side];
      RingWeekRMS[ring][side] /= (double)NbValWeek[ring][side];
      double rms = sqrt(RingWeekRMS[ring][side] - x * x);
      std::cout << " ring " << ring + 1 << " mean " << x << " rms " << rms << "    ";
      hRingSide[side]->Fill(ring, x);
      hRingSide[side]->Fill(ring, x - rms);
      hRingSide[side]->Fill(ring, x + rms);
    }  //  loop over sides
    std::cout  << std::endl;
    if(ring == 16)   std::cout  << "*****  End caps *****     EE-                              EE+" << std::endl;
  }  //  loop over rings
  for (int iChannel = 0; iChannel < kEBChannels; iChannel++) {
    EBDetId ebId = EBDetId::unhashIndex(iChannel);
    int ieta = ebId.ieta();  // -85:-1,1:85
    int izz = 1;
    if(ieta < 0) izz = 0;
    int ring = (abs(ieta) - 1)/5;
    if(ring < 0 || ring > 16) std::cout << " EB channel " << iChannel << " ring " << ring + 1 << "\n";
    fWeek << ebId.rawId() << " " << RingWeekMean[ring][izz] << "\n";
  }
  for (int iChannel = 0; iChannel < kEEChannels; iChannel++) {
    EEDetId eeId = EEDetId::unhashIndex(iChannel);
    int iz = eeId.zside();
    int izz = iz;
    if(iz == -1) izz = 0;
    int ring = abs(ringEE_[iChannel]) - 1;
    if(ring < 17 || ring > 27) std::cout << " EE channel " << iChannel << " ring " << ring + 1 << "\n";
    fWeek << eeId.rawId() << " " << RingWeekMean[ring][izz] << "\n";
  }
  fWeek.close();

  TFile fRoot(Form("TranspVar_EBEE_mc_%s.root", runNb.c_str()),"RECREATE");
  for (int side = 0; side < 2; side++) hRingSide[side]->Write();
  fRoot.Close();

  int NbBadP = 0, NbBadM = 0;
  for (int iChannel = 0; iChannel < kEBChannels; iChannel++) {
    if(EB_status_[iChannel] > 0) {
      EBDetId ebId = EBDetId::unhashIndex(iChannel);
      int ieta = ebId.ieta();
      if(ieta > 0) NbBadP++;
      else NbBadM++;
    }
  }  // loop over EB channels
  std::cout << " Nb of bad channel EB- " << NbBadM << " EB+ " << NbBadP << "\n";

  int NbEEBadP = 0, NbEEBadM = 0;
  for (int iChannel = 0; iChannel < kEEChannels; iChannel++) {
    if(EE_status_[iChannel] > 0) {
      EEDetId eeId = EEDetId::unhashIndex(iChannel);
      int iz = eeId.zside();
      if(iz > 0) NbEEBadP++;
      else NbEEBadM++;
    }
  }  // loop over EE channels
  std::cout << " Nb of bad channel EE- " << NbEEBadM << " EE+ " << NbEEBadP << "\n";

  transaction.commit();
  return 0;
}

int main( int argc, char** argv ) {
  cond::LaserValidation validate;
  return validate.run(argc,argv);
}

