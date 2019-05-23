// Sorter headers
#include "CDataManager.hh"
#include "CParameterManager.hh"
#include "CHistoManager.hh"

//#include <fstream>
//#include "Wasa.hh"
//#include "CConst.hh"
//#include "TRandom.h"
//#include "CLog.hh"
//#include "WCluster.hh"
//#include "FPCTracksVH.hh"
//#include "WTrack.hh"
//#include "WVertex.hh"
//#include "WHitScint.hh"
//#include <TString.h>
//#include "WTrackFinder.hh"
//#include "TStyle.h"
//#include "TBits.h"
//#include "EmsEvent.hh"
//#include "TMinuit.h"
//#include "LVDSSetup.hh"
//#include "TRandom.h"
//
//#include "TFile.h"
//#include "TTree.h"
//#include "TGraphErrors.h"

#include "JediPolAnalysis.hh"
#include "TreeTrack.hh"

// Global
const char *cLayerNames[8]  = {"FWC1", "FWC2", "FTH", "FRH1", "FRH2", "FRH3", "FRH4", "FRH5"};
const int iLayerElements[8] = {    24,     24,    48,     24,     24,     24,     24,     24};
Int_t nLayers;
Int_t *iSelectedLayers; // array to store numbers for selected layers

ClassImp(JediPolAnalysis);

// Sorter options
SorterOption o1("verbose","-v","verbose level", 0);

////////////////////////////////////////////////////////////////////////////////
//   Constructors
////////////////////////////////////////////////////////////////////////////////
JediPolAnalysis::JediPolAnalysis()
{
	m_Verbose = o1.GetIntValue();
	m_PeakTimeCut = kFALSE;

	// Debugging message
	if ( m_Verbose > 0 )
	{
		std::cout << "\033[1;32m";
		std::cout << "[Analysis] Analysis module constructed";
		std::cout << "\033[0m" << std::endl;
	}
}

