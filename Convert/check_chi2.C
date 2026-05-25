// check_chi2.C
// Reads a spin_reco.root DST file and prints chi2 values for the first
// few dimuons, so we can verify which method names return valid values.
// Usage: root -l -b -q 'check_chi2.C("path/to/spin_reco.root")'

R__LOAD_LIBRARY(libdimu_ana_rus)

void check_chi2(const string fname =
    "/pnfs/e1039/persistent/users/kenichi/RecoData2024/reco-20260512"
    "/run_006111/spill_001936172/out/run_006111_spill_001936172_spin_reco.root")
{
    Fun4AllServer* se = Fun4AllServer::instance();
    Fun4AllInputManager* in = new Fun4AllDstInputManager("DUMMY");
    se->registerInputManager(in);
    in->fileopen(fname);

    PHCompositeNode* topNode = se->topNode();

    // Read one event at a time manually
    int n_printed = 0;
    int n_evt     = 0;

    while (se->run(1) == 0 && n_printed < 20) {
        n_evt++;
        SQEvent*        evt      = findNode::getClass<SQEvent       >(topNode, "SQEvent");
        SQDimuonVector* dim_vec  = findNode::getClass<SQDimuonVector>(topNode, "SQRecDimuonVector_PM");
        SQTrackVector*  trk_vec  = findNode::getClass<SQTrackVector >(topNode, "SQRecTrackVector");
        if (!evt || !dim_vec || !trk_vec) continue;
        if (!evt->get_trigger(SQEvent::MATRIX1)) continue;
        if (dim_vec->size() == 0) continue;

        for (size_t i = 0; i < dim_vec->size(); i++) {
            SRecDimuon* sdim = dynamic_cast<SRecDimuon*>(dim_vec->at(i));
            if (!sdim) continue;
            SRecTrack* tp = dynamic_cast<SRecTrack*>(trk_vec->at(sdim->get_track_id_pos()));
            SRecTrack* tn = dynamic_cast<SRecTrack*>(trk_vec->at(sdim->get_track_id_neg()));
            if (!tp || !tn) continue;

            printf("evt %4d  dim %zu  z_vtx=%.1f\n", n_evt, i, sdim->get_pos().Z());
            printf("  mu+  chisqTarget=%-8.2f  getChisqDump=%-8.2f  get_chisq_dump=%-8.2f  get_chisq_upstream=%-8.2f  get_chsiq_upstream=%-8.2f\n",
                   tp->getChisqTarget(),
                   tp->getChisqDump(),
                   tp->get_chisq_dump(),
                   tp->get_chisq_upstream(),
                   tp->get_chsiq_upstream());
            printf("  mu-  chisqTarget=%-8.2f  getChisqDump=%-8.2f  get_chisq_dump=%-8.2f  get_chisq_upstream=%-8.2f  get_chsiq_upstream=%-8.2f\n",
                   tn->getChisqTarget(),
                   tn->getChisqDump(),
                   tn->get_chisq_dump(),
                   tn->get_chisq_upstream(),
                   tn->get_chsiq_upstream());
            n_printed++;
            if (n_printed >= 20) break;
        }
    }

    se->End();
    delete se;
    gSystem->Exit(0);
}
