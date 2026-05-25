#!/bin/bash
# run_local.sh — run Fun4All locally on GPVM for a single spin_reco.root file
# Usage: bash run_local.sh <input_spin_reco.root> [output.root] [n_events]
#
# Example (single spill test):
#   bash run_local.sh \
#     /pnfs/e1039/persistent/users/kenichi/RecoData2024/reco-20260512/run_006111/spill_007314288_data/out/run_006111_spill_007314288_data_spin_reco.root \
#     test_output.root \
#     0

set -euo pipefail

DIR_TOP="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
source "${DIR_TOP}/setup.sh"

INPUT=${1:?"Usage: $0 <input_spin_reco.root> [output.root] [n_events]"}
OUTPUT=${2:-"${DIR_TOP}/test_output.root"}
N_EVT=${3:-0}

echo "==> Input:   ${INPUT}"
echo "==> Output:  ${OUTPUT}"
echo "==> N_evt:   ${N_EVT}  (0 = all)"

cd "${DIR_TOP}/Convert"
time root -l -b -q "Fun4All.C(\"${INPUT}\", \"${OUTPUT}\", ${N_EVT})"
