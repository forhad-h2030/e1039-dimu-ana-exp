#!/bin/bash
# run_all.sh — run from the Convert directory on GPVM
# Processes each run into a separate ROOT file.
# Skips runs whose output already exists — safe to re-run after a crash.
# bash run_all.sh

set -eo pipefail

DIR_TOP="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
source "${DIR_TOP}/setup.sh"

CONVERT="${DIR_TOP}/Convert"
OUTDIR_UP="${DIR_TOP}/out/up"
OUTDIR_DOWN="${DIR_TOP}/out/down"
mkdir -p "${OUTDIR_UP}" "${OUTDIR_DOWN}"

BASE=/pnfs/e1039/persistent/users/kenichi/RecoData2024/reco-20260512

UP_RUNS=(6111 6112 6113 6114 6115 6116 6117 6118 6156)
DOWN_RUNS=(6135 6136 6137 6138 6139 6149 6150 6151 6152 6153 6154 6155)

cd "${CONVERT}"

process_runs() {
    local outdir=$1
    shift
    local runs=("$@")

    for run in "${runs[@]}"; do
        run_fmt=$(printf "%06d" ${run})
        outfile="${outdir}/run_${run_fmt}.root"

        if [ -f "${outfile}" ]; then
            echo "==> [SKIP] run_${run_fmt} — output already exists"
            continue
        fi

        # Build per-run spill list
        list_tmp=$(mktemp /tmp/list_${run_fmt}_XXXXXX.txt)
        run_dir="${BASE}/run_${run_fmt}"

        if ! ls "${run_dir}" &>/dev/null; then
            echo "==> [WARN] run_${run_fmt} — directory not accessible, skipping"
            rm -f "${list_tmp}"
            continue
        fi

        for spill_dir in "${run_dir}"/spill_*/; do
            spill=$(basename "${spill_dir}")
            reco="${spill_dir}out/run_${run_fmt}_${spill}_spin_reco.root"
            if ls "${reco}" &>/dev/null; then
                echo "${reco}" >> "${list_tmp}"
            fi
        done

        n_spills=$(wc -l < "${list_tmp}")
        if [ "${n_spills}" -eq 0 ]; then
            echo "==> [WARN] run_${run_fmt} — no spill files found, skipping"
            rm -f "${list_tmp}"
            continue
        fi

        echo "==> run_${run_fmt}: ${n_spills} spills → $(basename ${outfile})"
        time root -l -b -q "Fun4All.C(\"${list_tmp}\", \"${outfile}\", 0)"
        rm -f "${list_tmp}"
        echo ""
    done
}

echo "==> Processing spin-UP runs ..."
process_runs "${OUTDIR_UP}" "${UP_RUNS[@]}"

echo ""
echo "==> Processing spin-DOWN runs ..."
process_runs "${OUTDIR_DOWN}" "${DOWN_RUNS[@]}"

echo ""
echo "==> All done."
echo "    UP   (${#UP_RUNS[@]} runs): $(ls ${OUTDIR_UP}/*.root 2>/dev/null | wc -l) files in out/up/"
echo "    DOWN (${#DOWN_RUNS[@]} runs): $(ls ${OUTDIR_DOWN}/*.root 2>/dev/null | wc -l) files in out/down/"
