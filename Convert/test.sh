#!/bin/bash
# test.sh — run from the Convert directory on GPVM
# bash test.sh

DIR_TOP="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
source "${DIR_TOP}/setup.sh"

INPUT=/pnfs/e1039/persistent/users/kenichi/RecoData2024/reco-20260512/run_006111/spill_001936172/out/run_006111_spill_001936172_spin_reco.root
OUTPUT=${DIR_TOP}/test_output.root

echo "==> Input:  ${INPUT}"
echo "==> Output: ${OUTPUT}"

cd "${DIR_TOP}/Convert"
time root -l -b -q "Fun4All.C(\"${INPUT}\", \"${OUTPUT}\", 0)"
