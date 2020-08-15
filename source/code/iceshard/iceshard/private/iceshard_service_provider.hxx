#pragma once
#include <iceshard/service_provider.hxx>
#include <iceshard/engine.hxx>
#include <iceshard/component/component_block_allocator.hxx>

#include <core/allocator.hxx>
#include <core/pod/hash.hxx>

namespace iceshard
{

    //! \brief Service provider for the Engine instance.
    class IceshardServiceProvider : public iceshard::ServiceProvider
    {
    public:
        IceshardServiceProvider(
            core::allocator& alloc,
            iceshard::EntityManager* entity_manager_ptr
        ) noexcept;

        ~IceshardServiceProvider() noexcept override = default;

        //! \brief Engine entity manager.
        auto entity_manager() noexcept -> EntityManager* override;
        auto entity_manager() const noexcept -> const EntityManager* override;

        auto entity_index() noexcept -> EntityIndex* override;
        auto entity_index() const noexcept -> EntityIndex const* override;

        auto archetype_index() noexcept -> iceshard::ecs::ArchetypeIndex* override;
        auto archetype_index() const noexcept -> iceshard::ecs::ArchetypeIndex const* override;

        auto component_block_allocator() noexcept -> ComponentBlockAllocator* override;

        //! \brief Checks if the given component system exists.
        bool has_component_system(core::stringid_arg_type component_system_name) const noexcept override;

        //! \brief A component system with the given name.
        //!
        //! \remarks If the given component system does not exist the engine will abort the process.
        auto component_system(
            core::stringid_arg_type name
        ) noexcept -> ComponentSystem* override;

        auto component_system(
            core::stringid_arg_type name
        ) const noexcept -> const ComponentSystem* override;

        void add_system(
            core::stringid_arg_type name,
            ComponentSystem* system
        ) noexcept override;

        auto remove_system(
            core::stringid_arg_type name
        ) noexcept -> ComponentSystem* override;

        template<typename Func>
        void for_each_system(Func&& func) noexcept
        {
            for (auto const& entry : _component_systems)
            {
                std::forward<Func>(func)(*entry.value);
            }
        }

    private:
        core::allocator& _allocator;
        iceshard::EntityManager* const _entity_manager;

        iceshard::ComponentBlockAllocator _component_block_allocator;

        core::pod::Hash<ComponentSystem*> _component_systems;
        core::memory::unique_pointer<iceshard::ecs::ArchetypeIndex> _archetype_index;
    };


} // namespace iceshard