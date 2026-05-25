# e1039-dimu-ana-exp

## Goal
Read SpinQuest reconstructed DST files (`spin_reco.root`) and convert them into a flat ROOT tree containing dimuon-level variables for physics analysis. Cuts are applied inline; the output contains only good dimuons.

---

## Repository structure

```
e1039-dimu-ana-exp/
├── setup.sh          # Environment setup + cmake-this / make-this helpers
├── src/
│   ├── CMakeLists.txt
│   ├── DimuAnaRUS.h / .cc   # Main analysis module
│   └── LinkDef.h
├── Convert/
│   ├── Fun4All.C     # ROOT macro — runs the analysis over a list of input files
│   ├── make_lists.sh # Generates list_up.txt / list_down.txt from the data path
│   ├── test.sh       # Quick single-spill test
│   └── run_all.sh    # Processes all spin-up and spin-down files
├── inst/             # Build output (headers + library) — generated, not committed
└── build/            # CMake build directory — generated, not committed
```

---

## Data

Input files (`spin_reco.root`) are DST files produced by ktracker, located at:
```
/pnfs/e1039/persistent/users/kenichi/RecoData2024/reco-20260512/
  run_XXXXXX/spill_XXXXXXXXX/out/run_XXXXXX_spill_XXXXXXXXX_spin_reco.root
```

### Spin state assignments

| Spin state | Runs |
|---|---|
| **Spin UP**   | 6111, 6112, 6113, 6114, 6115, 6116, 6117, 6118, 6156 |
| **Spin DOWN** | 6135, 6136, 6137, 6138, 6139, 6149, 6150, 6151, 6152, 6153, 6154, 6155 |

---

## Build (on GPVM)

```bash
cd e1039-dimu-ana-exp
source setup.sh   # loads e1039 environment, defines cmake-this / make-this
cmake-this        # configures build from src/CMakeLists.txt → build/
make-this         # compiles and installs library into inst/
```

The library `inst/lib/libdimu_ana_rus.so` must exist before running the macro.

---

## Run

### Generate file lists

Run once on GPVM to populate `list_up.txt` and `list_down.txt`:

```bash
cd Convert
bash make_lists.sh
```

### Quick test (single spill)

```bash
cd Convert
bash test.sh
```

Output: `e1039-dimu-ana-exp/test_output.root`

### Full run (all spills)

```bash
cd Convert
bash run_all.sh
```

Output:
- `e1039-dimu-ana-exp/output_up.root` — all spin-up spills
- `e1039-dimu-ana-exp/output_down.root` — all spin-down spills

Spin assignment is encoded by which file a dimuon lands in.

---

## Selection cuts applied inline

| Cut | Value |
|---|---|
| Trigger | MATRIX1 (FPGA1) only |
| Track vertex z (μ⁺ and μ⁻) | > −690 cm |
| χ² target hypothesis | ≥ 0 for both tracks |
| χ² dump − χ² target | ≥ 0 for both tracks (target is best vertex) |
| χ² upstream − χ² target | ≥ 0 for both tracks |

The road flag (`rec_dimuon_roads`) is stored but **not** used as a cut.

---

## Counting dimuons after downstream cuts

`count_cuts.py` applies the full DocDB #11359 cut set on the output files and prints progressive counts:

```bash
source setup.sh
python3 count_cuts.py
```

Downstream cuts (applied in `count_cuts.py`):
1. z\_track > −600 cm
2. |y\_st1| > 3 cm
3. py\_st1\_pos × py\_st1\_neg < 0
4. x\_st1 < 25 cm (both tracks)
5. χ² cuts (re-verified from saved branches)
6. 0 ≤ M(μμ) ≤ 6 GeV

---

## Output tree branches

### Event-level
| Branch | Type | Description |
|---|---|---|
| `eventID` | `int` | Event ID |
| `runID` | `int` | Run number |
| `spillID` | `int` | Spill number |

### Dimuon-level (per dimuon, vectors indexed per dimuon)
| Branch | Type | Description |
|---|---|---|
| `rec_dimuon_id` | `vector<int>` | Dimuon ID |
| `rec_dimuon_true_id` | `vector<int>` | Reconstructed dimuon ID |
| `rec_dimuon_track_id_pos/neg` | `vector<int>` | Index of μ⁺/μ⁻ in the track vector |
| `rec_dimuon_roads` | `vector<int>` | 1 if top-bottom road combination satisfied, 0 otherwise |
| `rec_dimuon_x/y/z` | `vector<double>` | Dimuon vertex position (cm) |
| `rec_dimuon_px/py/pz_pos` | `vector<double>` | μ⁺ momentum at dimuon vertex (GeV/c) |
| `rec_dimuon_px/py/pz_neg` | `vector<double>` | μ⁻ momentum at dimuon vertex (GeV/c) |
| `rec_dimuon_px/py/pz_pos_tgt` | `vector<double>` | μ⁺ momentum — target hypothesis (GeV/c) |
| `rec_dimuon_px/py/pz_neg_tgt` | `vector<double>` | μ⁻ momentum — target hypothesis (GeV/c) |
| `rec_dimuon_x/y/z_pos_vtx` | `vector<double>` | μ⁺ track vertex position (cm) |
| `rec_dimuon_px/py/pz_pos_vtx` | `vector<double>` | μ⁺ track momentum at vertex (GeV/c) |
| `rec_dimuon_x/y/z_neg_vtx` | `vector<double>` | μ⁻ track vertex position (cm) |
| `rec_dimuon_px/py/pz_neg_vtx` | `vector<double>` | μ⁻ track momentum at vertex (GeV/c) |
| `rec_dimuon_x/y/z_pos_st1` | `vector<double>` | μ⁺ position at station 1 (cm) |
| `rec_dimuon_px/py/pz_pos_st1` | `vector<double>` | μ⁺ momentum at station 1 (GeV/c) |
| `rec_dimuon_x/y/z_neg_st1` | `vector<double>` | μ⁻ position at station 1 (cm) |
| `rec_dimuon_px/py/pz_neg_st1` | `vector<double>` | μ⁻ momentum at station 1 (GeV/c) |
| `rec_dimuon_x/y/z_pos_st3` | `vector<double>` | μ⁺ position at station 3 (cm) |
| `rec_dimuon_px/py/pz_pos_st3` | `vector<double>` | μ⁺ momentum at station 3 (GeV/c) |
| `rec_dimuon_x/y/z_neg_st3` | `vector<double>` | μ⁻ position at station 3 (cm) |
| `rec_dimuon_px/py/pz_neg_st3` | `vector<double>` | μ⁻ momentum at station 3 (GeV/c) |
| `rec_dimuon_chisq_target_pos/neg` | `vector<double>` | χ² of target vertex fit (μ⁺/μ⁻) |
| `rec_dimuon_chisq_dump_pos/neg` | `vector<double>` | χ² of dump vertex fit (μ⁺/μ⁻) |
| `rec_dimuon_chisq_upstream_pos/neg` | `vector<double>` | χ² of upstream vertex fit (μ⁺/μ⁻) |
