#include "stdafx.h"
#include "Mp3Stream.h"
#include "Mp3Header.h"


namespace LowLevel
{
    Mp3Stream::Mp3Stream(std::string path)
        : _file_path(path)
       // : _file(new std::ifstream(path, std::ios::binary | std::ios::ate))
    {
    }

    bool Mp3Stream::Parse()
    {
        std::ifstream input(_file_path, std::ios::binary | std::ios::in);
        bool success = true;
        std::vector<Mp3Header> headers;
        while (success)
        {
            auto header = ReadNextHeader(input, success);
            headers.push_back(header);
            auto frameSize = header.GetFrameSize();
            //frameSize += header.IsProtected() ? 2 : 0; // CRC bytes
            //frameSize += header.GetChannelMode() == ChannelMode::SingleChannel ? 17 : 32; // Side information length
            std::streamoff f = frameSize - 4/*header len*/;
            input.seekg(input.tellg() + f);
        }
        input.close();
        return success;
    }

    ByteArray Mp3Stream::ReadBytes(std::ifstream & input, unsigned int count)
    {
        if (input.fail())
            return ByteArray{};

        ByteArray result;
        for (unsigned int i = 0; i < count && !input.eof(); ++i)
        {
            char byte;
            input.read(&byte, 1);
            result.push_back(static_cast<unsigned char>(byte));
        }
        return result;
    }

    Mp3Header Mp3Stream::ReadNextHeader(std::ifstream & stream, bool& success)
    {
        const auto headerLength = 4;
        auto headerContainer = ReadBytes(stream, headerLength);
        unsigned int header = (static_cast<unsigned int>(headerContainer[0]) << 24) | 
            (static_cast<unsigned int>(headerContainer[1]) << 16) |
            (static_cast<unsigned int>(headerContainer[2]) << 8) |
            (static_cast<unsigned int>(headerContainer[3]));
        
        const unsigned int frameHeaderMark = 0x000007ff;
        auto frameFound = ((header >> 21) & frameHeaderMark) == frameHeaderMark;
        while (!stream.fail() && !frameFound)
        {
            header = (header << 8) | static_cast<unsigned int>(ReadBytes(stream, 1)[0]);
            frameFound = ((header >> 21) & frameHeaderMark) == frameHeaderMark;
        }
        success = frameFound;
        return Mp3Header(header);
    }

    //Mp3Stream::~Mp3Stream()
    //{
    //    _file->close();
    //}
}