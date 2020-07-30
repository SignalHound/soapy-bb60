#pragma once

#include <SoapySDR/Device.hpp>
#include <SoapySDR/Logger.h>
#include <SoapySDR/Types.h>

#include <stdexcept>
#include <thread>
#include <mutex>
#include <atomic>
#include <condition_variable>
#include <string>
#include <cstring>
#include <algorithm>
#include <atomic>

#include <bb_api.h>

class SoapyBB60: public SoapySDR::Device {
public:
    SoapyBB60(const SoapySDR::Kwargs &args);

    ~SoapyBB60(void);

    /*******************************************************************
     * Identification API
     ******************************************************************/

    std::string getDriverKey(void) const;

    std::string getHardwareKey(void) const;

    SoapySDR::Kwargs getHardwareInfo(void) const;

    /*******************************************************************
     * Channels API
     ******************************************************************/

    size_t getNumChannels(const int) const;

    /*******************************************************************
     * Stream API
     ******************************************************************/

    std::vector<std::string> getStreamFormats(const int direction, const size_t channel) const;

    std::string getNativeStreamFormat(const int direction, const size_t channel, double &fullScale) const;

    SoapySDR::ArgInfoList getStreamArgsInfo(const int direction, const size_t channel) const;

    SoapySDR::Stream *setupStream(const int direction, const std::string &format,
            const std::vector<size_t> &channels = std::vector<size_t>(),
            const SoapySDR::Kwargs &args = SoapySDR::Kwargs());

    void closeStream(SoapySDR::Stream *stream);

    bool updateStream();

    int activateStream(
            SoapySDR::Stream *stream,
            const int flags = 0,
            const long long timeNs = 0,
            const size_t numElems = 0);

    int deactivateStream(SoapySDR::Stream *stream, const int flags = 0, const long long timeNs = 0);

    int readStream(
            SoapySDR::Stream *stream,
            void * const *buffs,
            const size_t numElems,
            int &flags,
            long long &timeNs,
            const long timeoutUs = 100000);

    /*******************************************************************
     * Antenna API
     ******************************************************************/

    std::vector<std::string> listAntennas(const int direction, const size_t channel) const;

    std::string getAntenna(const int direction, const size_t channel) const;

    /*******************************************************************
     * Gain API
     ******************************************************************/

    void setRefMode(const bool useRef);

    std::vector<std::string> listGains(const int direction, const size_t channel) const;

    void setGain(const int direction, const size_t channel, const std::string &name, const double value);

    void setGain(const int direction, const size_t channel, const double value);

    double getGain(const int dir, const size_t channel) const;

    double getGain(const int direction, const size_t channel, const std::string &name) const;

    SoapySDR::Range getGainRange(const int direction, const size_t channel, const std::string &name) const;

    /*******************************************************************
     * Frequency API
     ******************************************************************/

    void setFrequency(
            const int direction,
            const size_t channel,
            const std::string &name,
            const double frequency,
            const SoapySDR::Kwargs &args = SoapySDR::Kwargs());

    double getFrequency(const int direction, const size_t channel, const std::string &name) const;

    std::vector<std::string> listFrequencies(const int direction, const size_t channel) const;

    SoapySDR::RangeList getFrequencyRange(const int direction, const size_t channel, const std::string &name) const;

    SoapySDR::ArgInfoList getFrequencyArgsInfo(const int direction, const size_t channel) const;

    /*******************************************************************
     * Sample Rate API
     ******************************************************************/

    void setSampleRate(const int direction, const size_t channel, const double rate);

    double getSampleRate(const int direction, const size_t channel) const;

    std::vector<double> listSampleRates(const int direction, const size_t channel) const;

    /*******************************************************************
     * Bandwidth API
     ******************************************************************/

    void setBandwidth(const int direction, const size_t channel, const double bw);

    double getBandwidth(const int direction, const size_t channel) const;

    std::vector<double> listBandwidths(const int direction, const size_t channel) const;

    /*******************************************************************
     * Clocking API
     ******************************************************************/

    double getMasterClockRate(void) const;

    /*******************************************************************
     * Sensor API
     ******************************************************************/

    std::vector<std::string> listSensors(void) const;

    SoapySDR::ArgInfo getSensorInfo(const std::string &key) const;

    std::string readSensor(const std::string &key) const;

    /*******************************************************************
     * Utility
     ******************************************************************/

    void configIO(void) const;

    /*******************************************************************
     * Settings API
     ******************************************************************/

    SoapySDR::ArgInfoList getSettingInfo(void) const;

    void writeSetting(const std::string &key, const std::string &value);

    std::string readSetting(const std::string &key) const;

private:
    int deviceId;
    int serial;

    // Cached settings
    double sampleRate, centerFrequency, bandwidth;
    int decimation;
    bool streamActive;
    int rfGain;
    double refLevel;
    double attenLevel;
    bool refMode = true;
    unsigned int port1 = 0;
    unsigned int port2 = 0;
    const std::map<int, double> bb60Decimation = {
        {8192, 4e3},
        {4096, 8e3},
        {2048, 15e3},
        {1024, 30e3},
        {512, 65e3},
        {256, 140e3},
        {128, 250e3},
        {64, 500e3},
        {32, 1e6},
        {16, 2e6},
        {8, 3.75e6},
        {4, 8e6},
        {2, 17.8e6},
        {1, 27e6}
    };
};
