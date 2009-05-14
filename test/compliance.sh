#!/bin/bash
# just picking samples from the host of available bitstreams
files=("$(dirname $0)/compliance/layer1/fl*.mpg" "$(dirname $0)/compliance/layer2/fl*.mpg" "$(dirname $0)/compliance/layer3/compl.bit")
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


for l in 0 1 2
do
  let lay=$l+1
  echo
  echo "==== Layer $lay ===="

  echo "--> 16 bit signed integer output"
  for f in ${files[$l]}
  do
    bit=${f/mpg/bit}
    double=${f/mpg/double}
    double=${double/bit/double}
    if [[ -e "$bit" ]] && [[ -e "$double" ]]; then
       echo -n "$(basename $bit):	"
       $@ $nogap -q  -s "$bit" | "$s16conv" | "$rms" "$double" 2>/dev/null
    else
      echo "Layer $lay files are missing... do a make in the test dir!"
    fi
  done

  if $@ --longhelp 2>&1 | grep -q f32 ; then
    echo
    echo "--> 32 bit floating point output"
    for f in ${files[$l]}
    do
      bit=${f/mpg/bit}
      double=${f/mpg/double}
      double=${double/bit/double}
      if [[ -e "$bit" ]] && [[ -e "$double" ]]; then
        echo -n "$(basename $bit):	"
        $@ $nogap -q -e f32 -s "$bit" | "$f32conv" | "$rms" "$double" 2>/dev/null
      fi
    done
  fi
done
