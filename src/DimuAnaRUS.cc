#include <fstream>
#include <iomanip>
#include <TSystem.h>
#include <TFile.h>
#include <TTree.h>
#include <TChain.h>
#include <TH1D.h>
#include <TCanvas.h>
#include <interface_main/SQRun.h>
#include <interface_main/SQEvent.h>
#include <ktracker/SRecEvent.h>
#include <fun4all/Fun4AllReturnCodes.h>
#include <phool/getClass.h>
#include "DimuAnaRUS.h"
using namespace std;

DimuAnaRUS::DimuAnaRUS(const std::string& name)
  : SubsysReco  (name),
    m_file(0),
    m_tree(0),
    m_tree_name("tree"),
    m_file_name("output.root"),
    m_evt(0),
    m_srec(0),
    saveDimuonOnly(false),
    data_trig_mode(false),
    mc_trig_mode(true)
{
  ;
}

DimuAnaRUS::~DimuAnaRUS()
{
  ;
}

int DimuAnaRUS::Init(PHCompositeNode* startNode)
{
  return Fun4AllReturnCodes::EVENT_OK;
}

int DimuAnaRUS::InitRun(PHCompositeNode* startNode)
{
	m_file = new TFile(m_file_name.c_str(), "RECREATE");

	if (!m_file || m_file->IsZombie()) {
		std::cerr << "Error: Could not create file " << m_file_name << std::endl;
		exit(1);
	} else {
		std::cout << "File " << m_file->GetName() << " opened successfully." << std::endl;
	}

	m_tree = new TTree(m_tree_name.c_str(), "Tree for storing events");
	if (!m_tree) {
		std::cerr << "Error: Could not create tree " << m_tree_name << std::endl;
		exit(1);
	} else {
		std::cout << "Tree " << m_tree->GetName() << " created successfully." << std::endl;
	}

	m_evt  = findNode::getClass<SQEvent  >(startNode, "SQEvent");
	m_srec = findNode::getClass<SRecEvent>(startNode, "SRecEvent");

	if (!m_evt || !m_srec) return Fun4AllReturnCodes::ABORTEVENT;


	SQRun* sq_run = findNode::getClass<SQRun>(startNode, "SQRun");
	if (!sq_run) return Fun4AllReturnCodes::ABORTEVENT;
	int LBtop = sq_run->get_v1495_id(2);
	int LBbot = sq_run->get_v1495_id(3);
	int ret = m_rs.LoadConfig(LBtop, LBbot);
	if (ret != 0) {
		cout << "!!WARNING!!  OnlMonTrigEP::InitRunOnlMon():  roadset.LoadConfig returned " << ret << ".\n";
	}
	cout <<"Roadset " << m_rs.str(1) << endl;

	m_tree->Branch("eventID", &eventID, "eventID/I");
	m_tree->Branch("runID", &runID, "runID/I");
	m_tree->Branch("spillID", &spillID, "spillID/I");
	m_tree->Branch("rec_dimuon_id", &rec_dimuon_id);
	m_tree->Branch("rec_dimuon_true_id", &rec_dimuon_true_id);
	m_tree->Branch("rec_dimuon_track_id_pos", &rec_dimuon_track_id_pos);
	m_tree->Branch("rec_dimuon_track_id_neg", &rec_dimuon_track_id_neg);
	m_tree->Branch("rec_dimuon_roads", &rec_dimuon_roads);


	m_tree->Branch("rec_dimuon_x", &rec_dimuon_x);
	m_tree->Branch("rec_dimuon_y", &rec_dimuon_y);
	m_tree->Branch("rec_dimuon_z", &rec_dimuon_z);

	m_tree->Branch("rec_dimuon_px_pos", &rec_dimuon_px_pos);
	m_tree->Branch("rec_dimuon_py_pos", &rec_dimuon_py_pos);
	m_tree->Branch("rec_dimuon_pz_pos", &rec_dimuon_pz_pos);

	m_tree->Branch("rec_dimuon_px_neg", &rec_dimuon_px_neg);
	m_tree->Branch("rec_dimuon_py_neg", &rec_dimuon_py_neg);
	m_tree->Branch("rec_dimuon_pz_neg", &rec_dimuon_pz_neg);

	m_tree->Branch("rec_dimuon_px_pos_tgt", &rec_dimuon_px_pos_tgt);
	m_tree->Branch("rec_dimuon_py_pos_tgt", &rec_dimuon_py_pos_tgt);
	m_tree->Branch("rec_dimuon_pz_pos_tgt", &rec_dimuon_pz_pos_tgt);

	m_tree->Branch("rec_dimuon_px_neg_tgt", &rec_dimuon_px_neg_tgt);
	m_tree->Branch("rec_dimuon_py_neg_tgt", &rec_dimuon_py_neg_tgt);
	m_tree->Branch("rec_dimuon_pz_neg_tgt", &rec_dimuon_pz_neg_tgt);

	// Vertex position (x, y, z) for positive trk
	m_tree->Branch("rec_dimuon_x_pos_vtx", &rec_dimuon_x_pos_vtx);
	m_tree->Branch("rec_dimuon_y_pos_vtx", &rec_dimuon_y_pos_vtx);
	m_tree->Branch("rec_dimuon_z_pos_vtx", &rec_dimuon_z_pos_vtx);

	// Vertex momentum (px, py, pz) for positive trk
	m_tree->Branch("rec_dimuon_px_pos_vtx", &rec_dimuon_px_pos_vtx);
	m_tree->Branch("rec_dimuon_py_pos_vtx", &rec_dimuon_py_pos_vtx);
	m_tree->Branch("rec_dimuon_pz_pos_vtx", &rec_dimuon_pz_pos_vtx);

	// Vertex position (x, y, z) for negative trk
	m_tree->Branch("rec_dimuon_x_neg_vtx", &rec_dimuon_x_neg_vtx);
	m_tree->Branch("rec_dimuon_y_neg_vtx", &rec_dimuon_y_neg_vtx);
	m_tree->Branch("rec_dimuon_z_neg_vtx", &rec_dimuon_z_neg_vtx);

	// Vertex momentum (px, py, pz) for negative trk
	m_tree->Branch("rec_dimuon_px_neg_vtx", &rec_dimuon_px_neg_vtx);
	m_tree->Branch("rec_dimuon_py_neg_vtx", &rec_dimuon_py_neg_vtx);
	m_tree->Branch("rec_dimuon_pz_neg_vtx", &rec_dimuon_pz_neg_vtx);

	// Station 1 - mu+ position
	m_tree->Branch("rec_dimuon_x_pos_st1", &rec_dimuon_x_pos_st1);
	m_tree->Branch("rec_dimuon_y_pos_st1", &rec_dimuon_y_pos_st1);
	m_tree->Branch("rec_dimuon_z_pos_st1", &rec_dimuon_z_pos_st1);

	// Station 1 - mu-  position
	m_tree->Branch("rec_dimuon_x_neg_st1", &rec_dimuon_x_neg_st1);
	m_tree->Branch("rec_dimuon_y_neg_st1", &rec_dimuon_y_neg_st1);
	m_tree->Branch("rec_dimuon_z_neg_st1", &rec_dimuon_z_neg_st1);

	// Station 3 - mu+  position
	m_tree->Branch("rec_dimuon_x_pos_st3", &rec_dimuon_x_pos_st3);
	m_tree->Branch("rec_dimuon_y_pos_st3", &rec_dimuon_y_pos_st3);
	m_tree->Branch("rec_dimuon_z_pos_st3", &rec_dimuon_z_pos_st3);

	// Station 3 - mu- position
	m_tree->Branch("rec_dimuon_x_neg_st3", &rec_dimuon_x_neg_st3);
	m_tree->Branch("rec_dimuon_y_neg_st3", &rec_dimuon_y_neg_st3);
	m_tree->Branch("rec_dimuon_z_neg_st3", &rec_dimuon_z_neg_st3);

	// Station 1 - mu+ momentum
	m_tree->Branch("rec_dimuon_px_pos_st1", &rec_dimuon_px_pos_st1);
	m_tree->Branch("rec_dimuon_py_pos_st1", &rec_dimuon_py_pos_st1);
	m_tree->Branch("rec_dimuon_pz_pos_st1", &rec_dimuon_pz_pos_st1);

	// Station 1 - mu- momentum
	m_tree->Branch("rec_dimuon_px_neg_st1", &rec_dimuon_px_neg_st1);
	m_tree->Branch("rec_dimuon_py_neg_st1", &rec_dimuon_py_neg_st1);
	m_tree->Branch("rec_dimuon_pz_neg_st1", &rec_dimuon_pz_neg_st1);

	// Station 3 - mu+  momentum
	m_tree->Branch("rec_dimuon_px_pos_st3", &rec_dimuon_px_pos_st3);
	m_tree->Branch("rec_dimuon_py_pos_st3", &rec_dimuon_py_pos_st3);
	m_tree->Branch("rec_dimuon_pz_pos_st3", &rec_dimuon_pz_pos_st3);

	// Station 3 - mu- momentum
	m_tree->Branch("rec_dimuon_px_neg_st3", &rec_dimuon_px_neg_st3);
	m_tree->Branch("rec_dimuon_py_neg_st3", &rec_dimuon_py_neg_st3);
	m_tree->Branch("rec_dimuon_pz_neg_st3", &rec_dimuon_pz_neg_st3);


	return Fun4AllReturnCodes::EVENT_OK;
}


