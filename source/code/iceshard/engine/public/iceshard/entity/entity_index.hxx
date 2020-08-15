#pragma once
#include <iceshard/entity/entity.hxx>
#include <core/allocator.hxx>
#include <core/allocators/scratch_allocator.hxx>
#include <core/cexpr/stringid.hxx>
#include <core/pod/collections.hxx>

namespace iceshard
{


    class ComponentSystem;


    //! \brief This class is used to store information about components an entity uses.
    class EntityIndex final
    {
    public:
        EntityIndex(core::allocator& alloc) noexcept;
        ~EntityIndex() noexcept = default;

        void register_component(
            Entity entity,
            core::stringid_arg_type component_name,
            ComponentSystem* component_system
        ) noexcept;

        auto find_component_system(
            Entity entity,
            core::stringid_arg_type component_name
        ) noexcept -> ComponentSystem*;

        void remove_component(
            Entity entity,
            core::stringid_arg_type component_name
        ) noexcept;

        void remove_entity(Entity entity) noexcept;

    public:
        void query_prototypes_with_components(
            core::pod::Array<core::stringid_type>& prototypes,
            core::pod::Array<core::stringid_type> const& components
        ) noexcept;

    private:
        core::memory::scratch_allocator _temp_allocator;

        struct PrototypeInfo
        {
            core::stringid_type base_prototype;

            core::stringid_type component_name;

            ComponentSystem* component_system;
        };

        core::pod::Hash<core::stringid_type> _entity_index;
        core::pod::Hash<PrototypeInfo> _prototype_map;
    };


} // namespace iceshard