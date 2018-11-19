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

struct MeetingUnit {
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

int mergeAsrSpkdMeetingResult(std::vector<TextUnit> *textList, std::vector<SpkdUnit> *spkdList , std::vector<MeetingUnit> *meetingList) {
	int speakerNum = 0;
	//compute durTime or end time
	for (int i = 0; i < textList->size(); i++) {
		(*textList)[i].durTime = (*textList)[i].endTime - (*textList)[i].startTime;
	}
	for (int i = 0; i < spkdList->size(); i++) {
		(*spkdList)[i].endTime = (*spkdList)[i].startTime + (*spkdList)[i].durTime;
		if ((*spkdList)[i].speakerId > speakerNum) {
			speakerNum = (*spkdList)[i].speakerId;
		}
	}

	//method 1 asr 开始的时间点 为spkID
	for (int i = 0; i < textList->size(); i++) {
		TextUnit textUnit = (*textList)[i];
		MeetingUnit meetingResult;
		meetingResult.waveId = textUnit.waveId;
		meetingResult.startTime = textUnit.startTime;
		meetingResult.endTime = textUnit.endTime;
		meetingResult.sentence = textUnit.sentence;
		meetingResult.speakerId = 0;
		meetingList->emplace_back(meetingResult);
	}

	for (int i = 0; i < meetingList->size(); i++) {
		MeetingUnit &meetingResult=(*meetingList)[i];
		//compute spkId
		//TODO ADD SOME RULE
		float startTimeInText = meetingResult.startTime;
		float endTimeInText = meetingResult.endTime;

		std::vector<float>speakerStatSum(speakerNum, 0.0);

		for (int j = 0; j<spkdList->size(); j++) {
			SpkdUnit spkdUnit = (*spkdList)[j];

			if (spkdUnit.startTime > startTimeInText && spkdUnit.endTime < endTimeInText) {
				speakerStatSum[spkdUnit.speakerId - 1] += spkdUnit.endTime - spkdUnit.startTime;
			}

			if (spkdUnit.startTime < startTimeInText && spkdUnit.endTime > endTimeInText) {
				speakerStatSum[spkdUnit.speakerId - 1] += endTimeInText- startTimeInText;
			}

			if (spkdUnit.startTime < startTimeInText && spkdUnit.endTime > startTimeInText && spkdUnit.endTime <endTimeInText) {
				speakerStatSum[spkdUnit.speakerId - 1] += spkdUnit.endTime - startTimeInText;
			}

			if (spkdUnit.startTime > startTimeInText && spkdUnit.endTime > endTimeInText && spkdUnit.startTime <endTimeInText) {
				speakerStatSum[spkdUnit.speakerId - 1] += endTimeInText-spkdUnit.startTime;
			}
		}

		float maxDurTime = 0.0;
		int maxDurSpeakerId = 0;
		for (int k = 0; k < speakerStatSum.size(); k++) {
			if (maxDurTime < speakerStatSum[k]) {
				maxDurTime = speakerStatSum[k];
				maxDurSpeakerId = k+1;
			}
			
		}
		meetingResult.speakerId = maxDurSpeakerId;
	}

	return 0;
}

int outputMeetingResult(std::vector<MeetingUnit> *meetingList, const char* filename) {
	setlocale(LC_ALL, "chs");
	std::ofstream of(filename);
	of.setf(std::ios::fixed, std::ios::floatfield);
	of.precision(2);
	of << "语音文件名 开始时间 结束时间 说话人ID 说话内容文本" << '\n';
	for (int i = 0; i < meetingList->size(); i++){
		MeetingUnit meetingResult = (*meetingList)[i];
	/*	std::string sen = meetingResult.sentence;*/
		of << meetingResult.waveId << " "<< meetingResult.startTime<<" "<< meetingResult.endTime<< " " \
			<< meetingResult.speakerId<< " " << meetingResult.sentence<<"\n";
	}
	of.close();
	return 0;
}

int AddPuncMeetingResult(std::vector<MeetingUnit> *meetingList) {

	for (int i = 0; i < meetingList->size(); i++) {
		std::string &sentence = (*meetingList)[i].sentence;

		std::string ask1 = "吗";
		std::string ask2 = "呢";

		std::string sigh1 = "啊";
		std::string sigh2 = "吧";
		std::string sigh3 = "呀";

		std::string temp = sentence.substr(sentence.size() - 2, sentence.size() - 1);
		if (temp == ask1 || temp ==ask2) {
			sentence += "？";
		}
		else if (temp == sigh1 || temp==sigh2 | temp==sigh3 ) {
			sentence += "！";
		}
		else {
			//sentence += "。";
		}


	}
	return 0;
}
int main()
{

	std::vector<TextUnit> textResult; 
	std::vector<SpkdUnit> spkdResult;
	std::vector<MeetingUnit> meetingResult;
	readTextList("../res/talk3_8k.text", &textResult);
	readSpkdList("../res/talk3_8k.spkd", &spkdResult);


	mergeAsrSpkdMeetingResult(&textResult,&spkdResult,&meetingResult);

	AddPuncMeetingResult(&meetingResult);

	outputMeetingResult(&meetingResult,"../res/talk_8k.meeting");




    return 0;
}