JediPolAnalysis::JediPolAnalysis(const char* name): CAnalysisModule(name)
{
	m_Verbose = o1.GetIntValue();
	m_PeakTimeCut = kFALSE;

	// Debugging message
	if ( m_Verbose > 0 )
	{
		std::cout << "\033[1;32m";
		std::cout << "[Analysis] Analysis module constructed";
		std::cout << "\033[0m" << std::endl;
	}

	TString option = GetName();

	TString nameFWC = "FWCHitBankRaw";
	TString nameFTH = "FTHHitBankRaw";
	TString nameFRH = "FRHHitBankRaw";

	// FWC hits bank
	m_FWCHitBank = dynamic_cast<WHitBank*> (gDataManager -> GetDataObject("WHitBank", nameFWC));
	if ( m_FWCHitBank == 0 )
	{
		gLog(CException::kNullPointer) << "No FWCHitBank" << CLog::endl;
		return;
	}

	// FTH hits bank
	m_FTHHitBank = dynamic_cast<WHitBank*> (gDataManager -> GetDataObject("WHitBank", nameFTH));
	if ( m_FTHHitBank == 0 )
	{
		gLog(CException::kNullPointer) << "No FTHHitBank" << CLog::endl;
		return;
	}

	// FRH hits bank
	m_FRHHitBank = dynamic_cast<WHitBank*> (gDataManager -> GetDataObject("WHitBank", nameFRH));
	if ( m_FRHHitBank == 0 )
	{
		gLog(CException::kNullPointer) << "No FRHHitBank" << CLog::endl;
		return;
	}

	// FPC hits Bank
	m_FPCTrackBank = 0;
	m_FPCTrackFinder = dynamic_cast<FPCTracksVH*> (gDataManager -> GetAnalysisModule("FPCTracksVH", "Default"));
	if ( m_FPCTrackFinder == 0 )
	{
		gLog(CException::kNullPointer) << "No FPCTrackFinder" << CLog::endl;
		return;
	}
	m_FPCTrackBank = m_FPCTrackFinder -> GetTrackBank();
  
	// Header       
	m_Header = dynamic_cast<REventHeader*> (gDataManager -> GetDataObject("REventHeader", "Header"));
	if ( m_Header == 0 )
	{
		gLog(CException::kNullPointer) << "No Header" << CLog::endl;
		return;
	}

	// LVDS Setup
	m_Setup = dynamic_cast<LVDSSetup*> (gParameterManager -> GetParameterObject("LVDSSetup", "default"));
	if ( m_Setup == 0 )
	{
		gLog(CException::kNotSupported) << "EmsFilterLVDS: no LVDSSetup object returned." << CLog::endl;
		return;
	}

	m_OffsetTQDC = 0;
	m_OffsetTGPX = 0;
	const Double_t* delays = m_Setup -> GetDelayTable();
	if ( delays )
	{
		m_OffsetTQDC = delays[0];
		m_OffsetTGPX = delays[1];
	}
    
	m_Tracks = new TClonesArray("TreeTrack", 100);

	TString tName = SorterOption::GetStringValue("Name");
	tName += "-tree.root";

	m_File = new TFile(tName, "RECREATE");
	m_Tree = new TTree("data", "data"); 
	m_Tree -> Branch("EBeam"    , &m_EBeam    );
	m_Tree -> Branch("Tar"      , &m_Tar      );
	m_Tree -> Branch("Pol"      , &m_Pol      );
	m_Tree -> Branch("RunNo"    , &m_RunNo    );
	m_Tree -> Branch("TimeH"    , &m_TimeH    );
	m_Tree -> Branch("TimeCycle", &m_TimeCycle);
	m_Tree -> Branch("Trg"      , &m_Trg      );
	m_Tree -> Branch("Tracks"   , &m_Tracks   );
	m_Tree -> Branch("NTracks"  , &m_NTracks, "N/I");

	SetupSpectra(option);
	Clear();

	NotifyEndOfRun;
	NotifyBeginOfRun;

	// Peak timing
	// These number came from TrackRaw histograms
	// Updated by pDBR
	SetTimeQDCPeak(0, 350.132, 3.96652);
	SetTimeQDCPeak(1, 349.644, 3.99857);
	SetTimeQDCPeak(2, 333.131, 4.09288);
	SetTimeQDCPeak(3, 316.306, 4.48555);
	SetTimeQDCPeak(4, 316.032, 4.59732);
	SetTimeQDCPeak(5, 314.950, 4.71489);
	SetTimeQDCPeak(6, 307.575, 4.91127);
	SetTimeQDCPeak(7, 301.646, 5.88486);
	SetDeltaTimePeak(0, 49.8063, 1.70202);
	SetDeltaTimePeak(1, 49.0219, 1.47979);
	SetDeltaTimePeak(2, 38.9390, 1.48241);
	SetDeltaTimePeak(3, 23.0867, 1.94918);
	SetDeltaTimePeak(4, 23.6273, 1.99007);
	SetDeltaTimePeak(5, 24.1357, 1.84463);
	SetDeltaTimePeak(6, 29.8861, 4.00191);
	SetDeltaTimePeak(7, 31.0727, 4.20800);
}

////////////////////////////////////////////////////////////////////////////////
//   Destructor
////////////////////////////////////////////////////////////////////////////////
JediPolAnalysis::~JediPolAnalysis()
{
	// Debugging message
	m_Verbose = o1.GetIntValue();
	if ( m_Verbose > 0 )
	{
		std::cout << "\033[1;32m";
		std::cout << "[Analysis] Analysis module destructed";
		std::cout << "\033[0m" << std::endl;
	}

	if ( m_File )
	{
		m_File -> Write();
		m_File -> Close();
	}
}

////////////////////////////////////////////////////////////////////////////////
//   Begin of run
////////////////////////////////////////////////////////////////////////////////
void JediPolAnalysis::BeginOfRun(Int_t run)
{
	// Debugging message
	m_Verbose = o1.GetIntValue();
	if ( m_Verbose > 0 )
	{
		std::cout << "\033[1;32m";
		std::cout << "[Analysis::BeginOfRun] Begin of run";
		std::cout << "\033[0m" << std::endl;
	}
}

////////////////////////////////////////////////////////////////////////////////
//   End of run
////////////////////////////////////////////////////////////////////////////////
void JediPolAnalysis::EndOfRun(Int_t run)
{
	// Debugging message
	m_Verbose = o1.GetIntValue();
	if ( m_Verbose > 0 )
	{
		std::cout << "\032[1;33m";
		std::cout << "[Analysis::EndOfRun] End of run";
		std::cout << "\033[0m" << std::endl;
	}

	Clear();
}

