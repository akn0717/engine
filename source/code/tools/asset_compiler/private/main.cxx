#include <ice/memory/memory_globals.hxx>
#include <ice/platform_app.hxx>
#include <ice/platform_event.hxx>
#include <ice/resource_index.hxx>
#include <ice/asset_system.hxx>
#include <ice/asset_module.hxx>
#include <ice/collections.hxx>

#include <CLI/CLI.hpp>

// A single entry in a ziped task object.
//  This can be used to prepare a single file for asset compilation.
//  After we have a single package, we can easily distribute it to compile remotely.
struct TaskData
{
    ice::String target_path;
    ice::String target_fragment;
    ice::u32 offset;
    ice::u32 size;
};

// Define the task to be executed localy or remotely.
//  Note: Package creation (from multiple assets) can be only done localy, as it needs all other asset tasks to finish first.
struct Task
{
    ice::String name;
    ice::AssetType type;
    ice::Span<TaskData> data_entries;

    ice::Data data;
};

int main(int argc, char** argv)
{
    ice::memory::init();

    ice::Allocator& alloc = ice::memory::default_allocator();

    // CLI data
    CLI::App app{ };

    std::string asset_name;
    app.add_option("-a,--asset", asset_name, "The asset to compile.")
        ->required();

    ice::Map<std::string, ice::AssetType> types{ alloc };
    types.emplace("tex", ice::AssetType::Texture);
    types.emplace("texture", ice::AssetType::Texture);
    types.emplace("mesh", ice::AssetType::Mesh);

    ice::AssetType asset_type;
    app.add_option("-t,--type", asset_type, "The asset type.")
        ->transform(CLI::Transformer(types))
        ->required();

    std::string mount_dir;
    app.add_option("--mount", mount_dir, "The directory to be mounted, where the asset is located.")
        ->default_val(".")
        ->required();

    std::vector<std::string> module_dirs;
    app.add_option("--module-dir", module_dirs, "Location of Ice modules.")
        ->default_val(".");

    std::vector<std::string> modules;
    app.add_option("-m,--module", modules, "File names of asset modules (ex. asset_module.dll).")
        ->required();

    CLI11_PARSE(app, argc, argv);

    //// The main allocator object
    //auto& main_allocator = core::memory::globals::default_allocator();

    //// Special proxy allocators for the game and internal systems.
    //core::memory::proxy_allocator filesystem_allocator{ "resource-filesystem", main_allocator };
    //core::memory::proxy_allocator dynlib_allocator{ "resource-dynlib", main_allocator };

    //resource::ResourceSystem resource_system{ main_allocator };

    //{
    //    auto working_dir = core::working_directory(main_allocator);
    //    fmt::print("Initializing filesystem module at: {}\n", working_dir);

    //    core::pod::Array<core::stringid_type> schemes{ core::memory::globals::default_scratch_allocator() };
    //    core::pod::array::push_back(schemes, resource::scheme_file);
    //    core::pod::array::push_back(schemes, resource::scheme_directory);
    //    resource_system.add_module(
    //        core::memory::make_unique<resource::ResourceModule, resource::FileSystem>(
    //            main_allocator, filesystem_allocator, working_dir
    //        ),
    //        schemes
    //    );

    //    core::pod::array::clear(schemes);
    //    core::pod::array::push_back(schemes, resource::scheme_dynlib);
    //    resource_system.add_module(
    //        core::memory::make_unique<resource::ResourceModule, resource::DynLibSystem>(main_allocator, dynlib_allocator),
    //        schemes
    //    );
    //}

    //// Initial mount points
    //{
    //    using resource::URI;
    //    using resource::URN;

    //    for (auto const& modules_path : module_dirs)
    //    {
    //        resource_system.mount(URI{ resource::scheme_dynlib, modules_path });
    //    }

    //    asset::AssetSystem asset_system{ main_allocator, resource_system };
    //    core::Vector<core::memory::unique_pointer<iceshard::AssetModule>> loaded_modules{ main_allocator };
    //    for (auto const& module_name : modules)
    //    {
    //        auto* const assimp_module_location = resource_system.find(URN{ module_name });
    //        if (assimp_module_location != nullptr)
    //        {
    //            loaded_modules.emplace_back(
    //                iceshard::load_asset_module(
    //                    main_allocator,
    //                    assimp_module_location->location().path,
    //                    asset_system
    //                )
    //            );
    //        }
    //    }

    //    resource_system.flush_messages();
    //    resource_system.mount(URI{ resource::scheme_directory, mount_dir });

    //    asset_system.update();

    //    asset::AssetData data;
    //    asset::AssetStatus status = asset_system.read(
    //        asset::Asset{ asset_name.c_str(), asset_type },
    //        data
    //    );

    //    if (status != asset::AssetStatus::Compiled)
    //    {
    //        if (status != asset::AssetStatus::Available)
    //        {
    //            fmt::print(stderr, "Failed to compile asset {}", asset_name);
    //        }
    //        else
    //        {
    //            fmt::print(stderr, "Trying to re-compile asset {}, skiping...", asset_name);
    //        }
    //    }
    //    else
    //    {
    //        fmt::print("Asset compiled {}", asset_name);

    //        core::data_view header_magic;
    //        header_magic._data = "ISRA";
    //        header_magic._size = 4;

    //        core::Buffer header_meta{ main_allocator };
    //        resource::store_meta_view(data.metadata, header_meta);

    //        uint32_t header_size = 4 + 4 + core::buffer::size(header_meta);
    //        core::data_view header_size_data;
    //        header_size_data._data = &header_size;
    //        header_size_data._size = 4;

    //        std::string asset_name_out = asset_name + ".isr";

    //        resource::OutputResource* asset_out = resource_system.open(URI{ resource::scheme_file, asset_name_out });
    //        asset_out->write(header_magic);
    //        asset_out->write(header_size_data);
    //        asset_out->write(header_meta);
    //        asset_out->write(data.content);
    //        asset_out->flush();
    //    }
    //}

    ice::memory::shutdown();
    return 0;
}
