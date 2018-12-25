#pragma once

namespace LowLevel
{
    enum class AudioVersion
    {
        MPEG25,
        MPEG2,
        MPEG1
    };

    enum class LayerIndex
    {
        LayerIII,
        LayerII,
        LayerI
    };

    enum class ChannelMode
    {
        Stereo,
        JointStereo,
        DualChannel,
        SingleChannel
    };

    using BitrateTable = std::map<AudioVersion, std::map<LayerIndex, std::map<int, int>>>;
    using SamplingRateTable = std::map<AudioVersion, std::map<int, int>>;

    class Mp3Header
    {
    public:
        // sourceBytes is a raw 4 bytes of mp3 header
        Mp3Header(unsigned int sourceBytes) noexcept;

        AudioVersion GetAudioVersion();
        LayerIndex GetLayerIndex();
        bool IsProtected() noexcept;
        unsigned int GetBitrate();
        unsigned int GetSamplingRate();
        bool IsPaddingOn() noexcept;
        ChannelMode GetChannelMode() noexcept;
        
        unsigned int GetFrameSize();

    private:

        static const BitrateTable _bitrateTable;
        static const SamplingRateTable _samplingRateTable;
        static const std::map<LayerIndex, int> _coefficients;
        static const std::map <LayerIndex, int> _slotSizes;

        static int GetBitrateFromIndex(AudioVersion audioVersion, LayerIndex layer, int index);
        static int GetSamplingRateFromIndex(AudioVersion audioVersion, int index);
        static int GetCoefficientForLayer(LayerIndex layer) noexcept;
        static int GetSlotSizeForLayer(LayerIndex layer) noexcept;

        const unsigned int _rawHeader;
    };
}