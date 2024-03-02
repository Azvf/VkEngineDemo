#pragma once

#include "runtime/core/meta/serializer/serializer.h"

#include <filesystem>
#include <fstream>
#include <functional>
#include <sstream>
#include <string>
#include <string_view>

#include "_generated/serializer/all_serializer.h"

namespace Chandelier
{
    class Mesh;
    class Texture;
    class VKContext;

    class AssetManager
    {
    public:
        template<class AssetType>
        bool loadAsset(const std::string& asset_url, AssetType& out_asset) const
        {
            // read json file to string
            std::filesystem::path asset_path = getFullPath(asset_url);
            std::ifstream         asset_json_file(asset_path);
            if (!asset_json_file)
            {
                assert(0);
                return false;
            }

            std::stringstream buffer;
            buffer << asset_json_file.rdbuf();
            std::string asset_json_text(buffer.str());

            // parse to json object and read to runtime res object
            std::string error;
            auto&&      asset_json = Json::parse(asset_json_text, error);
            if (!error.empty())
            {
                assert(0);
                return false;
            }

            Serializer::read(asset_json, out_asset);
            return true;
        }

        template<typename AssetType>
        bool saveAsset(const AssetType& out_asset, const std::string& asset_url) const
        {
            std::ofstream asset_json_file(getFullPath(asset_url));
            if (!asset_json_file)
            {
                assert(0);
                return false;
            }

            // write to json object and dump to string
            auto&&        asset_json      = Serializer::write(out_asset);
            std::string&& asset_json_text = asset_json.dump();

            // write to file
            asset_json_file << asset_json_text;
            asset_json_file.flush();

            return true;
        }
    };

    extern std::filesystem::path GetFullPath(std::string_view relative_path);

    extern std::shared_ptr<Mesh> LoadObjModel(std::shared_ptr<VKContext> context, std::string_view path);

    extern std::shared_ptr<Texture> LoadTexture(std::shared_ptr<VKContext> context, std::string_view path);

    extern std::shared_ptr<Texture> LoadTextureHDR(std::shared_ptr<VKContext> context, std::string_view path, int desired_channels);

    extern std::shared_ptr<Texture>
    LoadSkybox(std::shared_ptr<VKContext> context, std::string_view path, int desired_channels);

    extern std::shared_ptr<Texture>
    LoadSkybox(std::shared_ptr<VKContext> context, std::array<std::shared_ptr<Texture>, 6> faces, int desired_channels);

} // namespace Chandelier