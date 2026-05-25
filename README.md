# DimuAnaExp

## Goal
Read SpinQuest reconstructed DST files (`spin_reco.root`) and convert them into a flat ROOT tree (RUS format) containing dimuon and track variables for physics analysis.

---

## Repository structure

```
DimuAnaExp/
├── setup.sh          # Environment setup + cmake-this / make-this helpers
├── src/
│   ├── CMakeLists.txt
│   ├── DimuAnaRUS.h / .cc   # Main analysis module
│   └── LinkDef.h
├── Convert/
│   ├── Fun4All.C     # ROOT macro — runs the analysis on one input file
│   ├── gridrun.sh    # Per-job wrapper executed by HTCondor
│   ├── gridsub.sh    # Submits jobs (grid or local) over a file list
│   └── make_lists.sh # Generates list_up.txt / list_down.txt from the data path
├── inst/             # Build output (headers + library) — generated, not committed
└── data/             # Local test file
```

---

## Data

Input files are located at:
```
/pnfs/e1039/persistent/users/kenichi/RecoData2024/reco-20260512/
  run_XXXXXX/spill_XXXXXXXXX/out/run_XXXXXX_spill_XXXXXXXXX_spin_reco.root
```

### Spin state assignments

| Spin state | Runs |
|---|---|
| **Spin UP** | 6111, 6112, 6113, 6114, 6115, 6116, 6117, 6118, 6156 |
| **Spin DOWN** | 6135, 6136, 6137, 6138, 6139, 6149, 6150, 6151, 6152, 6153, 6154, 6155 |

---

## Build (on GPVM)

```bash
cd DimuAnaExp
source setup.sh   # loads e1039 environment, defines cmake-this / make-this
cmake-this        # configures build from src/CMakeLists.txt → build/
make-this         # compiles and installs library into inst/
```

The library `inst/lib/libdimu_ana_rus.so` must exist before running the macro.

---

## Generate file lists

Run once on GPVM to populate `list_up.txt` and `list_down.txt`:

```bash
cd Convert
bash make_lists.sh
```

---

## Run

### Local test (single file)

```bash
cd Convert
source ../setup.sh
root -l -b -q "Fun4All.C(\"path/to/input_spin_reco.root\", \"output.root\", 0)"
```

### Grid / local batch via `gridsub.sh`

Select the input list at the top of `gridsub.sh` (line 4):
```bash
LIST_VEC=list_up.txt    # or list_down.txt
```

Then submit:

```bash
# Submit all files to the grid
bash gridsub.sh -n <job_name> -g

# Process only rows N–M of the list (1-based)
bash gridsub.sh -n <job_name> -g -j 1-5

# Run locally (no grid), overwrite existing output
bash gridsub.sh -n <job_name> -o
```

| Flag | Meaning |
|---|---|
| `-n <name>` | Job name — used as output subdirectory |
| `-g` | Submit to grid via jobsub |
| `-j N-M` | Process only rows N through M of the input list |
| `-o` | Overwrite existing job output directories |
| `-m <N>` | Max concurrent grid jobs (throttle) |

Output lands in:
- **Grid**: `/pnfs/e1039/scratch/mhossain/vec_data_in/<job_name>/job_N/out/out_<filename>.root`
- **Local**: `Convert/scratch/<job_name>/job_N/out/out_<filename>.root`

---

## Output tree branches

The output ROOT file contains a tree named `tree` with one entry per event (MATRIX1-triggered events only).

### Event-level
| Branch | Type | Description |
|---|---|---|
| `eventID` | `int` | Event ID |
| `runID` | `int` | Run number |
| `spillID` | `int` | Spill number |

### Dimuon (per dimuon in event)
| Branch | Type | Description |
|---|---|---|
| `rec_dimuon_x/y/z` | `vector<double>` | Dimuon vertex position |
| `rec_dimuon_px/py/pz_pos` | `vector<double>` | μ⁺ momentum at vertex |
| `rec_dimuon_px/py/pz_neg` | `vector<double>` | μ⁻ momentum at vertex |
| `rec_dimuon_px/py/pz_pos_tgt` | `vector<double>` | μ⁺ momentum (target hypothesis) |
| `rec_dimuon_px/py/pz_neg_tgt` | `vector<double>` | μ⁻ momentum (target hypothesis) |
| `rec_dimuon_px/py/pz_pos_dump` | `vector<double>` | μ⁺ momentum (dump hypothesis) |
| `rec_dimuon_px/py/pz_neg_dump` | `vector<double>` | μ⁻ momentum (dump hypothesis) |
| `rec_dimuon_x/y/z_pos_vtx` | `vector<double>` | μ⁺ individual track vertex position |
| `rec_dimuon_x/y/z_neg_vtx` | `vector<double>` | μ⁻ individual track vertex position |
| `rec_dimuon_px/py/pz_pos_vtx` | `vector<double>` | μ⁺ momentum at individual vertex |
| `rec_dimuon_px/py/pz_neg_vtx` | `vector<double>` | μ⁻ momentum at individual vertex |
| `rec_dimuon_x/y/z_pos/neg_st1` | `vector<double>` | μ±  position at station 1 |
| `rec_dimuon_px/py/pz_pos/neg_st1` | `vector<double>` | μ± momentum at station 1 |
| `rec_dimuon_x/y/z_pos/neg_st3` | `vector<double>` | μ± position at station 3 |
| `rec_dimuon_px/py/pz_pos/neg_st3` | `vector<double>` | μ± momentum at station 3 |
| `rec_dimuon_roads` | `vector<int>` | 1 if top-bottom road combination satisfied |
| `rec_dimuon_track_id_pos/neg` | `vector<int>` | Index into the track vector |

### Track (per track in event)
| Branch | Type | Description |
|---|---|---|
| `rec_track_charge` | `vector<int>` | Track charge |
| `rec_track_vx/vy/vz` | `vector<double>` | Track vertex position |
| `rec_track_px/py/pz` | `vector<double>` | Track momentum at vertex |
| `rec_track_x/y/z_st1` | `vector<double>` | Position at station 1 |
| `rec_track_px/py/pz_st1` | `vector<double>` | Momentum at station 1 |
| `rec_track_x/y/z_st3` | `vector<double>` | Position at station 3 |
| `rec_track_px/py/pz_st3` | `vector<double>` | Momentum at station 3 |
| `rec_track_x/y/z_tgt` | `vector<double>` | Position extrapolated to target |
| `rec_track_px/py/pz_tgt` | `vector<double>` | Momentum at target |
| `rec_track_x/y/z_dump` | `vector<double>` | Position at dump face |
| `rec_track_px/py/pz_dump` | `vector<double>` | Momentum at dump face |
| `rec_track_chisq` | `vector<double>` | Total χ² |
| `rec_track_chisq_tgt` | `vector<double>` | χ² for target vertex hypothesis |
| `rec_track_chisq_dump` | `vector<double>` | χ² for dump vertex hypothesis |
| `rec_track_chisq_upstream` | `vector<double>` | χ² for upstream vertex hypothesis |
| `rec_track_num_hits` | `vector<int>` | Number of hits on track |
| `rec_track_hit_x/y` | `vector<vector<double>>` | Hit positions |
