#include "SoapyBB60.hpp"

#define BB60_CLOCK 40e6

std::map<std::string, unsigned int> port1_config = {
    {"DEFAULT", 0},
    {"INT_REF_OUT_AC", BB_PORT1_INT_REF_OUT|BB_PORT1_AC_COUPLED},
    {"INT_REF_OUT_DC", BB_PORT1_INT_REF_OUT|BB_PORT1_DC_COUPLED},
    {"EXT_REF_IN_AC", BB_PORT1_EXT_REF_IN|BB_PORT1_AC_COUPLED},
    {"EXT_REF_IN_DC", BB_PORT1_EXT_REF_IN|BB_PORT1_DC_COUPLED},
    {"EXT_REF_IN_DC", BB_PORT1_EXT_REF_IN|BB_PORT1_DC_COUPLED},
    {"OUT_LOGIC_LOW_AC", BB_PORT1_OUT_LOGIC_LOW|BB_PORT1_AC_COUPLED},
    {"OUT_LOGIC_LOW_DC", BB_PORT1_OUT_LOGIC_LOW|BB_PORT1_DC_COUPLED},
    {"OUT_LOGIC_HIGH_AC", BB_PORT1_OUT_LOGIC_HIGH|BB_PORT1_AC_COUPLED},
    {"OUT_LOGIC_HIGH_DC", BB_PORT1_OUT_LOGIC_HIGH|BB_PORT1_DC_COUPLED},
};

std::map<std::string, unsigned int> port2_config = {
    {"DEFAULT", 0},
    {"OUT_LOGIC_LOW", BB_PORT2_OUT_LOGIC_LOW},
    {"OUT_LOGIC_HIGH", BB_PORT2_OUT_LOGIC_HIGH},
    {"IN_TRIGGER_RISING_EDGE", BB_PORT2_IN_TRIGGER_RISING_EDGE},
    {"IN_TRIGGER_FALLING_EDGE", BB_PORT2_IN_TRIGGER_FALLING_EDGE},
};

SoapyBB60::SoapyBB60(const SoapySDR::Kwargs &args)
{
    deviceId = -1;

    centerFrequency = 100e6;
    decimation = 1;
    bandwidth = BB60_CLOCK/decimation;
    sampleRate = 0;

    rfGain = 0;
    attenLevel = 0;
    refLevel = -30;

    streamActive = false;

    bool serial_specified = false;
    bbStatus status;

    if(args.count("serial") != 0) {
        try {
            serial = std::stoull(args.at("serial"), nullptr, 10);
        } catch (const std::invalid_argument &) {
            throw std::runtime_error("serial is not a number");
        } catch (const std::out_of_range &) {
            throw std::runtime_error("serial value of out range");
        }
        serial_specified = true;
    } else if(args.count("device_id") != 0) {
        try {
            deviceId = std::stoi(args.at("device_id"));
        } catch (const std::invalid_argument &) {
            throw std::runtime_error("device_id is not a number");
        } catch (const std::out_of_range &) {
            throw std::runtime_error("device_id of out range");
        }
    }

    int serials[BB_MAX_DEVICES];
    int numDevices = -1;
    status = bbGetSerialNumberList(serials, &numDevices);
    if(status != bbNoError) {
        throw std::runtime_error("Failed to retrieve list of BB60 devices");
    }

    if(numDevices < 1) {
        throw std::runtime_error("No BB60 devices found");
    }

    if(serial_specified) {
        // Find serial
        for(int i = 0; i < numDevices; i++) {
            if(serials[i] == serial) {
                deviceId = i;
                break;
            }
        }
        if(deviceId < 0) {
            throw std::runtime_error("BB60 device with S/N " + std::to_string(serial) + " not found");
        }
    } else { // Find device id
        if(deviceId < 0) {
            deviceId = 0; // Default
        } else if(deviceId >= numDevices) {
            throw std::runtime_error("BB60 device_id out of range [0 .. " + std::to_string(numDevices-1) + "].");
        }
        serial = serials[deviceId];
    }
    if(bbOpenDeviceBySerialNumber(&deviceId, serial) != bbNoError) {
        throw std::runtime_error("Unable to open BB60 device " + std::to_string(deviceId) +
            "with S/N " + std::to_string(serial));
    }

    bbConfigureIQ(deviceId, decimation, bandwidth);
    bbConfigureIQCenter(deviceId, centerFrequency);

    for(const auto &info : this->getSettingInfo()) {
        const auto it = args.find(info.key);
        if(it != args.end()) this->writeSetting(it->first, it->second);
    }
}

