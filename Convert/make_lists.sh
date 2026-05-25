#!/bin/bash
# make_lists.sh  — run on GPVM
# Scans kenichi's reco-20260512 directory and writes list_up.txt / list_down.txt
#
# Spin UP  : runs 6111-6118, 6156
# Spin DOWN: runs 6135-6139, 6149-6155

BASE=/pnfs/e1039/persistent/users/kenichi/RecoData2024/reco-20260512
OUTDIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

UP_RUNS=(6111 6112 6113 6114 6115 6116 6117 6118 6156)
DOWN_RUNS=(6135 6136 6137 6138 6139 6149 6150 6151 6152 6153 6154 6155)

make_list() {
    local outfile=$1
    shift
    local runs=("$@")
    > "${outfile}"   # truncate / create
    for run in "${runs[@]}"; do
        run_dir=$(printf "%s/run_%06d" "${BASE}" "${run}")
        if ! ls "${run_dir}" &>/dev/null ; then
            echo "  [WARN] ${run_dir} not accessible, skipping"
            continue
        fi
        for spill_dir in "${run_dir}"/spill_*/; do
            spill=$(basename "${spill_dir}")
            reco="${spill_dir}out/run_$(printf '%06d' ${run})_${spill}_spin_reco.root"
            if ls "${reco}" &>/dev/null ; then
                echo "${reco}" >> "${outfile}"
            fi
        done
        echo "  run_$(printf '%06d' ${run}): $(grep -c "run_$(printf '%06d' ${run})" ${outfile} 2>/dev/null || echo 0) spills"
    done
    echo "  → $(wc -l < ${outfile}) total files written to $(basename ${outfile})"
}

echo "==> Building list_up.txt (spin up: ${UP_RUNS[*]}) ..."
make_list "${OUTDIR}/list_up.txt"   "${UP_RUNS[@]}"

echo ""
echo "==> Building list_down.txt (spin down: ${DOWN_RUNS[*]}) ..."
make_list "${OUTDIR}/list_down.txt" "${DOWN_RUNS[@]}"

echo ""
echo "Done."
