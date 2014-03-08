#!/bin/sh
cd "$(dirname $0)" &&
maj=$(cat current_version | cut -d . -f 1) &&
min=$(cat current_version | cut -d . -f 2) &&
cd pics &&
template=logo.inkscape.1.x.svg &&
curlogo=logo-$maj.$min &&
perl -pe 's:>1\.23<:>'$maj.$min'<:' < $template > $curlogo.svg &&
ln -sf $curlogo.svg logo-current.svg &&
DISPLAY= inkscape -d=55 -e $curlogo.png $curlogo.svg &&
ln -sf $curlogo.png logo-current.png
