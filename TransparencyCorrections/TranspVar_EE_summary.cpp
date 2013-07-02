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
      //     time1st = 1325462400, // unix time for 1st 2012 Monday (Jan 2nd) 00:00:00
      time1st = 1333324800, // unix time for 2012 April 2 00:00:00 first Monday with useable data
      //      time1st = 1356998400, // unix time for 2013 Jan 1 00:00:00
      week = 604800  // seconds in a week
    };
    std::vector<int>ringEE_;
    std::vector<int> ch_status_;

  };

}

cond::LaserValidation::LaserValidation():Utilities("cmscond_list_iov") {
        addConnectOption();
        addAuthenticationOptions();
        addOption<bool>("verbose","v","verbose");
        addOption<bool>("all","a","list all tags (default mode)");
        addOption<bool>("summary","s","stprint also the summary for each payload");
        addOption<cond::Time_t>("beginTime","b","begin time (first since) (optional)");
        addOption<cond::Time_t>("endTime","e","end time (last till) (optional)");
        addOption<bool>("doPlot","p","Produce some plots for the selected interval (optional)");
        addOption<std::string>("tag","t","list info of the specified tag");
        addOption<std::string>("geom","g","geometry file (default: detid_geom.dat)");
        addOption<std::string>("output","o","output file (default: ecallaserplotter.root)");
        addOption<int>("niov","n","number of IOV");
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
  int timeWeek = 325462400; // unix time for 1st 2012 Monday (Jan 2nd) 00:00:00
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

    std::string geom = hasOptionValue("geom") ? getOptionValue<std::string>("geom") : "detid_geom.dat";
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

    //    since = std::max((cond::Time_t)2, cond::timeTypeSpecs[iov.timetype()].beginValue); // avoid first IOV
    //    till  = std::min(till,  cond::timeTypeSpecs[iov.timetype()].endValue);
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
    //    time_t iov_first = (iov.begin() + 1)->since()>>32;
    //    time_t iov_last  = (iov.end() - 1)->since()>>32;
    char buf[256];
    strftime(buf, sizeof(buf), "%F %R:%S", gmtime(&iov_first));
    long int tmin = (long int) iov_first;
    printf("First IOV: %li (%s UTC)\n", tmin, buf);
    strftime(buf, sizeof(buf), "%F %R:%S", gmtime(&iov_last));
    long int tmax = (long int) iov_last;
    printf("Last IOV: %li (%s UTC)\n", tmax, buf);

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
    //    std::ofstream fTwoWeek;

    TProfile** hEERing = new TProfile*[22];
    TProfile** hEERingWeek = new TProfile*[22];
    TProfile** hEEError = new TProfile*[22];
    TH1F** hEEErrorProj = new TH1F*[22];
    //    TProfile** hEERingTwoWeek = new TProfile*[22];
    TProfile** hEEChanError = new TProfile*[22];
    TH1F** hEEChanErrorProj = new TH1F*[22];
    double EERingWeekMean[22], EERingWeekRMS[22], EERingPreviousWeek[22], 
      EEChanWeekMean[kEEChannels], EEChanWeekRMS[kEEChannels], EEChanPreviousWeek[kEEChannels];
    // EERingTwoWeekMean[22], EERingTwoWeekRMS[22];
    int NbValWeek[22], NbValChanWeek[kEEChannels];
    // NbValTwoWeek[22];

    // make some rounding...
    Int_t xmin = tmin/10000;
    xmin = (xmin - 1) * 10000;	//xmin = Tmin but rounding, for histogram.
    Int_t xmax = tmax/10000;
    xmax = (xmax + 1) * 10000;
    std::cout << " histo xmin " << xmin << " xmax " << xmax  << std::endl;
    for (int ring = 0; ring < 22; ring++) {
      if(ring < 11) {
	int ringNb = ring + 18;
	hEERing[ring] = new TProfile(Form("EERing_%i",ring),
				     Form("EE- Ring %i",ringNb),10000, xmin, xmax);
	hEERingWeek[ring] = new TProfile(Form("EERingWeek_%i",ring),
					 Form("EE- Ring %i",ringNb),10000, xmin, xmax);
	hEEError[ring] = new TProfile(Form("EEError_%i",ring),
				      Form("EE- Error Ring %i",ringNb),10000, xmin, xmax);
	hEEErrorProj[ring] = new TH1F(Form("EEErrorProj_%i",ring),
				      Form("EE- Error Ring %i",ringNb),100, -0.3, 0.3);
	//	hEERingTwoWeek[ring] = new TProfile(Form("EERingTwoWeek_%i",ring),
	//				     Form("EE- Ring %i",ringNb),8000, 0., 8000.);
	//				     Form("EE- Ring %i",ringNb),10000, 0., 10000.);
	hEEChanError[ring] = new TProfile(Form("EEChanError_%i",ring),
					  Form("EE- Error Ring %i",ringNb),10000, xmin, xmax);
	hEEChanErrorProj[ring] = new TH1F(Form("EEChanErrorProj_%i",ring),
					  Form("EE- Error Ring %i",ringNb),100, -0.3, 0.3);
      }
      else {
	int ringNb = ring + 7;
	hEERing[ring] = new TProfile(Form("EERing_%i",ring),
				     Form("EE+ Ring %i",ringNb),10000, xmin, xmax);
	hEERingWeek[ring] = new TProfile(Form("EERingWeek_%i",ring),
					 Form("EE+ Week Ring %i",ringNb),10000, xmin, xmax);
	hEEError[ring] = new TProfile(Form("EEError_%i",ring),
				     Form("EE+ Error Ring %i",ringNb),10000, xmin, xmax);
	hEEErrorProj[ring] = new TH1F(Form("EEErrorProj_%i",ring),
				      Form("EE+ Error Ring %i",ringNb),100, -0.3, 0.3);
	//	hEERingTwoWeek[ring] = new TProfile(Form("EERingTwoWeek_%i",ring),
	//				     Form("EE+ TwoWeek Ring %i",ringNb),8000, 0., 8000.);
	//				     Form("EE+ TwoWeek Ring %i",ringNb),10000, 0., 10000.);
	hEEChanError[ring] = new TProfile(Form("EEChanError_%i",ring),
					  Form("EE+ Error Ring %i",ringNb),10000, xmin, xmax);
	hEEChanErrorProj[ring] = new TH1F(Form("EEChanErrorProj_%i",ring),
					  Form("EE+ Error Ring %i",ringNb),100, -0.3, 0.3);
      }
      EERingWeekMean[ring] = 0.;
      EERingWeekRMS[ring] = 0.;
      EERingPreviousWeek[ring] = 0.;
      //      EERingTwoWeekMean[ring] = 0.;
      //      EERingTwoWeekRMS[ring] = 0.;
      NbValWeek[ring] = 0;
      //      NbValTwoWeek[ring] = 0;
    } //  loop over rings
    for (int chan = 0; chan < kEEChannels; chan++) {
      EEChanWeekMean[chan] = 0;
      EEChanWeekRMS[chan] = 0;
      NbValChanWeek[chan] = 0;
      EEChanPreviousWeek[chan] = 0.;
    }
    // checks
    TH2F** hEEGeometry = new TH2F*[2];
    TH2F** hEEStatus = new TH2F*[2];
    for(int iz = 0; iz < 2; iz++) {
      int izz = iz;
      if(iz == 0) izz = -1;
      hEEGeometry[iz] = new TH2F(Form("EEOcupancy_%i",iz),Form("Endcaps ring check side %i", izz),
				   100, 1., 101., 100, 1.,101.);
      hEEStatus[iz] = new TH2F(Form("EEStatus_%i",iz),Form("Endcaps bad channels side %i", izz),
				   100, 1., 101., 100, 1.,101.);
    }

    ch_status_.resize(kEEChannels, 0);
    int cnt = 0, usedcnt = 0;
    long int start_time = 0;
    int SecWeek = 0;

    //    EcalLaserPlotter lp(geom.c_str());
      /*********************************************************************************************/
     /***********************           main loop over all IOVs         ***************************/
    /*********************************************************************************************/
    long int thisWeekTime = time1st;
    long int nextWeekTime = thisWeekTime + week;
    std::cout << "since \t till \t payloadToken" << std::endl;
    for (cond::IOVProxy::const_iterator ita = iov.begin(); ita != iov.end() - 2; ++ita) {
      if (ita == iov.begin()) continue;
      time_t iov_time = ita->since()>>32;
      long int curr_time = static_cast<int> (iov_time);
      if(curr_time < time1st) continue; // only 2012 data
      if(curr_time > 1356998400) break; // unix time for 2013 Jan 1 00:00:00
      cnt++;
      if (start_time == 0) start_time = curr_time;
      //      if(cnt%100 == 1) {
      //	strftime(buf, sizeof(buf), "%F %R:%S", gmtime(&iov_time));
      //	std::cout << cnt << " " << ita->since() << " (UTC) " << buf << " -> " << ita->till() << "\n";
      //      }
      if(curr_time >=  nextWeekTime) {   // 1 elasped week
	struct tm * timeinfo;
	timeinfo = gmtime(&iov_time);
	string stringtime = asctime(timeinfo);
	std::cout << " cnt " << cnt << " strt " << thisWeekTime << " curr " << curr_time << " stime " << stringtime;
	std::ostringstream oss;
	long int full_time = static_cast<long int> (ita->since());

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
	/*
	string sub1;
	if(stringtime.substr(8,1) == " ")
	  sub1 = stringtime.substr(4,3) + "_0" + stringtime.substr(9,1)  + "_" + stringtime.substr(11,8) ;
	else
	  sub1 = stringtime.substr(4,3) + "_" + stringtime.substr(8,2)  + "_" + stringtime.substr(11,8);
	string fname = "weekly" + sub1    std::string result = oss.str();
	*/
	string fname = "weekly_EE_summary_" + oss.str();
	fWeek.open(fname.c_str());
	SecWeek++;
	/*
	if(SecWeek == 2) {
	  fname = "biweekly" + sub1;
	  fTwoWeek.open(fname.c_str());
	}
	*/
	for (int ring = 0; ring < 22; ring++) {
	  if(NbValWeek[ring] == 0) {
	    std::cout << " No entry for ring " << ring << "\n";
	    EERingWeekMean[ring] = 0.;
	    EERingWeekRMS[ring] = 0.;
	  }
	  else {
	    EERingWeekMean[ring] /= (double)NbValWeek[ring];
	    double x = EERingWeekMean[ring];
	    EERingWeekRMS[ring] /= (double)NbValWeek[ring];
	    double rms = sqrt(EERingWeekRMS[ring] - x * x);
	  //	  hEERingWeek[ring]->Fill(cnt, x);
	  //	  hEERingWeek[ring]->Fill(cnt, x - rms);
	  //	  hEERingWeek[ring]->Fill(cnt, x + rms);
	    hEERingWeek[ring]->Fill(curr_time, x);
	    hEERingWeek[ring]->Fill(curr_time, x - rms);
	    hEERingWeek[ring]->Fill(curr_time, x + rms);
	    EERingPreviousWeek[ring] = x;
	  /*
 	  if(SecWeek == 2) {
	    //	    fTwoWeek.open(Form("biweekly_%s",sub1));
	    if(NbValTwoWeek[ring] <0) {
	      std::cout << " TwoWeek No entry for ring " << ring << "\n";
	      exit(-1);
	    }
	    double x = EERingTwoWeekMean[ring] / (double)NbValTwoWeek[ring];
	    EERingTwoWeekRMS[ring] /= (double)NbValTwoWeek[ring];
	    double rms = sqrt(EERingTwoWeekRMS[ring] - x * x);
	    //	    fTwoWeek << ring << " " << NbValWeek[ring] << " " << x << " " << rms << "\n"; 
	    hEERingTwoWeek[ring]->Fill(cnt, x);
	    hEERingTwoWeek[ring]->Fill(cnt, x - rms);
	    hEERingTwoWeek[ring]->Fill(cnt, x + rms);
	    EERingTwoWeekMean[ring] = 0.;
	    EERingTwoWeekRMS[ring] = 0.;
	    NbValTwoWeek[ring] = 0;
	  }  // 2 elasped weeks
	  */
	  }
	}  //  loop over rings
	for (int iChannel = 0; iChannel < kEBChannels; iChannel++) {
	  EBDetId ebId = EBDetId::unhashIndex(iChannel);
	  fWeek << ebId.rawId() << " 1.0" << "\n";
	}
	for (int iChannel = 0; iChannel < kEEChannels; iChannel++) {
	  EEDetId eeId = EEDetId::unhashIndex(iChannel);
	  int ring;
	  if(iChannel < 7324) ring = - 18 - ringEE_[iChannel];
	  else ring = ringEE_[iChannel] - 7;
	  if(ring < 0 || ring > 21) std::cout << " channel " << iChannel << " ring " << ring << "\n";
	  fWeek << eeId.rawId() << " " << EERingWeekMean[ring] << "\n";

	  if(NbValChanWeek[iChannel] == 0) {
	    EEChanWeekMean[iChannel] = 0.;
	    EEChanWeekRMS[iChannel] = 0.;
	  }
	  else {
	    EEChanWeekMean[iChannel] /= (double)NbValChanWeek[iChannel];
	    double x = EEChanWeekMean[iChannel];
	    EEChanWeekRMS[iChannel] /= (double)NbValChanWeek[iChannel];
	    //	    double rms = sqrt(EEChanWeekRMS[iChannel] - x * x);
	    EEChanPreviousWeek[iChannel] = x;
	  }
	}  //  loop over EE channels

	for (int ring = 0; ring < 22; ring++) {
	  EERingWeekMean[ring] = 0.;
	  EERingWeekRMS[ring] = 0.;
	  NbValWeek[ring] = 0;
	}
	for (int iChannel = 0; iChannel < kEEChannels; iChannel++) {
	  EEChanWeekMean[iChannel] = 0.;
	  EEChanWeekRMS[iChannel] = 0.;
	  NbValChanWeek[iChannel] = 0;
	}
	thisWeekTime += week;
	nextWeekTime += week;
	fWeek.close();
	/*
	if (SecWeek == 2) {
	  //	  fTwoWeek.close();
	  SecWeek = 0;
	}
	*/
      }  // 1 elasped week
      boost::shared_ptr<A> pa = session.getTypedObject<A>(ita->token());
      //      lp.fill(*pa, (time_t)ita->since()>>32);
      
      if(curr_time < nextWeekTime - 108000       // do not use the last 30 h due to prompt tag
	 //	 && curr_time > thisWeekTime + 259200   // do not use the first 3 days for test
	 && curr_time > nextWeekTime - 194400   // only use 1 day for test
	 ) {
	usedcnt++;
	Int_t ch_status_thisIov = 0;
	for (int iChannel = 0; iChannel < kEEChannels; iChannel++) {
	  vector<int>::iterator result;
	  result = find(BadChannels.begin(), BadChannels.end(), iChannel);
	  if (result == BadChannels.end()) {
	    EEDetId eeId = EEDetId::unhashIndex(iChannel);
	    int ring;
	    if(iChannel < 7324) ring = - 18 - ringEE_[iChannel];
	    else ring = ringEE_[iChannel] - 7;
	    if(ring < 0 || ring > 21) std::cout << " channel " << iChannel << " ring " << ring << "\n";
	    
	    // check
	    if(usedcnt == 1) {
	      int iz = eeId.zside();
	      int izz = iz;
	      int ringDraw = ring - 10;
	      if(iz == -1) {
		izz = 0;
		ringDraw = ring + 1;
	      }
	      int ix = eeId.ix();
	      int iy = eeId.iy();
	      hEEGeometry[izz]->Fill(ix, iy, ringDraw);
	    }

	    EcalLaserAPDPNRatios::EcalLaserAPDPNRatiosMap::const_iterator itAPDPN;
	    itAPDPN = pa->getLaserMap().find(eeId);
	    float p2 = (*itAPDPN).p2;
	    //	if(cnt%100 == 1 && iChannel%5000 == 0) std::cout << "EE channel " << iChannel 
	    //       	 << " p1 " << (*itAPDPN).p1 << " p2 " << p2 << " p3 " << (*itAPDPN).p3 << std::endl;
	    if(p2 == 1. || p2 == 0. || p2 > 1.1) {
	      ch_status_[iChannel]++;
	      ch_status_thisIov++;
	    }
	    else {
	      //	    if(p2 > 1.1) std::cout << " Channel " << iChannel << " p2 " << p2 << "\n";
	      //	    hEERing[ring]->Fill(cnt, p2);
	      hEERing[ring]->Fill(curr_time, p2);
	      EERingWeekMean[ring] += p2;
	      EERingWeekRMS[ring] += p2 * p2;
	      //	  EERingTwoWeekMean[ring] += p2;
	      //	  EERingTwoWeekRMS[ring] += p2 * p2;
	      NbValWeek[ring]++;
	      //	  NbValTwoWeek[ring]++;
	      EEChanWeekMean[iChannel] += p2;
	      EEChanWeekRMS[iChannel] += p2 * p2;
	      NbValChanWeek[iChannel]++;
	    }
	  }  // not a BadChannel
	}  // loop over EE channels
	if(ch_status_thisIov > 10000) 
	  cout << " cnt " << cnt << " nb of bad channels " << ch_status_thisIov << endl;
      }   //  IOV before Saturday 18:00
      /*      else {   // check IOVs after Saturday 18:00
      	strftime(buf, sizeof(buf), "%F %R:%S", gmtime(&iov_time));
      	std::cout << " Do not use " << ita->since() << " (UTC) " << buf << "\n";
	} */
      for (int iChannel = 0; iChannel < kEEChannels; iChannel++) {
	vector<int>::iterator result;
	result = find(BadChannels.begin(), BadChannels.end(), iChannel);
	if (result == BadChannels.end()) {
	  EEDetId eeId = EEDetId::unhashIndex(iChannel);
	  int ring;
	  if(iChannel < 7324) ring = - 18 - ringEE_[iChannel];
	  else ring = ringEE_[iChannel] - 7;
	  if(EERingPreviousWeek[ring] != 0) {
	    EcalLaserAPDPNRatios::EcalLaserAPDPNRatiosMap::const_iterator itAPDPN;
	    itAPDPN = pa->getLaserMap().find(eeId);
	    float p2 = (*itAPDPN).p2;
	    double x = EERingPreviousWeek[ring] - p2;
	    hEEError[ring]->Fill(curr_time, x);
	    hEEErrorProj[ring]->Fill(x);
	  }  //  valid previous week value
	  if(EEChanPreviousWeek[iChannel]  != 0) {
	    EcalLaserAPDPNRatios::EcalLaserAPDPNRatiosMap::const_iterator itAPDPN;
	    itAPDPN = pa->getLaserMap().find(eeId);
	    float p2 = (*itAPDPN).p2;
	    double x = EEChanPreviousWeek[iChannel] - p2;
	    hEEChanError[ring]->Fill(curr_time, x);
	    hEEChanErrorProj[ring]->Fill(x);
	  }  //  valid previous week value
	}   // not a bad channel
      }    // loop over EE channels
      if (niov > 0 && cnt >= niov) break;
    }
      /*********************************************************************************************/
     /********************        end  main loop over all IOVs       ******************************/
    /*********************************************************************************************/

    //    lp.save(output.c_str());
    TFile fRoot("TranspVar_EE_summary.root","RECREATE");
    for (int ring = 0; ring < 22; ring++) {
      hEERing[ring]->Write();
      hEERingWeek[ring]->Write();
      hEEError[ring]->Write();
      hEEErrorProj[ring]->Write();
      //      hEERingTwoWeek[ring]->Write();
      hEEChanError[ring]->Write();
      hEEChanErrorProj[ring]->Write();
    }
    hEEGeometry[0]->Write();
    hEEGeometry[1]->Write();
    std::cout << " Nb of entries " << cnt << " Used ones " << usedcnt << "\n";

    int NbBadP = 0, NbBadM = 0;
    for (int iChannel = 0; iChannel < kEEChannels; iChannel++) {
      if(ch_status_[iChannel] > 0)  {
	EEDetId eeId = EEDetId::unhashIndex(iChannel);
	int iz = eeId.zside();
	int izz = iz;
	if(iz == -1) izz = 0;
	int ix = eeId.ix();
	int iy = eeId.iy();
	hEEStatus[izz]->Fill(ix, iy, ch_status_[iChannel]);
	if(ch_status_[iChannel] > cnt/10) {
	  if(iz > 0) NbBadP++;
	  else NbBadM++;
	  std::cout << " Channel " << iChannel << " z " << iz << " x " << ix << " y " << iy
		    << " Nb bad entries " << ch_status_[iChannel] << "\n";
	}
      }
    }  // loop over EE channels
    hEEStatus[0]->Write();
    hEEStatus[1]->Write();
    fRoot.Close();
    std::cout << " Nb of bad channel EE- " << NbBadM << " EE+ " << NbBadP << "\n";

    transaction.commit();
  } // get the tag
  return 0;
}

int main( int argc, char** argv ) {
  cond::LaserValidation valida;
  return valida.run(argc,argv);
}

