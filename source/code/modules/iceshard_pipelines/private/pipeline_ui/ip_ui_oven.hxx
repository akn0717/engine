#pragma once
#include <ice/string.hxx>
#include <ice/pod/array.hxx>
#include <ice/ui_types.hxx>

#include <rapidxml_ns/rapidxml_ns.hpp>
#undef assert

namespace ice
{

    using ice::ui::ElementType;

    struct RawElement;
    struct RawShard;
    struct RawResource;
    struct RawStyle;

    static constexpr ice::String Constant_ISUINamespaceUI = "https://www.iceshard.net/docs/engine/v1_alpha/isui/ui/";
    static constexpr ice::String Constant_ISUINamespaceIceShard = "https://www.iceshard.net/docs/engine/v1_alpha/isui/iceshard/";

    static constexpr ice::String Constant_UIElement_Root = "isui";

    void parse_ui_file(
        ice::Allocator& alloc,
        rapidxml_ns::xml_document<char>& doc,
        ice::pod::Array<ice::RawElement>& raw_elements,
        ice::pod::Array<ice::RawResource>& ui_resources,
        ice::pod::Array<ice::RawShard>& raw_shards,
        ice::pod::Array<ice::RawStyle>& raw_styles
    ) noexcept;

} // namespace ice