SoapyBB60::~SoapyBB60(void)
{
    bbAbort(deviceId);
    bbCloseDevice(deviceId);
}

/*******************************************************************
 * Identification API
 ******************************************************************/

std::string SoapyBB60::getDriverKey(void) const
{
    return "BB60";
}

std::string SoapyBB60::getHardwareKey(void) const
{
    return "BB60";
}

SoapySDR::Kwargs SoapyBB60::getHardwareInfo(void) const
{
    // *Also displayed by --probe

    int firmware = 0;
    bbGetFirmwareVersion(deviceId, &firmware);

    float temp, volt, curr;
    bbGetDeviceDiagnostics(deviceId, &temp, &volt, &curr);

    SoapySDR::Kwargs args;

    args["device_id"] = std::to_string(deviceId);
    args["serial"] = std::to_string(serial);
    args["api_version"] = bbGetAPIVersion();
    args["firmware"] = std::to_string(firmware);
    args["temperature"] = std::to_string(temp);
    args["voltage"] = std::to_string(volt);
    args["current"] = std::to_string(curr);

    return args;
}

/*******************************************************************
 * Channels API
 ******************************************************************/

size_t SoapyBB60::getNumChannels(const int dir) const
{
    return (dir == SOAPY_SDR_RX) ? 1 : 0;
}

/*******************************************************************
 * Antenna API
 ******************************************************************/

std::vector<std::string> SoapyBB60::listAntennas(const int direction, const size_t channel) const
{
    std::vector<std::string> antennas;
    antennas.push_back("RX");
    return antennas;
}

std::string SoapyBB60::getAntenna(const int direction, const size_t channel) const
{
    return "RX";
}

/*******************************************************************
 * Gain API
 ******************************************************************/

void SoapyBB60::setRefMode(const bool useRef)
{
    refMode = useRef;
    double atten = -attenLevel;
    int gain = rfGain;

    if(useRef) {
        gain = BB_AUTO_GAIN;
        atten = BB_AUTO_ATTEN;
    }

    bbStatus status = bbNoError;

    status = bbConfigureGain(deviceId, gain);
    if(status != bbNoError) {
        SoapySDR_logf(SOAPY_SDR_ERROR, "ConfigureGain: %s", bbGetErrorString(status));
    }

    status = bbConfigureLevel(deviceId, refLevel, atten);
    if(status != bbNoError) {
        SoapySDR_logf(SOAPY_SDR_ERROR, "ConfigureLevel: %s", bbGetErrorString(status));
    }

    updateStream();
}

std::vector<std::string> SoapyBB60::listGains(const int direction, const size_t channel) const
{
    std::vector<std::string> results;

    results.push_back("RF");
    results.push_back("ATT");
    results.push_back("REF");

    return results;
}

SoapySDR::Range SoapyBB60::getGainRange(const int direction, const size_t channel, const std::string &name) const
{
    if(name == "ATT") {
        return SoapySDR::Range(-BB_MAX_ATTENUATION, 0);
    }

    if(name == "RF") {
        return SoapySDR::Range(0, BB60C_MAX_GAIN);
    }

    if(name == "REF") {
        return SoapySDR::Range(-120, BB_MAX_REFERENCE);
    }

    throw std::runtime_error(std::string("Unsupported gain: ") + name);

    return SoapySDR::Range(0,0);
}

