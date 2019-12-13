//  Copyright (C) 2015-2019 Virgil Security, Inc.
//
//  All rights reserved.
//
//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted provided that the following conditions are
//  met:
//
//      (1) Redistributions of source code must retain the above copyright
//      notice, this list of conditions and the following disclaimer.
//
//      (2) Redistributions in binary form must reproduce the above copyright
//      notice, this list of conditions and the following disclaimer in
//      the documentation and/or other materials provided with the
//      distribution.
//
//      (3) Neither the name of the copyright holder nor the names of its
//      contributors may be used to endorse or promote products derived from
//      this software without specific prior written permission.
//
//  THIS SOFTWARE IS PROVIDED BY THE AUTHOR ''AS IS'' AND ANY EXPRESS OR
//  IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
//  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
//  DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT,
//  INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
//  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
//  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
//  HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
//  STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
//  IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
//  POSSIBILITY OF SUCH DAMAGE.
//
//  Lead Maintainer: Virgil Security Inc. <support@virgilsecurity.com>

#include <VSQIoTKit.h>
#include <VSQSnapINFOClient.h>

using namespace VirgilIoTKit;

bool
VSQDeviceInfo::equal(const VSQDeviceInfo &deviceInfo) const {
    return m_mac == deviceInfo.m_mac && m_deviceRoles == deviceInfo.m_deviceRoles &&
           m_manufactureId == deviceInfo.m_manufactureId && m_deviceType == deviceInfo.m_deviceType &&
           m_fwVer == deviceInfo.m_fwVer && m_tlVer == deviceInfo.m_tlVer && m_sent == deviceInfo.m_sent &&
           m_received == deviceInfo.m_received && m_lastTimestamp == deviceInfo.m_lastTimestamp;
}

VSQSnapInfoClient::VSQSnapInfoClient() {
    m_snapInfoImpl.device_start = startNotify;
    m_snapInfoImpl.general_info = generalInfo;
    m_snapInfoImpl.statistics = statistics;

    m_snapService = vs_snap_info_client(m_snapInfoImpl);

    constexpr auto deadDevicesCheckMSec = 1000;
    m_deviceAliveTimer = startTimer(deadDevicesCheckMSec, Qt::VeryCoarseTimer);
    if (!m_deviceAliveTimer) {
        VS_LOG_WARNING("Unable to start timer for alive device check for INFO Client");
    }
}

/* static */ vs_status_e
VSQSnapInfoClient::startNotify(vs_snap_info_device_t *deviceRaw) {
    VSQDeviceInfo &device = instance().getDevice(deviceRaw->mac);

    device.m_hasGeneralInfo = false;
    device.m_hasStatistics = false;
    device.m_isActive = true;
    device.m_lastTimestamp = QDateTime::currentDateTime();

    emit instance().fireNewDevice(device);
    emit instance().fireDeviceInfo(device);

    if (!instance().startFullPolling(device.m_mac)) {
        VS_LOG_CRITICAL("Unable to start polling for device %s", device.m_mac.description().toStdString().c_str());
        return VS_CODE_ERR_POLLING_INFO_CLIENT;
    }
    return VS_CODE_OK;
}

/* static */ vs_status_e
VSQSnapInfoClient::generalInfo(vs_info_general_t *generalData) {
    VSQDeviceInfo &device = instance().getDevice(generalData->default_netif_mac);

    device.m_manufactureId = generalData->manufacture_id;
    device.m_deviceType = generalData->device_type;
    device.m_deviceRoles = generalData->device_roles;
    device.m_fwVer = generalData->fw_ver;
    device.m_tlVer = generalData->tl_ver;

    device.m_isActive = true;
    device.m_hasGeneralInfo = true;
    device.m_lastTimestamp = QDateTime::currentDateTime();

    emit instance().fireDeviceInfo(device);

    return VS_CODE_OK;
}

/* static */ vs_status_e
VSQSnapInfoClient::statistics(vs_info_statistics_t *statistics) {
    VSQDeviceInfo &device = instance().getDevice(statistics->default_netif_mac);

    device.m_sent = statistics->sent;
    device.m_received = statistics->received;

    device.m_isActive = true;
    device.m_hasStatistics = true;
    device.m_lastTimestamp = QDateTime::currentDateTime();

    emit instance().fireDeviceInfo(device);

    return VS_CODE_OK;
}

bool
VSQSnapInfoClient::changePolling(std::initializer_list<EPolling> pollingOptions,
                                 const VSQMac &deviceMac,
                                 bool enable,
                                 uint16_t periodSeconds) const {
    vs_mac_addr_t mac = deviceMac;
    uint32_t pollingElements = 0;

    for (auto pollingOption : pollingOptions) {
        pollingElements |= pollingOption;
    }

    if (vs_snap_info_set_polling(netif(), &mac, pollingElements, enable, periodSeconds) != VS_CODE_OK) {
        VS_LOG_ERROR("Unable to setup info polling");
        return false;
    }

    return true;
}

VSQDeviceInfo &
VSQSnapInfoClient::getDevice(const VSQMac &mac) {
    VSQDeviceInfo *device = nullptr;

    for (auto &curDevice : m_devicesInfo) {
        if (curDevice.m_mac == mac) {
            device = &curDevice;
            break;
        }
    }

    if (!device) {
        m_devicesInfo.push_back(VSQDeviceInfo(mac));
        device = &m_devicesInfo.last();
    }

    return *device;
}

void
VSQSnapInfoClient::timerEvent(QTimerEvent *event) {
    if (event->timerId() == m_deviceAliveTimer) {
        auto currentTime = QDateTime::currentDateTime();

        for (auto &device : m_devicesInfo) {
            constexpr auto deadDelayMSec = 5000;
            if (device.m_lastTimestamp.msecsTo(currentTime) > deadDelayMSec) {
                device.m_isActive = false;
                emit fireDeviceInfo(device);
            }
        }
    }
}