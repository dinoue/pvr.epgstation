/*
 *         Copyright (C) 2015-2016 Yuki MIZUNO
 *         https://github.com/mzyy94/pvr.chinachu/
 *
 *
 * This file is part of pvr.chinachu.
 *
 * pvr.chinachu is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * pvr.chinachu is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with pvr.chinachu.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
#include <iostream>
#include "xbmc/libKODI_guilib.h"
#include "xbmc/libXBMC_addon.h"
#include "xbmc/libXBMC_pvr.h"
#include "chinachu/chinachu.h"

extern ADDON::CHelper_libXBMC_addon *XBMC;
extern CHelper_libXBMC_pvr *PVR;
extern chinachu::Recorded g_recorded;
extern chinachu::Schedule g_schedule;
chinachu::Reserve g_reserve;

using namespace ADDON;

extern "C" {

int GetTimersAmount(void) {
	g_reserve.refreshIfNeeded();
	return g_reserve.reserves.size();
}

PVR_ERROR GetTimers(ADDON_HANDLE handle) {
	if (g_reserve.refreshIfNeeded()) {

		time_t now;
		time(&now);
		for (unsigned int i = 0, lim = g_reserve.reserves.size(); i < lim; i++) {
			chinachu::RESERVE_ITEM resv = g_reserve.reserves[i];

			PVR_TIMER timer;
			memset(&timer, 0, sizeof(PVR_TIMER));

			timer.iClientIndex = i;
			timer.startTime = resv.startTime;
			timer.endTime = resv.endTime;
			if (resv.state == PVR_TIMER_STATE_SCHEDULED) {
				if (now < resv.startTime) {
					timer.state = PVR_TIMER_STATE_SCHEDULED;
				} else if (resv.startTime < now && now < resv.endTime) {
					timer.state = PVR_TIMER_STATE_RECORDING;
				} else {
					timer.state = PVR_TIMER_STATE_COMPLETED;
				}
			} else {
				timer.state = resv.state;
			}
			strncpy(timer.strTitle, resv.strTitle.c_str(), PVR_ADDON_NAME_STRING_LENGTH - 1);
			strncpy(timer.strSummary, resv.strSummary.c_str(), PVR_ADDON_DESC_STRING_LENGTH - 1);
			timer.iGenreType = resv.iGenreType;
			timer.iGenreSubType = resv.iGenreSubType;
			timer.iClientChannelUid = resv.iClientChannelUid;
			timer.bIsRepeating = resv.bIsRepeating;
			// strncpy(timer.strDirectory, "Directory", PVR_ADDON_URL_STRING_LENGTH - 1); /* not implemented */
			// timer.iPriority = 100; /* not implemented */
			// timer.iLifetime = 0; /* not implemented */
			// timer.firstDay = 0; /* not implemented */
			// timer.iWeekdays = 0; /* not implemented */
			// timer.iEpgUid = 0; /* not implemented */
			// timer.iMarginStart = 0; /* not implemented */
			// timer.iMarginEnd = 0; /* not implemented */

			PVR->TransferTimerEntry(handle, &timer);
		}

		return PVR_ERROR_NO_ERROR;
	}

	return PVR_ERROR_SERVER_ERROR;
}