////////////////////////////////////////////////////////////////////////////////
//   Clear
////////////////////////////////////////////////////////////////////////////////
void JediPolAnalysis::Clear(Option_t* option)
{
	// Debugging message
	m_Verbose = o1.GetIntValue();
	if ( m_Verbose > 1 )
	{
		std::cout << "\033[1;33m";
		std::cout << "[Analysis::Clear] Method called";
		std::cout << "\033[0m" << std::endl;
	}

	fProcessed  = kFALSE;
	m_EBeam     = -1;
	m_Tar       = -1;
	m_Pol       = -1;
	m_RunNo     = -1;
	m_TimeH     = -1;
	m_TimeCycle = -1;
	m_Trg       = -1;
	m_NTracks   =  0;
	if ( m_Tracks ) m_Tracks -> Clear();
}

////////////////////////////////////////////////////////////////////////////////
//   Process event
////////////////////////////////////////////////////////////////////////////////
void JediPolAnalysis::ProcessEvent()
{
	// Debugging message
	m_Verbose = o1.GetIntValue();
	if ( m_Verbose > 1 )
	{
		std::cout << "\033[1;33m";
		std::cout << "[Analysis::ProcessEvent] An event processing";
		std::cout << "\033[0m" << std::endl;
	}

	if ( fProcessed ) return;
	fProcessed = kTRUE;

	if ( m_Header -> IsScalerEvent() ) return;

	m_TimeH     = m_Header -> GetTimeH();
	m_TimeCycle = m_Header -> GetTimeInCycle();
	m_Trg       = m_Header -> GetTrigger();
	m_Pol       = m_Header -> GetPolarisationState();
	m_NTracks   = 0;

	Bool_t goodTrack = kFALSE;

	WHitIter it(m_FTHHitBank);
	while ( WHitScint* hit = static_cast<WHitScint*> (it.Next()) )
	{
		TrackReco(hit, m_FWCHitBank, m_FRHHitBank, m_FPCTrackBank, m_NTracks, goodTrack);
		if ( goodTrack ) m_NTracks++;
	}
//	std::cout << m_NTracks << std::endl;
	m_HNTracks -> Fill(m_NTracks); 

	if ( m_Tree && m_NTracks > 0 ) m_Tree -> Fill();

	// Raw Info
	TrackRaw(m_FWCHitBank, 1, 1);
	TrackRaw(m_FWCHitBank, 2, 2);
	TrackRaw(m_FTHHitBank, 3, 1);
	for ( Int_t p = 0; p < 5; p++ )
	{
		TrackRaw(m_FRHHitBank, p + 4, p + 1);
	}

	Clear();
	return;
}


////////////////////////////////////////////////////////////////////////////////
//   Set TDC and QDC time peaks
////////////////////////////////////////////////////////////////////////////////
void JediPolAnalysis::SetTimeQDCPeak(Int_t layer, Double_t peakTime, Double_t dev)
{
	m_TimeQDCPeak[layer] = peakTime;
	m_TimeQDCPeakDev[layer] = dev;
}

void JediPolAnalysis::SetDeltaTimePeak(Int_t layer, Double_t peakTime, Double_t dev)
{
	m_DeltaTimePeak[layer] = peakTime;
	m_DeltaTimePeakDev[layer] = dev;
}

////////////////////////////////////////////////////////////////////////////////
//   Setup spectra
////////////////////////////////////////////////////////////////////////////////
void JediPolAnalysis::SetupSpectra(const char* path)
{
	TString rootPath = path;
	TString subPath = Form("/Tracks/");

	m_HFailReas = new TH1D("FailReas", "FailReas", 11, -0.5, 10.5);
	gHistoManager -> Add(m_HFailReas, rootPath + subPath);
	
	m_HNTracks  = new TH1D("NTracks" , "NTracks" , 11, -0.5, 10.5);
	gHistoManager -> Add(m_HNTracks , rootPath + subPath);

	TString name;
	for ( Int_t p = 0; p < 8; p++ )
	{
		name = Form("TimeADC_Plane%d"     , p);
		m_HTimeADC[p]       = new TH1D(name, name, 500,   0., 500.);
		gHistoManager -> Add(m_HTimeADC[p],         rootPath + subPath);

		name = Form("TimeADCvsEl_Plane%d"  , p);
		m_HTimeADCvsEl[p]   = new TH2D(name, name, 48, 0.5, 48.5, 500,    0., 500.);
		gHistoManager -> Add(m_HTimeADCvsEl[p],   rootPath + subPath);

		name = Form("TimeDelta_Plane%d"    , p);
		m_HTimeDelta[p]     = new TH1D(name, name, 500, -100, 100.);
		gHistoManager -> Add(m_HTimeDelta[p],       rootPath + subPath);

		name = Form("TimeDeltavsEl_plane%d", p);
		m_HTimeDeltavsEl[p] = new TH2D(name, name, 48, 0.5, 48.5, 500, -100., 100.);
		gHistoManager -> Add(m_HTimeDeltavsEl[p], rootPath + subPath);
	}
}

