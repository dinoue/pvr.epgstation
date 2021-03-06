/*
 * Copyright (C) 2015-2020 Yuki MIZUNO
 * https://github.com/Harekaze/pvr.epgstation/
 * SPDX-License-Identifier: GPL-3.0-only
 */
#include "epgstation/rules.h"
#include "epgstation/api.h"
#include "epgstation/recorded.h"
#include "epgstation/reserves.h"
#include "epgstation/schedule.h"
#include "epgstation/types.h"
#include "kodi/libXBMC_addon.h"
#include "json/json.hpp"
#include <algorithm>
#include <string>

extern ADDON::CHelper_libXBMC_addon* XBMC;

namespace epgstation {
bool Rule::refresh()
{
    nlohmann::json response;
    rules.clear();

    if (api::getRules(response) == api::REQUEST_FAILED) {
        return false;
    }

    std::copy(response["rules"].begin(), response["rules"].end(), std::back_inserter(rules));

    XBMC->Log(ADDON::LOG_NOTICE, "Updated rules: ammount = %d", rules.size());
    return true;
}

bool Rule::add(bool enabled, const std::string searchText, bool fullText, uint64_t channelId, unsigned int weekdays, unsigned int startHour, unsigned int endHour, bool anytime, const std::string directory)
{
    const auto success = api::postRules(enabled, searchText, fullText, channelId, weekdays, startHour, endHour, anytime, directory) != api::REQUEST_FAILED;
    XBMC->Log(success ? ADDON::LOG_NOTICE : ADDON::LOG_ERROR,
        "Rule creation: \"%s\"", searchText.c_str());
    return success;
}

bool Rule::edit(int id, bool enabled, const std::string searchText, bool fullText, uint64_t channelId, unsigned int weekdays, unsigned int startHour, unsigned int endHour, bool anytime, const std::string directory)
{
    const auto success = api::putRule(id, enabled, searchText, fullText, channelId, weekdays, startHour, endHour, anytime, directory) != api::REQUEST_FAILED;
    XBMC->Log(success ? ADDON::LOG_NOTICE : ADDON::LOG_ERROR,
        "Rule update: #%d", id);
    return success;
}

bool Rule::enable(int id, bool enabled)
{
    const auto success = api::putRuleAction(id, enabled) != api::REQUEST_FAILED;
    XBMC->Log(success ? ADDON::LOG_NOTICE : ADDON::LOG_ERROR,
        "Rule state change: %s #%d", enabled ? "enable" : "disable", id);
    return success;
}

bool Rule::remove(int id)
{
    const auto success = api::deleteRule(id) != api::REQUEST_FAILED;
    XBMC->Log(success ? ADDON::LOG_NOTICE : ADDON::LOG_ERROR, "Rule delete: #%d", id);
    return success;
}

} // namespace epgstation
