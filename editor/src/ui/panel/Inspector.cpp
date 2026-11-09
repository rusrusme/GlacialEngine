//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "Inspector.hpp"

#include "Config.hpp"
#include "asset/AssetManager.hpp"
#include "audio/AudioSystem.hpp"
#include "core/SystemContext.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "render/mesh/Mesh.hpp"

namespace {
    void begin_property_grid(const char* table_id = "PropertyGrid") {
        ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(4.0f, 3.0f));
        ImGui::BeginTable(table_id, 2, ImGuiTableFlags_SizingStretchSame | ImGuiTableFlags_PadOuterX);
        ImGui::TableSetupColumn("Label", ImGuiTableColumnFlags_WidthFixed, 90.0f);
        ImGui::TableSetupColumn("Widget", ImGuiTableColumnFlags_WidthStretch);
    }

    void end_property_grid() {
        ImGui::EndTable();
        ImGui::PopStyleVar();
    }

    void draw_property_label(const char* label) {
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::AlignTextToFramePadding();
        ImGui::TextUnformatted(label);
        ImGui::TableSetColumnIndex(1);
        ImGui::SetNextItemWidth(-1.0f);
    }

    void begin_card_panel(const char* id, float rounding = 5.0f) {
        ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, rounding);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8.0f, 8.0f));
        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.13f, 0.13f, 0.15f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.24f, 0.24f, 0.28f, 1.0f));

        ImGui::BeginChild(id, ImVec2(0.0f, 0.0f), ImGuiChildFlags_Borders | ImGuiChildFlags_AutoResizeY, 0);
    }

    void end_card_panel() {
        ImGui::EndChild();
        ImGui::PopStyleColor(2);
        ImGui::PopStyleVar(2);
    }

    void draw_vec3_control(const std::string& label, glm::vec3& values, float reset_value = 0.0f, float speed = 0.1f) {
        ImGui::PushID(label.c_str());

        draw_property_label(label.c_str());

        const float avail_width = ImGui::GetContentRegionAvail().x;
        const float item_width = (avail_width - 8.0f) / 3.0f;

        ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4.0f, 0.0f));

        const ImGuiIO& io = ImGui::GetIO();

        // --- X AXIS (RED BORDER) ---
        ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.85f, 0.25f, 0.25f, 0.85f));
        ImGui::SetNextItemWidth(item_width);
        // ImGuiSliderFlags_NoInput prevents Ctrl+Click from opening manual text box input
        ImGui::DragFloat("##X", &values.x, speed, 0.0f, 0.0f, "X: %.2f", ImGuiSliderFlags_NoInput);

        if (ImGui::IsItemHovered() && io.KeyCtrl && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
            values.x = reset_value;
        }
        ImGui::PopStyleColor();
        ImGui::SameLine();

        // --- Y AXIS (GREEN BORDER) ---
        ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.25f, 0.75f, 0.35f, 0.85f));
        ImGui::SetNextItemWidth(item_width);
        ImGui::DragFloat("##Y", &values.y, speed, 0.0f, 0.0f, "Y: %.2f", ImGuiSliderFlags_NoInput);

        if (ImGui::IsItemHovered() && io.KeyCtrl && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
            values.y = reset_value;
        }
        ImGui::PopStyleColor();
        ImGui::SameLine();

        // --- Z AXIS (BLUE BORDER) ---
        ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.30f, 0.50f, 0.90f, 0.85f));
        ImGui::SetNextItemWidth(item_width);
        ImGui::DragFloat("##Z", &values.z, speed, 0.0f, 0.0f, "Z: %.2f", ImGuiSliderFlags_NoInput);

        if (ImGui::IsItemHovered() && io.KeyCtrl && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
            values.z = reset_value;
        }
        ImGui::PopStyleColor();

        ImGui::PopStyleVar(2);
        ImGui::PopID();
    }

    // Unified Component Header
    template<typename T, typename UIFunction>
    void draw_component_header(const std::string& title, engine::Entity entity, UIFunction ui_function, bool allow_removal = true) {
        if (!entity.has_component<T>()) return;

        constexpr ImGuiTreeNodeFlags tree_flags = ImGuiTreeNodeFlags_DefaultOpen
            | ImGuiTreeNodeFlags_Framed
            | ImGuiTreeNodeFlags_SpanAvailWidth
            | ImGuiTreeNodeFlags_AllowOverlap
            | ImGuiTreeNodeFlags_FramePadding;

        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4.0f, 4.0f));
        ImGui::Spacing();

        std::string popup_id = title + "_HeaderContextMenu";
        const bool open = ImGui::TreeNodeEx(reinterpret_cast<void*>(typeid(T).hash_code()), tree_flags, "%s", title.c_str());
        ImGui::PopStyleVar();

        bool remove_component = false;
        if (ImGui::BeginPopupContextItem(popup_id.c_str())) {
            if constexpr (std::is_same_v<T, engine::TransformComponent>) {
                if (ImGui::MenuItem("Reset Transform")) {
                    auto& transform = entity.get_component<engine::TransformComponent>();
                    transform.position = glm::vec3(0.0f);
                    transform.rotation = glm::vec3(0.0f);
                    transform.scale = glm::vec3(1.0f);
                }
            }
            else if (allow_removal) {
                if (ImGui::MenuItem("Remove Component")) {
                    remove_component = true;
                }
            }
            ImGui::EndPopup();
        }

        if (open) {
            ImGui::Unindent();

            begin_property_grid((title + "_Grid").c_str());
            ui_function(entity.get_component<T>());
            end_property_grid();

            ImGui::Indent();
            ImGui::TreePop();
        }

        if (remove_component) {
            entity.remove_component<T>();
        }
    }
}

