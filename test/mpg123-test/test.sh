#!/bin/bash

#### VERSION HISTORY ###########################################################
#
# V1.0      2009-05-25    Andreas Neustifter
#                         - initial version
#
# V2.0      2009-05-26    Andreas Neustifter
#                         - added INTERACTIVE switch that completey turns of
#                           any audio output and simply runs non-interactive
#                           parts of script
#                         - clearer output for interactive mode during
#                           generation of reference files
#                         - removed check for -g parameter
#                         - added version history
#                         - added note about ascii vs. utf8 output for option -c
#                         - "play" more reference files during BEPICKY=auto due
#                           to better recognition of text files (grep utf|ascii)
#
# V2.1      2009-05-30    Andreas Neustifter
#                         - reenabled -g tests
#
# V2.2      2009-06-12    Andreas Neustifter
#                         - added -C option to -g for user to be able to skip
#
# V2.3      2009-07-29    Andreas Neustifter
#                         - added showing of differences when not halting on
#                           error
#                         - do not remove erronous output
#                         - filter out filenames in warning messages (this 
#                           ensures comparability of output)
#                         - do not output error when --version output changes
#                         - more flexible version masking
#
# V2.4      2009-08-20    Andreas Neustifter
#                         - added OpenAL output test
#                         - show at startup which binary is used
#
################################################################################

#### DEFAULT SETTINGS ##########################################################
# these defines the default settings for the script

# the mpg123 binary to test
BINFILE=mpg123

# master switch for running the script interactively or not
INTERACTIVE=true

# the prefix for the test files, there have to be two files present, one named
# $TESTFILE.mp3 and one named $TESTFILE.mp2. preferably the mp3 file contains
# garbage bytes.
TESTFILE=test

# whether the script should halt on error (miscompare with reference file) or
# not
HALTONERROR=true

# be picky, for each newly created reference file the script stops and asks
# user to check validity of the new file, possible values are
# true           the script stops after the creation of each reference file
# auto           the script "plays" each reference file and asks if it "sounds"
#                okay. (text files are cated, audio files are played.)
# false          just generated reference files silently
if [[ $INTERACTIVE == true ]]; then
    BEPICKY=auto
else
    BEPICKY=false
fi

#### CHECK OR CREATE REFERENCE FILE ############################################
# this function checks a given file ($1) against a given reference file ($2) or
# creates such reference file
checkorcreatereffile() {
    # its no use if the file to check does not exist...
    if [[ ! -e "$1" ]]; then
        continue
    fi
    # if reference file exists, compare current output to reference
    if [[ -e "$2" ]]; then
        if ! diff --binary -q "$2" "$1" > /dev/null; then
            echo "ERROR: diff \"$2\" \"$1\""
            if [[ $HALTONERROR == true ]]; then exit; fi
            diff "$2" "$1"
            echo "ERROR ignored (press key or wait to continue)"
            if [[ $INTERACTIVE == true ]]; then
                read -n 1 -t 5 -s
            fi
        else
            rm "$1"
        fi
    # if reference file does not exist, create one
    else
        mv "$1" "$2"
        # depending on picky setting, perform action
        if [[ $BEPICKY == true ]]; then
            echo "-------- being picky, please check newly created reference file \"$2\" manually"
            exit
        elif [[ $BEPICKY == auto ]] && [[ -s "$2" ]]; then
            echo "-------- playing new reference file \"$2\", please check if output is as expected"
            echo -n "-------- file type:"; file "$2"
            if file "$2" | grep -q '\(UTF-8\|ASCII\)'; then
                cat "$2"
            else
                mplayer "$2" > /dev/null 2>&1
            fi
            read -n 1 -p "-------- was this okay? (y/n) " ANSWER; echo
            if [[ $ANSWER != y ]]; then
                rm "$2"
                exit
            fi
        fi
    fi
}

