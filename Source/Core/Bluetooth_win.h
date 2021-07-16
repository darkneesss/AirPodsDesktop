//
// AirPodsDesktop - AirPods Desktop User Experience Enhancement Program.
// Copyright (C) 2021 SpriteOvO
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//

#pragma once

#include <Config.h>

#if !defined APD_OS_WIN
    #error "This file shouldn't be compiled."
#endif

#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.Storage.Streams.h>
#include <winrt/Windows.Devices.Enumeration.h>
#include <winrt/Windows.Devices.Bluetooth.Advertisement.h>
#include <winrt/Windows.Networking.h>

#include "Bluetooth_abstract.h"

namespace Core::Bluetooth {

namespace WinrtFoundation = winrt::Windows::Foundation;
namespace WinrtBlutooth = winrt::Windows::Devices::Bluetooth;
namespace WinrtBlutoothAdv = winrt::Windows::Devices::Bluetooth::Advertisement;
namespace WinrtDevicesEnumeration = winrt::Windows::Devices::Enumeration;

bool Initialize();

class Device final : public Details::DeviceAbstract<uint64_t>
{
public:
    Device(WinrtBlutooth::BluetoothDevice device);
    Device(const Device &rhs);
    Device(Device &&rhs) noexcept;
    ~Device();

    Device &operator=(const Device &rhs);
    Device &operator=(Device &&rhs) noexcept;

    uint64_t GetAddress() const override;
    std::string GetDisplayName() const override;
    uint16_t GetVendorId() const override;
    uint16_t GetProductId() const override;
    DeviceState GetConnectionState() const override;

private:
    constexpr static auto kPropertyBluetoothVendorId = L"System.DeviceInterface.Bluetooth.VendorId";
    constexpr static auto kPropertyBluetoothProductId =
        L"System.DeviceInterface.Bluetooth.ProductId";
    constexpr static auto kPropertyAepContainerId = L"System.Devices.Aep.ContainerId";

    std::optional<WinrtBlutooth::BluetoothDevice> _device;
    mutable std::optional<WinrtDevicesEnumeration::DeviceInformation> _info;
    winrt::event_token _tokenConnectionStatusChanged, _tokenNameChanged;

    void RegisterHandlers();
    void UnregisterHandlers();
    void CopyFrom(const Device &rhs);
    void MoveFrom(Device &&rhs) noexcept;

    const std::optional<WinrtDevicesEnumeration::DeviceInformation> &GetInfo() const;

    template <class T>
    inline T GetProperty(const winrt::hstring &name, const T &defaultValue) const
    {
        try {
            const auto &optInfo = GetInfo();
            if (!optInfo.has_value()) {
                SPDLOG_WARN("optInfo.has_value() false.");
                return defaultValue;
            }

            const auto boxed = optInfo->Properties().TryLookup(name);
            return winrt::unbox_value_or<T>(boxed, defaultValue);
        }
        catch (const OS::Windows::Winrt::Exception &ex) {
            SPDLOG_WARN("GetProperty() failed. {}", Helper::ToString(ex));
        }
        return defaultValue;
    }

    winrt::hstring GetAepId() const;

    void OnConnectionStatusChanged(const WinrtBlutooth::BluetoothDevice &sender);
    void OnNameChanged(const WinrtBlutooth::BluetoothDevice &sender);
};

namespace DeviceManager {

std::vector<Device> GetDevicesByState(DeviceState state);
std::optional<Device> FindDevice(uint64_t address);

} // namespace DeviceManager

class AdvertisementWatcher final
    : public Details::AdvertisementWatcherAbstract<AdvertisementWatcher>
{
public:
    using Timestamp = winrt::Windows::Foundation::DateTime;

    explicit AdvertisementWatcher();

    Status Start() override;
    Status Stop() override;

private:
    WinrtBlutoothAdv::BluetoothLEAdvertisementWatcher _bleWatcher;

    void OnReceived(const WinrtBlutoothAdv::BluetoothLEAdvertisementReceivedEventArgs &args);
    void OnStopped(const WinrtBlutoothAdv::BluetoothLEAdvertisementWatcherStoppedEventArgs &args);
};
} // namespace Core::Bluetooth
