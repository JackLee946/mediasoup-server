/************************************************************************
* @Copyright: 2023-2024
* @FileName:
* @Description: Open source mediasoup C++ controller library
* @Version: 1.0.0
* @Author: Jackie Ou
* @CreateTime: 2023-10-30
*************************************************************************/

#include "rtp_stream.h"
#include "srv_logger.h"

namespace srv {

    std::shared_ptr<RtpStreamParameters> parseRtpStreamParameters(const FBS::RtpStream::Params* data)
    {
        auto parameters = std::make_shared<RtpStreamParameters>();

        if (!data) {
            SRV_LOGE("parseRtpStreamParameters() | data is null");
            return parameters;
        }

        parameters->encodingIdx = data->encodingIdx();
        parameters->ssrc = data->ssrc();
        parameters->payloadType = data->payloadType();
        if (data->mimeType()) parameters->mimeType = data->mimeType()->str();
        parameters->clockRate = data->clockRate();
        if (data->rid()) parameters->rid = data->rid()->str();
        if (data->cname()) parameters->cname = data->cname()->str();
        parameters->rtxSsrc = data->rtxSsrc().value_or(0);
        parameters->rtxPayloadType = data->rtxPayloadType().value_or(0);
        parameters->useNack = data->useNack();
        parameters->usePli = data->usePli();
        parameters->useFir = data->useFir();
        parameters->useInBandFec = data->useInBandFec();
        parameters->useDtx = data->useDtx();
        parameters->spatialLayers = data->spatialLayers();
        parameters->temporalLayers = data->temporalLayers();

        return parameters;
    }

    std::shared_ptr<RtxStreamParameters> parseRtxStreamParameters(const FBS::RtxStream::Params* data)
    {
        auto parameters = std::make_shared<RtxStreamParameters>();

        if (!data) {
            SRV_LOGE("parseRtxStreamParameters() | data is null");
            return parameters;
        }

        parameters->ssrc = data->ssrc();
        parameters->payloadType = data->payloadType();
        if (data->mimeType()) parameters->mimeType = data->mimeType()->str();
        parameters->clockRate = data->clockRate();
        if (data->rrid()) parameters->rrid = data->rrid()->str();
        if (data->cname()) parameters->cname = data->cname()->str();

        return parameters;
    }

    std::shared_ptr<RtxStreamDump> parseRtxStream(const FBS::RtxStream::RtxDump* data)
    {
        auto dump = std::make_shared<RtxStreamDump>();

        if (!data) {
            SRV_LOGE("parseRtxStream() | data is null");
            return dump;
        }

        if (data->params()) {
            dump->params = *parseRtxStreamParameters(data->params());
        }

        return dump;
    }

    std::shared_ptr<RtpStreamDump> parseRtpStream(const FBS::RtpStream::Dump* data)
    {
        auto dump = std::make_shared<RtpStreamDump>();

        if (!data) {
            SRV_LOGE("parseRtpStream() | data is null");
            return dump;
        }

        if (data->params()) {
            dump->params = *parseRtpStreamParameters(data->params());
        }
        std::shared_ptr<RtxStreamDump> rtxStream;
        if (data->rtxStream()) {
            rtxStream = parseRtxStream(data->rtxStream());
        }
        if (rtxStream) {
            dump->rtxStream = *rtxStream;
        }
        dump->score = data->score();

        return dump;
    }

    std::vector<std::shared_ptr<BitrateByLayer>> parseBitrateByLayer(const FBS::RtpStream::RecvStats* binary)
    {
        std::vector<std::shared_ptr<BitrateByLayer>> result;
        
        for (const auto& layer : *binary->bitrateByLayer()) {
            auto l = std::make_shared<BitrateByLayer>();
            l->layer = layer->layer()->str();
            l->bitrate = layer->bitrate();
            result.emplace_back(l);
        }
        
        return result;
    }
}
