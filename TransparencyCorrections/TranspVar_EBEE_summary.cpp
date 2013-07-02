/*
Command :

 cmsenv

 TranspVar_EBEE_summary -c frontier://FrontierProd/CMS_COND_42X_ECAL_LAS -t EcalL
aserAPDPNRatios_prompt -d  frontier://FrontierProd/CMS_COND_31X_ECAL

Effect :   creates, for each week of 2012,a file EcalChannelStatusCode_TimeStampwith bad channels infos and a file weekly_EBEE_summary_runNb ; finally  TranspVar_EBEE_summary.root  to be  used with  Plot_EBEE_summary.C.

*/

#include "CondCore/Utilities/interface/Utilities.h"

#include "CondCore/DBCommon/interface/DbConnection.h"
#include "CondCore/DBCommon/interface/DbScopedTransaction.h"
#include "CondCore/DBCommon/interface/DbTransaction.h"
#include "CondCore/DBCommon/interface/Exception.h"
#include "CondCore/DBCommon/interface/Auth.h"
#include "CondCore/MetaDataService/interface/MetaData.h"

#include "CondCore/DBCommon/interface/Time.h"
#include "CondFormats/Common/interface/TimeConversions.h"

#include "CondCore/IOVService/interface/IOVProxy.h"
#include "CondFormats/EcalObjects/interface/EcalLaserAPDPNRatios.h"
#include "CondFormats/DataRecord/interface/EcalLaserAPDPNRatiosRcd.h"
#include "CondFormats/EcalObjects/interface/EcalLaserAPDPNRatiosRef.h"
#include "CondFormats/EcalObjects/interface/EcalChannelStatus.h"
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
    typedef EcalChannelStatus ECS;

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
      //      time1st = 1325462400, // unix time for 1st 2012 Monday (Jan 2nd) 00:00:00
      time1st = 1333324800, // unix time for 2012 April 2 00:00:00 first Monday with useable data
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
	//        addOption<std::string>("badChannel","b","bad channels DB account");
        addOption<int>("niov","n","number of IOV");
	addConnectOption("sourceConnect","d","source connection string");
	addSQLOutputOption();
}

cond::LaserValidation::~LaserValidation(){
}


