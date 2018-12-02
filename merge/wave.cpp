/*************************************************************************
	> File Name: wave.cpp
	> Author:lights 
	> Mail:guangxuhuang@gmail.com 
	> Created Time: 2018年01月10日 星期三 20时58分00秒
 ************************************************************************/
#pragma once  
 
#include "stdafx.h"

 
#include "wave.h"
#include <stdio.h>




#define FCC(ch1, ch2, ch3, ch4) ((((uint)(ch4)) << 24) |   \
             					 (((uint)(ch3)) << 16) |   \
								 (((uint)(ch2)) << 8)  |   \
								 (((uint)(ch1))))


 int Wave::readWave(__IN__ const char *szFileName){
    RIFFFormat riffChunk;
    WaveFormat waveChunk;
    DataFormat dataChunk;
    FILE *fp;
    fp=fopen(szFileName,"rb");
    if(fp == NULL )
    {
        printf("wave cannot open");
       return -1;
    }
    if (!fread((char*)&riffChunk, sizeof(RIFFFormat), 1, fp))
    {
       printf("Cannot read the RiffChunk.");
       return -1;
    }
    if ((riffChunk.RIFF != FCC('R', 'I', 'F', 'F')) ||
        (riffChunk.WAVE != FCC('W', 'A', 'V', 'E')) ||
        (riffChunk.fmt != FCC('f', 'm', 't', ' ')))
    {
        printf("wav format seems wrong.");
        return -2;
    }
    if (!fread((char*)&waveChunk, sizeof(WaveFormat), 1, fp))
    {
        printf("cannot read the WavChunk.");
        return -3;
    }
    
    uint strData;
    if(!fread(&strData,4,1,fp))
    {
      printf("failed to read dataChunk");
     return -4;
    }
    
    if (fread((char*)&dataChunk.nDataSize, sizeof(dataChunk.nDataSize), 1, fp) != 1)
    {
        printf("read wav failed.");
        return -400;
    }
    unsigned long loc = ftell(fp);
    fseek(fp, 0, SEEK_END);
    unsigned long len = ftell(fp);    /** @brief< 文件长度 */
    fseek(fp, loc, SEEK_SET);
    if (len <= 44)
    {
        printf("wav format is wrong.");
        return -5;
    }

    /** @brief 标准文件应该有dataChunk.nDataSize + loc == riffChunk.nSize_8 + 8, 此处稍微放松 */
    if (dataChunk.nDataSize + loc > riffChunk.nSize_8 + 8)    /** @brief< 按两个标示分别计算文件大小 */
    {
        dataChunk.nDataSize = riffChunk.nSize_8 + 8 - loc;
        printf("wav format seems wrong riffChunk nsize.");
    }
    if (dataChunk.nDataSize + loc > len || dataChunk.nDataSize > UINT_MAX - loc)
    {
		if (len - loc <= 0)
		{
            printf("wav format is wrong dataChunk nDataSize.");
			return -6;
		}
        dataChunk.nDataSize = len - loc;
        printf("wav format seems wrong.");
    }
    int m_nFs = waveChunk.nSamplesPerSec;
    int m_nOrigChannel = waveChunk.nChannels;
	if (m_nFs <= 0)
	{
		printf("wav sampling rate <= 0.");
		return -7;
	}
	if (m_nOrigChannel <= 0 || m_nOrigChannel > 2)
	{
		printf("wav channel <=0 || > 2.");
		return -8;
	}
    int nBytePerSample = waveChunk.wBitsPerSample / 8;
    if (waveChunk.nAvgBytesPerSec != (nBytePerSample * m_nOrigChannel * m_nFs))
    {
        printf("wav format is wrong nAvgBytesPerSec!");
        return -9;
    }
    nSample = dataChunk.nDataSize / nBytePerSample;
    dataChunk.nDataSize = nSample * nBytePerSample;  
    if (nSample <= 0)
    {
        printf("wav format is wrong m_nSample!");
        return -10;
    }
    if (pData != NULL)
    {
        //delete []m_pData;
        pData = NULL;

    }
    pData = new short[nSample];
        if (fread((char*)pData, dataChunk.nDataSize, 1, fp) != 1)
        {
            printf("read wav failed.");
            return -11;
        }
    return 0;
    }

