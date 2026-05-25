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
#include <interface_main/SQTrackVector.h>
#include <interface_main/SQDimuonVector.h>
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
    m_sq_dim_vec(0),
    m_sq_trk_vec(0)
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

	m_evt        = findNode::getClass<SQEvent      >(startNode, "SQEvent");
	m_sq_dim_vec = findNode::getClass<SQDimuonVector>(startNode, "SQRecDimuonVector_PM");
	m_sq_trk_vec = findNode::getClass<SQTrackVector >(startNode, "SQRecTrackVector");

	if (!m_evt       ) { std::cerr << "ERROR: SQEvent node not found\n";            return Fun4AllReturnCodes::ABORTEVENT; }
	if (!m_sq_dim_vec) { std::cerr << "ERROR: SQRecDimuonVector_PM not found\n";    return Fun4AllReturnCodes::ABORTEVENT; }
	if (!m_sq_trk_vec) { std::cerr << "ERROR: SQRecTrackVector not found\n";        return Fun4AllReturnCodes::ABORTEVENT; }

	SQRun* sq_run = findNode::getClass<SQRun>(startNode, "SQRun");
	if (!sq_run) { std::cerr << "ERROR: SQRun node not found\n"; return Fun4AllReturnCodes::ABORTEVENT; }
	int LBtop = sq_run->get_v1495_id(2);
	int LBbot = sq_run->get_v1495_id(3);
	int ret = m_rs.LoadConfig(LBtop, LBbot);
	if (ret != 0) {
		cout << "!!WARNING!!  OnlMonTrigEP::InitRunOnlMon():  roadset.LoadConfig returned " << ret << ".\n";
	}
	cout <<"Roadset " << m_rs.str(1) << endl;

	m_tree->SetAutoSave(100000000); // flush to disk every ~100 MB

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

	// chi2 vertex-hypothesis values
	m_tree->Branch("rec_dimuon_chisq_target_pos",   &rec_dimuon_chisq_target_pos);
	m_tree->Branch("rec_dimuon_chisq_dump_pos",     &rec_dimuon_chisq_dump_pos);
	m_tree->Branch("rec_dimuon_chisq_upstream_pos", &rec_dimuon_chisq_upstream_pos);
	m_tree->Branch("rec_dimuon_chisq_target_neg",   &rec_dimuon_chisq_target_neg);
	m_tree->Branch("rec_dimuon_chisq_dump_neg",     &rec_dimuon_chisq_dump_neg);
	m_tree->Branch("rec_dimuon_chisq_upstream_neg", &rec_dimuon_chisq_upstream_neg);

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
    runID   = m_evt->get_run_id();
    spillID = m_evt->get_spill_id();

    m_run_n_events[runID]++;  // count every MATRIX1-triggered event

    ResetRecoDimuBranches();
    for (size_t i_dim = 0; i_dim < m_sq_dim_vec->size(); i_dim++) {
        SRecDimuon* sdim = dynamic_cast<SRecDimuon*>(m_sq_dim_vec->at(i_dim));
        if (!sdim) continue;
        int trk_id_pos = sdim->get_track_id_pos();
        int trk_id_neg = sdim->get_track_id_neg();
        SRecTrack* trk_pos = dynamic_cast<SRecTrack*>(m_sq_trk_vec->at(trk_id_pos));
        SRecTrack* trk_neg = dynamic_cast<SRecTrack*>(m_sq_trk_vec->at(trk_id_neg));
        if (!trk_pos || !trk_neg) continue;

        // --- Cut: z vertex > -690 cm (dimuon vertex + both track vertices) ---
        if (trk_pos->get_pos_vtx().Z()    < -690.) continue;
        if (trk_neg->get_pos_vtx().Z()    < -690.) continue;

        // --- Cut: |y_st1| > 3 cm (both tracks) ---
        if (fabs(trk_pos->get_pos_st1().Y()) < 3.) continue;
        if (fabs(trk_neg->get_pos_st1().Y()) < 3.) continue;

        // --- Cut: chi2 — all hypotheses valid, target must be the best ---
        double chisq_target_pos   = trk_pos->getChisqTarget();
        double chisq_dump_pos     = trk_pos->getChisqDump();
        double chisq_upstream_pos = trk_pos->getChisqUpstream();
        double chisq_target_neg   = trk_neg->getChisqTarget();
        double chisq_dump_neg     = trk_neg->getChisqDump();
        double chisq_upstream_neg = trk_neg->getChisqUpstream();

        bool pass_chisq_cut =
            (chisq_target_pos   >= 0 &&
             chisq_dump_pos     >= 0 &&
             chisq_upstream_pos >= 0 &&
             (chisq_target_pos - chisq_dump_pos)     <= 0 &&
             (chisq_target_pos - chisq_upstream_pos) <= 0 &&
             chisq_target_neg   >= 0 &&
             chisq_dump_neg     >= 0 &&
             chisq_upstream_neg >= 0 &&
             (chisq_target_neg - chisq_dump_neg)     <= 0 &&
             (chisq_target_neg - chisq_upstream_neg) <= 0);

        if (!pass_chisq_cut) continue;

        // --- Target hypothesis & mass > 0 ---
        sdim->calcVariables(1); // 1 = target
        TLorentzVector mom_dimuon = sdim->p_pos_target + sdim->p_neg_target;
        if (mom_dimuon.M() <= 0.) continue;

        m_run_n_dimuons[runID]++;  // this dimuon passed all cuts

        // --- Road check (stored as flag, not a cut) ---
        int road_pos = trk_pos->getTriggerRoad();
        int road_neg = trk_neg->getTriggerRoad();
        bool pos_top = m_rs.PosTop()->FindRoad(road_pos);
        bool pos_bot = m_rs.PosBot()->FindRoad(road_pos);
        bool neg_top = m_rs.NegTop()->FindRoad(road_neg);
        bool neg_bot = m_rs.NegBot()->FindRoad(road_neg);
        bool top_bot = pos_top && neg_bot;
        bool bot_top = pos_bot && neg_top;
        if (top_bot || bot_top) rec_dimuon_roads.push_back(1);
        else                    rec_dimuon_roads.push_back(0);

        rec_dimuon_id.push_back(sdim->get_dimuon_id());
        rec_dimuon_true_id.push_back(sdim->get_rec_dimuon_id());
        rec_dimuon_track_id_pos.push_back(trk_id_pos);
        rec_dimuon_track_id_neg.push_back(trk_id_neg);
        rec_dimuon_px_pos.push_back(sdim->get_mom_pos().Px());
        rec_dimuon_py_pos.push_back(sdim->get_mom_pos().Py());
        rec_dimuon_pz_pos.push_back(sdim->get_mom_pos().Pz());
        rec_dimuon_px_neg.push_back(sdim->get_mom_neg().Px());
        rec_dimuon_py_neg.push_back(sdim->get_mom_neg().Py());
        rec_dimuon_pz_neg.push_back(sdim->get_mom_neg().Pz());
        rec_dimuon_x.push_back(sdim->get_pos().X());
        rec_dimuon_y.push_back(sdim->get_pos().Y());
        rec_dimuon_z.push_back(sdim->get_pos().Z());
        // ===== Target hypothesis (already called above) =====
        rec_dimuon_px_pos_tgt.push_back(sdim->p_pos_target.Px());
        rec_dimuon_py_pos_tgt.push_back(sdim->p_pos_target.Py());
        rec_dimuon_pz_pos_tgt.push_back(sdim->p_pos_target.Pz());
        rec_dimuon_px_neg_tgt.push_back(sdim->p_neg_target.Px());
        rec_dimuon_py_neg_tgt.push_back(sdim->p_neg_target.Py());
        rec_dimuon_pz_neg_tgt.push_back(sdim->p_neg_target.Pz());
        // chi2 values (already fetched above for the cut)
        rec_dimuon_chisq_target_pos.push_back(chisq_target_pos);
        rec_dimuon_chisq_dump_pos.push_back(chisq_dump_pos);
        rec_dimuon_chisq_upstream_pos.push_back(chisq_upstream_pos);
        rec_dimuon_chisq_target_neg.push_back(chisq_target_neg);
        rec_dimuon_chisq_dump_neg.push_back(chisq_dump_neg);
        rec_dimuon_chisq_upstream_neg.push_back(chisq_upstream_neg);
        //--------
        // vtx
        rec_dimuon_px_pos_vtx.push_back(trk_pos->get_mom_vtx().Px());
        rec_dimuon_py_pos_vtx.push_back(trk_pos->get_mom_vtx().Py());
        rec_dimuon_pz_pos_vtx.push_back(trk_pos->get_mom_vtx().Pz());

        rec_dimuon_x_pos_vtx.push_back(trk_pos->get_pos_vtx().X());
        rec_dimuon_y_pos_vtx.push_back(trk_pos->get_pos_vtx().Y());
        rec_dimuon_z_pos_vtx.push_back(trk_pos->get_pos_vtx().Z());

        rec_dimuon_px_neg_vtx.push_back(trk_neg->get_mom_vtx().Px());
        rec_dimuon_py_neg_vtx.push_back(trk_neg->get_mom_vtx().Py());
        rec_dimuon_pz_neg_vtx.push_back(trk_neg->get_mom_vtx().Pz());

        rec_dimuon_x_neg_vtx.push_back(trk_neg->get_pos_vtx().X());
        rec_dimuon_y_neg_vtx.push_back(trk_neg->get_pos_vtx().Y());
        rec_dimuon_z_neg_vtx.push_back(trk_neg->get_pos_vtx().Z());

        // Station 1 - positive trk
        rec_dimuon_px_pos_st1.push_back(trk_pos->get_mom_st1().Px());
        rec_dimuon_py_pos_st1.push_back(trk_pos->get_mom_st1().Py());
        rec_dimuon_pz_pos_st1.push_back(trk_pos->get_mom_st1().Pz());

        rec_dimuon_x_pos_st1.push_back(trk_pos->get_pos_st1().X());
        rec_dimuon_y_pos_st1.push_back(trk_pos->get_pos_st1().Y());
        rec_dimuon_z_pos_st1.push_back(trk_pos->get_pos_st1().Z());

        // Station 3 - positive trk
        rec_dimuon_px_pos_st3.push_back(trk_pos->get_mom_st3().Px());
        rec_dimuon_py_pos_st3.push_back(trk_pos->get_mom_st3().Py());
        rec_dimuon_pz_pos_st3.push_back(trk_pos->get_mom_st3().Pz());

        rec_dimuon_x_pos_st3.push_back(trk_pos->get_pos_st3().X());
        rec_dimuon_y_pos_st3.push_back(trk_pos->get_pos_st3().Y());
        rec_dimuon_z_pos_st3.push_back(trk_pos->get_pos_st3().Z());


        // Station 1 - negative trk
        rec_dimuon_px_neg_st1.push_back(trk_neg->get_mom_st1().Px());
        rec_dimuon_py_neg_st1.push_back(trk_neg->get_mom_st1().Py());
        rec_dimuon_pz_neg_st1.push_back(trk_neg->get_mom_st1().Pz());

        rec_dimuon_x_neg_st1.push_back(trk_neg->get_pos_st1().X());
        rec_dimuon_y_neg_st1.push_back(trk_neg->get_pos_st1().Y());
        rec_dimuon_z_neg_st1.push_back(trk_neg->get_pos_st1().Z());

        // Station 3 - negative trk
        rec_dimuon_px_neg_st3.push_back(trk_neg->get_mom_st3().Px());
        rec_dimuon_py_neg_st3.push_back(trk_neg->get_mom_st3().Py());
        rec_dimuon_pz_neg_st3.push_back(trk_neg->get_mom_st3().Pz());

        rec_dimuon_x_neg_st3.push_back(trk_neg->get_pos_st3().X());
        rec_dimuon_y_neg_st3.push_back(trk_neg->get_pos_st3().Y());
        rec_dimuon_z_neg_st3.push_back(trk_neg->get_pos_st3().Z());
    }   
    m_tree->Fill();
    return Fun4AllReturnCodes::EVENT_OK;
}

