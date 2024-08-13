cmake_minimum_required(VERSION 3.5)

project(ffmpeg-fang)

include_directories(/usr/local/include)

link_directories(/usr/local/lib)

set(FFMPEG_LIBS
    avcodec
    avdevice
    avfilter
    avformat
    avutil
    swresample
    swscale
    )

# add_executable(spmp4 split_mp4_to_av.cc)
# add_executable(dfv decode_fillter_video.cc)

# add_executable(oper_yuv operate_yuv.cc)
# add_executable(rgbToBmp rgb24_to_bmp.cc)
# add_executable(rgbToYuv420p rgb24_to_yuv420p.cc)
add_executable(operPcm operate_pcm.cc)
add_executable(pcmToWav pcm_to_wav.cc)

# target_link_libraries(dfv ${FFMPEG_LIBS})