int Wave::writeWave(__OUT__ const char *szFileName){
	FILE *fp;
	fp=fopen(szFileName,"wb");
    if (fp == NULL)
    {
        return -1;
    }
    RIFFFormat riffChunk;
    WaveFormat waveChunk;
    DataFormat dataChunk;
    riffChunk.RIFF = FCC('R', 'I', 'F', 'F');   // FCC('RIFF');
    riffChunk.nSize_8 = nSample * 2 + sizeof(WaveFormat) + sizeof(RIFFFormat);
    riffChunk.WAVE = FCC('W', 'A', 'V', 'E');   // FCC('WAVE');
    riffChunk.fmt = FCC('f', 'm', 't', ' ');   // FCC('fmt ');
    riffChunk.nFmtSize = sizeof(WaveFormat);

    waveChunk.wFormatTag = 1;
    waveChunk.nChannels = 1;
    waveChunk.nSamplesPerSec = 16000;
    waveChunk.nAvgBytesPerSec = 16000 * 2;
    waveChunk.nBlockAlign = 2;
    waveChunk.wBitsPerSample = 16;

    dataChunk.data = FCC('d', 'a', 't', 'a');   // FCC('data');
    dataChunk.nDataSize = nSample * 2;

    if (!fwrite((char*)&riffChunk, sizeof(RIFFFormat), 1, fp))
    {
        return -2;
    }
    if (!fwrite((char*)&waveChunk, sizeof(WaveFormat), 1, fp))
    {
        return -3;
    }
    if (!fwrite((char*)&dataChunk, sizeof(DataFormat), 1, fp))
    {
        return -4;
    }
    if (!fwrite((char*)pData, nSample * 2, 1, fp))
    {
        return -5;
    }
	fclose(fp);
    return 0;
    }

int Wave::writeWaveSplit(float startTime,float endTime,__OUT__ const char *szFileName) {

	FILE *fp;
	fp = fopen(szFileName, "wb");
	if (fp == NULL)
	{
		return -1;
	}

	int write_nSample = (endTime-startTime)*8000;

	RIFFFormat riffChunk;
	WaveFormat waveChunk;
	DataFormat dataChunk;
	riffChunk.RIFF = FCC('R', 'I', 'F', 'F');   // FCC('RIFF');
	riffChunk.nSize_8 = write_nSample * 2 + sizeof(WaveFormat) + sizeof(RIFFFormat);
	riffChunk.WAVE = FCC('W', 'A', 'V', 'E');   // FCC('WAVE');
	riffChunk.fmt = FCC('f', 'm', 't', ' ');   // FCC('fmt ');
	riffChunk.nFmtSize = sizeof(WaveFormat);

	waveChunk.wFormatTag = 1;
	waveChunk.nChannels = 1;
	waveChunk.nSamplesPerSec = 8000;
	waveChunk.nAvgBytesPerSec = 8000 * 2;
	waveChunk.nBlockAlign = 2;
	waveChunk.wBitsPerSample = 16;

	dataChunk.data = FCC('d', 'a', 't', 'a');   // FCC('data');
	dataChunk.nDataSize = write_nSample * 2;

	if (!fwrite((char*)&riffChunk, sizeof(RIFFFormat), 1, fp))
	{
		return -2;
	}
	if (!fwrite((char*)&waveChunk, sizeof(WaveFormat), 1, fp))
	{
		return -3;
	}
	if (!fwrite((char*)&dataChunk, sizeof(DataFormat), 1, fp))
	{
		return -4;
	}
	int add_position = startTime * 2 * 8000;

	if (!fwrite((char*)pData+ add_position, write_nSample * 2, 1, fp))
	{
		return -5;
	}
	fclose(fp);



	return 0;
}