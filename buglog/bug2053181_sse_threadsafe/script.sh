#!/bin/sh
LD_LIBRARY_PATH=/data/mpg123/svn/trunk/test/lib; export LD_LIBRARY_PATH
rm results.csv
./main cond1 SSE /data/thorma/var/music/mike_lehmann/kannste_abhaken/01-vinyl.mp3
./main cond2 SSE /data/thorma/var/music/mike_lehmann/kannste_abhaken/01-vinyl.mp3
./main cond3 SSE /data/thorma/var/music/mike_lehmann/kannste_abhaken/01-vinyl.mp3
./main cond1 MMX /data/thorma/var/music/mike_lehmann/kannste_abhaken/01-vinyl.mp3
./main cond2 MMX /data/thorma/var/music/mike_lehmann/kannste_abhaken/01-vinyl.mp3
./main cond3 MMX /data/thorma/var/music/mike_lehmann/kannste_abhaken/01-vinyl.mp3
./main cond1 i586 /data/thorma/var/music/mike_lehmann/kannste_abhaken/01-vinyl.mp3
./main cond2 i586 /data/thorma/var/music/mike_lehmann/kannste_abhaken/01-vinyl.mp3
./main cond3 i586 /data/thorma/var/music/mike_lehmann/kannste_abhaken/01-vinyl.mp3
./main cond1 i386 /data/thorma/var/music/mike_lehmann/kannste_abhaken/01-vinyl.mp3
./main cond2 i386 /data/thorma/var/music/mike_lehmann/kannste_abhaken/01-vinyl.mp3
./main cond3 i386 /data/thorma/var/music/mike_lehmann/kannste_abhaken/01-vinyl.mp3
./main cond1 generic /data/thorma/var/music/mike_lehmann/kannste_abhaken/01-vinyl.mp3
./main cond2 generic /data/thorma/var/music/mike_lehmann/kannste_abhaken/01-vinyl.mp3
./main cond3 generic /data/thorma/var/music/mike_lehmann/kannste_abhaken/01-vinyl.mp3
