#!/bin/tcsh
echo $SHELL
cd /afs/cern.ch/work/d/depasse/cmssw/CMSSW_6_1_0/src/TestPulse/test
eval `scramv1 runtime -csh`
cmsRun runTestPulse_${1}_cfg.py