int DimuAnaRUS::process_event(PHCompositeNode* startNode)
{

    if (! m_evt->get_trigger(SQEvent::MATRIX1)) {
        return Fun4AllReturnCodes::EVENT_OK;
    }

    eventID = m_evt->get_event_id();
    runID = m_evt->get_run_id();
    spillID = m_evt->get_spill_id();

    ResetRecoDimuBranches();
    int n_dim = m_srec->getNDimuons();
    for (int i_dim = 0; i_dim < n_dim; i_dim++) {
        SRecDimuon sdim = m_srec->getDimuon(i_dim);
        int trk_id_pos = sdim.get_track_id_pos();
        int trk_id_neg = sdim.get_track_id_neg();
        SRecTrack trk_pos = m_srec->getTrack(trk_id_pos);
        SRecTrack trk_neg = m_srec->getTrack(trk_id_neg);

	    int road_pos = trk_pos.getTriggerRoad();
    	int road_neg = trk_neg.getTriggerRoad();
   		bool pos_top = m_rs.PosTop()->FindRoad(road_pos);
   	 	bool pos_bot = m_rs.PosBot()->FindRoad(road_pos);
    	bool neg_top = m_rs.NegTop()->FindRoad(road_neg);
    	bool neg_bot = m_rs.NegBot()->FindRoad(road_neg);

		bool top_bot = pos_top && neg_bot;
      	bool bot_top = pos_bot && neg_top;

        //cout << "top_bot: "<< top_bot << "bot_top: "<< bot_top << endl;

      	if (top_bot || bot_top) rec_dimuon_roads.push_back(1);
		else rec_dimuon_roads.push_back(0);

        // Chi2 origin cuts (Cut #1, applied downstream):
        // target must be best vertex for each track:
        //   chi2_tgt > 0
        //   chi2_dump     - chi2_tgt > 0  (dump   is worse than target)
        //   chi2_upstream - chi2_tgt > 0  (upstream is worse than target)
        //if (trk_pos.getChisqTarget() <= 0 ||
        //    trk_pos.get_chisq_dump()     - trk_pos.getChisqTarget() <= 0 ||
        //    trk_pos.get_chisq_upstream() - trk_pos.getChisqTarget() <= 0) continue;
        //if (trk_neg.getChisqTarget() <= 0 ||
        //    trk_neg.get_chisq_dump()     - trk_neg.getChisqTarget() <= 0 ||
        //    trk_neg.get_chisq_upstream() - trk_neg.getChisqTarget() <= 0) continue;

        rec_dimuon_id.push_back(sdim.get_dimuon_id());
        rec_dimuon_true_id.push_back(sdim.get_rec_dimuon_id());
        rec_dimuon_track_id_pos.push_back(trk_id_pos);
        rec_dimuon_track_id_neg.push_back(trk_id_neg);
        rec_dimuon_px_pos.push_back(sdim.get_mom_pos().Px());
        rec_dimuon_py_pos.push_back(sdim.get_mom_pos().Py());
        rec_dimuon_pz_pos.push_back(sdim.get_mom_pos().Pz());
        rec_dimuon_px_neg.push_back(sdim.get_mom_neg().Px());
        rec_dimuon_py_neg.push_back(sdim.get_mom_neg().Py());
        rec_dimuon_pz_neg.push_back(sdim.get_mom_neg().Pz());
        rec_dimuon_x.push_back(sdim.get_pos().X());
        rec_dimuon_y.push_back(sdim.get_pos().Y());
        rec_dimuon_z.push_back(sdim.get_pos().Z());
        // ===== Target hypothesis =====
        sdim.calcVariables(1); // 1 = target
        rec_dimuon_px_pos_tgt.push_back(sdim.p_pos_target.Px());
        rec_dimuon_py_pos_tgt.push_back(sdim.p_pos_target.Py());
        rec_dimuon_pz_pos_tgt.push_back(sdim.p_pos_target.Pz());
        rec_dimuon_px_neg_tgt.push_back(sdim.p_neg_target.Px());
        rec_dimuon_py_neg_tgt.push_back(sdim.p_neg_target.Py());
        rec_dimuon_pz_neg_tgt.push_back(sdim.p_neg_target.Pz());
        //--------
		//vtx
        rec_dimuon_px_pos_vtx.push_back(trk_pos.get_mom_vtx().Px());
        rec_dimuon_py_pos_vtx.push_back(trk_pos.get_mom_vtx().Py());
        rec_dimuon_pz_pos_vtx.push_back(trk_pos.get_mom_vtx().Pz());

        rec_dimuon_x_pos_vtx.push_back(trk_pos.get_pos_vtx().X());
        rec_dimuon_y_pos_vtx.push_back(trk_pos.get_pos_vtx().Y());
        rec_dimuon_z_pos_vtx.push_back(trk_pos.get_pos_vtx().Z());

        rec_dimuon_px_neg_vtx.push_back(trk_neg.get_mom_vtx().Px());
        rec_dimuon_py_neg_vtx.push_back(trk_neg.get_mom_vtx().Py());
        rec_dimuon_pz_neg_vtx.push_back(trk_neg.get_mom_vtx().Pz());

        rec_dimuon_x_neg_vtx.push_back(trk_neg.get_pos_vtx().X());
        rec_dimuon_y_neg_vtx.push_back(trk_neg.get_pos_vtx().Y());
        rec_dimuon_z_neg_vtx.push_back(trk_neg.get_pos_vtx().Z());

        // Station 1 - positive trk
        rec_dimuon_px_pos_st1.push_back(trk_pos.get_mom_st1().Px());
        rec_dimuon_py_pos_st1.push_back(trk_pos.get_mom_st1().Py());
        rec_dimuon_pz_pos_st1.push_back(trk_pos.get_mom_st1().Pz());

        rec_dimuon_x_pos_st1.push_back(trk_pos.get_pos_st1().X());
        rec_dimuon_y_pos_st1.push_back(trk_pos.get_pos_st1().Y());
        rec_dimuon_z_pos_st1.push_back(trk_pos.get_pos_st1().Z());

        // Station 3 - positive trk
        rec_dimuon_px_pos_st3.push_back(trk_pos.get_mom_st3().Px());
        rec_dimuon_py_pos_st3.push_back(trk_pos.get_mom_st3().Py());
        rec_dimuon_pz_pos_st3.push_back(trk_pos.get_mom_st3().Pz());

        rec_dimuon_x_pos_st3.push_back(trk_pos.get_pos_st3().X());
        rec_dimuon_y_pos_st3.push_back(trk_pos.get_pos_st3().Y());
        rec_dimuon_z_pos_st3.push_back(trk_pos.get_pos_st3().Z());


        // Station 1 - negative trk
        rec_dimuon_px_neg_st1.push_back(trk_neg.get_mom_st1().Px());
        rec_dimuon_py_neg_st1.push_back(trk_neg.get_mom_st1().Py());
        rec_dimuon_pz_neg_st1.push_back(trk_neg.get_mom_st1().Pz());

        rec_dimuon_x_neg_st1.push_back(trk_neg.get_pos_st1().X());
        rec_dimuon_y_neg_st1.push_back(trk_neg.get_pos_st1().Y());
        rec_dimuon_z_neg_st1.push_back(trk_neg.get_pos_st1().Z());

        // Station 3 - negative trk
        rec_dimuon_px_neg_st3.push_back(trk_neg.get_mom_st3().Px());
        rec_dimuon_py_neg_st3.push_back(trk_neg.get_mom_st3().Py());
        rec_dimuon_pz_neg_st3.push_back(trk_neg.get_mom_st3().Pz());

        rec_dimuon_x_neg_st3.push_back(trk_neg.get_pos_st3().X());
        rec_dimuon_y_neg_st3.push_back(trk_neg.get_pos_st3().Y());
        rec_dimuon_z_neg_st3.push_back(trk_neg.get_pos_st3().Z());
    }   
    m_tree->Fill();
    return Fun4AllReturnCodes::EVENT_OK;
}

