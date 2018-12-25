#pragma once

namespace LowLevel
{
    class Mp3Header;

    class Mp3Stream
    {
    public:
        Mp3Stream(std::string path);
        //virtual ~Mp3Stream();

        Mp3Stream(const Mp3Stream&) = delete;
        Mp3Stream& operator=(const Mp3Stream&) = delete;
        Mp3Stream(const Mp3Stream&&) = delete;
        Mp3Stream&& operator=(const Mp3Stream&&) = delete;

        bool Parse();

        static ByteArray ReadBytes(std::ifstream &input, unsigned int count);
    private:

        Mp3Header ReadNextHeader(std::ifstream& stream, bool &success);
        //std::unique_ptr<std::ifstream> _file;
        std::string _file_path;
    };
}