int DimuAnaRUS::End(PHCompositeNode* startNode)
{
  m_file->cd();
  m_file->Write();
  m_file->Close();

  // ── Per-run summary ───────────────────────────────────────────────────────
  int total_events  = 0;
  int total_dimuons = 0;
  std::cout << "\n"
            << std::setw(10) << "Run"
            << std::setw(16) << "Trig events"
            << std::setw(16) << "Dimuons saved"
            << "\n"
            << std::string(42, '-') << "\n";
  for (const auto& kv : m_run_n_events) {
      int run     = kv.first;
      int n_evt   = kv.second;
      int n_dimu  = (m_run_n_dimuons.count(run) ? m_run_n_dimuons.at(run) : 0);
      total_events  += n_evt;
      total_dimuons += n_dimu;
      std::cout << std::setw(10) << run
                << std::setw(16) << n_evt
                << std::setw(16) << n_dimu
                << "\n";
  }
  std::cout << std::string(42, '-') << "\n"
            << std::setw(10) << "Total"
            << std::setw(16) << total_events
            << std::setw(16) << total_dimuons
            << "\n\n";

  return Fun4AllReturnCodes::EVENT_OK;
}


void DimuAnaRUS::ResetRecoDimuBranches() {
    rec_dimuon_id.clear(); rec_dimuon_true_id.clear(); rec_dimuon_track_id_pos.clear(); rec_dimuon_track_id_neg.clear();
    rec_dimuon_x.clear(); rec_dimuon_y.clear(); rec_dimuon_z.clear();
    rec_dimuon_px_pos.clear(); rec_dimuon_py_pos.clear(); rec_dimuon_pz_pos.clear();
    rec_dimuon_px_neg.clear(); rec_dimuon_py_neg.clear(); rec_dimuon_pz_neg.clear();
    rec_dimuon_px_pos_tgt.clear(); rec_dimuon_py_pos_tgt.clear(); rec_dimuon_pz_pos_tgt.clear();
    rec_dimuon_px_neg_tgt.clear(); rec_dimuon_py_neg_tgt.clear(); rec_dimuon_pz_neg_tgt.clear();
    rec_dimuon_chisq_target_pos.clear();   rec_dimuon_chisq_dump_pos.clear();   rec_dimuon_chisq_upstream_pos.clear();
    rec_dimuon_chisq_target_neg.clear();   rec_dimuon_chisq_dump_neg.clear();   rec_dimuon_chisq_upstream_neg.clear();
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