namespace editor {
    void Inspector::on_init(SystemContext& context) {
        UIComponent::on_init(context);
    }

    void Inspector::on_render_ui(SystemContext& context) {
        if (!visible_) return;

        ImGui::Begin("Inspector");

        if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows)) {
            Shortcuts::set_active_context(shortcut_context());
        }

        if (selected_entity_) {
            draw_component(selected_entity_, context);
        }

        ImGui::End();
    }

    void Inspector::load_config() {
        visible_ = config::get_value("InspectorPanel_visible", visible_);
    }

    void Inspector::save_config() {
        config::set_value("InspectorPanel_visible", visible_);
    }

    void Inspector::draw_component(Entity entity, SystemContext& context) const {
        draw_header_section(entity);
        draw_transform_component(entity);
        draw_mesh_component(entity, context);
        draw_material_component(entity, context);
        // draw_lod_component(entity, context);
        // draw_lighting_components(entity);
        // draw_box_collider_component(entity, context);
        draw_audio_source_component(entity, context);

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        draw_add_component_button(entity, context);
    }

    void Inspector::draw_header_section(Entity entity) const {
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4, 6));

        bool is_active = entity.is_active();
        if (ImGui::Checkbox("##Active", &is_active)) {
            entity.set_active(is_active);
        }
        ImGui::SameLine();

        if (entity.has_component<LabelComponent>()) {
            auto& label = entity.get_component<LabelComponent>();

            char buffer[256]{};
            std::strncpy(buffer, label.tag.c_str(), sizeof(buffer) - 1);

            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
            if (ImGui::InputText("##Tag", buffer, sizeof(buffer))) {
                label.tag = std::string(buffer);
            }
        }
        ImGui::PopStyleVar();
        ImGui::Spacing();
    }

    void Inspector::draw_transform_component(Entity entity) const {
        draw_component_header<TransformComponent>("Transform", entity, [](auto& transform) {
            draw_vec3_control("Position", transform.position, 0.0f, 0.1f);

            auto wrap_angle_360 = [](float angle) {
                angle = std::fmod(angle, 360.0f);
                return angle < 0.0f ? angle + 360.0f : angle;
            };

            glm::vec3 prev_rotation = transform.rotation;
            draw_vec3_control("Rotation", transform.rotation, 0.0f, 1.0f);
            if (prev_rotation != transform.rotation) {
                transform.rotation.x = wrap_angle_360(transform.rotation.x);
                transform.rotation.y = wrap_angle_360(transform.rotation.y);
                transform.rotation.z = wrap_angle_360(transform.rotation.z);
            }

            draw_vec3_control("Scale", transform.scale, 1.0f, 0.05f);
        }, false);
    }

    void Inspector::draw_mesh_component(Entity entity, SystemContext& context) const {
        draw_component_header<MeshComponent>("Mesh", entity, [&](auto& mesh_comp) {
            draw_property_label("Mesh Asset");

            std::string current_label = "None / Select Mesh";
            if (mesh_comp.handle != 0) {
                const auto& meta = context.assets.metadata(mesh_comp.handle);
                if (meta.valid()) {
                    current_label = meta.filepath.filename().string();
                }
            }

            if (ImGui::BeginCombo("##MeshCombo", current_label.c_str())) {
                for (const auto& [handle, meta] : context.assets.registry()) {
                    if (meta.type != AssetType::MESH) continue;
                    const bool is_selected = (mesh_comp.handle == handle);
                    std::string item_name = meta.filepath.filename().string();
                    if (ImGui::Selectable(item_name.c_str(), is_selected)) {
                        mesh_comp.handle = handle;
                    }
                    if (is_selected) ImGui::SetItemDefaultFocus();
                }
                ImGui::EndCombo();
            }

            draw_property_label("Asset UUID");
            ImGui::TextDisabled("%llu", static_cast<uint64_t>(mesh_comp.handle));
        });
    }

    void Inspector::draw_material_component(Entity entity, SystemContext& context) const {
        draw_component_header<MaterialComponent>("Material", entity, [&](auto& material) {
            draw_property_label("Color");
            ImGui::ColorEdit4("##Color", glm::value_ptr(material.color));

            draw_property_label("Roughness");
            ImGui::SliderFloat("##Roughness", &material.roughness, 0.0f, 1.0f);

            draw_property_label("Metallic");
            ImGui::SliderFloat("##Metallic", &material.metallic, 0.0f, 1.0f);

            draw_property_label("Albedo Texture");
            std::string tex_label = "None (Drag & Drop)";
            if (material.texture_handle != 0) {
                const auto& meta = context.assets.metadata(material.texture_handle);
                if (meta.valid()) {
                    tex_label = meta.filepath.filename().string();
                }
            }

            if (ImGui::BeginCombo("##TextureCombo", tex_label.c_str())) {
                if (ImGui::Selectable("None", material.texture_handle == 0)) {
                    material.texture_handle = {0};
                }
                for (const auto& [handle, meta] : context.assets.registry()) {
                    if (meta.type != AssetType::TEXTURE_2D) continue;
                    const bool is_selected = (material.texture_handle == handle);
                    if (ImGui::Selectable(meta.filepath.filename().string().c_str(), is_selected)) {
                        material.texture_handle = handle;
                    }
                    if (is_selected) ImGui::SetItemDefaultFocus();
                }
                ImGui::EndCombo();
            }

            if (ImGui::BeginDragDropTarget()) {
                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSET_BROWSER_ITEM")) {
                    AssetHandle dropped_handle = *static_cast<const AssetHandle*>(payload->Data);
                    const auto& meta = context.assets.metadata(dropped_handle);
                    if (meta.valid() && meta.type == AssetType::TEXTURE_2D) {
                        material.texture_handle = dropped_handle;
                    }
                }
                ImGui::EndDragDropTarget();
            }
        });
    }

    void Inspector::draw_audio_source_component(Entity entity, SystemContext& context) const {
        draw_component_header<AudioSourceComponent>("Audio Source", entity, [&](auto& audio) {
            draw_property_label("Audio Asset");
            const auto& current_meta = context.assets.metadata(audio.sound_handle);
            std::string audio_label = current_meta.valid() ? current_meta.filepath.filename().string() : "None / Select Audio";

            if (ImGui::BeginCombo("##AudioCombo", audio_label.c_str())) {
                if (ImGui::Selectable("None", audio.sound_handle == 0)) {
                    audio.sound_handle = {0};
                    audio.filepath = "";
                }
                for (const auto& [handle, meta] : context.assets.registry()) {
                    if (meta.type != AssetType::AUDIO) continue;
                    const bool is_selected = (audio.sound_handle == handle);
                    if (ImGui::Selectable(meta.filepath.filename().string().c_str(), is_selected)) {
                        audio.sound_handle = handle;
                        audio.filepath = meta.filepath.string();
                    }
                    if (is_selected) ImGui::SetItemDefaultFocus();
                }
                ImGui::EndCombo();
            }

            draw_property_label("Playback");
            {
                const bool has_audio = (audio.sound_handle != 0);
                const bool is_playing = context.audio.is_source_playing(audio);

                ImGui::BeginDisabled(!has_audio);

                if (is_playing) {
                    if (ImGui::Button("Pause##Audio", ImVec2(55.0f, 0.0f))) {
                        context.audio.pause_source(audio);
                    }
                }
                else {
                    if (ImGui::Button("Play##Audio", ImVec2(55.0f, 0.0f))) {
                        context.audio.play_source(context, audio);
                    }
                }

                ImGui::SameLine();
                if (ImGui::Button("Stop##Audio", ImVec2(55.0f, 0.0f))) {
                    context.audio.stop_source(audio);
                }

                ImGui::EndDisabled();
            }

            draw_property_label("Volume");
            ImGui::SliderFloat("##Volume", &audio.volume, 0.0f, 1.0f);

            draw_property_label("Pitch");
            ImGui::SliderFloat("##Pitch", &audio.pitch, 0.1f, 2.0f);

            draw_property_label("Loop");
            ImGui::Checkbox("##Loop", &audio.loop);

            draw_property_label("Spatial (3D)");
            ImGui::Checkbox("##Spatial", &audio.is_spatial);
        });
    }

    void Inspector::draw_add_component_button(Entity entity, SystemContext& context) const {
        ImGui::SetCursorPosX((ImGui::GetContentRegionAvail().x - 150.0f) * 0.5f);
        if (ImGui::Button("+ Add Component", ImVec2(150.0f, 30.0f))) {
            ImGui::OpenPopup("AddComponentPopup");
        }

        if (ImGui::BeginPopup("AddComponentPopup")) {
            if (!entity.has_component<MaterialComponent>()) {
                if (ImGui::MenuItem("Material")) {
                    entity.add_component<MaterialComponent>(glm::vec4{1.0f, 1.0f, 1.0f, 1.0f});
                    ImGui::CloseCurrentPopup();
                }
            }
            // if (!entity.has_component<LODComponent>()) {
            //     if (ImGui::MenuItem("Level of Detail")) {
            //         entity.add_component<LODComponent>();
            //         ImGui::CloseCurrentPopup();
            //     }
            // }
            // if (!entity.has_component<BoxColliderComponent>()) {
            //     if (ImGui::MenuItem("Box Collider")) {
            //         auto& collider = entity.add_component<BoxColliderComponent>();
            //         if (entity.has_component<MeshComponent>()) {
            //             const auto& mesh_comp = entity.get_component<MeshComponent>();
            //             if (auto mesh = context.assets.get<Mesh>(mesh_comp.handle)) {
            //                 collider.size = mesh->local_bounds.max - mesh->local_bounds.min;
            //                 collider.center = (mesh->local_bounds.min + mesh->local_bounds.max) * 0.5f;
            //             }
            //         }
            //         ImGui::CloseCurrentPopup();
            //     }
            // }
            if (!entity.has_component<AudioSourceComponent>()) {
                if (ImGui::MenuItem("Audio Source")) {
                    entity.add_component<AudioSourceComponent>();
                    ImGui::CloseCurrentPopup();
                }
            }
            ImGui::EndPopup();
        }
    }
}
