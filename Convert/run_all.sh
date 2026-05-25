#!/bin/bash
# run_all.sh — run from the Convert directory on GPVM
# Processes all spin-up and spin-down runs into two output ROOT files.
# bash run_all.sh

set -eo pipefail

DIR_TOP="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
source "${DIR_TOP}/setup.sh"

CONVERT="${DIR_TOP}/Convert"
OUTDIR="${DIR_TOP}"

LIST_UP="${CONVERT}/list_up.txt"
LIST_DOWN="${CONVERT}/list_down.txt"

# Generate lists if they don't exist
if [ ! -f "${LIST_UP}" ] || [ ! -f "${LIST_DOWN}" ]; then
    echo "==> Lists not found — running make_lists.sh ..."
    bash "${CONVERT}/make_lists.sh"
fi

echo ""
echo "==> list_up.txt   : $(wc -l < ${LIST_UP}) files"
echo "==> list_down.txt : $(wc -l < ${LIST_DOWN}) files"
echo ""

cd "${CONVERT}"

echo "==> Processing spin-UP ..."
time root -l -b -q "Fun4All.C(\"${LIST_UP}\", \"${OUTDIR}/output_up.root\", 0)"

echo ""
echo "==> Processing spin-DOWN ..."
time root -l -b -q "Fun4All.C(\"${LIST_DOWN}\", \"${OUTDIR}/output_down.root\", 0)"

echo ""
echo "==> Done."
echo "    ${OUTDIR}/output_up.root"
echo "    ${OUTDIR}/output_down.root"