////////////////////////////////////////////////////////////////////////////////
//   Raw info
////////////////////////////////////////////////////////////////////////////////
void JediPolAnalysis::TrackRaw(WHitBank* hitBank, Int_t planeGlobal, Int_t plane)
{
	Int_t    el    = 0.;
	Int_t    pl    = 0.;
	Double_t delta = 0.;
	WHitIter it(hitBank);
	while ( WHitScint* hit = static_cast<WHitScint*> (it . Next()) ) // Loop over hits
	{
		el = hit -> ID() . GetEl();
		pl = hit -> ID() . GetPlane();
		if ( pl == plane )
		{
			WHitScintRaw* rawHit = static_cast<WHitScintRaw*> (hit);
			QDCRawInfo* rawInfo = dynamic_cast<QDCRawInfo*> (rawHit -> GetQDCRawInfo());

			// QDC time
			Int_t nADCPulses = rawInfo -> GetNumOfPulses();
			for ( Int_t iADCHit = 0; iADCHit < nADCPulses; iADCHit++ ) // Loop over ADC pulses
			{
				Float_t timeADCTemp = rawInfo -> GetZCTime(iADCHit);
				m_HTimeADC[planeGlobal - 1] -> Fill(timeADCTemp);
				m_HTimeADCvsEl[planeGlobal - 1] -> Fill(el, timeADCTemp);
				// TDC time
				Int_t nTDCHitsLR = hit -> GetNTimes();
				Double_t timeTDCTemp = - 10000;
				for ( Int_t iTDCHit = 0; iTDCHit < nTDCHitsLR; iTDCHit++ )
				{
					timeTDCTemp = hit -> GetT(iTDCHit);
					delta = (timeADCTemp+m_OffsetTQDC)*m_ScaleTQDC - (timeTDCTemp+m_OffsetTGPX)*m_ScaleTDC;
					m_HTimeDelta[planeGlobal - 1]     -> Fill(delta);
					m_HTimeDeltavsEl[planeGlobal - 1] -> Fill(el, delta);
				}
			}
		}
	}

	return;
}

