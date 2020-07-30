#include "SoapyBB60.hpp"

#include <SoapySDR/Formats.hpp>

std::vector<std::string> SoapyBB60::getStreamFormats(const int direction, const size_t channel) const {
    std::vector<std::string> formats;

    formats.push_back(SOAPY_SDR_CF32);
    formats.push_back(SOAPY_SDR_CS16);

    return formats;
}

std::string SoapyBB60::getNativeStreamFormat(const int direction, const size_t channel, double &fullScale) const {
     fullScale = 1.0;

     return SOAPY_SDR_CF32;
}

SoapySDR::ArgInfoList SoapyBB60::getStreamArgsInfo(const int direction, const size_t channel) const {
    SoapySDR::ArgInfoList streamArgs;

    return streamArgs;
}

SoapySDR::Stream *SoapyBB60::setupStream(
        const int direction,
        const std::string &format,
        const std::vector<size_t> &channels,
        const SoapySDR::Kwargs &args)
{
    // Check channel config
    if(channels.size() > 1 or (channels.size() > 0 and channels.at(0) != 0)) {
        throw std::runtime_error("setupStream invalid channel selection");
    }

    // Check format
    if(format == SOAPY_SDR_CF32) {
        SoapySDR_log(SOAPY_SDR_INFO, "Using format CF32");
        bbConfigureIQDataType(deviceId, bbDataType32fc);
    } else if(format == SOAPY_SDR_CS16) {
        SoapySDR_log(SOAPY_SDR_INFO, "Using format CS16");
        bbConfigureIQDataType(deviceId, bbDataType16sc);
    } else {
        throw std::runtime_error("setupStream: Invalid format '" + format
            + "' -- Only CF32 and CS16 are supported by SoapyBB60C module.");
    }

    return (SoapySDR::Stream *)this;
}

void SoapyBB60::closeStream(SoapySDR::Stream *stream)
{
    bbAbort(deviceId);
    bbCloseDevice(deviceId);
}

bool SoapyBB60::updateStream()
{
    if(streamActive) {
        bbStatus status = bbInitiate(deviceId, BB_STREAMING, BB_STREAM_IQ);
        if(status != bbNoError) {
            SoapySDR_logf(SOAPY_SDR_ERROR, "Initiate: %s", bbGetErrorString(status));
            return false;
        }
    }

    return true;
}

int SoapyBB60::activateStream(SoapySDR::Stream *stream, const int flags, const long long timeNs, const size_t numElems)
{
    if(flags != 0) {
        return SOAPY_SDR_NOT_SUPPORTED;
    }

    // Choose the smaller - bandwidth or sample rate
    double actual_bw = std::min(bb60Decimation.at(decimation), bandwidth);
    bbStatus status = bbConfigureIQ(deviceId, decimation, actual_bw);
    if(status != bbNoError) {
        SoapySDR_logf(SOAPY_SDR_ERROR, "ConfigureIQ: %s", bbGetErrorString(status));
    }

    streamActive = true;
    if(!updateStream()) {
        streamActive = false;
        return SOAPY_SDR_NOT_SUPPORTED;
    }

    return 0;
}

int SoapyBB60::deactivateStream(SoapySDR::Stream *stream, const int flags, const long long timeNs)
{
    if(flags != 0) {
        return SOAPY_SDR_NOT_SUPPORTED;
    }

    bbAbort(deviceId);

    streamActive = false;

    return 0;
}

int SoapyBB60::readStream(
        SoapySDR::Stream *stream,
        void * const *buffs,
        const size_t numElems,
        int &flags,
        long long &timeNs,
        const long timeoutUs)
{
    bbIQPacket pkt;
    memset(&pkt, 0, sizeof(pkt));
    pkt.iqData = buffs[0];
    pkt.iqCount = numElems;

    bbStatus status = bbGetIQ(deviceId, &pkt);
    if(status != bbNoError) {
        SoapySDR_logf(SOAPY_SDR_ERROR, "GetIQ: %s", bbGetErrorString(status));
        return 0;
    }

    if(pkt.sampleLoss == BB_TRUE) {
        SoapySDR_logf(SOAPY_SDR_WARNING, "Sample Overrun");
    }

    return numElems;
}
