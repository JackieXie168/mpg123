#!/bin/sh
# You check out MPlayer and mpg123 sources before running this script.
mplayersrc=$HOME/mplayer
mpg123src=$HOME/mpg123

# number of runs for each configuration,
# to be used for statistics in postprocessing
runs=20

# also place the old and new variant of ad_mpg123 in there
ad_mpg123=$mplayersrc/libmpcodecs/ad_mpg123.c
if test -f $ad_mpg123.old && test -f $ad_mpg123.new; then
  echo "good, ad_mpg123 sources exist"
else
  echo "missing ad_mpg123 variants"
  exit 1
fi

host=`hostname`
if test $host = duron; then
  arch=athlon
elif test $host = k6-3; then
  arch=k6-3
else
  arch=native
fi

export CFLAGS="-O4 -march=$arch -mtune=$arch -pipe -ffast-math -fomit-frame-pointer -fno-tree-vectorize"

mpg123prefix=$HOME/mpg123-$arch
# prepare builds
rm -rf $mpg123prefix &&
cd $mpg123src &&
make clean &&
./configure --prefix=$mpg123prefix --with-optimization=0 --enable-static &&
make &&
make install &&
cd $mplayersrc &&
make clean &&
# prepare a first build, then hack around the libmpg123 building/linking
./configure && make &&

# add possible CPPFLAGS, like -I$mpg123prefix/include
build_ad_mpg123() {
  cc -MD -MP -Wundef -Wall -Wno-switch -Wno-parentheses \
  -Wpointer-arith -Wredundant-decls -Wstrict-prototypes -Wmissing-prototypes \
  -Wdisabled-optimization -Wno-pointer-sign -Wdeclaration-after-statement -std=gnu99 \
  -Werror-implicit-function-declaration \
  $CFLAGS "$@" \
  -D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64 -D_LARGEFILE64_SOURCE \
  -Ilibdvdread4 -I. -Iffmpeg -D_REENTRANT \
  -c -o libmpcodecs/ad_mpg123.o libmpcodecs/ad_mpg123.c
}