////////////////////////////////////////////////////////////////////////////////
//   Track reconstruction
////////////////////////////////////////////////////////////////////////////////
void JediPolAnalysis::TrackReco(WHitScint* FTHHit, WHitBank* FWCHitBank, WHitBank* FRHHitBank,
                                WTrackBank* FPCTrackBank, Int_t iTrack, Bool_t &isGoodTrack)
{
	// Initialize
	isGoodTrack = kTRUE;
	Int_t     El =   0;
	Double_t ADC = - 1;
	Double_t TDC = - 1;
	for ( Int_t i = 0; i < 8; i++ )
	{
		m_El[i]  = - 1;
		m_ADC[i] = - 1;
		m_TDC[i] = - 1;
	}


	if ( HitInfo(FTHHit, 3, ADC, TDC) )
	{
		El = FTHHit -> ID() . GetEl();
		m_El[2]  = El;
		m_ADC[2] = ADC;
		m_TDC[2] = TDC;
	}
	else
	{
		if ( m_Verbose > 1 )
		{
			std::cout << "\033[1;33m";
			std::cout << "[Analysis::TrackReco] No valid FTH hit";
			std::cout << "\033[0m" << std::endl;
		}
		isGoodTrack = kFALSE;
		m_HFailReas -> Fill(0);
		return;
	}

	WHitScint* hit1;
	WHitScint* hit2;
	WHitScint* hit;

	ADC = - 1;
	TDC = - 1;

	Int_t El1 = - 1;
	Int_t El2 = - 1; 
 
	//////////////////////////////////////////////////
	//   FWC1
	//////////////////////////////////////////////////
	if ( El % 2 == 0 ) // Even case
	{
		if ( El == 48 ) El2 = 1;
		else El2 = El/2 + 1;
		El1 = El / 2;
	}
	else               // Odd case
	{
		if ( El == 1 ) El2 = 24; 
		else El2 = (El+1)/2 - 1;
		El1 = (El+1) / 2;
	}
	hit1 = dynamic_cast<WHitScint*> (FWCHitBank -> GetHit(1, El1));
	hit2 = dynamic_cast<WHitScint*> (FWCHitBank -> GetHit(1, El2)); 
  
	if ( hit1 )
	{
		hit = hit1;
		m_El[0] = El1;
	}
	else if ( hit2 )
	{
		hit = hit2;
		m_El[0] = El2;
	}
	else
	{
		if ( m_Verbose > 1 )
		{
			std::cout << "\033[1;33m";
			std::cout << "[Analysis::TrackReco] No FWC1 match";
			std::cout << "\033[0m" << std::endl;
			WHitIter it(FWCHitBank);
			while ( WHitScint* hit = static_cast<WHitScint*> (it.Next()) )
			{
				Int_t elt = hit -> ID().GetEl();
				Int_t pl  = hit -> ID().GetPlane();
				if ( pl == 1 )
				{
					std::cout << "  FWC pl: " << pl << " elt: " << elt;
					std::cout << " el1: "     << El1 << " el2: " << El2 << std::endl;
				}
			}
		}
		m_HFailReas -> Fill(1);
		isGoodTrack = kFALSE;
		return;
	}
 
	if ( HitInfo(hit, 1, ADC, TDC) )
	{
		m_ADC[0] = ADC;
		m_TDC[0] = TDC;
	}
	else
	{
		if ( m_Verbose > 1 )
		{
			std::cout << "\033[1;33m";
			std::cout << "TrackReco: No valid FWC1 hit";
			std::cout << "\033[0m" << std::endl;
		}
		isGoodTrack = kFALSE;
		m_HFailReas -> Fill(1);
		return;
	}

	//////////////////////////////////////////////////
	//   FWC2
	//////////////////////////////////////////////////
	if ( El % 2 == 0 ) // Even case
	{
		if ( El == 48 ) El1 = 1;
		else El1 = (El+2) / 2;
		El2 = El / 2;
	}
	else               // Odd case
	{
		if ( El == 47 ) El2 = 1; 
		else El2 = (El+3) / 2;
		El1 = (El+1) / 2;
	}
	hit1 = dynamic_cast<WHitScint*> (FWCHitBank -> GetHit(2, El1));
	hit2 = dynamic_cast<WHitScint*> (FWCHitBank -> GetHit(2, El2)); 
  
	if ( hit1 )
	{
		hit = hit1;
		m_El[1] = El1;
	}
	else if ( hit2 )
	{
		hit = hit2;
		m_El[1] = El2;
	}
	else
	{
		if ( m_Verbose > 1 )
		{
			std::cout << "\033[1;33m";
			std::cout << "[Analysis::TrackReco] No FWC2 match";
			std::cout << "\033[0m" << std::endl;
		}
		isGoodTrack = kFALSE;
		m_HFailReas -> Fill(2);
		return;
	}

	if ( HitInfo(hit, 2, ADC, TDC) )
	{
		m_ADC[1] = ADC;
		m_TDC[1] = TDC;
	}
	else
	{
		if ( m_Verbose > 1 )
		{
			std::cout << "\033[1;33m";
			std::cout << "[Analysis::TrackReco] No valid FWC2 hit";
			std::cout << "\033[0m" << std::endl;
		}
		isGoodTrack = kFALSE;
		m_HFailReas -> Fill(2);
		return;
	}

	//////////////////////////////////////////////////
	//   FRH
	//////////////////////////////////////////////////
	if ( El % 2 == 0 ) // Even case
	{
		if ( El == 48 ) El2 = 1;
		else El2 = El/2 + 1;
		El1 = El / 2;
	}
	else             // Odd case
	{
		if ( El == 1 ) El2 = 24; 
		else El2 = (El+1)/2 - 1;
		El1 = (El+1) / 2;
	}

	for ( Int_t p = 1; p <= 5; p++ )
	{
		hit1 = dynamic_cast<WHitScint*> (FRHHitBank -> GetHit(p, El1));
		hit2 = dynamic_cast<WHitScint*> (FRHHitBank -> GetHit(p, El2)); 
    
		if ( hit1 )
		{
			hit = hit1;
			m_El[p + 2] = El1;
		}
		else if ( hit2 )
		{
			hit = hit2;
			m_El[p + 2] = El2;
		}
		else
		{
			if ( p == 1 )
			{
				// Reguire plane 1 of FRH
				if ( m_Verbose )
				{
					std::cout << "\033[1;33m";
					std::cout << "[Analysis::TrackReco] No FRH match plane: " << p;
					std::cout << "\033[0m" << std::endl;
				}
				isGoodTrack = kFALSE;
				m_HFailReas -> Fill(3);
				return;
			}
			else
			{
				break;
			}
		}
    
		if ( HitInfo(hit, p + 3, ADC, TDC) )
		{
			m_ADC[p+2] = ADC;
			m_TDC[p+2] = TDC;
		}
		else
		{
			if ( p == 1 )
			{
				if ( m_Verbose > 1 )
				{
					std::cout << "\033[1;33m";
					std::cout << "[Analysis::TrackReco] No valid FRH hit plane: " << p;
					std::cout << "\033[0m" << std::endl;
				}
				isGoodTrack = kFALSE;
				m_HFailReas -> Fill(3);
				return;
			}
			else
			{
				m_El[p+2] = - 1;
				break;
			}
		}
	}

	//////////////////////////////////////////////////
	//   FPC
	//////////////////////////////////////////////////
	Int_t nFPCTracks = FPCTrackBank -> GetEntries();
	if ( nFPCTracks < 1 )
	{
		if ( m_Verbose > 1 )
		{
			std::cout << "\033[1;33m";
			std::cout << "[Analysis::TrackReco] No FPC hit";
			std::cout << "\033[0m" << std::endl;
		}
		isGoodTrack = kFALSE;
		m_HFailReas -> Fill(4);
		return;
	}

	Int_t bin = El - 1;
	Double_t bestdBin = 10.;
	Double_t bestQual = 10.;
	WTrack* bestTr = 0;
	WTrackIter trIt(FPCTrackBank);
	while ( WTrack* tr = dynamic_cast<WTrack*> (trIt.Next()) )
	{
		Int_t binFPC = TMath::Floor(48. * TVector2::Phi_0_2pi(tr -> Phi()) / TMath::TwoPi());
		Double_t qual = tr -> dP() -> Theta();
		Double_t dBin = binFPC - bin;
		if      ( dBin >  23 ) dBin -= 48;
		else if ( dBin < -24 ) dBin += 48;
		dBin = fabs(dBin);
		if ( dBin > 1 || dBin > bestdBin ) continue;
		if ( dBin < bestdBin)
		{
			bestdBin = dBin;
			bestQual = qual;
			bestTr   = tr;
		}
		else if ( qual < bestQual )
		{
			bestdBin = dBin;
			bestQual = qual;
			bestTr   = tr;
		}
	}

	if ( bestTr == 0 )
	{
		if ( m_Verbose > 1 )
		{
			std::cout << "\033[1;33m";
			std::cout << "TrackReco: No FPC match";
			std::cout << "\033[0m" << std::endl;
		}
		isGoodTrack = kFALSE;
		m_HFailReas -> Fill(4);
		return;
	}

	// Reconstructed track to be saved
	m_Track = new ((*m_Tracks)[iTrack]) TreeTrack();

	m_Track -> SetTheta(bestTr -> Theta());
	m_Track -> SetPhi(bestTr -> Phi());
	m_Track -> SetTDCs(m_TDC);
	m_Track -> SetElements(m_El);
	m_Track -> SetADCs(m_ADC);

	return;
}

