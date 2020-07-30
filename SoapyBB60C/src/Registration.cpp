#include "SoapyBB60.hpp"

#include <SoapySDR/Registry.hpp>

static SoapySDR::KwargsList findBB60(const SoapySDR::Kwargs &args)
{
    int serials[BB_MAX_DEVICES];
    int count = -1;
    bbStatus status = bbGetSerialNumberList(serials, &count);
    if(status != bbNoError) {
      SoapySDR_logf(SOAPY_SDR_ERROR, "Error: %s\n", bbGetErrorString(status));
    }

    SoapySDR::KwargsList devices;

    for(int i = 0; i < count; i++) {
        SoapySDR::Kwargs deviceInfo;

        deviceInfo["device_id"] = std::to_string(i);
        deviceInfo["label"] = "BB60C [" + std::to_string(serials[i]) + "]";
        deviceInfo["serial"] = std::to_string(serials[i]);

        devices.push_back(deviceInfo);
    }

    return devices;
}

static SoapySDR::Device *makeBB60(const SoapySDR::Kwargs &args)
{
    return new SoapyBB60(args);
}

static SoapySDR::Registry registerBB60("bb60c", &findBB60, &makeBB60, SOAPY_SDR_ABI_VERSION);
