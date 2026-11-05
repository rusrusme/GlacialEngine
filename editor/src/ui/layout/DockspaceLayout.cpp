//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "DockspaceLayout.hpp"

namespace editor {
    void DockspaceLayout::on_init(SystemContext& context) {
        UIComponent::on_init(context);
    }

    void DockspaceLayout::on_render_ui(SystemContext& context) {
        constexpr ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_PassthruCentralNode;
        const ImGuiID dockspace_id = ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport(), dockspace_flags);

        const ImGuiDockNode* node = ImGui::DockBuilderGetNode(dockspace_id);

        if (node == nullptr || node->ChildNodes[0] == nullptr || reset_layout_) {
            reset_layout_ = false;

            ImGui::DockBuilderRemoveNode(dockspace_id);
            ImGui::DockBuilderAddNode(dockspace_id, dockspace_flags | ImGuiDockNodeFlags_DockSpace);
            ImGui::DockBuilderSetNodeSize(dockspace_id, ImGui::GetMainViewport()->WorkSize);

            ImGuiID dock_main_id = dockspace_id;

            ImGuiID dock_left_bottom_id;
            ImGuiID dock_bottom_id = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Down, 0.25f, nullptr, &dock_main_id);
            ImGuiID dock_right_id = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Right, 0.22f, nullptr, &dock_main_id);
            ImGuiID dock_left_id = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Left, 0.18f, nullptr, &dock_main_id);
            ImGuiID dock_left_top_id = ImGui::DockBuilderSplitNode(dock_left_id, ImGuiDir_Up, 0.40f, nullptr, &dock_left_bottom_id);

            ImGui::DockBuilderDockWindow("Asset Browser", dock_left_top_id);
            ImGui::DockBuilderDockWindow("Hierarchy", dock_left_bottom_id);
            ImGui::DockBuilderDockWindow("Inspector", dock_right_id);
            ImGui::DockBuilderDockWindow("Console", dock_bottom_id);
            ImGui::DockBuilderDockWindow("Scene", dock_main_id);

            ImGuiID float_node_id = ImGui::DockBuilderAddNode();
            ImGuiViewport* viewport = ImGui::GetMainViewport();
            ImGui::DockBuilderSetNodePos(float_node_id, ImVec2(viewport->Pos.x + 100.0f, viewport->Pos.y + 100.0f));
            ImGui::DockBuilderSetNodeSize(float_node_id, ImVec2(400.0f, 500.0f));

            ImGui::DockBuilderDockWindow("Debug Panel", float_node_id);
            ImGui::DockBuilderFinish(float_node_id);

            ImGui::DockBuilderFinish(dockspace_id);
        }
    }

    void DockspaceLayout::load_config() {
        UIComponent::load_config();
    }

    void DockspaceLayout::save_config() {
        UIComponent::save_config();
    }
}
