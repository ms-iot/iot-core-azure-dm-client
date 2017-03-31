/*
Copyright 2017 Microsoft
Permission is hereby granted, free of charge, to any person obtaining a copy of this software 
and associated documentation files (the "Software"), to deal in the Software without restriction, 
including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, 
and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, 
subject to the following conditions:

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT 
LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, 
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH 
THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
#pragma once

#include <string>

class PolicyCSP
{
public:
    static unsigned int GetActiveHoursStart();
    static void SetActiveHoursStart(unsigned int hours);

    static unsigned int GetActiveHoursEnd();
    static void SetActiveHoursEnd(unsigned int hours);

    static unsigned int GetAllowAutoUpdate();
    static void SetAllowAutoUpdate(unsigned int allowLevel);

    static unsigned int GetAllowMUUpdateService();
    static void SetAllowMUUpdateService(unsigned int allowLevel);

    static unsigned int GetAllowNonMicrosoftSignedUpdate();
    static void SetAllowNonMicrosoftSignedUpdate(unsigned int allowLevel);

    static unsigned int GetAllowUpdateService();
    static void SetAllowUpdateService(unsigned int allowLevel);

    static unsigned int GetBranchReadinessLevel();
    static void SetBranchReadinessLevel(unsigned int level);

    static unsigned int GetDeferFeatureUpdatesPeriodInDays();
    static void SetDeferFeatureUpdatesPeriodInDays(unsigned int days);

    static unsigned int GetDeferQualityUpdatesPeriodInDays();
    static void SetDeferQualityUpdatesPeriodInDays(unsigned int days);

    static unsigned int GetExcludeWUDrivers();
    static void SetExcludeWUDrivers(unsigned int excludeLevel);

    static unsigned int GetPauseFeatureUpdates();
    static void SetPauseFeatureUpdates(unsigned int pauseLevel);

    static unsigned int GetPauseQualityUpdates();
    static void SetPauseQualityUpdates(unsigned int pauseLevel);

    static unsigned int GetRequireUpdateApproval();
    static void SetRequireUpdateApproval(unsigned int value);

    static unsigned int GetScheduledInstallDay();
    static void SetScheduledInstallDay(unsigned int dayOfWeek);

    static unsigned int GetScheduledInstallTime();
    static void SetScheduledInstallTime(unsigned int hour);

    static std::wstring GetUpdateServiceUrl();
    static void SetUpdateServiceUrl(const std::wstring& serviceUrl);
};
