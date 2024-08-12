extern "C" {
    #include <libavcodec/avcodec.h>
    #include <libavutil/log.h>
    #include <libavformat/avformat.h>
}

int avformat_open_codec(int *avStreamIndex, AVCodecContext **dec_ctx ,AVFormatContext *pFormatCtx, AVMediaType type) {


    
    int ret = av_find_best_stream(pFormatCtx, type, -1, -1, NULL, 0);
    if (ret < 0) {
        av_log(NULL, AV_LOG_ERROR, "Could not find video stream in the file\n");
        return ret;
    }

    *avStreamIndex = ret;
    AVStream *st = pFormatCtx->streams[ret];


    const AVCodec *decode = avcodec_find_decoder(st->codecpar->codec_id);
    if (!decode) {
        av_log(NULL, AV_LOG_ERROR, "Codec not found\n");
        return AVERROR(ENOMEM);
    }

    *dec_ctx = avcodec_alloc_context3(decode);  
    if(!*dec_ctx) {
        av_log(NULL, AV_LOG_ERROR, "Could not allocate audio codec context\n");
        return AVERROR(ENOMEM);
    }

    ret = avcodec_parameters_to_context(*dec_ctx, st->codecpar);
    if (ret < 0) {
        av_log(NULL, AV_LOG_ERROR, "Could not copy audio codec parameters to audio codec context\n");
        return ret;
    }

    ret = avcodec_open2(*dec_ctx, decode, NULL);
    if (ret < 0) {
        av_log(NULL, AV_LOG_ERROR, "Could not open audio codec\n");
        return ret;
    }

}

int decode_packet(AVCodecContext *dec_ctx, AVPacket *packet, FILE *outfile, AVFrame *frame) {
    int ret = avcodec_send_packet(dec_ctx, packet);
    if (ret < 0) {
        av_log(NULL, AV_LOG_ERROR, "Error submitting a packet for decoding\n");
        return ret;
    }
    while (ret >= 0) {
        ret = avcodec_receive_frame(dec_ctx, frame);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
            return 0;
        else if (ret < 0) {
            av_log(NULL, AV_LOG_ERROR, "Error during decoding\n");
            return ret;
        }
        if (dec_ctx->codec->type == AVMEDIA_TYPE_AUDIO) {

        } else if (dec_ctx->codec->type == AVMEDIA_TYPE_VIDEO) {

        }
    }
}

int output_audio_frame(FILE *outfile, AVFrame *frame) {
    
}

int output_video_frame(FILE *outfile, AVFrame *frame) {

}
int main() {
    const char *filename = "../monkey.mp4";
    const char *audio_out_filename = "../monkey.wav";
    const char *video_out_filename = "../monkey.mp4";

    av_log_set_level(AV_LOG_DEBUG);
    av_log_set_callback(av_log_default_callback);


    // not nullptr segment error
    AVFormatContext *pFormatCtx = nullptr;    
    if (avformat_open_input(&pFormatCtx, filename, NULL, NULL) != 0) {
        av_log(NULL, AV_LOG_ERROR, "Could not open input file\n");
        return -1;
    }

    if (avformat_find_stream_info(pFormatCtx, NULL) < 0) {
        av_log(NULL, AV_LOG_ERROR, "Could not find stream information\n");
        return -1;
    }

    AVCodecContext *audio_dec_ctx = nullptr;
    AVStream *audio_st = nullptr;
    FILE *audio_outfile = nullptr; 
    int audio_stream_idx = -1;
    if (avformat_open_codec(&audio_stream_idx, &audio_dec_ctx, pFormatCtx, AVMEDIA_TYPE_AUDIO) >= 0) {
        av_log(NULL, AV_LOG_INFO, "Audio codec name: %s\n", audio_dec_ctx->codec->name);
        audio_st = pFormatCtx->streams[audio_stream_idx];
        audio_outfile = fopen(audio_out_filename, "wb");
        if (!audio_outfile) {
            av_log(NULL, AV_LOG_ERROR, "Could not open audio output file\n");
            return -1;
        }
    } 
    AVCodecContext *video_dec_ctx = nullptr;
    AVStream *video_st = nullptr; 
    FILE *video_outfile = nullptr;
    int video_stream_idx = -1;
    if (avformat_open_codec(&video_stream_idx, &video_dec_ctx, pFormatCtx, AVMEDIA_TYPE_VIDEO) >= 0) {
        av_log(NULL, AV_LOG_INFO, "Video codec name: %s\n", video_dec_ctx->codec->name);
        video_st = pFormatCtx->streams[video_stream_idx];
        video_outfile = fopen(video_out_filename, "wb");
        if (!video_outfile) {
            av_log(NULL, AV_LOG_ERROR, "Could not open video output file\n");
            return -1;
        }
    }

    if (!audio_st && !video_st) {
        av_log(NULL, AV_LOG_ERROR, "Could not find audio or video stream\n");
        return -1;
    }

    AVFrame *frame = nullptr;
    AVPacket *packet = nullptr;
    frame = av_frame_alloc();
    packet = av_packet_alloc();
    if (!frame || !packet) {
        av_log(NULL, AV_LOG_ERROR, "Could not allocate frame or packet\n");
        return -1;
    }
    while (av_read_frame(pFormatCtx, packet) >= 0) {
        if (packet->stream_index == audio_stream_idx) {

        } else if (packet->stream_index == video_stream_idx) {

        }
        av_packet_unref(packet);
    }

    av_dump_format(pFormatCtx, 0, filename, 0);
}