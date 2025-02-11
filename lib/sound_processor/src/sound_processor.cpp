#include "sound_processor.h"
#include "WAVWriter.h"

void SoundProcessor::Info()
{
    std::cout << "Mute Converter mutes the sound over a specified time range. "
                 "It is used with the command [mute a b] in the configuration file line,"
                 "where a and b are the start and end points of the mute." << std::endl;
    std::cout << "Mix Converter mixes two main audio streams with an external one"
                 " from a specific second until the end of the main stream. "
                 "It uses the command mix $a b, "
                 "where $a refers to the stream (corresponding to the file number), "
                 "and b is the second from which to start mixing" << std::endl;
    std::cout << "RandomBoost Converter multiplies the value of each sample by a random"
                 " number from a specified range. It is used with the command rboost a b, "
                 "where a and b define the range.";

}

SoundProcessor::SoundProcessor(int argc, char *argv[]):output_wav_file_(argv[2]), config_(argv[1])
{
    if (argc < 4)
    {
        throw std::invalid_argument("Bad run arguments");
    }
    input_header_ = WAVReader(argv[3]).GetHeader();
    input_samples_.resize(argc - 3);

    for (size_t i = 3; i != argc; ++i)
    {
        input_samples_.at(i - 3) = WAVReader(argv[i]).GetSamples();
    }
}

void SoundProcessor::Run()
{
    std::vector<ConfigLine> config = config_.GetConfig();
    std::vector<sample> current_sample_stream = input_samples_.at(0);
    if (config.size() == 0)
    {
        throw std::invalid_argument("Empty config");
    }
    for (size_t i = 0; i != config.size(); ++i)
    {
        if (config.at(i).converter_name_ == "mute")
        {
            MuteConverterCreator mute_creator;
            std::unique_ptr<Converter> mute = mute_creator.Create(config.at(i));
            mute->Convert(current_sample_stream, input_samples_);
        }
        if (config.at(i).converter_name_ == "mix")
        {
            MixConverterCreator mix_creator;
            std::unique_ptr<Converter> mix = mix_creator.Create(config.at(i));
            mix->Convert(current_sample_stream, input_samples_);
        }
        if (config.at(i).converter_name_ == "rboost")
        {
            RandomBoostConverterCreator rboost_creator;
            std::unique_ptr<Converter> rboost = rboost_creator.Create(config.at(i));
            rboost->Convert(current_sample_stream, input_samples_);
        }
    }
    WAVWriter output(current_sample_stream, input_header_, output_wav_file_);
    output.WriteToFile();
}
