# RbxLib

RbxLib is a C++20 library for Roblox's internal client state.

It gives you external access to Roblox instances, player data, game state and other things through a type-safe API (though at the cost of verbosity)

## Getting started

A small example is here if you desire, it reads the game's place ID:

```cpp
#include <RbxLib/RbxLib.h>

#include <iostream>

int main() {
    if (!RbxLib::Core::Process::Initialize().Succeeded()) {
        return 1;
    }

    const auto process = RbxLib::Core::Process::Find(
        L"RobloxPlayerBeta.exe");

    if (!process.Succeeded() || !process.Value().has_value()) {
        return 1;
    }

    RbxLib::Offsets::OffsetProvider offsets;
    if (!offsets.Refresh().Success) {
        return 1;
    }

    RbxLib::Roblox::RobloxClient client(*process.Value(), offsets);

    const auto dataModel = client.GetDataModel();
    if (!dataModel.Succeeded()) {
        std::cerr << dataModel.Message() << '\n';
        return 1;
    }

    const auto placeId = dataModel.Value().PlaceId();
    if (placeId.Succeeded()) {
        std::cout << "Place ID: " << placeId.Value() << '\n';
    }

    RbxLib::Core::Process::Shutdown();
}
```

## Building

RbxLib uses CMake and currently targets Windows.

You'll need CMake 3.25 or newer.

```powershell
cmake -S . -B out/build/rbxlib-windows -DRBXLIB_BUILD_EXAMPLES=ON
cmake --build out/build/rbxlib-windows --target RbxLibShowcase
```