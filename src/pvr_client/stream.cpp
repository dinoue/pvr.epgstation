/*
 * Copyright (C) 2015-2020 Yuki MIZUNO
 * https://github.com/Harekaze/pvr.epgstation/
 * SPDX-License-Identifier: GPL-3.0-only
 */
#include <iostream>
#include "kodi/libKODI_guilib.h"
#include "kodi/libXBMC_addon.h"
#include "kodi/libXBMC_pvr.h"

extern ADDON::CHelper_libXBMC_addon *XBMC;
extern CHelper_libXBMC_pvr *PVR;

extern "C" {

bool CanPauseStream(void) {
	return true;
}

/* not implemented */
PVR_ERROR SignalStatus(PVR_SIGNAL_STATUS &signalStatus) { return PVR_ERROR_NOT_IMPLEMENTED; }
PVR_ERROR GetStreamProperties(PVR_STREAM_PROPERTIES *pProperties) { return PVR_ERROR_NOT_IMPLEMENTED; }
PVR_ERROR GetStreamReadChunkSize(int* chunksize) { return PVR_ERROR_NOT_IMPLEMENTED; };
void PauseStream(bool bPaused) {}
bool CanSeekStream(void) { return false; }
bool SeekTime(int time, bool backwards, double *startpts) { return false; }
void SetSpeed(int speed) {}

}
