#!/usr/bin/env python3
import ROOT
ROOT.gROOT.SetBatch(True)

FILES = [
    ("/seaquest/users/mhossain/e1039-dimu-ana-exp/out/down/exp_data_down_may25_2026.root",
     "spin-down"),
    ("/seaquest/users/mhossain/e1039-dimu-ana-exp/out/up/exp_data_up_may25_2026.root",
     "spin-up"),
]

MUON_MASS = 0.10565837  # GeV


def count_file(path, label):
    f = ROOT.TFile.Open(path)
    if not f or f.IsZombie():
        raise RuntimeError(f"Cannot open: {path}")
    t = f.Get("tree")

    n_raw = n_z = n_y = n_py = n_x = n_chi2 = n_mass = 0

    mu_pos = ROOT.TLorentzVector()
    mu_neg = ROOT.TLorentzVector()

    for evt in t:
        for i in range(len(evt.rec_dimuon_px_pos_tgt)):
            n_raw += 1

            # 1. z_track > -600 cm
            if evt.rec_dimuon_z_pos_vtx[i] <= -600 or \
               evt.rec_dimuon_z_neg_vtx[i] <= -600:
                continue
            n_z += 1

            # 2. |y_st1| > 3 cm
            if abs(evt.rec_dimuon_y_pos_st1[i]) <= 3 or \
               abs(evt.rec_dimuon_y_neg_st1[i]) <= 3:
                continue
            n_y += 1

            # 3. py sign cut
            if evt.rec_dimuon_py_pos_st1[i] * evt.rec_dimuon_py_neg_st1[i] >= 0:
                continue
            n_py += 1

            # 4. x_st1 < 25 cm
            if evt.rec_dimuon_x_pos_st1[i] >= 25 or \
               evt.rec_dimuon_x_neg_st1[i] >= 25:
                continue
            n_x += 1

            # 5. chi2 cuts
            if not (evt.rec_dimuon_chisq_target_pos[i] > 0 and
                    evt.rec_dimuon_chisq_dump_pos[i]     - evt.rec_dimuon_chisq_target_pos[i] > 0 and
                    evt.rec_dimuon_chisq_upstream_pos[i] - evt.rec_dimuon_chisq_target_pos[i] > 0 and
                    evt.rec_dimuon_chisq_target_neg[i] > 0 and
                    evt.rec_dimuon_chisq_dump_neg[i]     - evt.rec_dimuon_chisq_target_neg[i] > 0 and
                    evt.rec_dimuon_chisq_upstream_neg[i] - evt.rec_dimuon_chisq_target_neg[i] > 0):
                continue
            n_chi2 += 1

            # 6. mass 0–6 GeV
            mu_pos.SetXYZM(evt.rec_dimuon_px_pos_tgt[i], evt.rec_dimuon_py_pos_tgt[i],
                           evt.rec_dimuon_pz_pos_tgt[i], MUON_MASS)
            mu_neg.SetXYZM(evt.rec_dimuon_px_neg_tgt[i], evt.rec_dimuon_py_neg_tgt[i],
                           evt.rec_dimuon_pz_neg_tgt[i], MUON_MASS)
            if mu_pos.Vect().Mag() <= 0 or mu_neg.Vect().Mag() <= 0:
                continue
            mass = (mu_pos + mu_neg).M()
            if mass < 0 or mass > 6:
                continue
            n_mass += 1

    f.Close()

    w = 10
    print(f"\n── {label} ──────────────────────────")
    print(f"  {'Raw dimuons':<22}: {n_raw:{w},}")
    print(f"  {'z_track > -600 cm':<22}: {n_z:{w},}")
    print(f"  {'|y_st1| > 3 cm':<22}: {n_y:{w},}")
    print(f"  {'py sign cut':<22}: {n_py:{w},}")
    print(f"  {'x_st1 < 25 cm':<22}: {n_x:{w},}")
    print(f"  {'chi2 cuts':<22}: {n_chi2:{w},}")
    print(f"  {'mass 0–6 GeV':<22}: {n_mass:{w},}")

    return n_mass


def main():
    total = 0
    for path, label in FILES:
        total += count_file(path, label)
    print(f"\n  {'COMBINED TOTAL':<22}: {total:>10,}")


if __name__ == "__main__":
    main()