#### CHECK OPTION ##############################################################
# this function performs the check of a single option. it does: 
# * print a header and optional note
# * executes test, generating wav and txt output
# * compares the wav and txt output against the reference files (or saves them
#   as reference files)
#
# the option or paramter to test is given as arguments to the function.
# the optional variable NOTE can be set to print a note before the start of the
# test
# the optional variable OUTPUT can choose a different output from wav files
# currently allowed values are wav and custom
NOTE=""
OUTPUT=wav
checkoption() {
    # print header and note
    echo "---- checking \"$*\""
    if [[ -n $NOTE ]]; then
        echo "-------- NOTE: $NOTE"
        NOTE=""
    fi

    FILE="test.`echo "$*" | tr -d '\/'`"
    REFFILE="$FILE".ref

    # execute test
    if [[ $OUTPUT == wav ]]; then
        $BINFILE -w "$FILE".wav $* 1>1.file 2>2.file
        cat 1.file | grep -v 'version [0-9.]*;' | sed -e 's/^\[.*:.*\] //' > "$FILE".out
        cat 2.file | grep -v 'version [0-9.]*;' | sed -e 's/^\[.*:.*\] //' > "$FILE".txt
        rm 1.file 2.file
    elif [[ $OUTPUT == custom ]]; then
        $BINFILE $* 1>1.file 2>2.file
        mv 1.file "$FILE".out
        cat 2.file | grep -v 'version [0-9.]*;' | sed -e 's/^\[.*:.*\] //' > "$FILE".txt
        rm 2.file
    else
        echo "-------- invalid option to \$OUTPUT"
        exit
    fi

    # check for each extension if output exists, if yes, test against reference
    # file or save current output as refernce
    for EXT in ".wav" ".txt" ".out"; do
        checkorcreatereffile "$FILE$EXT" "$REFFILE$EXT"
    done

    echo
}

#### REMOVE JUNK ###############################################################
# this function removes junk from previous runs
removejunk() {
    rm -f playlist.txt
    for (( i = 0; i < 2; i++ )); do
        for EXT in mp2 mp3; do
            rm -f "$TESTFILE.$i.$EXT"
        done
    done
}

#### START OF SCRIPT ###########################################################
removejunk

#### PATH message #############################################################
echo ---- testing $(which mpg123)
read -n 1 -s -t 5

# check for existence of binary- and test-files, this prevents strange messages
# in checkoption()
if [[ ! -x `which $BINFILE` ]]; then
    echo "ERROR with execution of $BINFILE"
    exit
fi
for EXT in mp2 mp3; do
    if [[ ! -r $TESTFILE.$EXT ]]; then
        echo "ERROR with finding $TESTFILE.$EXT"
        exit
    fi
done

#### START OF ACTUAL TESTS #####################################################
# these tests are ordered according to the manpage tests that are still not
# implemented are tagged with #open
checkoption "$TESTFILE.mp3"

#       -k num, --skip num
#              Skip first num frames.  By default the decoding starts at the
#              first frame.
checkoption -k 64 "$TESTFILE.mp3"

#       -n num, --frames num
#              Decode only num frames.  By default the complete stream is
#              decoded.
checkoption -n 64 "$TESTFILE.mp3"

#       --fuzzy
#              Enable fuzzy seeks (guessing byte offsets or using approximate
#              seek points from Xing TOC).  Without that, seeks need a first
#              scan through the file before they can jump at positions.   You
#              can decide here: sample-accurate operation with gapless features
#              or faster (fuzzy) seeking.
checkoption --fuzzy -k 64 "$TESTFILE.mp3"
checkoption --fuzzy -n 64 "$TESTFILE.mp3"

#       -y, --no-resync
#              Do  NOT  try  to resync and continue decoding if an error occurs
#              in the input file. Normally, mpg123 tries to keep the playback
#              alive at all costs, including skipping invalid material and
#              searching new header when something goes wrong.  With this
#              switch you can make it bail out on data errors (and perhaps
#              spare your ears a bad time). Note that this switch has been
#              renamed from --resync.  The old name still works, but is not
#              advertised or recommened to use (subject to removal in future).
checkoption -y "$TESTFILE.mp3"

#
#       --resync-limit bytes
#              Set number of bytes to search for valid MPEG data; <0 means
#              search whole stream.  If you know there are huge chunks of
#              invalid data in your files... here is your hammer.
for i in 700 900 -1; do
    checkoption --resync-limit $i "$TESTFILE.mp3"
done

checkoption 'http://web.student.tuwien.ac.at/~e0325716/mpg123-test/test.mp3'