void SoapyBB60::setGain(const int direction, const size_t channel, const double value)
{
    setGain(direction, channel, "REF", value);
}

void SoapyBB60::setGain(const int direction, const size_t channel, const std::string &name, const double value)
{
    bool useref = true;
    if(name == "RF") {
        rfGain = value;
        useref = false;
    } else if(name == "ATT") {
        attenLevel = value;
        useref = false;
    } else if(name == "REF") {
        refLevel = value;
        useref = true;
    } else {
        throw std::runtime_error(std::string("Unknown GAIN ")+name);
    }

    setRefMode(useref);
}

double SoapyBB60::getGain(const int direction, const size_t channel) const
{
    if(refMode) {
        return getGain(direction, channel, "REF");
    } else {
        return getGain(direction, channel, "RF");
    }
}

double SoapyBB60::getGain(const int direction, const size_t channel, const std::string &name) const
{
    if(name=="RF") {
        if(refMode) return 0.;
        else return rfGain;
    } else if(name=="ATT") {
        if(refMode) return -BB_MAX_ATTENUATION;
        else return attenLevel;
    } else if(name=="REF") {
        if(refMode) return refLevel;
        else  return -120.;
    }

    throw std::runtime_error(std::string("Unsupported GAIN ")+name);

    return 0.0;
}

/*******************************************************************
 * Frequency API
 ******************************************************************/

void SoapyBB60::setFrequency(
        const int direction,
        const size_t channel,
        const std::string &name,
        const double frequency,
        const SoapySDR::Kwargs &args)
{
    if(name == "RF") {
        centerFrequency = (double)frequency;

        bbStatus status = bbConfigureIQCenter(deviceId, centerFrequency);
        if(status != bbNoError) {
            SoapySDR_logf(SOAPY_SDR_ERROR, "ConfigureIQCenter: %s", bbGetErrorString(status));
        }

        updateStream();
    }
}

double SoapyBB60::getFrequency(const int direction, const size_t channel, const std::string &name) const
{
    if(name == "RF") {
        return (double)centerFrequency;
    }

    return 0;
}

std::vector<std::string> SoapyBB60::listFrequencies(const int direction, const size_t channel) const
{
    std::vector<std::string> names;

    names.push_back("RF");

    return names;
}

SoapySDR::RangeList SoapyBB60::getFrequencyRange(
        const int direction,
        const size_t channel,
        const std::string &name) const
{
    SoapySDR::RangeList results;

    if(name == "RF") {
        results.push_back(SoapySDR::Range(BB60_MIN_FREQ, BB60_MAX_FREQ));
    }

    return results;
}

SoapySDR::ArgInfoList SoapyBB60::getFrequencyArgsInfo(const int direction, const size_t channel) const
{
    SoapySDR::ArgInfoList freqArgs;

    return freqArgs;
}

/*******************************************************************
 * Sample Rate API
 ******************************************************************/


void SoapyBB60::setSampleRate(const int direction, const size_t channel, const double rate)
{
    if(sampleRate != rate) {
        auto revii = bb60Decimation.rbegin();
        int dec = revii->first;
        double bw = bb60Decimation.at(dec);

        while(revii != bb60Decimation.rend()) {
            if((BB60_CLOCK/revii->first) >= rate) {
                decimation = revii->first;
                bw = revii->second;
                break;
            }
            revii++;
        }

        sampleRate = rate;
        std::stringstream sstream;
        sstream << "BB60 set decimation " << decimation << " BW " << bw/1e6 << "MHz SR: " << BB60_CLOCK/decimation/1e6 << "MHz";
        SoapySDR_log(SOAPY_SDR_INFO, sstream.str().c_str());
    }
}

double SoapyBB60::getSampleRate(const int direction, const size_t channel) const
{
    return BB60_CLOCK/decimation;
}

std::vector<double> SoapyBB60::listSampleRates(const int direction, const size_t channel) const
{
    std::vector<double> results;

    for(auto &ii: bb60Decimation) {
        results.insert(results.begin(),BB60_CLOCK/ii.first);
    }

    return results;
}