# only argument: the part that links mpg123
# so, -lmpg123 in the simplest case
link_mplayer() {
  # Why is there -ffast-math in the linker command?
  cc -o mplayer command.o m_property.o mixer.o mp_fifo.o \
  mplayer.o parser-mpcmd.o pnm_loader.o input/input.o \
    libao2/ao_mpegpes.o libao2/ao_null.o libao2/ao_pcm.o \
    libao2/audio_out.o libvo/aspect.o libvo/geometry.o \
    libvo/video_out.o libvo/vo_mpegpes.o libvo/vo_null.o \
    sub/spuenc.o libao2/ao_alsa.o input/appleir.o libvo/vo_fbdev.o \
    libvo/vo_fbdev2.o libvo/vo_png.o libvo/vo_md5sum.o \
    udp_sync.o libao2/ao_oss.o libvo/vo_pnm.o libvo/vo_tga.o \
    libvo/vo_v4l2.o libao2/ao_v4l2.o libvo/vo_cvidix.o \
    libvo/vosub_vidix.o vidix/vidix.o vidix/drivers.o vidix/dha.o \
    vidix/mtrr.o vidix/pci.o vidix/pci_names.o vidix/pci_dev_ids.o \
    vidix/cyberblade_vid.o vidix/mach64_vid.o vidix/mga_vid.o \
    vidix/mga_crtc2_vid.o vidix/nvidia_vid.o vidix/pm2_vid.o \
    vidix/pm3_vid.o vidix/radeon_vid.o vidix/rage128_vid.o \
    vidix/s3_vid.o vidix/sis_vid.o vidix/sis_bridge.o \
    vidix/unichrome_vid.o libvo/vo_yuv4mpeg.o asxparser.o \
    bstr.o codec-cfg.o cpudetect.o edl.o fmt-conversion.o \
    m_config.o m_option.o m_struct.o mp_msg.o mp_strings.o \
    mpcommon.o parser-cfg.o path.o playtree.o playtreeparser.o \
    subopt-helper.o libaf/af.o libaf/af_center.o libaf/af_channels.o \
    libaf/af_comp.o libaf/af_delay.o libaf/af_dummy.o libaf/af_equalizer.o \
    libaf/af_extrastereo.o libaf/af_format.o libaf/af_gate.o libaf/af_hrtf.o \
    libaf/af_karaoke.o libaf/af_pan.o libaf/af_resample.o libaf/af_scaletempo.o \
    libaf/af_sinesuppress.o libaf/af_stats.o libaf/af_sub.o libaf/af_surround.o \
    libaf/af_sweep.o libaf/af_tools.o libaf/af_volnorm.o libaf/af_volume.o \
    libaf/filter.o libaf/format.o libaf/reorder_ch.o libaf/window.o \
    libmpcodecs/ad.o libmpcodecs/ad_alaw.o libmpcodecs/ad_dk3adpcm.o \
    libmpcodecs/ad_dvdpcm.o libmpcodecs/ad_hwac3.o libmpcodecs/ad_hwmpa.o \
    libmpcodecs/ad_imaadpcm.o libmpcodecs/ad_msadpcm.o libmpcodecs/ad_pcm.o \
    libmpcodecs/dec_audio.o libmpcodecs/dec_teletext.o libmpcodecs/dec_video.o \
    libmpcodecs/img_format.o libmpcodecs/mp_image.o \
    libmpcodecs/pullup.o libmpcodecs/vd.o libmpcodecs/vd_hmblck.o \
    libmpcodecs/vd_lzo.o libmpcodecs/vd_mpegpes.o libmpcodecs/vd_mtga.o \
    libmpcodecs/vd_null.o libmpcodecs/vd_raw.o libmpcodecs/vd_sgi.o \
    libmpcodecs/vf.o libmpcodecs/vf_1bpp.o libmpcodecs/vf_2xsai.o \
    libmpcodecs/vf_blackframe.o libmpcodecs/vf_boxblur.o libmpcodecs/vf_crop.o \
    libmpcodecs/vf_cropdetect.o libmpcodecs/vf_decimate.o \
    libmpcodecs/vf_delogo.o libmpcodecs/vf_denoise3d.o libmpcodecs/vf_detc.o \
    libmpcodecs/vf_dint.o libmpcodecs/vf_divtc.o libmpcodecs/vf_down3dright.o \
    libmpcodecs/vf_dsize.o libmpcodecs/vf_dvbscale.o \
    libmpcodecs/vf_eq.o libmpcodecs/vf_eq2.o libmpcodecs/vf_expand.o \
    libmpcodecs/vf_field.o libmpcodecs/vf_fil.o libmpcodecs/vf_filmdint.o \
    libmpcodecs/vf_fixpts.o libmpcodecs/vf_flip.o libmpcodecs/vf_format.o \
    libmpcodecs/vf_framestep.o libmpcodecs/vf_gradfun.o \
    libmpcodecs/vf_halfpack.o libmpcodecs/vf_harddup.o \
    libmpcodecs/vf_hqdn3d.o libmpcodecs/vf_hue.o libmpcodecs/vf_il.o \
    libmpcodecs/vf_ilpack.o libmpcodecs/vf_ivtc.o libmpcodecs/vf_kerndeint.o \
    libmpcodecs/vf_mirror.o libmpcodecs/vf_noformat.o libmpcodecs/vf_noise.o \
    libmpcodecs/vf_ow.o libmpcodecs/vf_palette.o libmpcodecs/vf_perspective.o \
    libmpcodecs/vf_phase.o libmpcodecs/vf_pp7.o libmpcodecs/vf_pullup.o \
    libmpcodecs/vf_rectangle.o libmpcodecs/vf_remove_logo.o \
    libmpcodecs/vf_rgbtest.o libmpcodecs/vf_rotate.o libmpcodecs/vf_sab.o \
    libmpcodecs/vf_scale.o libmpcodecs/vf_smartblur.o \
    libmpcodecs/vf_softpulldown.o libmpcodecs/vf_stereo3d.o \
    libmpcodecs/vf_softskip.o libmpcodecs/vf_swapuv.o \
    libmpcodecs/vf_telecine.o libmpcodecs/vf_test.o libmpcodecs/vf_tfields.o \
    libmpcodecs/vf_tile.o libmpcodecs/vf_tinterlace.o libmpcodecs/vf_unsharp.o \
    libmpcodecs/vf_vo.o libmpcodecs/vf_yadif.o libmpcodecs/vf_yuvcsp.o \
    libmpcodecs/vf_yvu9.o libmpdemux/aac_hdr.o libmpdemux/asfheader.o \
    libmpdemux/aviheader.o libmpdemux/aviprint.o libmpdemux/demuxer.o \
    libmpdemux/demux_aac.o libmpdemux/demux_asf.o libmpdemux/demux_audio.o \
    libmpdemux/demux_avi.o libmpdemux/demux_demuxers.o \
    libmpdemux/demux_film.o libmpdemux/demux_fli.o libmpdemux/demux_lmlm4.o \
    libmpdemux/demux_mf.o libmpdemux/demux_mkv.o libmpdemux/demux_mov.o \
    libmpdemux/demux_mpg.o libmpdemux/demux_nsv.o libmpdemux/demux_pva.o \
    libmpdemux/demux_rawaudio.o libmpdemux/demux_rawvideo.o \
    libmpdemux/demux_realaud.o libmpdemux/demux_real.o libmpdemux/demux_roq.o \
    libmpdemux/demux_smjpeg.o libmpdemux/demux_ts.o libmpdemux/demux_ty.o \
    libmpdemux/demux_ty_osd.o libmpdemux/demux_viv.o libmpdemux/demux_vqf.o \
    libmpdemux/demux_y4m.o libmpdemux/ebml.o libmpdemux/extension.o \
    libmpdemux/mf.o libmpdemux/mp3_hdr.o libmpdemux/mp_taglists.o \
    libmpdemux/mpeg_hdr.o libmpdemux/mpeg_packetizer.o libmpdemux/parse_es.o \
    libmpdemux/parse_mp4.o libmpdemux/video.o libmpdemux/yuv4mpeg.o \
    libmpdemux/yuv4mpeg_ratio.o osdep/getch2.o osdep/timer-linux.o \
    stream/open.o stream/stream.o stream/stream_bd.o stream/stream_cue.o \
    stream/stream_file.o stream/stream_mf.o stream/stream_null.o \
    stream/url.o sub/eosd.o sub/find_sub.o sub/osd.o sub/spudec.o \
    sub/sub.o sub/sub_cc.o sub/subreader.o sub/vobsub.o stream/ai_alsa.o \
    stream/ai_oss.o sub/font_load.o stream/dvb_tune.o stream/stream_dvb.o \
    stream/stream_dvdnav.o libdvdnav/dvdnav.o libdvdnav/highlight.o \
    libdvdnav/navigation.o libdvdnav/read_cache.o libdvdnav/remap.o \
    libdvdnav/searching.o libdvdnav/settings.o libdvdnav/vm/decoder.o \
    libdvdnav/vm/vm.o libdvdnav/vm/vmcmd.o stream/stream_dvd.o \
    stream/stream_dvd_common.o libdvdread4/bitreader.o libdvdread4/dvd_input.o \
    libdvdread4/dvd_reader.o libdvdread4/dvd_udf.o libdvdread4/ifo_print.o \
    libdvdread4/ifo_read.o libdvdread4/md5.o libdvdread4/nav_print.o \
    libdvdread4/nav_read.o libvo/aclib.o av_helpers.o av_opts.o \
    libaf/af_lavcresample.o libmpcodecs/ad_ffmpeg.o libmpcodecs/ad_spdif.o \
    libmpcodecs/vd_ffmpeg.o libmpcodecs/vf_geq.o libmpcodecs/vf_lavc.o \
    libmpcodecs/vf_lavcdeint.o libmpcodecs/vf_pp.o libmpcodecs/vf_screenshot.o \
    libmpdemux/demux_lavf.o stream/stream_ffmpeg.o sub/av_sub.o \
    libaf/af_lavcac3enc.o libmpcodecs/vf_fspp.o libmpcodecs/vf_mcdeint.o \
    libmpcodecs/vf_qp.o libmpcodecs/vf_spp.o libmpcodecs/vf_uspp.o \
    stream/stream_ftp.o libmpcodecs/vf_bmovl.o libaf/af_export.o \
    osdep/mmap_anon.o libdvdcss/css.o libdvdcss/device.o libdvdcss/error.o \
    libdvdcss/ioctl.o libdvdcss/libdvdcss.o libmpcodecs/vd_libmpeg2.o \
    libmpeg2/alloc.o libmpeg2/cpu_accel.o libmpeg2/cpu_state.o \
    libmpeg2/decode.o libmpeg2/header.o libmpeg2/idct.o libmpeg2/motion_comp.o \
    libmpeg2/slice.o libmpeg2/idct_mmx.o libmpeg2/motion_comp_mmx.o \
    libmpcodecs/ad_mpg123.o libmpcodecs/ad_mp3lib.o mp3lib/sr1.o \
    mp3lib/decode_i586.o mp3lib/dct36_3dnow.o mp3lib/dct64_3dnow.o \
    mp3lib/dct36_k7.o mp3lib/dct64_k7.o mp3lib/dct64_mmx.o mp3lib/decode_mmx.o \
    stream/stream_rtsp.o stream/freesdp/common.o stream/freesdp/errorlist.o \
    stream/freesdp/parser.o stream/librtsp/rtsp.o stream/librtsp/rtsp_rtp.o \
    stream/librtsp/rtsp_session.o stream/stream_netstream.o \
    stream/asf_mmst_streaming.o stream/asf_streaming.o stream/cookies.o \
    stream/http.o stream/network.o stream/pnm.o stream/rtp.o \
    stream/udp.o stream/tcp.o stream/stream_rtp.o stream/stream_udp.o \
    stream/realrtsp/asmrp.o stream/realrtsp/real.o stream/realrtsp/rmff.o \
    stream/realrtsp/sdpplin.o stream/realrtsp/xbuffer.o stream/stream_pvr.o \
    libmpcodecs/ad_qtaudio.o libmpcodecs/vd_qtvideo.o libmpcodecs/ad_realaud.o \
    libmpcodecs/vd_realvid.o stream/cache2.o tremor/bitwise.o tremor/block.o \
    tremor/codebook.o tremor/floor0.o tremor/floor1.o tremor/framing.o \
    tremor/info.o tremor/mapping0.o tremor/mdct.o tremor/registry.o \
    tremor/res012.o tremor/sharedbook.o tremor/synthesis.o tremor/window.o \
    stream/stream_tv.o stream/tv.o stream/frequencies.o stream/tvi_dummy.o \
    stream/tvi_v4l.o stream/audio_in.o stream/tvi_v4l2.o sub/unrar_exec.o \
    stream/stream_vcd.o libmpcodecs/ad_libvorbis.o libmpdemux/demux_ogg.o \
    loader/wrapper.o loader/elfdll.o loader/ext.o loader/ldt_keeper.o \
    loader/module.o loader/pe_image.o loader/pe_resource.o loader/registry.o \
    loader/resource.o loader/win32.o libmpcodecs/ad_acm.o libmpcodecs/ad_dmo.o \
    libmpcodecs/ad_dshow.o libmpcodecs/ad_twin.o libmpcodecs/vd_dmo.o \
    libmpcodecs/vd_dshow.o libmpcodecs/vd_vfw.o libmpcodecs/vd_vfwex.o \
    libmpdemux/demux_avs.o loader/afl.o loader/drv.o loader/vfl.o \
    loader/dshow/DS_AudioDecoder.o loader/dshow/DS_Filter.o \
    loader/dshow/DS_VideoDecoder.o loader/dshow/allocator.o \
    loader/dshow/cmediasample.o loader/dshow/graph.o loader/dshow/guids.o \
    loader/dshow/inputpin.o loader/dshow/mediatype.o loader/dshow/outputpin.o \
    loader/dmo/DMO_AudioDecoder.o loader/dmo/DMO_VideoDecoder.o \
    loader/dmo/buffer.o loader/dmo/dmo.o loader/dmo/dmo_guids.o \
    libmpcodecs/vd_xanim.o osdep/shmem.o \
    ffmpeg/libpostproc/libpostproc.a ffmpeg/libswscale/libswscale.a \
    ffmpeg/libavfilter/libavfilter.a ffmpeg/libavformat/libavformat.a \
    ffmpeg/libavcodec/libavcodec.a ffmpeg/libavutil/libavutil.a \
    "$@" -Wl,-z,noexecstack -lm  -ffast-math \
    -lncurses -lasound -ldl -lpthread -lpthread -ldl -rdynamic &&
}

