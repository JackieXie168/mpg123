#!/bin/bash
res=resampled
rat=rated
./lame --resample 22.05 -r -x source.raw $res.mp3
./lame -s 22.05 -r -x source.raw $rat.mp3
decoders=`seq 0 2`
decname=("newmpg123" "oldmpg123" "lame")
decflags=("-s --gapless" "-s" "--decode -t ")
styles=("p" "l" "l")
decredirect=(">" ">" "") 

echo 'set xrange[0:2000]' > $res.begin.plot
echo 'set xrange[0:4000]' > $rat.begin.plot
echo 'set xrange[19000:22000]' > $res.end.plot
echo 'set xrange[38000:44000]' > $rat.end.plot

for o in $res $rat
do
	echo 'plot \' > $o.plots.plot
	echo 'set x2range[0:4000]' >> $o.begin.plot
	echo 'set x2range[38000:44000]' >> $o.end.plot
	for place in begin end
	do
		{
			echo "set title \"lame resampling cutting off? - $o $place\""
			echo 'set xlabel "output sample"'
			echo 'set x2label "input sample"'
			echo 'set ylabel "output level"'
			echo 'set y2label "input level"'
			echo 'set yrange[-20000:30000]'
			echo 'set y2range[-40000:60000]'
			echo 'set x2tics'
			echo 'set y2tics'
			echo 'set key box'
			echo "load '$o.plots.plot'"
		} >> $o.$place.plot 
	done
	for i in $decoders
	do
		bash -c "./${decname[$i]} ${decflags[$i]} $o.mp3 ${decredirect[$i]} $o.${decname[$i]}.raw"
		echo "'$o.${decname[$i]}.raw' binary format=\"%short%short\" using 1 axes x1y1 with ${styles[$i]} title \"${decname[$i]}\", \\" >> $o.plots.plot
	done
	echo "'source.raw' binary format=\"%short%short\" using 1 axes x2y2 with l title \"source\"" >> $o.plots.plot
	#creating png plots using my li'l frontend and recent gnuplot
	if [[ -x `which buntstift` ]]
	then
		for place in begin end
		do
			buntstift -t=png -o="size 800,600" $o.$place.plot
		done
	else
		echo buntstift not found... go plotting yourself
	fi
done

