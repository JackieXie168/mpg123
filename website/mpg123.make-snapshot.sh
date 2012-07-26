#!/bin/bash
work=`mktemp -p /tmp -d mpg123-svn.XXXXX`

date=$(date +%Y%m%d%H%M%S)
name=mpg123-$date
base=/www/mpg123
sdir=snapshot
location=$base/$sdir/$name.tar.bz2
compliance=$base/compliance.log
regression=$base/regression.log

echo "Starting compliance tests ..." > $compliance
echo "Starting regression tests ..." > $regression

echo workdir: $workbase
cd $work &&
svn co svn://localhost/mpg123/test &&
cd $work/test && make &&
svn co svn://localhost/mpg123/trunk &&
cd trunk &&
perl -pi -e 's:(AC_INIT\(\[mpg123\],\s+)\[[^\]]+\]:$1\['$date'\]:' configure.ac &&
autoreconf -iv &&
svn2cl &&
./configure &&
make distcheck &&
mv $name.tar.gz $work &&
./configure --prefix=$work/prefix --enable-int-quality && make && make install
( echo "compliance test for $name" &&
echo "First decoder in this list will be tested first, then generic: " &&
$work/prefix/bin/mpg123 --test-cpu &&
echo &&
echo "Testing default decoder..." &&
perl $work/test/compliance.pl $work/prefix/bin/mpg123 &&
echo &&
echo "Now the generic decoder:" &&
perl $work/test/compliance.pl $work/prefix/bin/mpg123 --cpu generic
) > $compliance &&
cd $work/test/regression &&
MPG123_PREFIX=$work/prefix make &> $regression
cd $work &&
gunzip $name.tar.gz &&
bzip2 --best $name.tar &&
mv -v $name.tar.bz2 $location &&
chmod a+r $location &&
perl -pi -e 's/^(RewriteRule \^snapshot.*mpg123-)\d+\./${1}'$date'./' $base/.htaccess &&
cd / && rm -rf $work