cp -v $ad_mpg123.old $ad_mpg123 &&
build_ad_mpg123 &&
link_mplayer -lmpg123 &&
cp -v mplayer mplayer-variant0-dynamic-sys &&
link_mplayer -Wl,-rpath=$mpg123prefix/lib -lmpg123 &&
cp -v mplayer mplayer-variant0-dynamic-trunk &&
link_mplayer $mpg123prefix/lib/libmpg123.a &&
cp -v mplayer mplayer-variant0-static-trunk &&

cp -v $ad_mpg123.new $ad_mpg123 &&
build_ad_mpg123 &&
link_mplayer -lmpg123 &&
cp -v mplayer mplayer-variant1-dynamic-sys &&
link_mplayer -Wl,-rpath=$mpg123prefix/lib -lmpg123 &&
cp -v mplayer mplayer-variant1-dynamic-trunk &&
link_mplayer $mpg123prefix/lib/libmpg123.a &&
cp -v mplayer mplayer-variant1-static-trunk &&

cp -v $ad_mpg123.new $ad_mpg123 &&
build_ad_mpg123 -I$mpg123prefix/include &&
# also variant 2 with old lib, not recommened,
# but should still work (or at least fail gracefully)
link_mplayer -lmpg123 &&
cp -v mplayer mplayer-variant2-dynamic-sys &&
link_mplayer -Wl,-rpath=$mpg123prefix/lib -lmpg123 &&
cp -v mplayer mplayer-variant2-dynamic-trunk &&
link_mplayer $mpg123prefix/lib/libmpg123.a &&
cp -v mplayer mplayer-variant2-static-trunk

time_run() {
  time "$@"
}

mark_run() {
  echo
  echo "=== This was run $1 of $2 ==="
  echo
}

album=/dev/shm/convergence_-_points_of_view
# some runs for mpg123 to set the baseline
for d in mpg123 $mpg123prefix/bin/mpg123
do
  for n in `seq 1 20`
  do
    time_run $d -w /dev/null $album/*.mp3
    mark_run $n $d
  done
done

# now the mplayers
for d in $mplayersrc/mplayer-variant*
do
  for ac in mp3lib mpg123
  do
  for n in `seq 1 20`
  do
    time_run $d -ac $ac -quiet -ao pcm:fast:file=/dev/null $album/*.mp3
    mark_run $n $d
  done
  done
done
