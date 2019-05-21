#ifndef _JediPolAnalysis_HH_
#define _JediPolAnalysis_HH_

// Standard
#include <iostream>

// Sorter headers
#include "CAnalysisModule.hh"
#include "CalibrationFDScint.hh" // Which has WHitBank, REventHeader definition
#include "FPCTracksVH.hh"
#include "FDFPCTracks.hh"
#include "LVDSSetup.hh"

// ROOT headers
#include "TFile.h"
#include "TTree.h"
#include "TString.h"
#include "TH1D.h"
#include "TH2D.h"

class TreeTrack;
class REventHeader;
class JediPolAnalysis: public CAnalysisModule
{
  public:
	JediPolAnalysis();
	JediPolAnalysis(const char* name);
	virtual ~JediPolAnalysis();

	virtual void BeginOfRun(Int_t run);
	virtual void EndOfRun(Int_t run);
	virtual void Clear(Option_t* option = "");
	virtual void ProcessEvent();

	void SetTimeQDCPeak(Int_t layer, Double_t peakTime, Double_t dev);
	void SetDeltaTimePeak(Int_t layer, Double_t peakTime, Double_t dev);
	void SetupSpectra(const char* path);
	void TrackRaw(WHitBank* hitBank, Int_t planeGlobal, Int_t plane);
	void TrackReco(WHitScint* FTHHit, WHitBank* FWCHitBank, WHitBank* FRHHitBank,
	               WTrackBank* FPCTrackBank, Int_t iTrack, Bool_t &isGoodTrack);
	Bool_t HitInfo(WHitScint* hit, Int_t planeGlobal, Double_t &ADC, Double_t &TDC);

  private:
	// Options
	Int_t m_Verbose;
	Bool_t m_PeakTimeCut;

	// Constants
	const static Double_t m_ScaleTDC  = 0.09497;
	const static Double_t m_ScaleTQDC = 1.5625 ;

	// Hit banks and info loaded at construction
	WHitBank*     m_FWCHitBank;
	WHitBank*     m_FTHHitBank;
	WHitBank*     m_FRHHitBank;
	FPCTracksVH*  m_FPCTrackFinder;
	WTrackBank*   m_FPCTrackBank;
	REventHeader* m_Header;  
	LVDSSetup*    m_Setup;
	Int_t m_OffsetTQDC;
	Int_t m_OffsetTGPX;

	// Saving result
	TFile* m_File;
	TTree* m_Tree;
	Double_t m_EBeam;
	Int_t m_Tar;
	Int_t m_Pol;
	Int_t m_RunNo;
	UInt_t m_TimeH;
	Double_t m_TimeCycle;
	Int_t m_Trg;
	TClonesArray* m_Tracks;
	Int_t m_NTracks;

	// Where are peaks
	Double_t m_TimeQDCPeak[8];
	Double_t m_TimeQDCPeakDev[8];
	Double_t m_DeltaTimePeak[8];
	Double_t m_DeltaTimePeakDev[8];

	// Spectra hists
	TH1D* m_HFailReas;
	TH1D* m_HNTracks;
	TH1D* m_HTimeADC[8];
	TH2D* m_HTimeADCvsEl[8];
	TH1D* m_HTimeDelta[8];
	TH2D* m_HTimeDeltavsEl[8];

	// Track reconstruction
	Int_t    m_El[8];
	Double_t m_ADC[8];
	Double_t m_TDC[8];
	TreeTrack* m_Track;

	ClassDef(JediPolAnalysis, 0)
};

#endif
