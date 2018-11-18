// merge.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"

#include <vector>
#include <iostream>
#include <fstream>
#include <codecvt>
#include <string>
#include <sstream>
#include <locale>

struct TextUnit {
	std::string waveId;
	float startTime;
	float durTime;
	float endTime;
	std::string sentence;

};

struct SpkdUnit {
	std::string  waveId;
	float startTime;
	float durTime;
	float endTime;
	int speakerId;
};

struct MeetingResult {
	std::string waveId;
	float startTime;
	float endTime;

	int speakerId;
	std::string sentence;
	
};


int readTextList(const char* filename, std::vector<TextUnit> *wordSymbol) {

	std::string lineStr;
	std::ifstream wordsFile(filename);

	while (std::getline(wordsFile, lineStr)) {
		TextUnit lineUnit;
		std::istringstream sin(lineStr);
		sin >> lineUnit.waveId >> lineUnit.startTime >> lineUnit.endTime >> lineUnit.sentence;
		wordSymbol->emplace_back(lineUnit);	
	}

	return 0;
}

int readSpkdList(const char* filename, std::vector<SpkdUnit> *spkdUnit) {
	std::string lineStr;
	std::ifstream SpkdFile(filename);

	while (std::getline(SpkdFile, lineStr)) {
		SpkdUnit lineUnit;
		std::istringstream sin(lineStr);
		sin >> lineUnit.waveId >> lineUnit.startTime >> lineUnit.durTime >>lineUnit.speakerId ;
		spkdUnit->emplace_back(lineUnit);
	}

	return 0;
}

int smoothMeetingResult(std::vector<TextUnit> *textList, std::vector<SpkdUnit> *spkdList , std::vector<MeetingResult> *meetingList) {
	//compute durTime or end time
	for (int i = 0; i < textList->size(); i++) {
		(*textList)[i].durTime = (*textList)[i].endTime - (*textList)[i].startTime;
	}
	for (int i = 0; i < spkdList->size(); i++) {
		(*spkdList)[i].endTime = (*spkdList)[i].startTime + (*spkdList)[i].durTime;
	}

	//method 1 asr 开始的时间点 为spkID
	for (int i = 0; i < textList->size(); i++) {
		TextUnit textUnit = (*textList)[i];
		MeetingResult meetingResult;
		meetingResult.waveId = textUnit.waveId;
		meetingResult.startTime = textUnit.startTime;
		meetingResult.endTime = textUnit.endTime;
		meetingResult.sentence = textUnit.sentence;
		meetingResult.speakerId = 0;
		meetingList->emplace_back(meetingResult);
	}

	for (int i = 0; i < meetingList->size(); i++) {
		MeetingResult &meetingResult=(*meetingList)[i];
		//compute spkId
		//TODO ADD SOME RULE
		float startTimeInText = meetingResult.startTime;
		float endTimeInText = meetingResult.endTime;

		for (int j = 0; j<spkdList->size(); j++) {
			SpkdUnit spkdUnit = (*spkdList)[j];
			//给第1段赋初值
			if (endTimeInText<(*spkdList)[0].startTime) {
				meetingResult.speakerId = (*spkdList)[0].speakerId;
				break;
			}

			if (startTimeInText < spkdUnit.endTime && endTimeInText > spkdUnit.startTime) {
					meetingResult.speakerId = spkdUnit.speakerId;
					break;
				}

			//if (startTimeInText <= spkdUnit.endTime && endTimeInText > spkdUnit.startTime)//按照结束时间算，因为spkd时间未覆盖全
			//	meetingResult.speakerId = spkdUnit.speakerId;
		}


	}

	return 0;
}

int outputMeetingResult(std::vector<MeetingResult> *meetingList, const char* filename) {
	setlocale(LC_ALL, "chs");
	//FILE *f_res = fopen(filename, "w");
	//std::locale &loc = std::locale::global(std::locale(std::locale(), "", LC_CTYPE));
	//std::locale::global(loc);
	std::ofstream of(filename);
	for (int i = 0; i < meetingList->size(); i++){
		MeetingResult meetingResult = (*meetingList)[i];
		std::string sen = meetingResult.sentence;
		of << sen;
		//fprintf(f_res, "%s %.2f %.2f  %d ", meetingResult.waveId.c_str() , meetingResult.startTime, meetingResult.endTime,  meetingResult.speakerId);
		//std::wstring wtemp;
		//std::wstring_convert< std::codecvt_utf8<wchar_t> > strCnv;
		//	wtemp = strCnv.from_bytes(meetingResult.sentence);
	    //std::wcout << wtemp << std::endl;
		//fwprintf(f_res, L"%s\n", wtemp.c_str() );
		//fwprintf(f_res, L"%s \n",  meetingResult.sentence.c_str());
	}
	of.close();
	//fclose(f_res);
	return 0;
}


int main()
{

	std::vector<TextUnit> textResult; 
	std::vector<SpkdUnit> spkdResult;
	std::vector<MeetingResult> meetingResult;
	readTextList("../res/talk3_8k.text", &textResult);
	readSpkdList("../res/talk3_8k.spkd", &spkdResult);


	smoothMeetingResult(&textResult,&spkdResult,&meetingResult);

	outputMeetingResult(&meetingResult,"../res/talk_8k.meetingResult");


    return 0;
}

