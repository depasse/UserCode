/*
Command (you are on Monday, week N+1):

 cmsenv

 TranspVar_EBEE_weekly -c frontier://FrontierProd/CMS_COND_42X_ECAL_LAS -t EcalLaserAPDPNRatios_prompt -n N-1 > week_EBEE_N.out 

Effect :   creates 3 files : week_EBEE_N.out, TranspVar_EBEE_weekly_N.root and weekly_EBEE_prepIOV where prepIOV is the first run from week N (on Monday), this file is to be introduced in DB (see EcalL1TransparencyCorrections Twiki)

week_EE_N.out gives a summary of transparency variations in EBEE during week N

Comparison :  root -l Diff_EBEE_weekly.C   shows variations in EBEE between week N and N-1
*/

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
      time1st = 1325462400, // unix time for 1st 2012 Monday (Jan 2nd) 00:00:00
      week = 604800  // seconds in a week
    };
    std::vector<int>ringEE_;
    std::vector<int> EB_status_, EE_status_;

  };

}

cond::LaserValidation::LaserValidation():Utilities("cmscond_list_iov") {
        addConnectOption();
        addAuthenticationOptions();
        addOption<bool>("verbose","v","verbose");
        addOption<bool>("summary","s","stprint also the summary for each payload");
        addOption<bool>("all","a","list all IOVs (default mode)");
        addOption<cond::Time_t>("beginTime","b","begin time (first since) (optional)");
        addOption<cond::Time_t>("endTime","e","end time (last till) (optional)");
        addOption<bool>("doPlot","p","Produce some plots for the selected interval (optional)");
        addOption<std::string>("tag","t","list info of the specified tag");
        addOption<std::string>("geom","g","geometry file (default: detid_geom.dat)");
        addOption<std::string>("output","o","output file (default: ecallaserplotter.root)");
        addOption<int>("nbweek","n","number of weeks before work");
}

cond::LaserValidation::~LaserValidation(){
}


