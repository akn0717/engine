#pragma once
#include <ice/ui_action.hxx>

#include "ip_ui_oven.hxx"

namespace ice
{

    static constexpr ice::String Constant_UIElement_Label = "label";
    static constexpr ice::String Constant_UIElement_Button = "button";

    static constexpr ice::String Constant_UIAttribute_Text = "text";
    static constexpr ice::String Constant_UIAttribute_Font = "font";
    static constexpr ice::String Constant_UIAttribute_OnClick = "on_click";

    struct RawData
    {
        ice::ui::DataSource data_type;
        ice::Utf8String data_source;
    };

    struct RawAction
    {
        static constexpr ice::Utf8String Constant_ActionType_Shard = u8"Shard";
        static constexpr ice::Utf8String Constant_ActionType_UIShow = u8"UIShow";
        static constexpr ice::Utf8String Constant_ActionType_Resource = u8"Resource";

        static constexpr ice::Utf8String Constant_ActionDataType_Resource = u8"Resource";
        static constexpr ice::Utf8String Constant_ActionDataType_Property = u8"Property";

        ice::ui::ActionType action_type;
        ice::Utf8String action_value;
        ice::RawData data;
    };

    struct RawButtonInfo
    {
        ice::RawData text;
        ice::RawData font;
        ice::RawAction action_on_click;
    };

    struct RawLabelInfo
    {
        ice::RawData text;
        ice::RawData font;
    };

    void parse_element_attribs(
        ice::Allocator& alloc,
        rapidxml_ns::xml_node<char> const* element,
        ice::RawElement& info
    ) noexcept;

    void parse_element_details(
        ice::Allocator& alloc,
        rapidxml_ns::xml_node<char> const* xml_element,
        ice::RawElement& info
    ) noexcept;

} // namespace ice
