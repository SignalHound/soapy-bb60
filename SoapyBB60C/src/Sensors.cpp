#include "SoapyBB60.hpp"

std::vector<std::string> SoapyBB60::listSensors(void) const
{
    std::vector<std::string> sensors;

    sensors.push_back("TEMP");
    sensors.push_back("VOLT");
    sensors.push_back("CURR");

    return sensors;
}

SoapySDR::ArgInfo SoapyBB60::getSensorInfo(const std::string &key) const
{
    SoapySDR::ArgInfo info;

    if(key == "TEMP") {
        info.key = key;
        info.value = "0";
        info.name = "Temperature";
        info.description = "FPGA temperature";
        info.units = "C";
        info.type = SoapySDR::ArgInfo::FLOAT;

        return info;
    }

    if(key == "VOLT") {
        info.key = key;
        info.value = "1";
        info.name = "Voltage";
        info.description = "Input voltage";
        info.units = "V";
        info.type = SoapySDR::ArgInfo::FLOAT;

        return info;
    }

    if(key == "CURR") {
        info.key = key;
        info.value = "2";
        info.name = "Current";
        info.description = "Input current draw";
        info.units = "mA";
        info.type = SoapySDR::ArgInfo::FLOAT;

        return info;
    }

    throw std::runtime_error("Unknown sensor: " + key);
}

std::string SoapyBB60::readSensor(const std::string &key) const
{
    float temp, volt, curr;
    bbGetDeviceDiagnostics(deviceId, &temp, &volt, &curr);

    if(key == "TEMP") {
        return std::to_string(temp);
    }

    if(key == "VOLT") {
        return std::to_string(volt);
    }

    if(key == "CURR") {
        return std::to_string(curr);
    }

    throw std::runtime_error("Unknown sensor: " + key);
}
