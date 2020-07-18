/*
 * Copyright (C) 2015-2020 Yuki MIZUNO
 * https://github.com/Harekaze/pvr.epgstation/
 * SPDX-License-Identifier: GPL-3.0-only
 */
#include "epgstation/epgstation.h"
#include "kodi/libKODI_guilib.h"
#include "kodi/libXBMC_addon.h"
#include "kodi/libXBMC_pvr.h"
#include <iostream>

extern epgstation::Schedule g_schedule;
extern ADDON::CHelper_libXBMC_addon* XBMC;
extern CHelper_libXBMC_pvr* PVR;

extern "C" {

int GetChannelsAmount(void)
{
    return g_schedule.schedule.size();
}

PVR_ERROR GetChannels(ADDON_HANDLE handle, bool bRadio)
{
    if (bRadio) {
        return PVR_ERROR_NO_ERROR;
    }

    if (!g_schedule.refresh()) {
        return PVR_ERROR_SERVER_ERROR;
    }

    for (const std::pair<std::string, std::vector<epgstation::channel>> schedule : g_schedule.channelGroups) {
        for (const epgstation::channel c : schedule.second) {
            PVR_CHANNEL ch;
            ch.iUniqueId = c.id;
            ch.bIsRadio = false;
            ch.bIsHidden = false;
            ch.iChannelNumber = c.remoteControlKeyId;
            ch.iSubChannelNumber = c.networkId;
            strncpy(ch.strChannelName, c.name.c_str(), PVR_ADDON_NAME_STRING_LENGTH - 1);

            if (c.hasLogoData) {
                snprintf(ch.strIconPath, PVR_ADDON_URL_STRING_LENGTH - 1,
                    (const char*)(epgstation::api::baseURL + g_schedule.channelLogoPath).c_str(),
                    c.id);
            } else {
                ch.strIconPath[0] = '\0';
            }
            PVR->TransferChannelEntry(handle, &ch);
        }
    }

    return PVR_ERROR_NO_ERROR;
}

int GetChannelGroupsAmount(void)
{
    return g_schedule.channelGroups.size();
}

PVR_ERROR GetChannelGroups(ADDON_HANDLE handle, bool bRadio)
{
    for (const std::pair<std::string, std::vector<epgstation::channel>> channelGroup : g_schedule.channelGroups) {
        PVR_CHANNEL_GROUP chGroup;
        memset(&chGroup, 0, sizeof(PVR_CHANNEL_GROUP));

        strncpy(chGroup.strGroupName, channelGroup.first.c_str(), PVR_ADDON_NAME_STRING_LENGTH - 1);
        chGroup.bIsRadio = false;
        // chGroup.iPosition = 0; /* not implemented */

        PVR->TransferChannelGroup(handle, &chGroup);
    }

    return PVR_ERROR_NO_ERROR;
}

PVR_ERROR GetChannelGroupMembers(ADDON_HANDLE handle, const PVR_CHANNEL_GROUP& group)
{
    for (const epgstation::channel channel : g_schedule.channelGroups[group.strGroupName]) {
        PVR_CHANNEL_GROUP_MEMBER chMem;
        memset(&chMem, 0, sizeof(PVR_CHANNEL_GROUP_MEMBER));

        chMem.iChannelUniqueId = channel.id;
        chMem.iChannelNumber = channel.remoteControlKeyId;
        strncpy(chMem.strGroupName, group.strGroupName, PVR_ADDON_NAME_STRING_LENGTH - 1);

        PVR->TransferChannelGroupMember(handle, &chMem);
    }

    return PVR_ERROR_NO_ERROR;
}

/* not implemented */
PVR_ERROR OpenDialogChannelScan(void) { return PVR_ERROR_NOT_IMPLEMENTED; }
PVR_ERROR DeleteChannel(const PVR_CHANNEL& channel) { return PVR_ERROR_NOT_IMPLEMENTED; }
PVR_ERROR RenameChannel(const PVR_CHANNEL& channel) { return PVR_ERROR_NOT_IMPLEMENTED; }
PVR_ERROR OpenDialogChannelSettings(const PVR_CHANNEL& channel) { return PVR_ERROR_NOT_IMPLEMENTED; }
PVR_ERROR OpenDialogChannelAdd(const PVR_CHANNEL& channel) { return PVR_ERROR_NOT_IMPLEMENTED; }
PVR_ERROR SetEPGTimeFrame(int iDays) { return PVR_ERROR_NOT_IMPLEMENTED; }
PVR_ERROR GetChannelStreamProperties(const PVR_CHANNEL* channel, PVR_NAMED_VALUE* properties, unsigned int* iPropertiesCount) { return PVR_ERROR_NOT_IMPLEMENTED; }
}