#       -p URL | none, --proxy URL | none
#              The  specified proxy will be used for HTTP requests.  It should
#              be specified as full URL (‘‘http://host.domain:port/’’), but the
#              ‘‘http://’’ prefix, the port number and the trailing slash are
#              optional (the default port is 80).  Specifying none means not to
#              use any proxy, and to retrieve files directly from the
#              respective servers.  See also the ‘‘HTTP SUPPORT’’ section.
#open

#       -u auth, --auth auth
#              HTTP authentication to use when recieving files via HTTP.  The
#              format used is user:password.
#open

#       -@ file, --list file
#              Read filenames and/or URLs of MPEG audio streams from the
#              specified file in addition to the ones specified on the command
#              line (if any).  Note that file can be either an ordinary file, a
#              dash  ‘‘-’’  to  indicate  that a list of filenames/URLs is to
#              be read from the standard input, or an URL pointing to a an
#              appropriate list file.  Note: only one -@ option can be used (if
#              more than one is specified, only the last one will be
#              recognized).
for (( i = 0; i < 2; i++ )); do
    for EXT in mp3 mp2; do
        ln -s "$TESTFILE.$EXT" "$TESTFILE.$i.$EXT"
        echo "$TESTFILE.$i.$EXT" >> playlist.txt
    done
done
checkoption --list playlist.txt
checkoption --list fritz -@ playlist.txt

#       -l n, --listentry n
#              Of the playlist, play specified entry only.  n is the number of
#              entry starting at 1. A value of 0 is the default and means
#              playling the whole list,  a negative value means showing of  the
#              list of titles with their numbers...
checkoption --list playlist.txt -l 2
checkoption --list playlist.txt -l 0
checkoption --list playlist.txt -l -9

#
#       --loop times
#              for looping track(s) a certain number of times, < 0 means
#              infinite loop (not with --random!).
checkoption --loop 3 "$TESTFILE.mp3"

#       --keep-open
#              For remote control mode: Keep loaded file open after reaching
#              end.
# tested in combination with -R and the other remote stuff

#       --timeout seconds
#              Timeout in (integer) seconds before declaring a stream dead (if
#              <= 0, wait forever).
#open

#       -z, --shuffle
#              Shuffle play.  Randomly shuffles the order of files specified on
#              the command line, or in the list file.
HALTONERROR=false
NOTE="testing shuffle, errors are to be expected..."
checkoption --list playlist.txt -z
HALTONERROR=true

#       -Z, --random
#              Continuous random play.  Keeps picking a random file from the
#              command line or the play list.  Unlike shuffle play above,
#              random play never ends, and plays individual songs more than
#              once.
# tested in combination with -C at end of script

#       --no-icy-meta
#              Do not accept ICY meta data.
#open

#       -i, --index
#              Index / scan through the track before playback.  This fills the
#              index table for seeking (if enabled in libmpg123) and may make
#              the operating system cache the file  contents  for  smoother
#              operating on playback.
checkoption -i "$TESTFILE.mp3"

#OUTPUT and PROCESSING OPTIONS
#       -o module, --output module
#              Select audio output module. You can provide a comma-separated
#              list to use the first one that works.
if [[ $INTERACTIVE == true ]]; then
    HALTONERROR=false
    OUTPUT=custom
    for i in alsa oss coreaudio sndio sun win32 esd jack portaudio pulse sdl openal nas arts dummy; do
        NOTE="testing output $i, errors are to be expected, skip with 'q'"
        checkoption -C -o $i "$TESTFILE.mp3"
    done
    HALTONERROR=true
    OUTPUT=wav
else
    echo "---- running interactively, actual output on audio modules is disabled"
    echo
fi

#
#       --list-modules
#              List the available modules.
checkoption --list-modules

#       -a dev, --audiodevice dev
#              Specify  the  audio  device  to use.  The default is
#              system-dependent (usually /dev/audio or /dev/dsp).  Use this
#              option if you have multiple audio devices and the default is not
#              what you want.
#open

#       -s, --stdout
#              The decoded audio samples are written to standard output,
#              instead of playing them through the audio device.  This option
#              must be used if your audio hardware is not  supported  by
#              mpg123.  The output format per default is raw (headerless)
#              linear PCM audio data, 16 bit, stereo, host byte order (you can
#              force mono or 8bit).
OUTPUT=custom
checkoption -s "$TESTFILE.mp3"

#       -O file, --outfile
#              Write raw output into a file (instead of simply redirecting
#              standard output to a file with the shell).
checkoption -O "test.-O.raw" "$TESTFILE.mp3"
checkorcreatereffile "test.-O.raw" "test.-O.ref.raw"