int cond::LaserValidation::execute() {
  int nbweek = 0;
  if (hasOptionValue("nbweek")) nbweek = getOptionValue<int>("nbweek");

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
  int timeWeek = 1325462400 + nbweek * week; // unix time for 1st 2012 Monday (Jan 2nd) 00:00:00 + N weeks
  cout << " timestamp on Monday 00:00:00 " << timeWeek << endl;
  map<int, long int> RunStart;
  if(mon_runs > 0){   
    std::ofstream fRunStart;
    fRunStart.open("../../../CondTools/Ecal/python/RunStartTime");
    for(int dqmRun = 0; dqmRun < mon_runs; dqmRun++){
      unsigned long iDqmRun=(unsigned long) run_vec[dqmRun].getRunNumber();  
      Tm starttime = run_vec[dqmRun].getRunStart();
      int timeInSec = starttime.cmsNanoSeconds()>>32;
      if(timeInSec > timeWeek) { // unix time for 1st 2012 Monday (Jan 2nd) 00:00:00
	RunStart.insert(pair<int, long int>(iDqmRun, starttime.cmsNanoSeconds()));
	fRunStart << iDqmRun << " " << starttime.cmsNanoSeconds() << "\n";
      }
    }   // runs loop
    fRunStart.close();
  }     // we have the DQM info 
  /**************************************************************************************************************/

  initializePluginManager();

  bool listAll = hasOptionValue("all");
  cond::DbSession session  = openDbSession("connect", true);
  cond::DbScopedTransaction pretransaction(session);
  pretransaction.start(true);

  std::string tag = getOptionValue<std::string>("tag");
  std::cout << " tag " << tag <<std::endl;

  std::string output = hasOptionValue("output") ? getOptionValue<std::string>("output") : "ecallaserplotter.root";

  cond::MetaData metadata_svc(session);
  std::string token;
  cond::DbScopedTransaction transaction(session);
  transaction.start(true);
  transaction.commit();
  token = metadata_svc.getToken(tag);
  std::cout << " token " << token <<std::endl;

  //std::string tokenb = metadata_svc.getToken(tagb);

  //transaction.commit();

  cond::Time_t since = std::numeric_limits<cond::Time_t>::min();
  if( hasOptionValue("beginTime" )) since = getOptionValue<cond::Time_t>("beginTime");
  cond::Time_t till = std::numeric_limits<cond::Time_t>::max();
  if( hasOptionValue("endTime" )) till = getOptionValue<cond::Time_t>("endTime");

  bool verbose = hasOptionValue("verbose");

  //cond::IOVProxy iov(session, getToken(session, tag));
  cond::IOVProxy iov(session, token);
  //    IOVProxyData iov(session, token);

  since = std::max((cond::Time_t)2, cond::timeTypeSpecs[iov.timetype()].beginValue); // avoid first IOV
  till  = std::min(till,  cond::timeTypeSpecs[iov.timetype()].endValue);

  std::cout << "since: " << since << "   till: " << till << "\n";

  //    iov.range(since,till);

  //std::string payloadContainer = iov.payloadContainerName();
  const std::set<std::string> payloadClasses = iov.payloadClasses();
  std::cout<<"Tag "<<tag;
  if (verbose) std::cout << "\nStamp: " << iov.iov().comment()
			 << "; time " <<  cond::time::to_boost(iov.iov().timestamp())
			 << "; revision " << iov.iov().revision();
  std::cout <<"\nTimeType " << cond::timeTypeSpecs[iov.timetype()].name
	    <<"\nPayloadClasses:\n";
  for (std::set<std::string>::const_iterator it = payloadClasses.begin(); it != payloadClasses.end(); ++it) {
    std::cout << " --> " << *it << "\n";
  }

  //  int niov = -1;
  //  if (hasOptionValue("niov")) niov = getOptionValue<int>("niov");

  static const unsigned int nIOVS = std::distance(iov.begin(), iov.end());

  std::cout << "nIOVS: " << nIOVS << "\n";
  //    time_t iov_first = since >>32;
  //    time_t iov_last  = till >>32;
  //    std::cout << " first " << (iov.begin() + 1)->since() << " last " << (iov.end() - 1)->since() << "\n";
  time_t iov_first = (iov.begin() + 1)->since()>>32;
  time_t iov_last  = (iov.end() - 1)->since()>>32;
  char buf[256];
  strftime(buf, sizeof(buf), "%F %R:%S", gmtime(&iov_first));
  printf("First IOV: (%s UTC)\n", buf);
  strftime(buf, sizeof(buf), "%F %R:%S", gmtime(&iov_last));
  printf("Last IOV: (%s UTC)\n", buf);

  if(listAll) {
    std::cout << "since \t till \t payloadToken" << std::endl;
    int cnt = 0;
    for (cond::IOVProxy::const_iterator ita = iov.begin(); ita != iov.end(); ++ita) {
      time_t iov_time = ita->since()>>32;
      //      long int curr_time = static_cast<int> (iov_time);
      strftime(buf, sizeof(buf), "%F %R:%S", gmtime(&iov_time));
      std::cout << cnt << " " << ita->since() << " (UTC) " << buf << " -> " << ita->till() << "\n";
      cnt++;
    }
    return 0;
  }

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

  std::vector<int>BadChannels;
  std::ifstream fBadChannel;
  fBadChannel.open("badChannel");
  if(!fBadChannel.is_open()) {
    std::cout << "ERROR : cannot open file badChannel" << std::endl;
    exit (1);
  }
  while(!fBadChannel.eof()) {
    int r, status;
    fBadChannel >> r >> status;
    BadChannels.push_back(r);
  }
  fBadChannel.close();

  std::ofstream fWeek;

  TProfile*** hRing = new TProfile**[28];
  TProfile*** hRingWeek = new TProfile**[28];
  double RingWeekMean[28][2],RingWeekRMS[28][2];
  int NbValWeek[28][2];
  for (int ring = 0; ring < 28; ring++) {
    hRing[ring] = new TProfile*[2];
    hRingWeek[ring] = new TProfile*[2];
    for (int side = 0; side < 2; side++) {
      string Z = "-";
      if(side == 1) Z = "+";
      hRing[ring][side] = new TProfile(Form("Ring_%i_%i_%i",ring, side, nbweek+1),
				       Form("Side %s Ring %i",Z.c_str(), ring),300, 0., 300.);
      hRingWeek[ring][side] = new TProfile(Form("RingWeek_%i_%i_%i",ring, side, nbweek+1),
					   Form("Side %s Ring %i",Z.c_str(), ring),300, 0., 300.);
      RingWeekMean[ring][side] = 0.;
      RingWeekRMS[ring][side] = 0.;
      NbValWeek[ring][side] = 0;
    }
  }

  TProfile** hRingSide = new TProfile*[2];
  for (int side = 0; side < 2; side++) {
    string EBEE = "EBEE-";
    if(side == 1) EBEE = "EBEE+";
    hRingSide[side] = new TProfile(Form("RingSide_%i",side),Form("%s_%i",EBEE.c_str(), nbweek+1),28, 0., 28.);
  }

  EB_status_.resize(kEBChannels, 0);
  EE_status_.resize(kEEChannels, 0);
  int cnt = 0, cnt_before = 0;
  //  long int start_time = 0;

  //  long int thisWeekTime = time1st + 10 * week;
  //  long int thisWeekTime = time1st;
  long int thisWeekTime = time1st + nbweek * week;
  long int nextWeekTime = thisWeekTime + week;
  struct tm * timeinfo;
  time_t timeUTC = (time_t)thisWeekTime;
  timeinfo = gmtime(&timeUTC);
  string stringtime = asctime(timeinfo);
  std::cout << " begins " << thisWeekTime << " UTC " << stringtime;
  timeUTC = (time_t)nextWeekTime;
  timeinfo = gmtime(&timeUTC);
  stringtime = asctime(timeinfo);
  std::cout << " ends " << nextWeekTime << " UTC " << stringtime;
  long int full_time = 0;
  //  std::cout << "since \t till \t payloadToken" << std::endl;
  for (cond::IOVProxy::const_iterator ita = iov.begin(); ita != iov.end(); ++ita) {
    time_t iov_time = ita->since()>>32;
    long int curr_time = static_cast<int> (iov_time);
    //    if(curr_time >= thisWeekTime && curr_time < nextWeekTime) {   // 1 elasped week
    if(curr_time >=  nextWeekTime) {   // 1 elasped week
      timeinfo = gmtime(&iov_time);
      stringtime = asctime(timeinfo);
      std::cout << " cnt " << cnt << " curr " << curr_time << " stime " << stringtime
		<< " 1 elapse week leaving" << std::endl;
      break;
    }
    if(curr_time >= thisWeekTime) {   // 1 elasped week
      timeinfo = gmtime(&iov_time);
      stringtime = asctime(timeinfo);
      std::cout << " cnt " << cnt << " curr " << curr_time << " stime " << stringtime;
      cnt++;
      if(cnt == 1) full_time = static_cast<long int> (ita->since());
      boost::shared_ptr<A> pa = session.getTypedObject<A>(ita->token());
      for (int iChannel = 0; iChannel < kEEChannels; iChannel++) {
	vector<int>::iterator result;
	result = find(BadChannels.begin(), BadChannels.end(), iChannel);
	if (result == BadChannels.end()) {
	  EEDetId eeId = EEDetId::unhashIndex(iChannel);
	  int iz = eeId.zside();
	  int izz = iz;
	  if(iz == -1) izz = 0;
	  int ring = abs(ringEE_[iChannel]) - 1;
	  if(ring < 17 || ring > 27) std::cout << " channel " << iChannel << " ring " << ring << "\n";

	  EcalLaserAPDPNRatios::EcalLaserAPDPNRatiosMap::const_iterator itAPDPN;
	  itAPDPN = pa->getLaserMap().find(eeId);
	  float p2 = (*itAPDPN).p2;
	  //	  if(iChannel == 5000) std::cout << " p1 " << (*itAPDPN).p1 << " p2 " << p2 << " p3 " << (*itAPDPN).p3 << std::endl;
	  //	  if(p2 != 1. || (*itAPDPN).p1 != 1. || (*itAPDPN).p3 != 1.) std::cout << " p1 " << (*itAPDPN).p1 << " p2 " << p2 << " p3 " << (*itAPDPN).p3 << std::endl;
	  if(cnt%20 == 1 && iChannel%5000 == 0) std::cout << "EE channel " << iChannel 
		      << " p1 " << (*itAPDPN).p1 << " p2 " << p2 << " p3 " << (*itAPDPN).p3 << std::endl;
	  if(p2 == 1. || p2 == 0. || p2 > 1.1) {
	    EE_status_[iChannel]++;
	  }
	  else {
	    //	    if(p2 > 1.1) std::cout << " Channel " << iChannel << " p2 " << p2 << "\n";
	    hRing[ring][izz]->Fill(cnt, p2);
	  }
	  RingWeekMean[ring][izz] += p2;
	  RingWeekRMS[ring][izz] += p2 * p2;
	  NbValWeek[ring][izz]++;
	}  // not a BadChannel
      }  // loop over EE channels
      for (int iChannel = 0; iChannel < kEBChannels; iChannel++) {
	vector<int>::iterator result;
	result = find(BadChannels.begin(), BadChannels.end(), iChannel);
	if (result == BadChannels.end()) {
	  EBDetId ebId = EBDetId::unhashIndex(iChannel);
	  int ieta = ebId.ieta();  // -85:-1,1:85
	  int izz = 1;
	  if(ieta < 0) izz = 0;
	  int ring = (abs(ieta) - 1)/5;
	  if(ring < 0 || ring > 16) std::cout << " EB channel " << iChannel << " ring " << ring << "\n";

	  EcalLaserAPDPNRatios::EcalLaserAPDPNRatiosMap::const_iterator itAPDPN;
	  itAPDPN = pa->getLaserMap().find(ebId);
	  float p2 = (*itAPDPN).p2;
	  //	  if(iChannel == 5000) std::cout << " p1 " << (*itAPDPN).p1 << " p2 " << p2 << " p3 " << (*itAPDPN).p3 << std::endl;
	  //	  if(p2 != 1. || (*itAPDPN).p1 != 1. || (*itAPDPN).p3 != 1.) std::cout << " p1 " << (*itAPDPN).p1 << " p2 " << p2 << " p3 " << (*itAPDPN).p3 << std::endl;
	  if(cnt%20 == 1 && iChannel%10000 == 0) std::cout << "EB channel " << iChannel 
		      << " p1 " << (*itAPDPN).p1 << " p2 " << p2 << " p3 " << (*itAPDPN).p3 << std::endl;
	  if(p2 == 1. || p2 == 0. || p2 > 1.1) {
	    EB_status_[iChannel]++;
	  }
	  else {
	    //	    if(p2 > 1.1) std::cout << " Channel " << iChannel << " p2 " << p2 << "\n";
	    hRing[ring][izz]->Fill(cnt, p2);
	  }
	  RingWeekMean[ring][izz] += p2;
	  RingWeekRMS[ring][izz] += p2 * p2;
	  NbValWeek[ring][izz]++;
	}  // not a BadChannel
      }  // loop over EB channels
    }  // end loop over time interval
    else if(curr_time >= time1st) {  // nb of iovs since the beginning of the year
      //      timeinfo = gmtime(&iov_time);
      //      stringtime = asctime(timeinfo);
      //      std::cout << " before " << cnt_before << " curr " << curr_time << " stime " << stringtime;
      cnt_before++;
    }
    //    else {
    //      timeinfo = gmtime(&iov_time);
    //      stringtime = asctime(timeinfo);
    //      std::cout << " curr " << curr_time << " stime " << stringtime << std::endl;
    //    }
  }  // end loop over iovs
  std::cout << " Nb of entries " << " before " << cnt_before << " during this week " << cnt << "\n";
  std::ostringstream oss;
  if(full_time == 0) {
    std::cout << " no data found, give up " << std::endl;
    exit(-1);
  }
  map<int, long int>::iterator it;
  map<int, long int>::iterator itm;
  for(it = RunStart.begin(); it != RunStart.end(); it++) {
    if((*it).second > full_time) {
      std::cout << " time " << full_time 
	//		      << " before " << (*itm).second << " run " << (*itm).first
		<< " after " << (*it).second << " run " << (*it).first  << "\n";
      oss << (*it).first;
      break;
    }
    //	  itm = it;
  }
  string fname = "weekly_EBEE_" + oss.str();
  fWeek.open(fname.c_str());
  std::cout  << "***** Barrel *****     EB-                               EB+" << std::endl;
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
      hRingWeek[ring][side]->Fill(cnt, x);
      hRingWeek[ring][side]->Fill(cnt, x - rms);
      hRingWeek[ring][side]->Fill(cnt, x + rms);
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

  //    lp.save(output.c_str());
  TFile fRoot(Form("TranspVar_EBEE_weekly_%i.root", nbweek + 1),"RECREATE");
  for (int ring = 0; ring < 28; ring++) {
    for (int side = 0; side < 2; side++) {
      hRing[ring][side]->Write();
      hRingWeek[ring][side]->Write();
    }
  }
  for (int side = 0; side < 2; side++) hRingSide[side]->Write();
  fRoot.Close();

  int NbBadP = 0, NbBadM = 0;
  for (int iChannel = 0; iChannel < kEBChannels; iChannel++) {
    if(EB_status_[iChannel] > cnt/10) {
      EBDetId ebId = EBDetId::unhashIndex(iChannel);
      int ieta = ebId.ieta();
      if(ieta > 0) NbBadP++;
      else NbBadM++;
    }
  }  // loop over EB channels
  std::cout << " Nb of bad channel EB- " << NbBadM << " EB+ " << NbBadP << "\n";

  int NbEEBadP = 0, NbEEBadM = 0;
  for (int iChannel = 0; iChannel < kEEChannels; iChannel++) {
    if(EE_status_[iChannel] > cnt/10) {
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

