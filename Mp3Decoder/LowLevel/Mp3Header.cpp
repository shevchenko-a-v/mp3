#include "stdafx.h"
#include "Mp3Header.h"

namespace LowLevel
{

    const BitrateTable Mp3Header::_bitrateTable = 
    {
        {
            AudioVersion::MPEG1,
            {   // MPEG1 start
                { 
                    LayerIndex::LayerI,
                    {{1,32}, {2,64}, {3,96}, {4,128}, {5,160}, {6,192}, {7,224}, {8,256},
                    {9,288}, {10,320}, {11,352}, {12,384}, {13,416}, {14,448}}
                },
                {
                    LayerIndex::LayerII,
                    {{1,32}, {2,48}, {3,56}, {4,64}, {5,80}, {6,96}, {7,112}, {8,128},
                    {9,160}, {10,192}, {11,224}, {12,256}, {13,320}, {14,384}}
                },
                {
                    LayerIndex::LayerIII,
                    {{1,32}, {2,40}, {3,48}, {4,56}, {5,64}, {6,80}, {7,96}, {8,112},
                    {9,128}, {10,160}, {11,192}, {12,224}, {13,256}, {14,320}}
                }
                //MPEG1 end
            }
        },
        {
            AudioVersion::MPEG2,
            {   // MPEG2 start
                {
                    LayerIndex::LayerI,
                    {{1,32}, {2,48}, {3,56}, {4,64}, {5,80}, {6,96}, {7,112}, {8,128},
                    {9,144}, {10,160}, {11,176}, {12,192}, {13,224}, {14,256}}
                },
                {
                    LayerIndex::LayerII,
                    {{1,8}, {2,16}, {3,24}, {4,32}, {5,40}, {6,48}, {7,56}, {8,64},
                    {9,80}, {10,96}, {11,112}, {12,128}, {13,144}, {14,160}}
                },
                {
                    LayerIndex::LayerIII, // same as LayerII
                    {{1,8}, {2,16}, {3,24}, {4,32}, {5,40}, {6,48}, {7,56}, {8,64},
                    {9,80}, {10,96}, {11,112}, {12,128}, {13,144}, {14,160}}
                }
                //MPEG2 end
            }
        },
        {
            AudioVersion::MPEG25,
            {   // MPEG25 start - same as MPEG2
                {
                    LayerIndex::LayerI,
                    {{1,32}, {2,48}, {3,56}, {4,64}, {5,80}, {6,96}, {7,112}, {8,128},
                    {9,144}, {10,160}, {11,176}, {12,192}, {13,224}, {14,256}}
                },
                {
                    LayerIndex::LayerII,
                    {{1,8}, {2,16}, {3,24}, {4,32}, {5,40}, {6,48}, {7,56}, {8,64},
                    {9,80}, {10,96}, {11,112}, {12,128}, {13,144}, {14,160}}
                },
                {
                    LayerIndex::LayerIII, // same as LayerII
                    {{1,8}, {2,16}, {3,24}, {4,32}, {5,40}, {6,48}, {7,56}, {8,64},
                    {9,80}, {10,96}, {11,112}, {12,128}, {13,144}, {14,160}}
                }
                //MPEG25 end
            }
        }
    };


    const SamplingRateTable Mp3Header::_samplingRateTable = 
    {
        {
            {AudioVersion::MPEG1, {{0,44100}, {1, 48000},{2, 32000}}},
            {AudioVersion::MPEG2, {{0,22050}, {1, 24000},{2, 16000}}},
            {AudioVersion::MPEG25, {{0,11025}, {1, 12000},{2, 8000}}}
        }
    };

    const std::map<LayerIndex, int> Mp3Header::_coefficients =
    {
        { LayerIndex::LayerI, 12 }, // (must be multiplied with 4, because of slot size)
        { LayerIndex::LayerII, 144 },
        { LayerIndex::LayerIII, 144 }
    };
    const std::map<LayerIndex, int> Mp3Header::_slotSizes =
    {
        { LayerIndex::LayerI, 4 },
        { LayerIndex::LayerII, 1 },
        { LayerIndex::LayerIII, 1 }
    };



