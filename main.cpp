#include <iostream>

extern "C"
{
#include <libavformat/avformat.h>
//#include <libavcodec/avcodec.h>
}

#pragma comment(lib, "avformat.lib")
#pragma comment(lib, "avutil.lib")
#pragma comment(lib, "avcodec.lib")

int XError(int errNum)
{
	char buf[1024] = { 0 };
	av_strerror(errNum, buf, sizeof(buf));
	std::cout << buf << std::endl;
	getchar();
	return -1;
}

int main(int argc, char* argv[])
{
	const char* inUrl = "video.mp4";
	const char* outUrl = "rtmp://192.168.1.44/live";

	// 初始化所有封装和解封装
	av_register_all();

	// 初始化网络库
	avformat_network_init();

	//////////////////////////////////////////////////////////////////
	// 1 打开文件，解封文件头
	AVFormatContext *ictx = nullptr;
	AVInputFormat *fmt = nullptr;
	AVDictionary *options = nullptr;
	int re = avformat_open_input(&ictx, inUrl, 0, 0);
	if (re != 0)
	{
		return XError(re);
	}
	std::cout << "Open File success..." << std::endl;

	// 获取音视频流信息,h263,flv
	re = avformat_find_stream_info(ictx, 0);
	if (re != 0)
	{
		return XError(re);
	}

	av_dump_format(ictx, 0, inUrl, 0);
	//////////////////////////////////////////////////////////////////
	
	// 输出流
	// 创建输出流上下文
	AVFormatContext *octx = nullptr;
	re = avformat_alloc_output_context2(&octx, nullptr, "flv", outUrl);
	if (octx == nullptr)
	{
		return XError(re);
	}
	std::cout << "octx creat success" << std::endl;

	// 配置输出流
	// 遍历输入的avStream
	for (size_t i = 0; i < ictx->nb_streams; i++)
	{
		AVStream *out = avformat_new_stream(octx, ictx->streams[i]->codec->codec);
		if (!out)
		{
			return XError(0);
		}

		// 复制配置信息,同mp4
		//re = avcodec_copy_context(out->codec, ictx->streams[i]->codec);
		//if (re != 0)
		//{
		//	return XError(re);
		//}
		re = avcodec_parameters_copy(out->codecpar, ictx->streams[i]->codecpar);
		out->codec->codec_tag = 0;
	}
	av_dump_format(octx, 0, outUrl, 1);

	////////////////////////////////////////////////////////////////////
	//rtmp推流

	// 打开io
	re = avio_open(&octx->pb, outUrl, AVIO_FLAG_WRITE);
	if (octx->pb == nullptr)
	{
		return XError(re);
	}

	// 写入头信息
	re = avformat_write_header(octx, 0);
	if (re < 0)
	{
		return XError(re);
	}
	std::cout << "avform_write_header " << re << std::endl;

	// 推流每一帧数据
	AVPacket pkt;
	for (;;)
	{
		re = av_read_frame(ictx, &pkt);
		if (re != 0)
		{
			break;
		}
		std::cout << pkt.pts << " " << std::flush;

		// 

		av_packet_unref(&pkt);

	}

	getchar();
	return 0;
}