////////////////////////////////////////////////////////////////////////////////
//   Is it good hit or not?
////////////////////////////////////////////////////////////////////////////////
Bool_t JediPolAnalysis::HitInfo(WHitScint* hit, Int_t planeGlobal, Double_t &ADC, Double_t &TDC)
{
	ADC = - 1;
	TDC = - 1;
	Int_t TQDC = - 1;
	Bool_t isGoodTrack = kTRUE;

	Int_t accepted = - 1;
	Double_t closest = 10000.;
	Float_t timeADCTemp = - 1;  
  
	WHitScintRaw* raw = static_cast<WHitScintRaw*> (hit);
	QDCRawInfo* rawInfo = dynamic_cast<QDCRawInfo*> (raw -> GetQDCRawInfo());
	Int_t nPulses = rawInfo -> GetNumOfPulses();

	// If no Q
	if ( nPulses == 0 )
	{
		if ( m_Verbose > 1 )
		{
			std::cout << "\033[1;33m";
			std::cout << "[Analysis::HitInfo] No Q, ADC pulses = 0";
			std::cout << "\033[0m" << std::endl;
		}
		return isGoodTrack = kFALSE;
	}

	for ( Int_t iADCHit = 0; iADCHit < nPulses; iADCHit++ )
	{
		timeADCTemp = rawInfo -> GetZCTime(iADCHit);
		if ( timeADCTemp > m_TimeQDCPeak[planeGlobal-1] - 6.*m_TimeQDCPeakDev[planeGlobal-1]
		  && timeADCTemp < m_TimeQDCPeak[planeGlobal-1] + 6.*m_TimeQDCPeakDev[planeGlobal-1] )
		{
			if ( fabs(timeADCTemp - m_TimeQDCPeak[planeGlobal-1]) < closest )
			{
				closest = fabs(timeADCTemp - m_TimeQDCPeak[planeGlobal-1]);
				accepted = iADCHit;
			}
		}
	}
	if ( accepted < 0 )
	{ 
		if ( m_Verbose > 1 )
		{
			std::cout << "\033[1;33m";
			std::cout << "[Anlaysis::HitInfo] No T_QDC in the window [";
			std::cout << m_TimeQDCPeak[planeGlobal-1] - 6.*m_TimeQDCPeakDev[planeGlobal-1];
			std::cout << ", ";
			std::cout << m_TimeQDCPeak[planeGlobal-1] + 6.*m_TimeQDCPeakDev[planeGlobal-1];
			std::cout << "]";
			std::cout << "\033[0m" << std::endl;
		}
		return isGoodTrack = kFALSE;
	}
	ADC  = rawInfo -> GetQ(accepted);
	TQDC = rawInfo -> GetZCTime(accepted);
          
	// If no T
	Int_t nTDCHits = hit -> GetNTimes();
	if ( nTDCHits == 0 )
	{
		if ( m_Verbose > 1 )
		{
			std::cout << "\033[1;33m";
			std::cout << "HitInfo: No T";
			std::cout << "\033[0m" << std::endl;
		}
		return isGoodTrack = kFALSE;
	}

	accepted = - 1;
	closest = 10000.;
  
	Double_t timeTDCTemp = - 10000;
	for ( Int_t iTDCHit = 0; iTDCHit < nTDCHits; iTDCHit++ )
	{
		WHitScintRaw* raw = static_cast<WHitScintRaw*> (hit);
		timeTDCTemp = raw -> GetAllTimes()[iTDCHit];
		Double_t delta = (TQDC+m_OffsetTQDC)*m_ScaleTQDC - (timeTDCTemp+m_OffsetTGPX)*m_ScaleTDC;
		if ( delta > m_DeltaTimePeak[planeGlobal-1] - 6.*m_DeltaTimePeakDev[planeGlobal-1]
		  && delta < m_DeltaTimePeak[planeGlobal-1] + 6.*m_DeltaTimePeakDev[planeGlobal-1] )
		{
			if ( fabs(delta - m_DeltaTimePeak[planeGlobal-1]) < closest )
			{
				closest = fabs(delta - m_DeltaTimePeak[planeGlobal-1]);
				accepted = iTDCHit;
			}
		}
	}

	if ( accepted < 0 )
	{
		if ( m_Verbose > 1 )
		{
			std::cout << "\033[1;33m";
			std::cout << "[Analysis::HitInfo] No T in window" << endl;
			std::cout << "\033[0m" << std::endl;
		}
		return isGoodTrack = kFALSE;
	}

	TDC = hit -> GetT(accepted);

	return isGoodTrack;
}
