//
// ROOT macro corrBeam.C
//
// Author : J. Blaha, 21/11/2006
//

void corrBeam()
{
  const Int_t kXtals = 1700;
  gStyle->SetTitleFontSize(0.06);
  //gStyle->SetStatFontSize(0.04);    
  gStyle->SetStatColor(10);
  gStyle->SetTitleFillColor(10);
  gStyle->SetCanvasColor(10);
  //  gStyle->SetOptStat("e");
  gStyle->SetOptStat(111111);
  gStyle->SetOptFit(1001);
  gStyle->SetPaperSize(20,26);
  //gStyle->SetStatH(0.10);  
  //gStyle->SetStatW(0.08);
  gStyle->SetStatH(0.15);  
  gStyle->SetStatW(0.25);

  TFile* fout = new TFile("corrBeam.root","recreate");

  hRatio126New  = new TH1F("hRatio126New","SM6, Gain ratio x12/x6", 
			      50, 1.85, 2.05);
  hRatio126Old = new TH1F("hRatio126Old","SM6, Gain ratio x12/x6", 
			   50, 1.85, 2.05);

  hCorr126 = new TH2F("hCorr126","SM6, Gain ratio x12/x6 corr.", 
			50,1.9, 2.02,50, 1.9, 2.02);
  
  hDiff126 = new TH1F("hDiff126","SM6, Gain ratio x12/x6 diff.", 
			50, -0.015,0.025);

  TCanvas cPrint("cPrint");
 
  //  Double_t ratio121TP[kXtals];
  Double_t ratio126New[kXtals];
  Double_t ratio121New[kXtals];
  Double_t ratio126Old[kXtals];
  Double_t ratio121Old[kXtals];
  for(Int_t iXtal = 0; iXtal < kXtals; iXtal++) {
    ratio126New[iXtal] = 0.;
    ratio126Old[iXtal] = 0.;
  }
  // Open new  ratio file
  ifstream fRatioInput;
  fRatioInput.open("TestPulse_gainratio.txt");
  if(!fRatioInput.is_open()) {
    cout  << "can't open new Ratio file " << endl;
    return;
  }

  for(Int_t iXtal = 0; !fRatioInput.eof(); iXtal++) {
    Int_t Xtal;
    fRatioInput >>  Xtal >> ratio121New[iXtal] >> ratio126New[iXtal];
    if(Xtal != iXtal + 1) cout << " pb Xtal " << Xtal << " " << iXtal << endl;
    //    cout << Xtal << " " << ratio126New[iXtal]  << endl;
  }
  fRatioInput.close();

  // Open old ratio file
  ifstream fRatioInput;
  fRatioInput.open("TestPulse_gainratio_16345_SM6.txt_old");
  if(!fRatioInput.is_open()) {
    cout  << "can't open old Ratio file " << endl;
    return;
  }
  
  for(Int_t iXtal = 0; !fRatioInput.eof(); iXtal++) {
    Int_t Xtal;
    fRatioInput >>  Xtal >> ratio121Old[iXtal] >> ratio126Old[iXtal];
    if(Xtal != iXtal + 1) cout << " pb Xtal " << Xtal << " " << iXtal << endl;
    //    cout << Xtal << " " << ratio126Old[Xtal] << endl;
  }
  fRatioInput.close();


  for(Int_t iXtal = 0; iXtal < kXtals; iXtal++) {
    if (ratio126New[iXtal] != 0) {
      hRatio126New->Fill(ratio126New[iXtal]);
      hRatio126Old->Fill(ratio126Old[iXtal]);
      hCorr126->Fill(ratio126New[iXtal], ratio126Old[iXtal]);
      hDiff126->
	  Fill((ratio126New[iXtal]-ratio126Old[iXtal])/ratio126New[iXtal]);
      //      cout << iXtal << " " << ratio126New[iXtal] << " " << ratio126Old[iXtal]
      //	   << endl;
    }
  }
  

  TLegend* legend1 = new TLegend(0.15,0.75,0.35,0.85);
  legend1->SetBorderSize(1); 
  legend1->SetFillColor(0); 
  legend1->SetTextSize(0.03); 
  legend1->SetTextFont(42);
 
  TCanvas* cRatio = new TCanvas("cRatio", "cRatio");
  cRatio->SetCanvasSize(630,885);
  cRatio->Divide(1,2);
  cRatio->cd(1);
  hRatio126New->Draw();
  gPad->Update();
  TPaveStats* st1 = 
    (TPaveStats*)hRatio126New->GetListOfFunctions()->FindObject("stats");
  st1->SetLineColor(3);
  hRatio126New->SetFillColor(3);
  hRatio126New->GetYaxis()->SetTitle("# Xtals");
  legend1->AddEntry(hRatio126New, "New","f");
  hRatio126New->Write();

  hRatio126Old->Draw("sames");
  gPad->Update();
  TPaveStats* st2 = 
    (TPaveStats*)hRatio126Old->GetListOfFunctions()->FindObject("stats");
  st2->SetLineColor(4);
  hRatio126Old->SetFillColor(4);
  hRatio126Old->GetYaxis()->SetTitle("# Xtals");
  legend1->AddEntry(hRatio126Old, "Old","f");
  legend1->Draw();
  cRatio->Modified();
  hRatio126Old->Write();
  gPad->Print("hRatio126Old.eps");

  
  cRatio->Write("cRatio");
  // cRatio->Print("corrBeam.ps(");


  TCanvas* cRatio = new TCanvas("cRatio","Gain Ratio");
  cRatio->SetCanvasSize(630,885);
  cRatio->Divide(1,2);
  cRatio->cd(1);
  hCorr126->SetMarkerStyle(20);
  hCorr126->Draw();
  hCorr126->GetXaxis()->SetTitle("New");
  hCorr126->GetYaxis()->SetTitle("Old");
  TLine* l = new TLine(1.9,1.9,2.02,2.02);
  l->SetLineColor(2);
  l->SetLineWidth(3);
  l->Draw();
  cRatio->Update();
  gPad->Print("hCorr126.eps");

  cRatio->cd(2);
  hDiff126->Draw();
  //  hDiff126->GetXaxis()->SetTitle("(Weights 6 - Weights 12)/Weights 6");
  hDiff126->GetXaxis()->SetTitle("(New - Old)/New");
  hDiff126->GetYaxis()->SetTitle("# Xtals");
  hDiff126->SetFillColor(5);
  gPad->Print("hDiff126.eps");
  cRatio->Print("corrBeam.ps");

  fout->Write();
  fout->Close();
 
  gROOT->ProcessLine(".q");
}

