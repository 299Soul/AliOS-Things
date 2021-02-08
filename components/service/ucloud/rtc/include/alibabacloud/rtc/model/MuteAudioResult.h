/*
 * Copyright 2009-2017 Alibaba Cloud All rights reserved.
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *      http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef ALIBABACLOUD_RTC_MODEL_MUTEAUDIORESULT_H_
#define ALIBABACLOUD_RTC_MODEL_MUTEAUDIORESULT_H_

#include <string>
#include <vector>
#include <utility>
#include <alibabacloud/core/ServiceResult.h>
#include <alibabacloud/rtc/RtcExport.h>

namespace AlibabaCloud
{
	namespace Rtc
	{
		namespace Model
		{
			class ALIBABACLOUD_RTC_EXPORT MuteAudioResult : public ServiceResult
			{
			public:
				struct Participant
				{
					std::string message;
					std::string id;
					std::string code;
				};


				MuteAudioResult();
				explicit MuteAudioResult(const std::string &payload);
				~MuteAudioResult();
				std::string getConferenceId()const;
				std::vector<Participant> getParticipants()const;

			protected:
				void parse(const std::string &payload);
			private:
				std::string conferenceId_;
				std::vector<Participant> participants_;

			};
		}
	}
}
#endif // !ALIBABACLOUD_RTC_MODEL_MUTEAUDIORESULT_H_