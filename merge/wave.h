/*************************************************************************
	> File Name: wave.h
	> Author:lights 
	> Mail: guangxuhuang@gmail.com
	> Created Time: 2018年01月10日 星期三 20时23分13秒
 ************************************************************************/

#ifndef _WAVE_H
#define _WAVE_H
#include "constvar.h"

class Wave
{
    public:
    int readWave(__IN__ const char *szFileName);
    int writeWave(__OUT__ const char *szFileName);
	int writeWaveSplit(float startTime, float endTime, __OUT__ const char *szFileName);

    private:
    short *pData;
    int nSample;
    struct RIFFFormat
    {
        uint RIFF;         //="RIFF"
        uint nSize_8;      //=filesize -8
        uint WAVE;         //="WAVE" 
        uint fmt;          //="fmt"
        uint nFmtSize;     //=下一个结构体大小
    };
    struct WaveFormat
    {
        ushort  wFormatTag;        //pcm=1
        ushort  nChannels;         //mono=1,stero=2...
        uint    nSamplesPerSec;    //=8k,16k...
        uint    nAvgBytesPerSec;   //=nSamplesPerSec*nChannels*wBitsPerSample/8
        ushort  nBlockAlign;       //=nChannels*wBitsPerSample/8
        ushort  wBitsPerSample;    //=16,24,32
    };
    struct DataFormat
    {
        uint data;        //="data"
        uint nDataSize;   //=数据长度
    };
};



#endif //_WAVE_H
