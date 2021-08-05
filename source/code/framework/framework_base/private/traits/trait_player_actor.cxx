#include "trait_player_actor.hxx"
#include <ice/game_anim.hxx>

#include <ice/engine_frame.hxx>
#include <ice/world/world_portal.hxx>

#include <ice/input/input_device.hxx>
#include <ice/input/input_event.hxx>
#include <ice/input/input_keyboard.hxx>

#include <ice/data_storage.hxx>

namespace ice
{

    void IceWorldTrait_PlayerActor::on_activate(
        ice::Engine& engine,
        ice::EngineRunner& runner,
        ice::WorldPortal& portal
    ) noexcept
    {
        portal.storage().create_named_object<Query>("ice.query.player_actors"_sid, portal.allocator(), portal.entity_storage().archetype_index());
    }

    void IceWorldTrait_PlayerActor::on_deactivate(
        ice::Engine& engine,
        ice::EngineRunner& runner,
        ice::WorldPortal& portal
    ) noexcept
    {
        portal.storage().destroy_named_object<Query>("ice.query.player_actors"_sid);
    }

    void IceWorldTrait_PlayerActor::on_update(
        ice::EngineFrame& frame,
        ice::EngineRunner& runner,
        ice::WorldPortal& portal
    ) noexcept
    {
        using ice::input::DeviceType;
        using ice::input::KeyboardKey;
        using ice::input::input_identifier;

        Query& query = *portal.storage().named_object<Query>("ice.query.player_actors"_sid);
        Query::ResultByEntity result = query.result_by_entity(frame.allocator(), portal.entity_storage());

        ice::vec2f movement;
        for (ice::input::InputEvent const& input : frame.input_events())
        {
            switch (input.identifier)
            {
            case input_identifier(DeviceType::Keyboard, KeyboardKey::KeyW):
                movement.y += 2.f;
                break;
            case input_identifier(DeviceType::Keyboard, KeyboardKey::KeyS):
                movement.y -= 2.f;
                break;
            case input_identifier(DeviceType::Keyboard, KeyboardKey::KeyA):
                movement.x -= 2.f;
                break;
            case input_identifier(DeviceType::Keyboard, KeyboardKey::KeyD):
                movement.x += 2.f;
                break;
            default:
                break;
            }
        }

        result.for_each(
            [&](ice::Actor const& actor, ice::Animation& anim, ice::Transform2DDynamic& xform) noexcept
            {
                if (actor.type == ActorType::Player)
                {
                    xform.position = xform.position + ice::vec3f{ movement.x, movement.y, 0 };
                }

                if (movement.x != 0.f)
                {
                    anim.speed = 1.f / 15.f;
                    if (movement.x > 0)
                    {
                        anim.animation = "cotm_run_right"_sid_hash;
                    }
                    else
                    {
                        anim.animation = "cotm_run_left"_sid_hash;
                    }
                }
                else
                {
                    anim.animation = "cotm_idle"_sid_hash;
                    anim.speed = 1.f / 15.f;
                }
            }
        );
    }

    auto create_trait_actor(
        ice::Allocator& alloc
    ) noexcept -> ice::UniquePtr<ice::WorldTrait>
    {
        return ice::make_unique<ice::WorldTrait, ice::IceWorldTrait_PlayerActor>(alloc);
    }

} // namespace ice