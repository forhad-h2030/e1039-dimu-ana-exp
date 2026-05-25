R__LOAD_LIBRARY(libdimu_ana_rus)

int Fun4All(const string list_file, const string DST_out, const int n_evt=0){
	Fun4AllServer* se = Fun4AllServer::instance();
	Fun4AllInputManager *in = new Fun4AllDstInputManager("DUMMY");
	se->registerInputManager(in);
	in->AddListFile(list_file);  // text file: one spin_reco.root path per line

	DimuAnaRUS* dimuAna = new DimuAnaRUS();
	dimuAna->SetTreeName("tree");
	dimuAna->SetOutputFileName(DST_out);
	se->registerSubsystem(dimuAna);

	se->run(n_evt);
	se->End();
	se->PrintTimer();
	delete se;
	gSystem->Exit(0);
	return 0;
}
