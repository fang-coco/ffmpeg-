extern "C" {
    #include <libavutil/log.h>
    #include <libavcodec/avcodec.h>
    #include <libavformat/avformat.h>
    #include <libavfilter/avfilter.h>
    #include <libavfilter/buffersink.h>
    #include <libavfilter/buffersrc.h>
    #include <libavutil/avutil.h>
    #include <libavutil/opt.h>
}

int main() {

    av_log_set_level(AV_LOG_DEBUG);

    const char *filename = "/home/fang/ffmpeg_learn/monkey.mp4";
    AVFormatContext *pFormatCtx = NULL;

    av_log(NULL, AV_LOG_DEBUG, "Opening file ...\n");
    int ret = avformat_open_input(&pFormatCtx, filename, NULL, NULL);
    if (ret < 0) {
        av_log(NULL, AV_LOG_ERROR, "Couldn't open input file.\n");
        return -1;
    }


    ret = avformat_find_stream_info(pFormatCtx, NULL);
    if (ret < 0) {
        av_log(NULL, AV_LOG_ERROR, "Failed to retrieve stream information\n");
        return -1;
    }

    av_dump_format(pFormatCtx, 0, filename, 0);

    int vedio_stream_index = -1;
    const AVCodec *pCodec = NULL;
    ret = av_find_best_stream(pFormatCtx, AVMEDIA_TYPE_VIDEO, -1, -1, &pCodec, 0);
    if (ret < 0) {
        av_log(NULL, AV_LOG_ERROR, "Failed to find a vedio stream information\n");
        return -1;
    }
    vedio_stream_index = ret;

    AVCodecContext *pCodecCtx = avcodec_alloc_context3(pCodec);
    if (!pCodecCtx) {
        av_log(NULL, AV_LOG_ERROR, "Failed to allocate memory for AVCodecContext\n");
        return AVERROR(ENOMEM);
    }

    ret = avcodec_parameters_to_context(pCodecCtx, pFormatCtx->streams[vedio_stream_index]->codecpar);
    if (ret < 0) {
        av_log(NULL, AV_LOG_ERROR, "Failed to copy codec parameters to context\n");
        return -1;
    }

    ret = avcodec_open2(pCodecCtx, pCodec, NULL);
    if (ret < 0) {
        av_log(NULL, AV_LOG_ERROR, "Failed to open codec\n");
        return -1;
    }

    AVFrame *pFrame = av_frame_alloc();
    AVFrame *pFramefileter = av_frame_alloc();
    AVPacket *packet = av_packet_alloc();
    if (!pFrame || !pFramefileter || !packet) {
        av_log(NULL, AV_LOG_ERROR, "Failed to allocate memory for frames\n");
        return AVERROR(ENOMEM);
    }

    AVFilterContext *buffersrc_ctx = NULL;
    AVFilterContext *buffersink_ctx = NULL;
    const AVFilter *buffersrc = avfilter_get_by_name("buffer");
    const AVFilter *buffersink = avfilter_get_by_name("buffersink");

    AVFilterGraph *filter_graph = avfilter_graph_alloc();
    AVFilterInOut *src = avfilter_inout_alloc();
    AVFilterInOut *sink = avfilter_inout_alloc();
    if (!filter_graph || !src || !sink) {
        av_log(NULL, AV_LOG_ERROR, "Failed to allocate memory for AVFilterGraph\n");
        return AVERROR(ENOMEM);
    }
    char args[512];
    pCodecCtx->time_base = pFormatCtx->streams[vedio_stream_index]->time_base;
    snprintf(args, sizeof(args), "video_size=%dx%d:pix_fmt=%d:time_base=%d/%d:pixel_aspect=%d/%d",
            pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt,
            pCodecCtx->time_base.num, pCodecCtx->time_base.den,
            pCodecCtx->sample_aspect_ratio.num, pCodecCtx->sample_aspect_ratio.den
    );

    ret = avfilter_graph_create_filter(&buffersrc_ctx, buffersrc, "in", args, NULL, filter_graph);
    if (ret < 0) {
        av_log(NULL, AV_LOG_ERROR, "Failed to create buffer source\n");
        return -1;
    }

    ret = avfilter_graph_create_filter(&buffersink_ctx, buffersink, "out", NULL, NULL, filter_graph);
    if (ret < 0) {
        av_log(NULL, AV_LOG_ERROR, "Failed to create buffer sink\n");
        return -1;
    }

    enum AVPixelFormat pix_fmts[] = {AV_PIX_FMT_YUV420P, AV_PIX_FMT_NONE};
    ret = av_opt_set_int_list(buffersink_ctx, "pix_fmts", pix_fmts, AV_PIX_FMT_NONE, AV_OPT_SEARCH_CHILDREN);
    if (ret < 0) {
        av_log(NULL, AV_LOG_ERROR, "Failed to set output pixel format\n");
        return -1;
    }

    // avfilter_link(buffersrc_ctx, 0, buffersink_ctx, 0);

    src->name = av_strdup("in");
    src->filter_ctx = buffersrc_ctx;
    src->pad_idx = 0;
    src->next = NULL;

    sink->name = av_strdup("out");
    sink->filter_ctx = buffersink_ctx;
    sink->pad_idx = 0;
    sink->next = NULL;
    const char* filter_descr = "scale=78:24,transpose=cclock";
    ret = avfilter_graph_parse_ptr(filter_graph, filter_descr, &sink, &src, NULL);
    if (ret < 0) {
        av_log(NULL, AV_LOG_ERROR, "Failed to parse filter graph\n");
        return -1;
    }

    ret = avfilter_graph_config(filter_graph, NULL);
    if (ret < 0) {
        av_log(NULL, AV_LOG_ERROR, "Failed to configure filter graph\n");
        return -1;
    }
    
    avfilter_inout_free(&src);
    avfilter_inout_free(&sink);

    while (av_read_frame(pFormatCtx, packet) >= 0) {
        if (packet->stream_index == vedio_stream_index) {
            ret = avcodec_send_packet(pCodecCtx, packet);
            if (ret < 0) {
                av_log(NULL, AV_LOG_ERROR, "Failed to send packet to decoder\n");
                return -1;
            }
            
            while (ret >= 0) {
                ret = avcodec_receive_frame(pCodecCtx, pFrame);
                if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
                    break;
                } else if (ret < 0) {
                    av_log(NULL, AV_LOG_ERROR, "Failed to receive frame from decoder\n");
                }
                pFrame->pts = pFrame->best_effort_timestamp;

                ret = av_buffersrc_add_frame_flags(buffersrc_ctx, pFrame, AV_BUFFERSRC_FLAG_KEEP_REF);
                if (ret < 0) {

                    
                    av_log(NULL, AV_LOG_ERROR, "Failed to add frame to buffer source\n");
                    return -1;
                }

                while (1) {
                    ret = av_buffersink_get_frame(buffersink_ctx, pFramefileter); 
                    if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
                        break;
                    } else if (ret < 0) {
                        av_log(NULL, AV_LOG_ERROR, "Failed to get frame from buffer sink\n");
                        return -1;
                    }
                    av_frame_unref(pFramefileter);
                }
                av_frame_unref(pFrame);
            }
        }
        av_packet_unref(packet);
    }
    return 0;
}