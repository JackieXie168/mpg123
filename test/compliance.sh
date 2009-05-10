#!/bin/bash
# just picking samples from the host of available bitstreams
l1="$(dirname $0)/compliance/layer1/fl1"
l2="$(dirname $0)/compliance/layer2/fl10"
l3="$(dirname $0)/compliance/layer3/compl"
rms="$(dirname $0)/rmsdouble.bin"
f32conv="$(dirname $0)/f32_double.bin"
s16conv="$(dirname $0)/s16_double.bin"

if [[ -e "$rms" ]] && [[ -e "$f32conv" ]] && [[ -e "$s16conv" ]]; then
  true
else
  echo "The test programs are missing, please run 'make' in $(dirname $0)"
fi

nogap=
$@ --longhelp 2>&1 | grep -q no-gapless && nogap=--no-gapless


for lay in 1 2 3
do
  echo
  echo "==== Layer $lay ===="
  eval "bit=\$l$lay.bit"
  eval "double=\$l$lay.double"
  if [[ -e "$bit" ]] && [[ -e "$double" ]]; then
    echo "layer $lay 16bit compliance (unlikely...)"
$@ $nogap -q  -s "$bit" | "$s16conv" | "$rms" "$double" 2>/dev/null
    if $@ --longhelp 2>&1 | grep -q f32 ; then
    echo "layer $lay float compliance (hopefully fine)"
$@ $nogap -q -e f32 -s "$bit" | "$f32conv" | "$rms" "$double" 2>/dev/null
    fi
  else
    echo "Layer $lay files are missing... do a make in the test dir!"
  fi
done