#       -w file, --wav
#              Write  output  as WAV file. This will cause the MPEG stream to
#              be decoded and saved as file file , or standard output if - is
#              used as file name. You can also use --au and --cdr for AU and
#              CDR format, respectively.
checkoption --wav - "$TESTFILE.mp3"

#       --au file
#              Does not play the MPEG file but writes it to file in SUN audio
#              format.  If - is used as the filename, the AU file is written to
#              stdout.
checkoption --au "test.--au.au" "$TESTFILE.mp3"
checkorcreatereffile "test.--au.au" "test.--au.ref.au"

#       --cdr file
#              Does not play the MPEG file but writes it to file as a CDR file.
#              If - is used as the filename, the CDR file is written to stdout.
checkoption --cdr "test.--cdr.cdr" "$TESTFILE.mp3"
checkorcreatereffile "test.--cdr.cdr" "test.--cdr.ref.cdr"
OUTPUT=wav

#       --reopen
#              Forces reopen of the audiodevice after ever song
checkoption --reopen "$TESTFILE.mp3"  "$TESTFILE.mp3"

#       --cpu decoder-type
#              Selects a certain decoder (optimized for specific CPU), for
#              example i586 or MMX.  The list of available decoders can vary;
#              depending on the build and what your CPU supports.  This options
#              is only availabe when the build actually includes several
#              optimized decoders.
for i in `$BINFILE --list-cpu | cut -d ":" -f 2`; do
    checkoption --cpu $i "$TESTFILE.mp3"
done

#       --test-cpu
#              Tests your CPU and prints a list of possible choices for --cpu.
checkoption --test-cpu

#       --list-cpu
#              Lists all available decoder choices, regardless of support by
#              your CPU.
checkoption --list-cpu

#       -g gain, --gain gain
#              Set audio hardware output gain (default: don’t change).
# is not tested because it seems no longer be built in
HALTONERROR=false
if [[ $INTERACTIVE == true ]]; then
    OUTPUT=custom
else
    echo "---- running interactively, actual output on audio modules is disabled"
    echo
fi
NOTE="testing output gain, this is done to audio device, skip with 'q'"
checkoption -C -g 30 "$TESTFILE.mp3"
OUTPUT=wav
HALTONERROR=true

#       -f factor, --scale factor
#              Change scale factor (default: 32768).
checkoption -f 12355 "$TESTFILE.mp3"

#       --rva-mix, --rva-radio
#              Enable  RVA  (relative  volume adjustment) using the values
#              stored for ReplayGain radio mode / mix mode with all tracks
#              roughly equal loudness.  The first valid information found in
#              ID3V2 Tags (Comment named RVA or the RVA2 frame) or ReplayGain
#              header in Lame/Info Tag is used.
#open

#       --rva-album, --rva-audiophile
#              Enable RVA (relative volume adjustment) using the values stored
#              for ReplayGain audiophile mode / album mode with usually the
#              effect of adjusting album loudness but keeping relative  loud‐
#              ness inside album.  The first valid information found in ID3V2
#              Tags (Comment named RVA_ALBUM or the RVA2 frame) or ReplayGain
#              header in Lame/Info Tag is used.
#open

#       -0, --single0; -1, --single1
#              Decode only channel 0 (left) or channel 1 (right), respectively.
#              These options are available for stereo MPEG streams only.
checkoption -0 "$TESTFILE.mp3"
checkoption -1 "$TESTFILE.mp3"

#       -m, --mono, --mix, --singlemix
#              Mix both channels / decode mono. It takes less CPU time than
#              full stereo decoding.
checkoption -m "$TESTFILE.mp3"

#       --stereo
#              Force stereo output
checkoption --stereo "$TESTFILE.mp3"

#       -r rate, --rate rate
#              Set  sample  rate  (default: automatic).  You may want to change
#              this if you need a constant bitrate independed of the mpeg
#              stream rate. mpg123 automagically converts the rate. You should
#              then combine this with --stereo or --mono.
checkoption -r 14000 "$TESTFILE.mp3"

