#include <fstream>
#include <iomanip>
#include <TSystem.h>
#include <TFile.h>
#include <TTree.h>
#include <TChain.h>
#include <TH1D.h>
#include <TCanvas.h>
#include <interface_main/SQRun.h>
#include <interface_main/SQHitVector.h>
#include <interface_main/SQEvent.h>
#include <interface_main/SQMCEvent.h>
#include <interface_main/SQRun.h>
#include <interface_main/SQTrackVector.h>
#include <interface_main/SQDimuonVector.h>
#include <interface_main/SQTrackVector_v1.h>
#include <ktracker/FastTracklet.h>
#include <ktracker/SRecEvent.h>
#include <fun4all/Fun4AllReturnCodes.h>
#include <phool/PHNodeIterator.h>
#include <phool/PHIODataNode.h>
#include <phool/getClass.h>
#include <geom_svc/GeomSvc.h>
#include "DimuAnaRUS.h"
using namespace std;

DimuAnaRUS::DimuAnaRUS(const std::string& name)
  : SubsysReco  (name),
    m_file(0),
    m_tree(0),
    m_tree_name("tree"),
    m_file_name("output.root"),
    m_evt(0),
    m_sp_map(0),
    m_hit_vec(0),
    m_sq_trk_vec(0),
    m_sq_dim_vec(0),
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

	m_evt = findNode::getClass<SQEvent>(startNode, "SQEvent");
	m_sq_dim_vec = findNode::getClass<SQDimuonVector>(startNode, "SQRecDimuonVector_PM");
	m_sq_trk_vec = findNode::getClass<SQTrackVector >(startNode, "SQRecTrackVector");

	if (!m_evt || !m_sq_dim_vec || !m_sq_trk_vec) return Fun4AllReturnCodes::ABORTEVENT;


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


	//m_tree->Branch("rec_track_id",             &rec_track_id);
	m_tree->Branch("rec_track_charge",         &rec_track_charge);
	m_tree->Branch("rec_track_vx",             &rec_track_vx);
	m_tree->Branch("rec_track_vy",             &rec_track_vy);
	m_tree->Branch("rec_track_vz",             &rec_track_vz);
	m_tree->Branch("rec_track_px",             &rec_track_px);
	m_tree->Branch("rec_track_py",             &rec_track_py);
	m_tree->Branch("rec_track_pz",             &rec_track_pz);
	m_tree->Branch("rec_track_x_st1",          &rec_track_x_st1);
	m_tree->Branch("rec_track_y_st1",          &rec_track_y_st1);
	m_tree->Branch("rec_track_z_st1",          &rec_track_z_st1);
	m_tree->Branch("rec_track_px_st1",         &rec_track_px_st1);
	m_tree->Branch("rec_track_py_st1",         &rec_track_py_st1);
	m_tree->Branch("rec_track_pz_st1",         &rec_track_pz_st1);
	m_tree->Branch("rec_track_x_st3",          &rec_track_x_st3);
	m_tree->Branch("rec_track_y_st3",          &rec_track_y_st3);
	m_tree->Branch("rec_track_z_st3",          &rec_track_z_st3);
	m_tree->Branch("rec_track_px_st3",         &rec_track_px_st3);
	m_tree->Branch("rec_track_py_st3",         &rec_track_py_st3);
	m_tree->Branch("rec_track_pz_st3",         &rec_track_pz_st3);
	m_tree->Branch("rec_track_num_hits",       &rec_track_num_hits);
	m_tree->Branch("rec_track_chisq",          &rec_track_chisq);
	m_tree->Branch("rec_track_chisq_tgt",   &rec_track_chisq_tgt);
	m_tree->Branch("rec_track_chisq_dump",     &rec_track_chisq_dump);
	m_tree->Branch("rec_track_chisq_upstream", &rec_track_chisq_upstream);
	m_tree->Branch("rec_track_x_tgt",          &rec_track_x_tgt);
	m_tree->Branch("rec_track_y_tgt",          &rec_track_y_tgt);
	m_tree->Branch("rec_track_z_tgt",          &rec_track_z_tgt);
	m_tree->Branch("rec_track_px_tgt",         &rec_track_px_tgt);
	m_tree->Branch("rec_track_py_tgt",         &rec_track_py_tgt);
	m_tree->Branch("rec_track_pz_tgt",         &rec_track_pz_tgt);
	m_tree->Branch("rec_track_x_dump",         &rec_track_x_dump);
	m_tree->Branch("rec_track_y_dump",         &rec_track_y_dump);
	m_tree->Branch("rec_track_z_dump",         &rec_track_z_dump);
	m_tree->Branch("rec_track_px_dump",        &rec_track_px_dump);
	m_tree->Branch("rec_track_py_dump",        &rec_track_py_dump);
	m_tree->Branch("rec_track_pz_dump",        &rec_track_pz_dump);
	//m_tree->Branch("rec_hit_ids",              &rec_hit_ids);
	m_tree->Branch("rec_track_hit_x",              &rec_track_hit_x);
	m_tree->Branch("rec_track_hit_y",              &rec_track_hit_y);


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

	m_tree->Branch("rec_dimuon_px_pos_dump", &rec_dimuon_px_pos_dump);
	m_tree->Branch("rec_dimuon_py_pos_dump", &rec_dimuon_py_pos_dump);
	m_tree->Branch("rec_dimuon_pz_pos_dump", &rec_dimuon_pz_pos_dump);

	m_tree->Branch("rec_dimuon_px_neg_dump", &rec_dimuon_px_neg_dump);
	m_tree->Branch("rec_dimuon_py_neg_dump", &rec_dimuon_py_neg_dump);
	m_tree->Branch("rec_dimuon_pz_neg_dump", &rec_dimuon_pz_neg_dump);

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

    const double muon_mass = 0.10566;


        int index = -1;
		ResetRecoBranches();
		for (auto it = m_sq_trk_vec->begin(); it != m_sq_trk_vec->end(); ++it) {
			index+=1;
			SRecTrack* trk = dynamic_cast<SRecTrack*>(*it);

            //cout << "track id: "<< trk->get_rec_track_id ()  <<" charge: " << trk->get_charge() << "index: "<<endl;

			rec_track_id.push_back(index);  // where i_trk is the index in the reco track loop
			// Basic track info
			rec_track_charge.push_back(trk->get_charge());
			rec_track_vx.push_back(trk->get_pos_vtx().X());
			rec_track_vy.push_back(trk->get_pos_vtx().Y());
			rec_track_vz.push_back(trk->get_pos_vtx().Z());
			rec_track_px.push_back(trk->get_mom_vtx().Px());
			rec_track_py.push_back(trk->get_mom_vtx().Py());
			rec_track_pz.push_back(trk->get_mom_vtx().Pz());

			// Station 1
			rec_track_x_st1.push_back(trk->get_pos_st1().X());
			rec_track_y_st1.push_back(trk->get_pos_st1().Y());
			rec_track_z_st1.push_back(trk->get_pos_st1().Z());
			rec_track_px_st1.push_back(trk->get_mom_st1().Px());
			rec_track_py_st1.push_back(trk->get_mom_st1().Py());
			rec_track_pz_st1.push_back(trk->get_mom_st1().Pz());

			// Station 3
			rec_track_x_st3.push_back(trk->get_pos_st3().X());
			rec_track_y_st3.push_back(trk->get_pos_st3().Y());
			rec_track_z_st3.push_back(trk->get_pos_st3().Z());
			rec_track_px_st3.push_back(trk->get_mom_st3().Px());
			rec_track_py_st3.push_back(trk->get_mom_st3().Py());
			rec_track_pz_st3.push_back(trk->get_mom_st3().Pz());

			// Chi-squared
			rec_track_chisq.push_back(trk->get_chisq());
			rec_track_chisq_tgt.push_back(trk->getChisqTarget());
			rec_track_chisq_dump.push_back(trk->get_chisq_dump());
			rec_track_chisq_upstream.push_back(trk->get_chisq_upstream());

			// Number of hits
			rec_track_num_hits.push_back(trk->get_num_hits());

			// Target region
			rec_track_x_tgt.push_back(trk->get_pos_target().X());
			rec_track_y_tgt.push_back(trk->get_pos_target().Y());
			rec_track_z_tgt.push_back(trk->get_pos_target().Z());
			rec_track_px_tgt.push_back(trk->get_mom_target().Px());
			rec_track_py_tgt.push_back(trk->get_mom_target().Py());
			rec_track_pz_tgt.push_back(trk->get_mom_target().Pz());

			// Dump region
			rec_track_x_dump.push_back(trk->get_pos_dump().X());
			rec_track_y_dump.push_back(trk->get_pos_dump().Y());
			rec_track_z_dump.push_back(trk->get_pos_dump().Z());
			rec_track_px_dump.push_back(trk->get_mom_dump().Px());
			rec_track_py_dump.push_back(trk->get_mom_dump().Py());
			rec_track_pz_dump.push_back(trk->get_mom_dump().Pz());
		}

    ResetRecoDimuBranches();
    for (auto it = m_sq_dim_vec->begin(); it != m_sq_dim_vec->end(); it++) {
        SRecDimuon& sdim = dynamic_cast<SRecDimuon&>(**it);
        int trk_id_pos = sdim.get_track_id_pos();
        int trk_id_neg = sdim.get_track_id_neg();
        SRecTrack& trk_pos = dynamic_cast<SRecTrack&>(*(m_sq_trk_vec->at(trk_id_pos))); 
        SRecTrack& trk_neg = dynamic_cast<SRecTrack&>(*(m_sq_trk_vec->at(trk_id_neg))); 

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

        //if (trk_pos.getChisqTarget() < 0 || trk_pos.get_chisq_dump() < 0 || trk_pos.get_chsiq_upstream() < 0 ||
        //    trk_pos.getChisqTarget() - trk_pos.get_chisq_dump() > 0 || trk_pos.getChisqTarget() - trk_pos.get_chsiq_upstream() > 0) continue;

       // if (trk_neg.getChisqTarget() < 0 || trk_neg.get_chisq_dump() < 0 || trk_neg.get_chsiq_upstream() < 0 ||
       ///         trk_neg.getChisqTarget() - trk_neg.get_chisq_dump() > 0 || trk_neg.getChisqTarget() - trk_neg.get_chsiq_upstream() > 0) continue;

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
        //cout << "mass neg: " << sdim.p_neg_target.M() << endl;
        // ===== Dump hypothesis =====
        sdim.calcVariables(2); // 2 = dump
        rec_dimuon_px_pos_dump.push_back(sdim.p_pos_dump.Px());
        rec_dimuon_py_pos_dump.push_back(sdim.p_pos_dump.Py());
        rec_dimuon_pz_pos_dump.push_back(sdim.p_pos_dump.Pz());
        rec_dimuon_px_neg_dump.push_back(sdim.p_neg_dump.Px());
        rec_dimuon_py_neg_dump.push_back(sdim.p_neg_dump.Py());
        rec_dimuon_pz_neg_dump.push_back(sdim.p_neg_dump.Pz());

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
    rec_dimuon_px_pos_dump.clear(); rec_dimuon_py_pos_dump.clear(); rec_dimuon_pz_pos_dump.clear();
    rec_dimuon_px_neg_dump.clear(); rec_dimuon_py_neg_dump.clear(); rec_dimuon_pz_neg_dump.clear();
    rec_dimuon_x_pos_st1.clear();  rec_dimuon_y_pos_st1.clear(); rec_dimuon_z_pos_st1.clear();
    rec_dimuon_px_neg.clear(); rec_dimuon_py_neg.clear(); rec_dimuon_pz_neg.clear();
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


void DimuAnaRUS::ResetRecoBranches() {
    rec_track_id.clear();
    rec_track_charge.clear();
    rec_track_vx.clear(); rec_track_vy.clear(); rec_track_vz.clear();
    rec_track_px.clear(); rec_track_py.clear(); rec_track_pz.clear();
    rec_track_x_st1.clear(); rec_track_y_st1.clear(); rec_track_z_st1.clear();
    rec_track_px_st1.clear(); rec_track_py_st1.clear(); rec_track_pz_st1.clear();
    rec_track_x_st3.clear(); rec_track_y_st3.clear(); rec_track_z_st3.clear();
    rec_track_px_st3.clear(); rec_track_py_st3.clear(); rec_track_pz_st3.clear();
    rec_track_chisq.clear();
    rec_track_chisq_tgt.clear();
    rec_track_chisq_dump.clear();
    rec_track_chisq_upstream.clear();
    rec_track_num_hits.clear();
    rec_track_x_tgt.clear(); rec_track_y_tgt.clear(); rec_track_z_tgt.clear();
    rec_track_px_tgt.clear(); rec_track_py_tgt.clear(); rec_track_pz_tgt.clear();
    rec_track_x_dump.clear(); rec_track_y_dump.clear(); rec_track_z_dump.clear();
    rec_track_px_dump.clear(); rec_track_py_dump.clear(); rec_track_pz_dump.clear();
    //rec_hit_ids.clear();
    rec_track_hit_x.clear();
    rec_track_hit_y.clear();
}
