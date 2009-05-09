#!/bin/bash
# just picking samples from the host of available bitstreams
l1="$(dirname $0)/compliance/layer1/fl1"
l2="$(dirname $0)/compliance/layer2/fl10"
l3="$(dirname $0)/compliance/layer3/compl"
rms="$(dirname $0)/rmsdouble.bin"
f32conv="$(dirname $0)/f32_double.bin"
s16conv="$(dirname $0)/s16_double.bin"

for lay in 1 2 3
do
  echo
  echo "==== Layer $lay ===="
  eval "bit=\$l$lay.bit"
  eval "double=\$l$lay.double"
  if [[ -e "$bit" ]] && [[ -e "$double" ]]; then
    echo "layer $lay 16bit compliance (unlikely...)"
$@ --no-gapless -q -e s16 -s "$bit" | "$s16conv" | "$rms" "$double" 2>/dev/null
    echo "layer $lay float compliance (hopefully fine)"
$@ --no-gapless -q -e f32 -s "$bit" | "$f32conv" | "$rms" "$double" 2>/dev/null
  else
    echo "Layer $lay files are missing... do a make in the test dir!"
  fi
done