    Mp3Header::Mp3Header(unsigned int sourceBytes) noexcept
        : _rawHeader(sourceBytes)
    {}

    AudioVersion LowLevel::Mp3Header::GetAudioVersion()
    {
        unsigned int rawAudioVersion = (_rawHeader >> 19) & 0x00000003;
        switch (rawAudioVersion)
        {
        case 0:
            return AudioVersion::MPEG25;
        case 2:
            return AudioVersion::MPEG2;
        case 3:
            return AudioVersion::MPEG1;
        }
        throw std::invalid_argument("Audio version cannot be correctly parsed from header.");
    }

    LayerIndex Mp3Header::GetLayerIndex()
    {
        unsigned int rawLayerIndex = (_rawHeader >> 17) & 0x00000003;
        switch (rawLayerIndex)
        {
        case 1:
            return LayerIndex::LayerIII;
        case 2:
            return LayerIndex::LayerII;
        case 3:
            return LayerIndex::LayerI;
        }
        throw std::invalid_argument("Layer index cannot be correctly parsed from header.");
    }

    bool Mp3Header::IsProtected() noexcept
    {
        return !((_rawHeader >> 16) & 0x00000001);
    }

    unsigned int Mp3Header::GetBitrate()
    {
        unsigned int rawBitrateIndex = (_rawHeader >> 12) & 0x0000000f;
        
        return GetBitrateFromIndex(GetAudioVersion(), GetLayerIndex(), rawBitrateIndex);
    }
    unsigned int Mp3Header::GetSamplingRate()
    {
        unsigned int rawSamplingRate = (_rawHeader >> 10) & 0x00000003;
        return GetSamplingRateFromIndex(GetAudioVersion(), rawSamplingRate);
    }

    bool Mp3Header::IsPaddingOn() noexcept
    {
        return (_rawHeader >> 9) & 0x00000001;
    }

    ChannelMode Mp3Header::GetChannelMode() noexcept
    {
        unsigned int rawChannelMode = (_rawHeader >> 10) & 0x00000003;
        switch (rawChannelMode)
        {
        case 0:
            return ChannelMode::Stereo;
        case 1:
            return ChannelMode::JointStereo;
        case 2:
            return ChannelMode::DualChannel;
        default: //case 3:
            return ChannelMode::SingleChannel;
        }
    }

    unsigned int Mp3Header::GetFrameSize() 
    {
        auto layer = GetLayerIndex();
        auto samplingRate = GetSamplingRate();
        return samplingRate == 0 ? 0 : ((GetCoefficientForLayer(layer) * GetBitrate() / samplingRate) + IsPaddingOn() ? 1 : 0) * GetSlotSizeForLayer(layer);
    }
    int Mp3Header::GetBitrateFromIndex(AudioVersion audioVersion, LayerIndex layer, int index) noexcept
    {
        auto itrAudioVersion = _bitrateTable.find(audioVersion);
        if (itrAudioVersion != _bitrateTable.cend())
        {
            auto itrLayer = itrAudioVersion->second.find(layer);
            if (itrLayer != itrAudioVersion->second.cend())
            {
                auto itrIndex = itrLayer->second.find(index);
                if (itrIndex != itrLayer->second.cend())
                    return itrIndex->second;
            }
        }
        return 0;
    }
    int Mp3Header::GetSamplingRateFromIndex(AudioVersion audioVersion, int index) noexcept
    {
        auto itrAudioVersion = _samplingRateTable.find(audioVersion);
        if (itrAudioVersion != _samplingRateTable.cend())
        {
            auto itrIndex = itrAudioVersion->second.find(index);
            if (itrIndex != itrAudioVersion->second.cend())
                return itrIndex->second;
        }
        return 0;
    }
    int Mp3Header::GetCoefficientForLayer(LayerIndex layer) noexcept
    {
        auto itrCoeff = _coefficients.find(layer);
        if (itrCoeff != _coefficients.cend())
            return itrCoeff->second;
        return 0;
    }
    int Mp3Header::GetSlotSizeForLayer(LayerIndex layer) noexcept
    {
        auto itrSlotSize = _slotSizes.find(layer);
        if (itrSlotSize != _slotSizes.cend())
            return itrSlotSize->second;
        return 0;
    }
}