int cond::LaserValidation::execute() {
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
  int timeWeek = 1325462400; // unix time for 1st 2012 Monday (Jan 2nd) 00:00:00
  map<int, long int> RunStart;
  if(mon_runs > 0){   
    for(int dqmRun = 0; dqmRun < mon_runs; dqmRun++){
      unsigned long iDqmRun=(unsigned long) run_vec[dqmRun].getRunNumber();  
      Tm starttime = run_vec[dqmRun].getRunStart();
      int timeInSec = starttime.cmsNanoSeconds()>>32;
      if(timeInSec > timeWeek) { // unix time for 1st 2012 Monday (Jan 2nd) 00:00:00
	RunStart.insert(pair<int, long int>(iDqmRun, starttime.cmsNanoSeconds()));
      }
    }   // runs loop
  }     // we have the DQM info 
  cout << " number of runs " << RunStart.size() << endl;
  /*************************************************************************************************/

  initializePluginManager();

  bool listAll = hasOptionValue("all");
  cond::DbSession session  = openDbSession("connect", true);
  cond::DbScopedTransaction transaction(session);
  transaction.start(true);

  if(listAll){
    cond::MetaData metadata_svc(session);
    std::vector<std::string> alltags;
    cond::DbScopedTransaction transaction(session);
    transaction.start(true);
    metadata_svc.listAllTags(alltags);
    transaction.commit();
    std::copy (alltags.begin(),
	       alltags.end(),
	       std::ostream_iterator<std::string>(std::cout,"\n")
	       );
  } else {
    std::string tag = getOptionValue<std::string>("tag");
    std::string output = hasOptionValue("output") ? getOptionValue<std::string>("output") : "ecallaserplotter.root";

    cond::MetaData metadata_svc(session);
    std::string token;
    cond::DbScopedTransaction transaction(session);
    transaction.start(true);
    transaction.commit();
    token = metadata_svc.getToken(tag);

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

    //  since = std::max((cond::Time_t)2, cond::timeTypeSpecs[iov.timetype()].beginValue); // avoid first IOV
    //  till  = std::min(till,  cond::timeTypeSpecs[iov.timetype()].endValue);
    since = std::max((cond::Time_t)time1st<<32, (iov.begin() + 1)->since()); 
    till  = std::min(till,  (iov.end() - 1)->since());

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

    int niov = -1;
    if (hasOptionValue("niov")) niov = getOptionValue<int>("niov");

    static const unsigned int nIOVS = std::distance(iov.begin(), iov.end());

    std::cout << "nIOVS: " << nIOVS << "\n";
    time_t iov_first = since >>32;
    time_t iov_last  = till >>32;
    //    std::cout << " first " << (iov.begin() + 1)->since() << " last " << (iov.end() - 1)->since() << "\n";
    //  time_t iov_first = (iov.begin() + 1)->since()>>32;
    //  time_t iov_last  = (iov.end() - 1)->since()>>32;
    char buf[256];
    strftime(buf, sizeof(buf), "%F %R:%S", gmtime(&iov_first));
    int tmin = (int) iov_first;
    printf("First IOV: %i (%s UTC)\n", tmin, buf);
    strftime(buf, sizeof(buf), "%F %R:%S", gmtime(&iov_last));
    int tmax = (int) iov_last;
    printf("Last IOV: %i (%s UTC)\n", tmax, buf);

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

    std::string sourceConnect = getOptionValue<std::string>("sourceConnect");
    cond::DbSession sourceDb = openDbSession("sourceConnect", Auth::COND_READER_ROLE, true);
    //    cond::DbScopedTransaction transactionBC(sourceDb);
    //    cout << " after DbScopedTransaction transaction" << endl;
    //    transactionBC.start(true);
    //    cout << " after transaction" << endl;
    sourceDb.transaction().start(true);
    cond::MetaData  sourceMetadata(sourceDb);
    std::string tokenBadChannel = sourceMetadata.getToken("EcalChannelStatus_v1_prompt");
    //    std::string tokenBadChannel = metadata_svc.getToken("EcalChannelStatus_v1_prompt");
    cond::IOVProxy iovBadChannel(sourceDb, tokenBadChannel);
    map<int, cond::IOVProxy::const_iterator> BadChannel;
    for (cond::IOVProxy::const_iterator ita = iovBadChannel.begin(); ita != iovBadChannel.end(); ++ita) {
      int runf = ita->since();
      //      int rune = ita->till();
      int timeBadChannel = 0;
      map<int, long int>::iterator it;
      map<int, long int>::iterator itm;
      if(runf > (*RunStart.begin()).first) {
	for(it = RunStart.begin(); it != RunStart.end(); it++) {
	  itm = std::next(it);
	  if(runf == (*it).first) {
	    timeBadChannel = (*it).second>>32;
	    break;
	  }
	  else if((runf > (*it).first && runf < (*itm).first)) {
	    timeBadChannel = (*itm).second>>32;
	    break;
	  }
	}
	cout << " BC since " << runf << " time " << timeBadChannel << endl;
	BadChannel.insert(pair<int, cond::IOVProxy::const_iterator>(timeBadChannel, ita));
      }
    }

    /*    map<int, cond::IOVProxy::const_iterator>::iterator itBC;
    cond::IOVProxy::const_iterator itIOVBC; 
    for(itBC = BadChannel.begin(); itBC != BadChannel.end(); itBC++) {
      cout << " IOV " << (*itBC).first << endl;
      itIOVBC = (*itBC).second;
      boost::shared_ptr<ECS> pb = sourceDb.getTypedObject<ECS>(itIOVBC->token());
      std::cout<< " size " << pb->size() <<std::endl;
      const std::vector<EcalChannelStatusCode> barrelItems = pb->barrelItems();
      const std::vector<EcalChannelStatusCode> endcapItems = pb->endcapItems();
      cout << std::endl << "------Barrel Items: " << std::endl;
      cout << "---Total: " << barrelItems.size() << std::endl;
      int unhashed = 0;
      for (std::vector<EcalChannelStatusCode>::const_iterator vIter = barrelItems.begin(); vIter != barrelItems.end(); ++vIter){
	unhashed++;
	unsigned int stcode = vIter->getStatusCode();
	if (stcode != 0) cout << " Xtal " << unhashed << " code " << stcode << endl;
      }

      cout << std::endl << "------Endcap Items: " << std::endl;
      cout << "---Total: " << endcapItems.size() << std::endl;
      unhashed = 0;
      for (std::vector<EcalChannelStatusCode>::const_iterator vIter = endcapItems.begin(); vIter != endcapItems.end(); ++vIter){
	unhashed++;
	unsigned int stcode = vIter->getStatusCode();
	if (stcode != 0) cout << " Xtal " << unhashed << " code " << stcode << endl;
      }
    }
    */
    std::ofstream fWeek;

    TProfile*** hRing = new TProfile**[28];
    TProfile*** hRingWeek = new TProfile**[28];
    double RingWeekMean[28][2], RingWeekRMS[28][2];
      //, RingMean[17][2], RingRMS[17][2];
    int NbValWeek[28][2];
    //, NbVal[17][2];
    for (int ring = 0; ring < 28; ring++) {
      hRing[ring] = new TProfile*[2];
      hRingWeek[ring] = new TProfile*[2];
      // make some rounding...
      Int_t xmin = tmin/10000;
      xmin = (xmin - 1) * 10000;	//xmin = Tmin but rounding, for histogram.
      Int_t xmax = tmax/10000;
      xmax = (xmax + 1) * 10000;
      for (int side = 0; side < 2; side++) {
	string Z = "-";
	if(side == 1) Z = "+";
	hRing[ring][side] = new TProfile(Form("Ring_%i_%i",ring, side),
					 Form("Side %s Ring %i",Z.c_str(), ring),10000, xmin, xmax);
	hRingWeek[ring][side] = new TProfile(Form("RingWeek_%i_%i",ring, side),
					     Form("Side %s Ring %i",Z.c_str(), ring),10000, xmin, xmax);
	RingWeekMean[ring][side] = 0.;
	RingWeekRMS[ring][side] = 0.;
	NbValWeek[ring][side] = 0;
      }
    }

    EB_status_.resize(kEBChannels, 0);
    EE_status_.resize(kEEChannels, 0);
    int cnt = 0, usedcnt = 0;
    long int start_time = 0;

    int thisWeekTime = time1st;
    int nextWeekTime = thisWeekTime + week;
    unsigned int EBBadChannels[kEBChannels], EEBadChannels[kEEChannels];
    for(int ic = 0; ic < kEBChannels; ic++)
      EBBadChannels[ic] = 0;
    for(int ic = 0; ic < kEEChannels; ic++)
      EEBadChannels[ic] = 0;
    std::cout << "since \t till \t payloadToken" << std::endl;
    //    map<int, cond::IOVProxy::const_iterator>::iterator itpresentBC = BadChannel.begin();
    map<int, cond::IOVProxy::const_iterator>::iterator itBC = BadChannel.begin();
    map<int, cond::IOVProxy::const_iterator>::iterator itBCn = std::next(itBC);
    cond::IOVProxy::const_iterator itIOVBC = (*BadChannel.begin()).second;
    // first bad channels  
    boost::shared_ptr<ECS> pb = sourceDb.getTypedObject<ECS>(itIOVBC->token());
    const std::vector<EcalChannelStatusCode> barrelItems = pb->barrelItems();
    const std::vector<EcalChannelStatusCode> endcapItems = pb->endcapItems();
    std::ofstream fEcalChannelStatusCode;
    std::ostringstream oss;
    oss << (*itBC).first;
    string fname = "EcalChannelStatusCode_" + oss.str();
    fEcalChannelStatusCode.open(fname.c_str());
    int unhashed = 0, EBbad = 0, EEbad = 0;
    for (std::vector<EcalChannelStatusCode>::const_iterator vIter = barrelItems.begin(); 
	 vIter != barrelItems.end(); ++vIter){
      EBBadChannels[unhashed] = vIter->getStatusCode();
      if(EBBadChannels[unhashed] != 0) {
	EBbad++;
	fEcalChannelStatusCode <<"EB " << unhashed << " " << EBBadChannels[unhashed] << "\n";
      }
      unhashed++;
    }
    unhashed = 0;
    for (std::vector<EcalChannelStatusCode>::const_iterator vIter = endcapItems.begin(); 
	 vIter != endcapItems.end(); ++vIter){
      EEBadChannels[unhashed] = vIter->getStatusCode();
      if(EEBadChannels[unhashed] != 0) {
	EEbad++;
	fEcalChannelStatusCode <<"EE " << unhashed << " " << EEBadChannels[unhashed] << "\n";
      }
      unhashed++;
    }
    fEcalChannelStatusCode.close();
    cout << " initial Bad Channel IOV " << (*itBC).first 
	 << " EB bad channels " << EBbad << " EE ones " << EEbad << endl;

      /*********************************************************************************************/
     /***********************           main loop over all IOVs         ***************************/
    /*********************************************************************************************/
    for (cond::IOVProxy::const_iterator ita = iov.begin(); ita != iov.end(); ++ita) {
      if (ita == iov.begin()) continue;
      time_t iov_time = ita->since()>>32;
      int curr_time = static_cast<int> (iov_time);
      if(curr_time < time1st) continue; // only 2012 data
      if(curr_time > 1356998400) break; // unix time for 2013 Jan 1 00:00:00
      cnt++;
      if (start_time == 0) start_time = curr_time;
      //      map<int, cond::IOVProxy::const_iterator>::iterator itBC;
      //      cond::IOVProxy::const_iterator itIOVBC = (*BadChannel.begin()).second; 
      /*
      for(itBC = itpresentBC; itBC != BadChannel.end(); itBC++) {
	map<int, cond::IOVProxy::const_iterator>::iterator itBCn = std::next(itBC);
	if(curr_time >= (*itBC).first && curr_time < (*itBCn).first)   {
	  cout << " new Bad Channel IOV " << (*itBC).first << endl;
	  itpresentBC = itBCn;
	  itIOVBC = (*itBC).second;
	  break;
	}
      }
      */
      if(itBCn != BadChannel.end() && curr_time >= (*itBCn).first) {
	itBC++;
	itBCn = std::next(itBC);
	itIOVBC = (*itBC).second;
	boost::shared_ptr<ECS> pb = sourceDb.getTypedObject<ECS>(itIOVBC->token());
      //	std::cout<< " size " << pb->size() <<std::endl;
	const std::vector<EcalChannelStatusCode> barrelItems = pb->barrelItems();
	const std::vector<EcalChannelStatusCode> endcapItems = pb->endcapItems();
	for(int ic = 0; ic < kEBChannels; ic++)
	  EBBadChannels[ic] = 0;
	for(int ic = 0; ic < kEEChannels; ic++)
	  EEBadChannels[ic] = 0;
	std::ofstream fEcalChannelStatusCode;
	std::ostringstream oss;
	oss << (*itBC).first;
	string fname = "EcalChannelStatusCode_" + oss.str();
	fEcalChannelStatusCode.open(fname.c_str());
	unhashed = 0; EBbad = 0; EEbad = 0;
	for (std::vector<EcalChannelStatusCode>::const_iterator vIter = barrelItems.begin(); vIter != barrelItems.end(); ++vIter){
	  EBBadChannels[unhashed] = vIter->getStatusCode();
	  if(EBBadChannels[unhashed] != 0) {
	    EBbad++;
	    fEcalChannelStatusCode <<"EB " << unhashed << " " << EBBadChannels[unhashed] << "\n";
	  }
	  unhashed++;
	}
	unhashed = 0;
	for (std::vector<EcalChannelStatusCode>::const_iterator vIter = endcapItems.begin(); vIter != endcapItems.end(); ++vIter){
	  EEBadChannels[unhashed] = vIter->getStatusCode();
	  if(EEBadChannels[unhashed] != 0) {
	    EEbad++;
	    fEcalChannelStatusCode <<"EE " << unhashed << " " << EEBadChannels[unhashed] << "\n";
	  }
	  unhashed++;
	}
	cout << " cnt " << cnt << " time " << curr_time  
	     << " new Bad Channel IOV " << (*itBC).first 
	     << " EB bad channels " << EBbad << " EE ones " << EEbad << endl;
      }
      fEcalChannelStatusCode.close();
      //      cout << std::endl << "------Barrel Items: ---Total: " << barrelItems.size() << std::endl;
	//	int unhashed = 0;
	//	for (std::vector<EcalChannelStatusCode>::const_iterator vIter = barrelItems.begin(); vIter != barrelItems.end(); ++vIter){
	//	  unhashed++;
	//	  unsigned int stcode = vIter->getStatusCode();
	//	  if (stcode != 0) cout << " Xtal " << unhashed << " code " << stcode << endl;
	//	}

      //      cout << std::endl << "------Endcap Items: ---Total: " << endcapItems.size() << std::endl;
	//	unhashed = 0;
	//	for (std::vector<EcalChannelStatusCode>::const_iterator vIter = endcapItems.begin(); vIter != endcapItems.end(); ++vIter){
	//	  unhashed++;
	//	  unsigned int stcode = vIter->getStatusCode();
	//	  if (stcode != 0) cout << " Xtal " << unhashed << " code " << stcode << endl;
	//	}
      if(curr_time >=  nextWeekTime) {   // 1 elasped week
	struct tm * timeinfo;
	timeinfo = gmtime(&iov_time);
	string stringtime = asctime(timeinfo);
	std::cout << " cnt " << cnt << " strt " << thisWeekTime << " curr " << curr_time << " stime " << stringtime;
	std::ostringstream oss;
	long int full_time = static_cast<long int> (ita->since());

	map<int, long int>::iterator it;
	//	map<int, long int>::iterator itm;
	for(it = RunStart.begin(); it != RunStart.end(); it++) {
	  if((*it).second > full_time) {
	    std::cout << " time " << full_time 
	      //		      << " before " << (*itm).second << " run " << (*itm).first
		      << " after " << (*it).second << " run " << (*it).first  << "\n";
	    oss << (*it).first;
	    break;
	  }
	}
	string fname = "weekly_EBEE_summary_" + oss.str();
	fWeek.open(fname.c_str());
	std::cout  << "***** Barrel *****     EB-                               EB+" << std::endl;
	for (int ring = 0; ring < 28; ring++) {
	  for (int side = 0; side < 2; side++) {
	    if(NbValWeek[ring][side] == 0) {
	      std::cout << "  ring " << ring + 1 << " no entry ";
	      RingWeekMean[ring][side] = 0.;
	      RingWeekRMS[ring][side] = 0.;
	    }
	    else {
	      RingWeekMean[ring][side] /= (double)NbValWeek[ring][side];
	      double x = RingWeekMean[ring][side];
	      RingWeekRMS[ring][side] /= (double)NbValWeek[ring][side];
	      double rms = sqrt(RingWeekRMS[ring][side] - x * x);
	      //	      hRingWeek[ring][side]->Fill(cnt, x);
	      //	      hRingWeek[ring][side]->Fill(cnt, x - rms);
	      //	      hRingWeek[ring][side]->Fill(cnt, x + rms);
	      hRingWeek[ring][side]->Fill(curr_time, x);
	      hRingWeek[ring][side]->Fill(curr_time, x - rms);
	      hRingWeek[ring][side]->Fill(curr_time, x + rms);
	      std::cout << " ring " << ring + 1 << " mean " << x << " rms " << rms << "    ";
	    }
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
	for (int ring = 0; ring < 28; ring++) {
	  for (int side = 0; side < 2; side++) {
	    RingWeekMean[ring][side] = 0.;
	    RingWeekRMS[ring][side] = 0.;
	    NbValWeek[ring][side] = 0;
	  }  //  loop over sides
	}
	thisWeekTime += week;
	nextWeekTime += week;
	fWeek.close();
      }  // 1 elasped week
      boost::shared_ptr<A> pa = session.getTypedObject<A>(ita->token());
      if(curr_time < nextWeekTime - 108000       // do not use the last 30 h due to prompt tag
	 //	 && curr_time > thisWeekTime + 259200   // do not use the first 3 days for test
	 //	 && curr_time > nextWeekTime - 194400   // only use 1 day for test
	 &&  curr_time > 1333554480
	 && (curr_time < 1334073775 || curr_time > 1334078081)
	 && (curr_time < 1335821950 || curr_time > 1335843739)
	 && (curr_time < 1335978988 || curr_time > 1336010000)
	 ) {
	usedcnt++;
	Int_t ch_status_thisIov = 0;
	for (int iChannel = 0; iChannel < kEEChannels; iChannel++) {
	  //	  vector<int>::iterator result;
	  //	  result = find(BadChannels.begin(), BadChannels.end(), iChannel);
	  //	  if (result == BadChannels.end()) {
	  //   is it a bad channel ?
	  /*
	  int unhashed = 0;
	  unsigned int stcode = 0;
	  for (std::vector<EcalChannelStatusCode>::const_iterator vIter = endcapItems.begin(); vIter != endcapItems.end(); ++vIter){
	    if(unhashed == iChannel) {
	      stcode = vIter->getStatusCode();
	      break;
	    }
	    unhashed++;
	  }
	  */
	  if (EEBadChannels[iChannel] == 0) {
	    EEDetId eeId = EEDetId::unhashIndex(iChannel);
	    int iz = eeId.zside();
	    int izz = iz;
	    if(iz == -1) izz = 0;
	    int ring = abs(ringEE_[iChannel]) - 1;
	    if(ring < 17 || ring > 27) std::cout << " channel " << iChannel << " ring " << ring << "\n";

	    EcalLaserAPDPNRatios::EcalLaserAPDPNRatiosMap::const_iterator itAPDPN;
	    itAPDPN = pa->getLaserMap().find(eeId);
	    float p2 = (*itAPDPN).p2;
	    //	    if(cnt%20 == 1 && iChannel%5000 == 0) std::cout << "EE channel " << iChannel 
	    //		     << " p1 " << (*itAPDPN).p1 << " p2 " << p2 << " p3 " << (*itAPDPN).p3 << std::endl;
	    if(p2 == 1. || p2 == 0. || p2 > 1.1) {
	      EE_status_[iChannel]++;
	    }
	    else {
	      //	    if(p2 > 1.1) std::cout << " Channel " << iChannel << " p2 " << p2 << "\n";
	      hRing[ring][izz]->Fill(curr_time, p2);
	      RingWeekMean[ring][izz] += p2;
	      RingWeekRMS[ring][izz] += p2 * p2;
	      NbValWeek[ring][izz]++;
	    }
	  }  // not a BadChannel
	}  // loop over EE channels
	/*
	for (int ring = 0; ring < 17; ring++) {
	  for (int side = 0; side < 2; side++) {
	    RingMean[ring][side] = 0.;
	    RingRMS[ring][side] = 0.;
	    NbVal[ring][side] = 0;
	  }
	  }
	*/
	for (int iChannel = 0; iChannel < kEBChannels; iChannel++) {
	  //   is it a bad channel ?
	  /*
	  int unhashed = 0;
	  unsigned int stcode = 0;
	  for (std::vector<EcalChannelStatusCode>::const_iterator vIter = barrelItems.begin(); vIter != barrelItems.end(); ++vIter){
	    if(unhashed == iChannel) {
	      stcode = vIter->getStatusCode();
	      break;
	    }
	    unhashed++;
	  }
	  */
	  if (EBBadChannels[iChannel] == 0) {
	    EBDetId ebId = EBDetId::unhashIndex(iChannel);
	    int ieta = ebId.ieta();  // -85:-1,1:85
	    int izz = 1;
	    if(ieta < 0) izz = 0;
	    int ring = (abs(ieta) - 1)/5;
	    if(ring < 0 || ring > 16) std::cout << " EB channel " << iChannel << " ring " << ring << "\n";

	    EcalLaserAPDPNRatios::EcalLaserAPDPNRatiosMap::const_iterator itAPDPN;
	    itAPDPN = pa->getLaserMap().find(ebId);
	    float p2 = (*itAPDPN).p2;
	    //	    if(cnt%20 == 1 && iChannel%10000 == 0) std::cout << "EB channel " << iChannel 
	    //		    << " p1 " << (*itAPDPN).p1 << " p2 " << p2 << " p3 " << (*itAPDPN).p3 << std::endl;
	    if(p2 == 1. || p2 == 0. || p2 > 1.1) {
	      EB_status_[iChannel]++;
	    }
	    else {
	      hRing[ring][izz]->Fill(curr_time, p2);
	      RingWeekMean[ring][izz] += p2;
	      RingWeekRMS[ring][izz] += p2 * p2;
	      NbValWeek[ring][izz]++;
	      //	      RingMean[ring][izz] += p2;
	      //	      RingRMS[ring][izz] += p2 * p2;
	      //	      NbVal[ring][izz]++;
	    }
	  }  // not a BadChannel
	}  // loop over EB channels
	/*   check bad data for EB
	for (int ring = 0; ring < 17; ring++) {
	  for (int side = 0; side < 2; side++) {
	    if(NbVal[ring][side] != 0) {
	      RingMean[ring][side] /= (double)NbVal[ring][side];
	      double x = RingMean[ring][side];
	      RingRMS[ring][side] /= (double)NbVal[ring][side];
	      double rms = sqrt(RingRMS[ring][side] - x * x);
	      if(curr_time < 1336100000) {
		if(x < 0.96) {
		  struct tm * timeinfo;
		  timeinfo = gmtime(&iov_time);
		  string stringtime = asctime(timeinfo);
		  cout << " time " << curr_time << " (UTC) " << stringtime
		       << " side " << side << " ring " << ring 
		       << " mean " << x << " rms " << rms << endl;
		}
		else if(side == 0 && ring == 0) {
		  struct tm * timeinfo;
		  timeinfo = gmtime(&iov_time);
		  string stringtime = asctime(timeinfo);
		  int size = stringtime.size() - 1;
		  string subtime = stringtime.substr(0, size - 1);  // remove <CR>
		  cout << " time " << curr_time << " " << subtime
		       << " side 0 ring 0 mean " << x << " rms " << rms << endl;
		}
	      }
	    }
	  }
	}
	*/
	if(ch_status_thisIov > 10000) 
	  cout << " cnt " << cnt << " nb of bad channels " << ch_status_thisIov << endl;
      }   //  IOV before Saturday 18:00
      if (niov > 0 && cnt >= niov) break;
    }
      /*********************************************************************************************/
     /********************        end  main loop over all IOVs       ******************************/
    /*********************************************************************************************/

    //    lp.save(output.c_str());
    TFile fRoot("TranspVar_EBEE_summary.root","RECREATE");
    for (int ring = 0; ring < 28; ring++) {
      for (int side = 0; side < 2; side++) {
	hRing[ring][side]->Write();
	hRingWeek[ring][side]->Write();
      }
    }
    std::cout << " Nb of entries " << cnt << " Used ones " << usedcnt << "\n";
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
  } // get the tag
  return 0;
}

int main( int argc, char** argv ) {
  cond::LaserValidation validate;
  return validate.run(argc,argv);
}

