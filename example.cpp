#include <SoapySDR/Device.hpp>
#include <SoapySDR/Formats.hpp>

int main() {
    // List all BB60C devices
    printf("Looking for Signal Hound BB60C devices..\n\n");

    SoapySDR::KwargsList bbDevices;
    SoapySDR::KwargsList results = SoapySDR::Device::enumerate();

    for(int i = 0; i < results.size(); i++) {
        if(results[i]["driver"] == "bb60c") {
            bbDevices.push_back(results[i]);
        }
    }

    if(bbDevices.size() < 1) {
        printf("No BB60C devices found\n");
        return EXIT_SUCCESS;
    }

    printf("%d BB60C devices found:\n", (int)bbDevices.size());

    for(int i = 0; i < bbDevices.size(); i++) {
        printf("\t%s\n", bbDevices[i]["serial"].c_str());
    }

    // Connect to first device
    SoapySDR::Kwargs args = bbDevices[0];

    printf("\nConnecting to first BB60C (%s)..\n", args["serial"].c_str());

    SoapySDR::Device *sdr = SoapySDR::Device::make(args);

    if(!sdr) {
        fprintf(stderr, "SoapySDR::Device::make failed\n");
        return EXIT_FAILURE;
    }

    printf("Connected\n\n");

    // Get hardware info
    SoapySDR::Kwargs hardInfo = sdr->getHardwareInfo();

    printf("Hardware Info:\n");
    printf("\tdevice id: %s\n", hardInfo["device_id"].c_str());
    printf("\tserial: %s\n", hardInfo["serial"].c_str());
    printf("\tapi version: %s\n", hardInfo["api_version"].c_str());
    printf("\tfirmware: %s\n", hardInfo["firmware"].c_str());
    printf("\ttemp: %s\n", hardInfo["temperature"].c_str());
    printf("\tvoltage: %s\n", hardInfo["voltage"].c_str());
    printf("\tcurrent: %s\n\n", hardInfo["current"].c_str());
    std::vector< std::string > str_list;    //string list

    // Config
    sdr->setFrequency(SOAPY_SDR_RX, 0, 2400.0e6);
    sdr->setGain(SOAPY_SDR_RX, 0, -20.0);
    sdr->setSampleRate(SOAPY_SDR_RX, 0, 40.0e6);

    // Setup a stream (complex floats)
    SoapySDR::Stream *rx_stream = sdr->setupStream(SOAPY_SDR_RX, SOAPY_SDR_CF32);
    if(!rx_stream) {
        fprintf(stderr, "Stream setup failed\n");
        SoapySDR::Device::unmake(sdr);
        return EXIT_FAILURE;
    }
    sdr->activateStream(rx_stream, 0, 0, 0);

    // Sample buffer
    const int NUM_SAMPLES = 1024;
    std::complex<float> buff[NUM_SAMPLES];

    // Receive samples
    for(int i = 0; i < 10; ++i) { // Get 10 blocks of 1024 IQ samples
        void *buffs[] = {buff};
        int flags;
        long long time_ns;
        int ret = sdr->readStream(rx_stream, buffs, NUM_SAMPLES, flags, time_ns, 1e5);

        printf("\nGot %d samples\n", ret);
        printf("First 5:\n");
        for(int j = 0; j < 5; j++) {
            printf("\t(%0.8f, %0.8f)\n", buff[j].real(), buff[j].imag());
        }

        // Do something with IQ data..
    }

    // Shutdown stream
    sdr->deactivateStream(rx_stream, 0, 0); // Stop streaming
    sdr->closeStream(rx_stream);

    // Cleanup device handle
    SoapySDR::Device::unmake(sdr);
    printf("Done\n");

    return EXIT_SUCCESS;
}