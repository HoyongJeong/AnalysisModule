/*
 * Software developement for WASA-at-COSY
 * (c) 2007 The WASA-at-COSY Collaboration
 * List of authors:
 *        P.P         IKP, Jülich
 *
 * License: see COPYRIGHT file
 */

/*!
  \file   dpi.cc
  \brief  

  \author I Ciepal
  \date   2012-11
*/

#include "TROOT.h"

#include "Wasa.hh"
#include "SorterConfig.hh"
#include "CLog.hh"
#include "Client.hh"

int main(int argc, char** argv)
{
	gROOT -> SetBatch();

	gSorterConfig -> ReadCmdLine(argc,argv);

	gLog.SetLogThresholds(CLog::kMessage, CLog::kMessage);
  
	// Register Sorter ID
	Wasa::Initialize("jedi");
	gWasa -> AddAnalysis("JediPolAnalysis", "JediPolAnalysis");
	gWasa -> Run();

	delete gWasa;
}



