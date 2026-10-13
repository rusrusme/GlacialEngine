//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "ShaderSystemTest.hpp"

#include <cassert>

#include "core/Log.hpp"
#include "render/shader/ShaderLibrary.hpp"

void ShaderTestLayer::on_attach(engine::SystemContext& context) {
    engine::ShaderLibrary shader_lib;

    // 1. Test Loading a Shader
    const std::string shader_path = "assets/shaders/compiled/test/basic.bshb";
    auto basic_shader = shader_lib.load("Basic", shader_path);

    if (!basic_shader || basic_shader->renderer_id() == 0) {
        CORE_ERROR("Test Failed: Shader failed to load or create GL program!");
        return;
    }
    CORE_INFO("Shader loaded successfully with Program ID: {0}", basic_shader->renderer_id());

    // 2. Test Retrieval & Existence
    if (shader_lib.exists("Basic")) {
        auto retrieved = shader_lib.get("Basic");
        assert(retrieved == basic_shader && "Retrieved shader pointer mismatch!");
    }

    // 3. Test Binding in Render Loop
    basic_shader->bind();

    // Draw dummy mesh / triangle here
    // glDrawArrays(GL_TRIANGLES, 0, 3);

    basic_shader->unbind();

    // 4. Test Hot-Reloading (Normal Path)
    // Modify/recompile your .bshb file in the background, then call:
    if (shader_lib.reload("Basic")) {
        CORE_INFO("Reload succeeded. New Program ID: {0}", basic_shader->renderer_id());
    }

    // 5. Test Hot-Reload Fallback (Error Path)
    // Point to a corrupted file or simulate an invalid reload.
    // Ensure the old program ID remains active and the engine doesn't crash.
    shader_lib.reload_all();

    // 6. Test Unused Cleanup
    {
        // Use an existing path so load() succeeds
        auto temp_shader = shader_lib.load("Temp", shader_path);
    } // temp_shader goes out of scope; library is now sole owner (use_count == 1)

    shader_lib.clear_unused(); // Erases "Temp" from map
    assert(!shader_lib.exists("Temp") && "clear_unused failed to remove orphan shader!");
}

int main() {
    ShaderSystemTest test;
    test.run();
}