#       -2, --2to1; -4, --4to1
#              Performs a downsampling of ratio 2:1 (22 kHz) or 4:1 (11 kHz) on
#              the output stream, respectively. Saves some CPU cycles, but at
#              least the 4:1 ratio sounds ugly.
checkoption -2 "$TESTFILE.mp3"
checkoption -4 "$TESTFILE.mp3"

#       --pitch value
#              Set hardware pitch (speedup/down, 0 is neutral; 0.05 is 5%).
#              This changes the output sampling rate, so it only works in the
#              range your audio system/hardware supports.
checkoption --pitch 0.48 "$TESTFILE.mp3"

#       --8bit Forces 8bit output
checkoption --8bit "$TESTFILE.mp3"

#       -d n, --doublespeed n
#              Only play every n’th frame.  This will cause the MPEG stream to
#              be played n times faster, which can be used for special effects.
#              Can also be combined with the --halfspeed option to  play 3 out
#              of 4 frames etc.  Don’t expect great sound quality when using
#              this option.
checkoption -d 2 "$TESTFILE.mp3"

#       -h n, --halfspeed n
#              Play each frame n times.  This will cause the MPEG stream to be
#              played at 1/n’th speed (n times slower), which can be used for
#              special effects. Can also be combined with the --doublespeed
#              option to double every third frame or things like that.  Don’t
#              expect great sound quality when using this option.
checkoption -h 3 "$TESTFILE.mp3"

#       -E file, --equalizer
#              Enables equalization, taken from file.  The file needs to
#              contain 32 lines of data, additional comment lines may be
#              prefixed with  #.   Each  data  line  consists  of  two
#              floating-point entries,  separated  by whitespace.  They specify
#              the multipliers for left and right channel of a certain
#              frequency band, respectively.  The first line corresponds to the
#              lowest, the 32nd to the highest frequency band.  Note that you
#              can control the equalizer interactively with the generic control
#              interface.
checkoption -E equalizer-rechts.txt "$TESTFILE.mp3"
checkoption -E equalizer-links.txt "$TESTFILE.mp3"

#       --gapless
#              Enable code that cuts (junk) samples at beginning and end of
#              tracks, enabling gapless transitions between MPEG files when
#              encoder padding and codec  delays  would  prevent  it.   This
#              is enabled per default beginning with mpg123 version 1.0.0 .
#       --no-gapless
#              Disable the gapless code. That gives you MP3 decodings that
#              include encoder delay and padding plus mpg123’s decoder delay.
checkoption --gapless "$TESTFILE.mp3" "$TESTFILE.mp3"
checkoption --no-gapless "$TESTFILE.mp3" "$TESTFILE.mp3"

#       -D n, --delay n
#              Insert a delay of n seconds before each track.
NOTE="testing delay, this does not introduce actual delay in wav file but rather pauses the script for 2s"
checkoption -D 2 "$TESTFILE.mp3" "$TESTFILE.mp3"

#       -o h, --headphones
#              Direct audio output to the headphone connector (some hardware
#              only; AIX, HP, SUN).
#open

#       -o s, --speaker
#              Direct audio output to the speaker  (some hardware only; AIX,
#              HP, SUN).
#open

#       -o l, --lineout
#              Direct audio output to the line-out connector (some hardware
#              only; AIX, HP, SUN).
#open

#       -b size, --buffer size
#              Use  an  audio  output  buffer of size Kbytes.  This is useful
#              to bypass short periods of heavy system activity, which would
#              normally cause the audio output to be interrupted.  You should
#              specify a buffer size of at least 1024 (i.e. 1 Mb, which equals
#              about 6 seconds of audio data) or more; less than about 300 does
#              not make much  sense.   The  default  is  0,  which  turns
#              buffering off.
HALTONERROR=false
if [[ $INTERACTIVE == true ]]; then
    OUTPUT=custom
else
    echo "---- running interactively, actual output on audio modules is disabled"
    echo
fi
NOTE="testing buffer, this is done to audio device, skip with 'q'"
checkoption -C -b 100 "$TESTFILE.mp3"

#       --preload fraction
#              Wait  for the buffer to be filled to fraction before starting
#              playback (fraction between 0 and 1). You can tune this
#              prebuffering to either get faster sound to your ears or safer
#              uninter‐ rupted web radio.  Default is 1 (wait for full buffer
#              before playback).
NOTE="testing buffer, this is done to audio device, skip with 'q'"
checkoption -C -b 100 --preload 0.7 "$TESTFILE.mp3"

