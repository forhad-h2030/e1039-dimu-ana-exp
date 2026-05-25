#ifndef _DIMU_ANA_RUS_H_
#define _DIMU_ANA_RUS_H_
#include <fun4all/SubsysReco.h>
#include <UtilAna/TrigRoadset.h>

class TFile;
class TTree;
class SQEvent;
class SRecEvent;

class DimuAnaRUS: public SubsysReco {
	SQEvent*   m_evt;
	SRecEvent* m_srec;

	std::string m_file_name;
	std::string m_tree_name;
	TFile*      m_file;
	TTree*      m_tree;

public:
	UtilTrigger::TrigRoadset m_rs;

	DimuAnaRUS(const std::string& name="DimuAnaRUS");
	virtual ~DimuAnaRUS();
	int Init(PHCompositeNode *startNode);
	int InitRun(PHCompositeNode *startNode);
	int process_event(PHCompositeNode *startNode);
	int End(PHCompositeNode *startNode);

	void SetOutputFileName(const std::string name) { m_file_name = name; }
	void SetTreeName(const std::string& name)       { m_tree_name = name; }

	void ResetRecoDimuBranches();

private:
	// Event scalars
	int eventID;
	int runID;
	int spillID;

	// Dimuon-level vectors
	std::vector<int>
	    rec_dimuon_id, rec_dimuon_true_id,
	    rec_dimuon_track_id_pos, rec_dimuon_track_id_neg,
	    rec_dimuon_roads;

	std::vector<double>
	    rec_dimuon_x, rec_dimuon_y, rec_dimuon_z,
	    // vertex-fit momenta
	    rec_dimuon_px_pos, rec_dimuon_py_pos, rec_dimuon_pz_pos,
	    rec_dimuon_px_neg, rec_dimuon_py_neg, rec_dimuon_pz_neg,
	    // target-hypothesis momenta
	    rec_dimuon_px_pos_tgt, rec_dimuon_py_pos_tgt, rec_dimuon_pz_pos_tgt,
	    rec_dimuon_px_neg_tgt, rec_dimuon_py_neg_tgt, rec_dimuon_pz_neg_tgt,
	    // mu+ track: vertex pos/mom
	    rec_dimuon_x_pos_vtx, rec_dimuon_y_pos_vtx, rec_dimuon_z_pos_vtx,
	    rec_dimuon_px_pos_vtx, rec_dimuon_py_pos_vtx, rec_dimuon_pz_pos_vtx,
	    // mu- track: vertex pos/mom
	    rec_dimuon_x_neg_vtx, rec_dimuon_y_neg_vtx, rec_dimuon_z_neg_vtx,
	    rec_dimuon_px_neg_vtx, rec_dimuon_py_neg_vtx, rec_dimuon_pz_neg_vtx,
	    // mu+ track: station 1 pos/mom
	    rec_dimuon_x_pos_st1, rec_dimuon_y_pos_st1, rec_dimuon_z_pos_st1,
	    rec_dimuon_px_pos_st1, rec_dimuon_py_pos_st1, rec_dimuon_pz_pos_st1,
	    // mu- track: station 1 pos/mom
	    rec_dimuon_x_neg_st1, rec_dimuon_y_neg_st1, rec_dimuon_z_neg_st1,
	    rec_dimuon_px_neg_st1, rec_dimuon_py_neg_st1, rec_dimuon_pz_neg_st1,
	    // mu+ track: station 3 pos/mom
	    rec_dimuon_x_pos_st3, rec_dimuon_y_pos_st3, rec_dimuon_z_pos_st3,
	    rec_dimuon_px_pos_st3, rec_dimuon_py_pos_st3, rec_dimuon_pz_pos_st3,
	    // mu- track: station 3 pos/mom
	    rec_dimuon_x_neg_st3, rec_dimuon_y_neg_st3, rec_dimuon_z_neg_st3,
	    rec_dimuon_px_neg_st3, rec_dimuon_py_neg_st3, rec_dimuon_pz_neg_st3;
};

#endif // _DIMU_ANA_RUS_H_
