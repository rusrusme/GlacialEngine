//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "gui/ObjectInspector.hpp"

#include "platform/FileDialogAPI.hpp"
#include "platform/win32/Win32FileDialog.hpp"

namespace editor {
    void ObjectInspector::init(std::shared_ptr<engine::Scene> scene, std::shared_ptr<engine::AssetManager> asset_manager, engine::AudioLayer* audio_layer) {
        scene_ = std::move(scene);
        asset_manager_ = std::move(asset_manager);
        audio_layer_ = audio_layer;
    }

    #ifdef ENABLE_IMGUI
    void ObjectInspector::render_imgui() {
        ImGui::Begin("Inspector");
        if (selected_entity_) {
            draw_component(selected_entity_);
        }
        ImGui::End();
    }

    void ObjectInspector::draw_component(engine::Entity entity) const {
        draw_header_section(entity);
        draw_transform_component(entity);
        draw_mesh_component(entity);
        draw_material_component(entity);
        draw_lighting_components(entity);
        draw_box_collider_component(entity);
        draw_audio_source_component(entity);

        ImGui::Separator();
        ImGui::Spacing();

        draw_add_component_button(entity);
    }

    void ObjectInspector::draw_header_section(engine::Entity entity) const {
        if (entity.has_component<engine::ActiveComponent>()) {
            auto& active_comp = entity.get_component<engine::ActiveComponent>();
            ImGui::Checkbox("##Active", &active_comp.active);
            ImGui::SameLine();
        }

        if (entity.has_component<engine::LabelComponent>()) {
            auto& label = entity.get_component<engine::LabelComponent>();

            char buffer[256]{};
            std::strncpy(buffer, label.tag.c_str(), sizeof(buffer) - 1);

            if (ImGui::InputText("##Tag", buffer, sizeof(buffer))) {
                label.tag = std::string(buffer);
            }
        }
        ImGui::Separator();
    }