#       --smooth
#              Keep buffer over track boundaries -- meaning, do not empty the
#              buffer between tracks for possibly some added smoothness.
NOTE="testing buffer, this is done to audio device, skip with 'q'"
checkoption -C -b 100 --smooth "$TESTFILE.mp3" "$TESTFILE.mp3"
OUTPUT=wav
HALTONERROR=true

#MISC OPTIONS
#       -t, --test
#              Test mode.  The audio stream is decoded, but no output occurs.
checkoption -t "$TESTFILE.mp3"

#       -c, --check
#              Check for filter range violations (clipping), and report them
#              for each frame if any occur.
NOTE="here the ARTIST field should contain *'s instead of the non-ASCII characters"
LC_ALL=C checkoption -c "$TESTFILE.mp3"

#       -v, --verbose
#              Increase the verbosity level.  For example, displays the frame
#              numbers during decoding.
checkoption -v "$TESTFILE.mp3"

#       -q, --quiet
#              Quiet.  Suppress diagnostic messages.
checkoption -q "$TESTFILE.mp3"

#       -C, --control
#              Enable terminal control keys. By default use ’s’ or the space
#              bar to stop/restart (pause, unpause) playback, ’f’ to jump
#              forward to the next song, ’b’ to jump back to the beginning of
#              the song, ’,’ to rewind, ’.’ to fast forward, and ’q’ to quit.
#              Type ’h’ for a full list of available controls.
# tested in combination with -Z at end of script

#       --title
#              In an xterm, or rxvt (compatible, TERM environment variable is
#              examined), change the window’s title to the name of song
#              currently playing.
# tested in combination with -Z and -C at end of script

#       --long-tag
#              Display ID3 tag info always in long format with one line per
#              item (artist, title, ...)
checkoption -t --long-tag "$TESTFILE.mp3"

#       --utf8 Regardless of environment, print metadata in UTF-8 (otherwise,
#              when not using UTF-8 locale, you’ll get ASCII stripdown).
LC_ALL="C" checkoption -t --utf8 "$TESTFILE.mp3"

#       -R, --remote
#              Activate  generic  control  interface.   mpg123 will then read
#              <filename> ’’ to play some file and the obvious ‘‘pause’’,
#              ‘‘command.  ‘‘jump <frame>’’ will jump/seek to a given point
#              (MPEG frame number).  Issue ‘‘help’’ to get a full list of
#              commands and syntax.
#
#       --remote-err
#              Print responses for generic control mode to standard error, not
#              standard out.  This is automatically triggered when using -s .
#
#       --fifo path
#              Create a fifo / named pipe on the given path and use that for
#              reading commands instead of standard input.
echo "---- checking remote"
mpg123 -w test.--remote.wav -R --keep-open --remote-err --fifo remote.fifo 2>test.--remote.out 1>test.--remote.txt &

sleep 1
echo -------- sending load
echo load "$TESTFILE".mp3 > remote.fifo
sleep 1
echo -------- sending quit
echo quit > remote.fifo
sleep 1

for EXT in ".wav" ".txt" ".out"; do
    checkorcreatereffile "test.--remote$EXT" "test.--remote.ref$EXT"
done
echo

#       --aggressive
#              Tries to get higher priority
checkoption --aggressive "$TESTFILE".mp2

#       -T, --realtime
#              Tries to gain realtime priority.  This option usually requires
#              root privileges to have any effect.
checkoption -T "$TESTFILE".mp2

#       -?, --help
#              Shows short usage instructions.
checkoption -?

#       --longhelp
#              Shows long usage instructions.
checkoption --longhelp

#       --version
#              Print the version string.
HALTONERROR=false
checkoption --version
HALTONERROR=true

#### COMPARE MAN OUTPUT ########################################################
# TODO

#### START OF INTERACTIVE TESTS ################################################
if [[ $INTERACTIVE == true ]]; then
    echo "---- testing random playback of playlist with keyboard control"
    echo "-------- please test keys, type 'h' for available keys"
    echo "-------- also check the title of your terminal changing"
    echo "-------- press key to start"
    read -n 1 -s
    mpg123 --title --list playlist.txt -Z -C
else
    echo "---- running interactively, actual output on audio modules is disabled"
    echo
fi

#### CLEAN UP ##################################################################
removejunk