void solveLinear(Double_t eps = 1.e-12)
{
  cout << "Perform the fit  y = c0 + c1 * x in four different ways" << endl;

  const Int_t nrVar  = 2;
  const Int_t nrPnts = 4;

  Double_t ax[] = {0.0,1.0,2.0,3.0};
  Double_t ay[] = {1.4,1.5,3.7,4.1};
  Double_t ae[] = {0.5,0.2,1.0,0.5};

  // Make the vectors 'Use" the data : they are not copied, the vector data
  // pointer is just set appropriately

  TVectorD x; x.Use(nrPnts,ax);
  TVectorD y; y.Use(nrPnts,ay);
  TVectorD e; e.Use(nrPnts,ae);

  TMatrixD A(nrPnts,nrVar);
  TMatrixDColumn(A,0) = 1.0;
  TMatrixDColumn(A,1) = x;

  cout << " - 1. solve through Normal Equations" << endl;

  const TVectorD c_norm = NormalEqn(A,y,e);

  cout << " - 2. solve through SVD" << endl;
  // numerically  preferred method

  // first bring the weights in place
  TMatrixD Aw = A;
  TVectorD yw = y;
  for (Int_t irow = 0; irow < A.GetNrows(); irow++) {
    TMatrixDRow(Aw,irow) *= 1/e(irow);
    yw(irow) /= e(irow);
  }

  TDecompSVD svd(Aw);
  Bool_t ok;
  const TVectorD c_svd = svd.Solve(yw,ok);

  cout << " - 3. solve with pseudo inverse" << endl;

  const TMatrixD pseudo1  = svd.Invert();
  TVectorD c_pseudo1 = yw;
  c_pseudo1 *= pseudo1;

  cout << " - 4. solve with pseudo inverse, calculated brute force" << endl;

  TMatrixDSym AtA(TMatrixDSym::kAtA,Aw);
  const TMatrixD pseudo2 = AtA.Invert() * Aw.T();
  TVectorD c_pseudo2 = yw;
  c_pseudo2 *= pseudo2;

  cout << " - 5. Minuit through TGraph" << endl;

  TGraphErrors *gr = new TGraphErrors(nrPnts,ax,ay,0,ae);
  TF1 *f1 = new TF1("f1","pol1",0,5);
  gr->Fit("f1","Q");
  TVectorD c_graph(nrVar);
  c_graph(0) = f1->GetParameter(0);
  c_graph(1) = f1->GetParameter(1);

  // Check that all 4 answers are identical within a certain
  // tolerance . The 1e-12 is somewhat arbitrary . It turns out that
  // the TGraph fit is different by a few times 1e-13.

  Bool_t same = kTRUE;
  same &= VerifyVectorIdentity(c_norm,c_svd,0,eps);
  same &= VerifyVectorIdentity(c_norm,c_pseudo1,0,eps);
  same &= VerifyVectorIdentity(c_norm,c_pseudo2,0,eps);
  same &= VerifyVectorIdentity(c_norm,c_graph,0,eps);
  if (same)
    cout << " All solutions are the same within tolerance of " << eps << endl;
  else
    cout << " Some solutions differ more than the allowed tolerance of " << eps << endl;
}