PVR_ERROR UpdateTimer(const PVR_TIMER &timer) {
	for (std::vector<chinachu::RESERVE_ITEM>::iterator itr = g_reserve.reserves.begin(); itr != g_reserve.reserves.end(); itr++) {
		if ((*itr).strTitle == (std::string)timer.strTitle && (*itr).iClientChannelUid == timer.iClientChannelUid &&
				(*itr).startTime == timer.startTime && (*itr).endTime == timer.endTime) {

			chinachu::RESERVE_ITEM resv = *itr;

			// Only reserving state changing is supported
			if (timer.state != resv.state) {
				switch (timer.state) {
					case PVR_TIMER_STATE_SCHEDULED:
						chinachu::api::putReservesUnskip(resv.strProgramId);
						XBMC->Log(LOG_NOTICE, "Unskip reserving: %s", resv.strProgramId.c_str());
						g_reserve.reserves[timer.iClientIndex].state = timer.state;
						break;
					case PVR_TIMER_STATE_CANCELLED:
						chinachu::api::putReservesSkip(resv.strProgramId);
						XBMC->Log(LOG_NOTICE, "Skip reserving: %s", resv.strProgramId.c_str());
						g_reserve.reserves[timer.iClientIndex].state = timer.state;
						break;
						break;
					default:
						XBMC->Log(LOG_ERROR, "Unknown state change: %s", resv.strProgramId.c_str());
						return PVR_ERROR_NOT_IMPLEMENTED;
				}

				return PVR_ERROR_NO_ERROR;
			}

			XBMC->Log(LOG_ERROR, "Only state change is supported: %s", resv.strProgramId.c_str());
			return PVR_ERROR_NOT_IMPLEMENTED;
		}
	}

	XBMC->Log(LOG_ERROR, "Only state change is supported");
	return PVR_ERROR_NOT_IMPLEMENTED;
}

PVR_ERROR AddTimer(const PVR_TIMER &timer) {
	for (std::vector<chinachu::CHANNEL_EPG>::iterator channel = g_schedule.schedule.begin(); channel != g_schedule.schedule.end(); channel++) {
		if ((*channel).channel.iUniqueId == timer.iClientChannelUid) {
			for (std::vector<chinachu::EPG_PROGRAM>::iterator program = (*channel).epgs.begin(); program != (*channel).epgs.end(); program++) {
				if ((*program).startTime == timer.startTime && (*program).endTime == timer.endTime) {
					if (chinachu::api::putProgram((*program).strUniqueBroadcastId) != -1) {
						XBMC->Log(LOG_NOTICE, "Reserved new program: %s", (*program).strUniqueBroadcastId.c_str());
						time_t now;
						time(&now);
						g_reserve.nextUpdateTime = now + 10; // refresh reserved programs after 10 sec.
						return PVR_ERROR_NO_ERROR;
					} else {
						XBMC->Log(LOG_ERROR, "Failed to reserve new program: %s", (*program).strUniqueBroadcastId.c_str());
						return PVR_ERROR_SERVER_ERROR;
					}
				}
			}
			break;
		}
	}

	XBMC->Log(LOG_ERROR, "Failed to reserve new program: nothing matched");
	return PVR_ERROR_FAILED;
}

PVR_ERROR DeleteTimer(const PVR_TIMER &timer, bool bForceDelete) {
	if (!timer.bIsRepeating) { // manual reserved
		for (std::vector<chinachu::CHANNEL_EPG>::iterator channel = g_schedule.schedule.begin(); channel != g_schedule.schedule.end(); channel++) {
			if ((*channel).channel.iUniqueId == timer.iClientChannelUid) {
				for (std::vector<chinachu::EPG_PROGRAM>::iterator program = (*channel).epgs.begin(); program != (*channel).epgs.end(); program++) {
					if ((*program).startTime == timer.startTime && (*program).endTime == timer.endTime) {
						if (chinachu::api::deleteReserves((*program).strUniqueBroadcastId) != -1) {
							XBMC->Log(LOG_NOTICE, "Delete manual reserved program: %s", (*program).strUniqueBroadcastId.c_str());
							time_t now;
							time(&now);
							g_reserve.nextUpdateTime = now; // refresh reserved programs immediately.
							return PVR_ERROR_NO_ERROR;
						} else {
							XBMC->Log(LOG_ERROR, "Failed to delete reserved program: %s", (*program).strUniqueBroadcastId.c_str());
							return PVR_ERROR_SERVER_ERROR;
						}
					}
				}
				break;
			}
		}

		XBMC->Log(LOG_ERROR, "Failed to delete reserved program: nothing matched");
		return PVR_ERROR_FAILED;
	} else {
		XBMC->Log(LOG_ERROR, "Only manual reserved program deletion is supported");
		return PVR_ERROR_NOT_IMPLEMENTED;
	}
}

}
