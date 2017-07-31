/**
* @file
* @brief
* @author   songshenyi
* <pre><b>copyright: Youku</b></pre>
* <pre><b>email: </b>songshenyi@youku.com</pre>
* <pre><b>company: </b>http://www.youku.com</pre>
* <pre><b>All rights reserved.</b></pre>
* @date 2015/07/24
* @see
*/

#pragma once

#include <stdint.h>

typedef struct _bitfile bitfile;

namespace live_stream_sdk
{

  struct adts_fixed_header
  {
    uint16_t syncword : 12;
    uint8_t	ID : 1;
    uint8_t	layer : 2;
    uint8_t	protection_absent : 1;
    uint8_t	profile : 2;
    uint8_t sampling_frequency_index : 4;
    uint8_t	private_bit : 1;
    uint8_t channel_configuration : 3;
    uint8_t original_copy : 1;
    uint8_t	home : 1;
  };
  struct adts_variable_header
  {
    uint8_t  copyright_identification_bit : 1;
    uint8_t	 copyright_identification_start : 1;
    uint16_t frame_length : 13;
    uint16_t adts_buffer_fullness : 11;
    uint8_t	 number_of_raw_data_blocks_in_frame : 2;
  };

  uint8_t aac_get_samplingFrequencyIndex(uint32_t samplerate);

  int aac_get_channel_by_index(int ch);

  class AACConfig
  {
  private:
    //normal 
    bool    _is_sbr;
    uint8_t _audio_object;
    uint32_t _sample_freq;
    uint8_t _channel_config;
    uint32_t _frame_length;
    //#define MAX_EXTRADATA_SIZE 7 /* LC + SBR + PS config */
    //uint8_t _audioSpecificConfig[MAX_EXTRADATA_SIZE];
    uint8_t _audioSpecificConfig[7];

    int downSampledSBR;
    int forceUpSampling;
  private:
    int8_t _GASpecificConfig(bitfile *ld);
  public:
    AACConfig();
    AACConfig(uint8_t*);

    uint8_t set_audioObjectType(uint8_t audioObjectType);
    uint8_t set_samplingFrequencyIndex(uint8_t samplingFrequencyIndex);
    uint8_t set_channelConfig(uint8_t channels);

    uint8_t get_audioObjectType();
    uint8_t get_samplingFrequencyIndex();
    uint8_t get_channelConfig();


    void set_samplingFrequency(uint32_t samplingFrequency);
    void set_frame_length(uint32_t length);
    void set_sbr(bool is_sbr);

    uint32_t get_samplingFrequency();
    uint32_t get_frame_length();
    bool is_sbr();

    int parse_aac_specific(uint8_t* data, int len);
    int parse_adts_header(uint8_t* data, int len);
    uint8_t * build_aac_specific(int& len);
    uint8_t * build_aac_adts(int& len);


    typedef struct
    {
      union
      {
        struct
        {
          uint8_t audioObjectType : 5;
          uint8_t samplingFrequencyIndex : 4;
          uint8_t channels : 4;
          uint8_t frameLengthFlag : 1;
          uint8_t dependsOnCoreCoder : 1;
          uint8_t extensionFlag : 1;
        };
        struct
        {
          uint8_t header_data[2];
        };
      };
    }AudioSpecificConfig;// LATM LC-AAC 2 bytes;

    AudioSpecificConfig latm_header;
  };
}
