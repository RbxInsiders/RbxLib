#include <RbxLib/RbxLib.h>

#include <iostream>

int main() {
    const auto initialized = RbxLib::Core::Process::Initialize();
    if (!initialized.Succeeded()) {
        std::cerr << initialized.Message() << '\n';
        return 1;
    }

    const auto process = RbxLib::Core::Process::Find(
        L"RobloxPlayerBeta.exe");
    if (!process.Succeeded() || !process.Value().has_value()) {
        std::cerr << "Roblox is not running\n";
        RbxLib::Core::Process::Shutdown();
        return 1;
    }

    RbxLib::Offsets::OffsetProvider offsets;
    const auto refresh = offsets.Refresh();
    if (!refresh.Success) {
        std::cerr << refresh.Error << '\n';
        RbxLib::Core::Process::Shutdown();
        return 1;
    }

    RbxLib::Roblox::RobloxClient client(*process.Value(), offsets);
    const auto dataModel = client.GetDataModel();
    if (!dataModel.Succeeded()) {
        std::cerr << dataModel.Message() << '\n';
        RbxLib::Core::Process::Shutdown();
        return 1;
    }

    const auto placeId = dataModel.Value().PlaceId();
    const auto gameId = dataModel.Value().GameId();
    if (placeId.Succeeded()) {
        std::cout << "Place ID: " << placeId.Value() << '\n';
    }
    if (gameId.Succeeded()) {
        std::cout << "Game ID: " << gameId.Value() << '\n';
    }

    RbxLib::Core::Process::Shutdown();
    return 0;
}
