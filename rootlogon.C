{
	/*
	gSystem->Load("libHist.so");
	gSystem->Load("libGui.so");
	gSystem->Load("libThread.so");
	gSystem->Load("libNet.so");

	gSystem->Load("libRSCore.so");
	gSystem->Load("libRSClient.so");
	gSystem->Load("libXMLParser.so");

	gSystem->Load("libRSXml.so");
	gSystem->Load("libRSGClient.so");
	*/  

	gSystem->Load("libHist.so");
	gSystem->Load("libGui.so");
	gSystem->Load("libThread.so");
	gSystem->Load("libRSCore.so");
	gSystem->Load("libNet.so");
	gSystem->Load("libRSClient.so");

	gSystem->Load("libRIO.so");
	gSystem->Load("libThread.so");
	gSystem->Load("libRSCommon.so");

	gSystem->Load("libTree.so");
	gSystem->Load("libPhysics.so");
	gSystem->Load("libRSREvent.so");
//	gSystem->Load("/usr/lib/libxerces-c.so");
	gSystem->Load("libRSXml.so");
	gSystem->Load("libXMLParser.so");
	gSystem->Load("libRSParameter.so");
	gSystem->Load("libWasaAlig.so");

	gSystem->Load("libGraf.so");
	gSystem->Load("libGpad.so");
	gSystem->Load("libWasaCommon.so");
	gSystem->Load("libWasaBanks.so");
	gSystem->Load("libWasaParameter.so");
	gSystem->Load("libWasaREvent.so");
	gSystem->Load("libRSEms.so");
	gSystem->Load("libRSGClient.so");

	gStyle->SetPalette(1,0);
//	gSClient->GetSorterList();
//	gSClient->Fetch(0);
}
