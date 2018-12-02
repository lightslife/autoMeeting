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
	float durTime;
	std::vector<float> sumProb;
	int validaFrame;
	int speakerId;
	std::string sentence;
	
};

struct SpkdFrameUnit{
	std::string waveId;
	int frameId;
	int valid_frame;
	std::vector<float> personProb;
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

int readSpkdFrameList(const char* filename, std::vector<SpkdFrameUnit> *spkdFrameUnit) {
	std::string lineStr;
	std::ifstream SpkdFile(filename);

	while (std::getline(SpkdFile, lineStr)) {
		SpkdFrameUnit lineUnit;
		lineUnit.valid_frame = 0;
		std::istringstream sin(lineStr);
		sin /*>> lineUnit.waveId*/ >> lineUnit.frameId;
		float prob = 0.0;
		while (sin >> prob) {
			if (prob != 0.0)
				lineUnit.valid_frame=1;

			lineUnit.personProb.push_back(prob);
		}
		spkdFrameUnit->emplace_back(lineUnit);
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



int mergeAsrSpkdFrameMeetingResult(std::vector<TextUnit> *textList, std::vector<SpkdFrameUnit> *spkdFrameList, std::vector<MeetingUnit> *meetingList) {
	if (spkdFrameList->size() < 1) {
		return -1;
	}

	int speakerNum = (*spkdFrameList)[0].personProb.size();

	//compute durTime or end time
	for (int i = 0; i < textList->size(); i++) {
		(*textList)[i].durTime = (*textList)[i].endTime - (*textList)[i].startTime;
	}

	//method 1 asr 开始的时间点 为spkID
	for (int i = 0; i < textList->size(); i++) {
		TextUnit textUnit = (*textList)[i];
		MeetingUnit meetingResult;
		meetingResult.waveId = textUnit.waveId;
		meetingResult.startTime = textUnit.startTime;
		meetingResult.endTime = textUnit.endTime;
		meetingResult.sentence = textUnit.sentence;
		meetingResult.durTime = textUnit.endTime - textUnit.startTime;

		meetingResult.speakerId = 0;
		meetingResult.validaFrame = 0;
		meetingList->emplace_back(meetingResult);
	}

	for (int i = 0; i < meetingList->size(); i++) {
		//累加每个用户的概率
		MeetingUnit &meetingResult = (*meetingList)[i];
		float startTime = meetingResult.startTime;
		float endTime = meetingResult.endTime;

		int startFrame = startTime / 0.01 ;
		int endFrame = endTime / 0.01;

		//check spkdFrame size
		if (endFrame > spkdFrameList->size()) {
			printf("something wrong\n");
			return -1;
		}

		//init sumProb
		std::vector<float> &sumProb = meetingResult.sumProb;
		sumProb.resize(speakerNum);
		for (int k = 0; k < sumProb.size(); k++) {
			sumProb[k] = 0.0;
		}


		for (int j = startFrame; j < endFrame; j++) {
			meetingResult.validaFrame += ((*spkdFrameList)[j]).valid_frame;
			for (int k = 0; k < sumProb.size(); k++) {
				sumProb[k] += ((*spkdFrameList)[j]).personProb[k];
			}
		}

		float maxProb = 0;
		int max_speaker = -1;
		for (int k = 0; k < sumProb.size(); k++) {
			if (maxProb < sumProb[k]) {
				max_speaker = k + 1;//speake frome 1 
				maxProb = sumProb[k];
			}

		}
		meetingResult.speakerId = max_speaker;

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
		of << meetingResult.waveId << " " << meetingResult.startTime << " " << meetingResult.endTime << "    "<< meetingResult.durTime<<" "<<meetingResult.validaFrame<< " ";
		for (int i = 0; i < meetingResult.sumProb.size(); i++) {
			of <<"\t"<<meetingResult.sumProb[i]/meetingResult.validaFrame << " \t";
		}
			
			of<< meetingResult.speakerId<< " " << meetingResult.sentence<<"\n";
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

	std::vector<SpkdFrameUnit> spkdFrameResult;
	readSpkdFrameList("../res/talk3_8k.post", &spkdFrameResult);





	//mergeAsrSpkdMeetingResult(&textResult,&spkdResult,&meetingResult);

	mergeAsrSpkdFrameMeetingResult(&textResult, &spkdFrameResult, &meetingResult);
	AddPuncMeetingResult(&meetingResult);

	outputMeetingResult(&meetingResult,"../res/talk3_8k.meetingFrame");




    return 0;
}