    void ObjectInspector::draw_transform_component(engine::Entity entity) const {
        if (!entity.has_component<engine::TransformComponent>()) return;

        if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen)) {
            auto& transform = entity.get_component<engine::TransformComponent>();

            ImGui::DragFloat3("Position", glm::value_ptr(transform.position), 0.1f);

            auto wrap_angle_360 = [](float angle) {
                angle = std::fmod(angle, 360.0f);
                if (angle < 0.0f) angle += 360.0f;
                return angle;
            };

            if (ImGui::DragFloat3("Rotation", glm::value_ptr(transform.rotation), 1.0f)) {
                transform.rotation.x = wrap_angle_360(transform.rotation.x);
                transform.rotation.y = wrap_angle_360(transform.rotation.y);
                transform.rotation.z = wrap_angle_360(transform.rotation.z);
            }
            ImGui::DragFloat3("Scale", glm::value_ptr(transform.scale), 0.05f, 0.001f, 1000.0f);
        }
    }

    void ObjectInspector::draw_mesh_component(engine::Entity entity) const {
        if (!entity.has_component<engine::MeshComponent>()) return;

        if (ImGui::CollapsingHeader("Mesh", ImGuiTreeNodeFlags_DefaultOpen)) {
            auto& mesh_comp = entity.get_component<engine::MeshComponent>();
            const auto& current_meta = asset_manager_->get_metadata(mesh_comp.handle);

            std::string current_label = current_meta.is_valid() ? current_meta.filepath.filename().string() : "None / Select Mesh";

            if (ImGui::BeginCombo("Mesh Asset", current_label.c_str())) {
                for (const auto& [handle, meta] : asset_manager_->get_registry()) {
                    if (meta.type != engine::AssetType::MESH) continue;
                    const bool is_selected = (mesh_comp.handle == handle);
                    std::string item_name = meta.filepath.filename().string();
                    if (ImGui::Selectable(item_name.c_str(), is_selected)) {
                        mesh_comp.handle = handle;
                    }
                    if (is_selected) ImGui::SetItemDefaultFocus();
                }
                ImGui::EndCombo();
            }
            ImGui::TextDisabled("Handle UUID: %llu", static_cast<uint64_t>(mesh_comp.handle));
        }
    }

    void ObjectInspector::draw_material_component(engine::Entity entity) const {
        if (!entity.has_component<engine::MaterialComponent>()) return;

        if (ImGui::CollapsingHeader("Material", ImGuiTreeNodeFlags_DefaultOpen)) {
            auto& material = entity.get_component<engine::MaterialComponent>();
            ImGui::ColorEdit4("Color", glm::value_ptr(material.color));

            ImGui::SliderFloat("Roughness", &material.roughness, 0.0f, 1.0f);
            ImGui::SliderFloat("Metallic", &material.metallic, 0.0f, 1.0f);
            ImGui::Separator();

            const auto& current_tex_meta = asset_manager_->get_metadata(material.texture_handle);
            std::string tex_label = current_tex_meta.is_valid()
                                        ? current_tex_meta.filepath.filename().string()
                                        : "None / Select Texture";

            if (ImGui::BeginCombo("Texture", tex_label.c_str())) {
                if (ImGui::Selectable("None", material.texture_handle == 0)) {
                    material.texture_handle = {0};
                }

                for (const auto& [handle, meta] : asset_manager_->get_registry()) {
                    if (meta.type != engine::AssetType::TEXTURE_2D) continue;

                    const bool is_selected = (material.texture_handle == handle);
                    std::string item_name = meta.filepath.filename().string();

                    if (ImGui::Selectable(item_name.c_str(), is_selected)) {
                        material.texture_handle = handle;
                    }
                    if (is_selected) ImGui::SetItemDefaultFocus();
                }
                ImGui::EndCombo();
            }

            if (ImGui::BeginDragDropTarget()) {
                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSET_BROWSER_ITEM")) {
                    engine::AssetHandle dropped_handle = *static_cast<const engine::AssetHandle*>(payload->Data);
                    const auto& meta = asset_manager_->get_metadata(dropped_handle);

                    if (meta.is_valid() && meta.type == engine::AssetType::TEXTURE_2D) {
                        material.texture_handle = dropped_handle;
                    }
                }
                ImGui::EndDragDropTarget();
            }

            if (material.texture_handle != 0) {
                ImGui::TextDisabled("Texture Handle: %llu", static_cast<uint64_t>(material.texture_handle));
            }
        }
    }

    void ObjectInspector::draw_lighting_components(engine::Entity entity) const {
        if (entity.has_component<engine::DirectionalLightComponent>()) {
            if (ImGui::CollapsingHeader("Lighting", ImGuiTreeNodeFlags_DefaultOpen)) {
                auto& dir_light = entity.get_component<engine::DirectionalLightComponent>();
                ImGui::DragFloat("Intensity", &dir_light.intensity, 0.01f, 0.0f, 100.0f);
                ImGui::ColorEdit3("Color", glm::value_ptr(dir_light.color));
                bool todo_shadow_button = true;
                ImGui::Checkbox("Cast Shadows", &todo_shadow_button);
            }
        }

        if (entity.has_component<engine::PointLightComponent>()) {
            if (ImGui::CollapsingHeader("Lighting", ImGuiTreeNodeFlags_DefaultOpen)) {
                auto& point_light = entity.get_component<engine::PointLightComponent>();
                ImGui::DragFloat("Intensity", &point_light.intensity, 0.01f, 0.0f, 100.0f);
                ImGui::ColorEdit3("Color", glm::value_ptr(point_light.color));
                ImGui::DragFloat("Radius", &point_light.radius, 0.1f, 0.0f, 1000.0f);
                bool todo_shadow_button = true;
                ImGui::Checkbox("Cast Shadows", &todo_shadow_button);
            }
        }
    }

    void ObjectInspector::draw_box_collider_component(engine::Entity entity) const {
        if (!entity.has_component<engine::BoxColliderComponent>()) return;

        if (ImGui::CollapsingHeader("Box Collider", ImGuiTreeNodeFlags_DefaultOpen)) {
            auto& collider = entity.get_component<engine::BoxColliderComponent>();

            ImGui::DragFloat3("Center", glm::value_ptr(collider.center), 0.05f);
            ImGui::DragFloat3("Size", glm::value_ptr(collider.size), 0.05f, 0.01f, 100.0f);
            ImGui::Checkbox("Is Trigger", &collider.is_trigger);
            ImGui::Checkbox("Show Hitbox", &collider.show_hitbox);

            if (entity.has_component<engine::MeshComponent>()) {
                if (ImGui::Button("Fit to Mesh")) {
                    const auto& mesh_comp = entity.get_component<engine::MeshComponent>();
                    if (auto mesh = asset_manager_->get_asset<engine::Mesh>(mesh_comp.handle)) {
                        collider.size = mesh->local_bounds.max - mesh->local_bounds.min;
                        collider.center = (mesh->local_bounds.min + mesh->local_bounds.max) * 0.5f;
                    }
                }
            }
        }
    }

    void ObjectInspector::draw_audio_source_component(engine::Entity entity) const {
        if (!entity.has_component<engine::AudioSourceComponent>()) return;

        if (ImGui::CollapsingHeader("Audio Source", ImGuiTreeNodeFlags_DefaultOpen)) {
            auto& audio = entity.get_component<engine::AudioSourceComponent>();

            // Asset Selection via AssetManager Combo
            const auto& current_meta = asset_manager_->get_metadata(audio.sound_handle);
            std::string audio_label = current_meta.is_valid()
                                          ? current_meta.filepath.filename().string()
                                          : "None / Select Audio Asset";

            if (ImGui::BeginCombo("Audio Asset", audio_label.c_str())) {
                if (ImGui::Selectable("None", audio.sound_handle == 0)) {
                    audio.sound_handle = {0};
                    audio.filepath = "";
                }

                for (const auto& [handle, meta] : asset_manager_->get_registry()) {
                    if (meta.type != engine::AssetType::AUDIO) continue;

                    const bool is_selected = (audio.sound_handle == handle);
                    std::string item_name = meta.filepath.filename().string();

                    if (ImGui::Selectable(item_name.c_str(), is_selected)) {
                        audio.sound_handle = handle;
                        audio.filepath = meta.filepath.string();
                    }
                    if (is_selected) ImGui::SetItemDefaultFocus();
                }
                ImGui::EndCombo();
            }

            // Drag & Drop Target
            if (ImGui::BeginDragDropTarget()) {
                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSET_BROWSER_ITEM")) {
                    engine::AssetHandle dropped_handle = *static_cast<const engine::AssetHandle*>(payload->Data);
                    const auto& meta = asset_manager_->get_metadata(dropped_handle);

                    if (meta.is_valid() && meta.type == engine::AssetType::AUDIO) {
                        audio.sound_handle = dropped_handle;
                        audio.filepath = meta.filepath.string();
                    }
                }
                ImGui::EndDragDropTarget();
            }

            // Filepath Input & Dialog
            char path_buffer[256]{};
            std::strncpy(path_buffer, audio.filepath.c_str(), sizeof(path_buffer) - 1);

            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - 35.0f);
            if (ImGui::InputText("##FilePath", path_buffer, sizeof(path_buffer))) {
                audio.filepath = std::string(path_buffer);
            }

            ImGui::SameLine();
            if (ImGui::Button("...##BrowseAudio")) {
                std::string selected_file = engine::platform::FileDialogAPI::open_file(engine::platform::FileDialogFilter::AUDIO);
                if (!selected_file.empty()) {
                    std::filesystem::path full_path(selected_file);
                    std::filesystem::path current_path = std::filesystem::current_path();
                    try {
                        audio.filepath = std::filesystem::relative(full_path, current_path).generic_string();
                    }
                    catch (...) {
                        audio.filepath = selected_file;
                    }
                    audio.sound_handle = asset_manager_->get_handle_from_path(audio.filepath);
                }
            }
            ImGui::SameLine();
            ImGui::Text("File Path");

            ImGui::Separator();

            // Trigger Conditions & Playback
            const char* trigger_types[] = {"Play On Awake", "On Physics Collision", "On Trigger Volume Enter", "Manual (Scripting)"};
            int current_trigger = static_cast<int>(audio.trigger_condition);
            if (ImGui::Combo("Play Condition", &current_trigger, trigger_types, IM_ARRAYSIZE(trigger_types))) {
                audio.trigger_condition = static_cast<engine::AudioTriggerCondition>(current_trigger);
            }

            ImGui::SliderFloat("Volume", &audio.volume, 0.0f, 1.0f);
            ImGui::SliderFloat("Pitch", &audio.pitch, 0.1f, 2.0f);
            ImGui::Checkbox("Looping", &audio.loop);
            ImGui::Checkbox("Spatial (3D Audio)", &audio.is_spatial);

            ImGui::Separator();

            // Audio Preview Controls
            if (ImGui::Button("Play Preview")) {
                audio_layer_->play_source(audio);
            }
            ImGui::SameLine();
            if (ImGui::Button("Stop Preview")) {
                audio_layer_->stop_source(audio);
            }
        }
    }

    void ObjectInspector::draw_add_component_button(engine::Entity entity) const {
        if (ImGui::Button("+ Add Component")) {
            ImGui::OpenPopup("AddComponentPopup");
        }

        if (ImGui::BeginPopup("AddComponentPopup")) {
            if (!entity.has_component<engine::MaterialComponent>()) {
                if (ImGui::MenuItem("Material")) {
                    entity.add_component<engine::MaterialComponent>(glm::vec4{1.0f, 1.0f, 1.0f, 1.0f});
                    ImGui::CloseCurrentPopup();
                }
            }
            if (!entity.has_component<engine::BoxColliderComponent>()) {
                if (ImGui::MenuItem("Box Collider")) {
                    auto& collider = entity.add_component<engine::BoxColliderComponent>();

                    if (entity.has_component<engine::MeshComponent>()) {
                        const auto& mesh_comp = entity.get_component<engine::MeshComponent>();
                        if (auto mesh = asset_manager_->get_asset<engine::Mesh>(mesh_comp.handle)) {
                            collider.size = mesh->local_bounds.max - mesh->local_bounds.min;
                            collider.center = (mesh->local_bounds.min + mesh->local_bounds.max) * 0.5f;
                        }
                    }
                    ImGui::CloseCurrentPopup();
                }
            }
            if (!entity.has_component<engine::AudioSourceComponent>()) {
                if (ImGui::MenuItem("Audio Source")) {
                    entity.add_component<engine::AudioSourceComponent>();
                    ImGui::CloseCurrentPopup();
                }
            }
            ImGui::EndPopup();
        }
    }
    #endif
}
