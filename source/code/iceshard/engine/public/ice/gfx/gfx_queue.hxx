#pragma once
#include <ice/span.hxx>
#include <ice/stringid.hxx>
#include <ice/render/render_declarations.hxx>

namespace ice::gfx
{

    class GfxPass;

    class GfxQueue
    {
    public:
        virtual ~GfxQueue() noexcept = default;

        virtual bool presenting() const noexcept = 0;
    };

} // namespace ice::gfx
