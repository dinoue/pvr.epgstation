/*
 * Copyright (C) 2015-2020 Yuki MIZUNO
 * https://github.com/Harekaze/pvr.epgstation/
 * SPDX-License-Identifier: GPL-3.0-only
 */
#include "recorded.h"
#include "api.h"
#include "kodi/libXBMC_addon.h"
#include "schedule.h"

extern ADDON::CHelper_libXBMC_addon* XBMC;

namespace epgstation {
bool Recorded::refresh()
{
    picojson::value response;

    if (epgstation::api::getRecorded(response) == epgstation::api::REQUEST_FAILED) {
        return false;
    }

    const bool showThumbnail = !recordedThumbnailPath.empty();
    programs.clear();

    picojson::object& root = response.get<picojson::object>();

    for (picojson::value& a : root["recorded"].get<picojson::array>()) {
        picojson::object& p = a.get<picojson::object>();
        PVR_RECORDING rec;
        char* endptr;

        strncpy(rec.strRecordingId, p["id"].is<double>() ? std::to_string((unsigned int)(p["id"].get<double>())).c_str() : "", PVR_ADDON_NAME_STRING_LENGTH - 1);
        strncpy(rec.strDirectory, "", PVR_ADDON_URL_STRING_LENGTH - 1); // FIXME: Specify a name of series
        strncpy(rec.strTitle, p["name"].is<std::string>() ? p["name"].get<std::string>().c_str() : "", PVR_ADDON_NAME_STRING_LENGTH - 1);
        strncpy(rec.strEpisodeName, "", PVR_ADDON_NAME_STRING_LENGTH - 1); // FIXME: Specify a name of episode
        strncpy(rec.strPlotOutline, p["description"].is<std::string>() ? p["description"].get<std::string>().c_str() : rec.strEpisodeName, PVR_ADDON_DESC_STRING_LENGTH - 1);
        strncpy(rec.strPlot, p["extended"].is<std::string>() ? p["extended"].get<std::string>().c_str() : "", PVR_ADDON_DESC_STRING_LENGTH - 1);
        strncpy(rec.strChannelName, "", PVR_ADDON_NAME_STRING_LENGTH - 1); // FIXME: Get channel name from channel API
        rec.iEpisodeNumber = p["episode"].is<double>() ? (unsigned int)(p["episode"].get<double>()) : 0;
        rec.recordingTime = (time_t)(p["startAt"].get<double>() / 1000);
        rec.iDuration = (int)((p["endAt"].get<double>() - p["startAt"].get<double>()) / 1000);
        rec.iPriority = 0;
        rec.iGenreType = p["genre1"].is<double>() ? (int)(p["genre1"].get<double>()) : 0;
        rec.iGenreSubType = p["genre2"].is<double>() ? (int)(p["genre2"].get<double>()) : 0;
        rec.iEpgEventId = p["programId"].is<double>() ? (unsigned int)(p["programId"].get<double>()) : 0;
        rec.channelType = PVR_RECORDING_CHANNEL_TYPE_TV;
        rec.bIsDeleted = false;
        rec.iChannelUid = p["channelId"].is<double>() ? (int)(p["channelId"].get<double>()) : 0;
        if (showThumbnail && p["hasThumbnail"].is<bool>() && p["hasThumbnail"].get<bool>()) {
            snprintf(rec.strThumbnailPath, PVR_ADDON_URL_STRING_LENGTH - 1, (const char*)(epgstation::api::baseURL + recordedThumbnailPath).c_str(), rec.strRecordingId);
        } else {
            strncpy(rec.strThumbnailPath, "", PVR_ADDON_URL_STRING_LENGTH - 1);
        }

        programs.push_back(rec);
    }

    XBMC->Log(ADDON::LOG_NOTICE, "Updated recorded program: ammount = %d", programs.size());

    return true;
}
}
