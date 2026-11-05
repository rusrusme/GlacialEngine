//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "ViewportPanel.hpp"

namespace editor {
    void ViewportPanel::on_render_ui(SystemContext& context) {
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{0.0f, 0.0f});
        ImGui::Begin("Scene");

        // TODO Add scene viewport
        //      Needs off-screen fbo rendering in RHI

        ImGui::End();
        ImGui::PopStyleVar();
    }
}
