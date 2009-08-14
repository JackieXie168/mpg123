#!/bin/bash
work=`mktemp -p /tmp -d mpg123-svn.XXXXX`
date=$(date +%Y%m%d%H%M%S)
name=mpg123-$date
base=/www/mpg123
sdir=snapshot
location=$base/$sdir/$name.tar.bz2
compliance=$base/compliance.log

echo workdir: $work
cd $work &&
svn co svn://localhost/mpg123/trunk &&
cd trunk &&
perl -pi -e 's:(AC_INIT\(\[mpg123\],\s+)\[[^\]]+\]:$1\['$date'\]:' configure.ac &&
./autogen.sh &&
svn2cl &&
./configure &&
make distcheck &&
mv $name.tar.gz .. &&
./configure --disable-shared --enable-int-quality && make &&
( echo "compliance test for $name" &&
echo "First decoder in this list will be tested first, then generic: " &&
src/mpg123 --test-cpu &&
echo &&
echo "Testing default decoder..." &&
perl ~thomas/mpg123-test/compliance.pl src/mpg123 &&
echo &&
echo "Now the generic decoder:" &&
perl ~thomas/mpg123-test/compliance.pl src/mpg123 --cpu generic
) > $compliance &&
cd .. &&
gunzip $name.tar.gz &&
bzip2 --best $name.tar &&
mv -v $name.tar.bz2 $location &&
chmod a+r $location &&
perl -pi -e 's/^(RewriteRule \^snapshot.*mpg123-)\d+\./${1}'$date'./' $base/.htaccess &&
cd / && rm -rf $work