void SoapyBB60::setBandwidth(const int direction, const size_t channel, const double bw)
{
    bandwidth = bw;
}

double SoapyBB60::getBandwidth(const int direction, const size_t channel) const
{
    return std::min(bb60Decimation.at(decimation), bandwidth);
}

std::vector<double> SoapyBB60::listBandwidths(const int direction, const size_t channel) const
{
    std::vector<double> results;

    for(auto &ii: bb60Decimation) {
        results.insert(results.begin(), ii.second);
    }

    return results;
}

/*******************************************************************
 * Clocking API
 ******************************************************************/

double SoapyBB60::getMasterClockRate(void) const
{
    return BB60_CLOCK;
}

/*******************************************************************
 * Utility
 ******************************************************************/

void SoapyBB60::configIO(void) const
{
    if(!streamActive) {
        bbStatus status = bbConfigureIO(deviceId, port1, port2);
        if(status != bbNoError) {
            SoapySDR_logf(SOAPY_SDR_ERROR, "ConfigureIO: %s", bbGetErrorString(status));
        } else {
            SoapySDR_logf(SOAPY_SDR_INFO, "ConfigureIO: %d %d", port1, port2);
        }
    } else {
        SoapySDR_logf(SOAPY_SDR_WARNING, "Can't configureIO while streaming");
    }
}

/*******************************************************************
 * Settings API
 ******************************************************************/

SoapySDR::ArgInfoList SoapyBB60::getSettingInfo(void) const
{
    SoapySDR::ArgInfoList setArgs;

    SoapySDR::ArgInfo arg;

    arg.key = "port1";
    arg.value = "DEFAULT";
    arg.name = "PORT 1";
    arg.description = "BNC connector 1";
    arg.type = SoapySDR::ArgInfo::STRING;
    arg.options = {"DEFAULT", "INT_REF_OUT_AC", "INT_REF_OUT_DC",
                   "EXT_REF_IN_AC", "EXT_REF_IN_DC",
                   "OUT_LOGIC_LOW_AC", "OUT_LOGIC_LOW_DC",
                   "OUT_LOGIC_HIGH_AC", "OUT_LOGIC_HIGH_DC"};

    setArgs.push_back(arg);

    arg.key = "port2";
    arg.value = "DEFAULT";
    arg.name = "PORT 2";
    arg.description = "BNC connector 2";
    arg.type = SoapySDR::ArgInfo::STRING;
    arg.options = {"DEFAULT", "OUT_LOGIC_LOW_DC", "OUT_LOGIC_HIGH_DC",
                   "IN_TRIGGER_RISING_EDGE", "IN_TRIGGER_FALLING_EDGE"};

    setArgs.push_back(arg);

    return setArgs;
}

void SoapyBB60::writeSetting(const std::string &key, const std::string &value)
{
    if(key == "port1" && port1_config.count(value) > 0) {
        port1 = port1_config[value];
        configIO();
        return;
    }

    if(key == "port2" && port2_config.count(value) > 0) {
        port2 = port2_config[value];
        configIO();
        return;
    }

    SoapySDR_logf(SOAPY_SDR_WARNING, "Invalid setting '%s'=='%s'", key.c_str(),value.c_str());
}

std::string SoapyBB60::readSetting(const std::string &key) const
{
    if(key == "port1") {
        std::string ret = "UNKNOWN";
        for(auto &ii: port1_config) {
            if(ii.second == port1) {
                ret = ii.first;
                break;
            }
        }
        return ret;
    }

    if(key == "port2") {
        std::string ret = "UNKNOWN";
        for(auto &ii: port2_config) {
            if(ii.second == port2) {
                ret = ii.first;
                break;
            }
        }
        return ret;
    }

    SoapySDR_logf(SOAPY_SDR_WARNING, "Unknown setting '%s'", key.c_str());

    return "";
}