int DimuAnaRUS::End(PHCompositeNode* startNode)
{
  m_file->cd();
  m_file->Write();
  m_file->Close();  
  return Fun4AllReturnCodes::EVENT_OK;
}


void DimuAnaRUS::ResetRecoDimuBranches() {
    rec_dimuon_id.clear(); rec_dimuon_true_id.clear(); rec_dimuon_track_id_pos.clear(); rec_dimuon_track_id_neg.clear();
    rec_dimuon_x.clear(); rec_dimuon_y.clear(); rec_dimuon_z.clear();
    rec_dimuon_px_pos.clear(); rec_dimuon_py_pos.clear(); rec_dimuon_pz_pos.clear();
    rec_dimuon_px_neg.clear(); rec_dimuon_py_neg.clear(); rec_dimuon_pz_neg.clear();
    rec_dimuon_px_pos_tgt.clear(); rec_dimuon_py_pos_tgt.clear(); rec_dimuon_pz_pos_tgt.clear();
    rec_dimuon_px_neg_tgt.clear(); rec_dimuon_py_neg_tgt.clear(); rec_dimuon_pz_neg_tgt.clear();
    rec_dimuon_x_pos_st1.clear();  rec_dimuon_y_pos_st1.clear(); rec_dimuon_z_pos_st1.clear();
    rec_dimuon_roads.clear();
    rec_dimuon_x_pos_vtx.clear();
    rec_dimuon_y_pos_vtx.clear();
    rec_dimuon_z_pos_vtx.clear();
    rec_dimuon_px_pos_vtx.clear();
    rec_dimuon_py_pos_vtx.clear();
    rec_dimuon_pz_pos_vtx.clear();
    rec_dimuon_x_neg_vtx.clear();
    rec_dimuon_y_neg_vtx.clear();
    rec_dimuon_z_neg_vtx.clear();
    rec_dimuon_px_neg_vtx.clear();
    rec_dimuon_py_neg_vtx.clear();
    rec_dimuon_pz_neg_vtx.clear();
    rec_dimuon_x_pos_st1.clear();
    rec_dimuon_y_pos_st1.clear();
    rec_dimuon_z_pos_st1.clear();
    rec_dimuon_x_neg_st1.clear();
    rec_dimuon_y_neg_st1.clear();
    rec_dimuon_z_neg_st1.clear();
    rec_dimuon_x_pos_st3.clear();
    rec_dimuon_y_pos_st3.clear();
    rec_dimuon_z_pos_st3.clear();
    rec_dimuon_x_neg_st3.clear();
    rec_dimuon_y_neg_st3.clear();
    rec_dimuon_z_neg_st3.clear();
    rec_dimuon_px_pos_st1.clear();
    rec_dimuon_py_pos_st1.clear();
    rec_dimuon_pz_pos_st1.clear();
    rec_dimuon_px_neg_st1.clear();
    rec_dimuon_py_neg_st1.clear();
    rec_dimuon_pz_neg_st1.clear();
    rec_dimuon_px_pos_st3.clear();
    rec_dimuon_py_pos_st3.clear();
    rec_dimuon_pz_pos_st3.clear();
    rec_dimuon_px_neg_st3.clear();
    rec_dimuon_py_neg_st3.clear();
    rec_dimuon_pz_neg_st3.clear();
}


