#include <emscripten/bind.h>
#include "engine.h"

using namespace emscripten;

EMSCRIPTEN_BINDINGS(tiny_tiger_engine_bindings) {

    class_<Color>("Color")
        .constructor<>()
        .constructor<int, int, int>()
        .constructor<int, int, int, int>()
        .property("redChannelValue", &Color::red_channel_value)
        .property("greenChannelValue", &Color::green_channel_value)
        .property("blueChannelValue", &Color::blue_channel_value)
        .property("alphaChannelValue", &Color::alpha_channel_value);

    class_<Vector2>("Vector2")
        .constructor<>()
        .constructor<float, float>()
        .property("xPosition", &Vector2::x_position)
        .property("yPosition", &Vector2::y_position)
        .function("add", &Vector2::add)
        .function("subtract", &Vector2::subtract)
        .function("scale", &Vector2::scale)
        .function("getLength", &Vector2::get_length);

    class_<Renderer>("Renderer")
        .constructor<int, int>()
        .property("canvasWidthInPixels", &Renderer::canvas_width_in_pixels)
        .property("canvasHeightInPixels", &Renderer::canvas_height_in_pixels)
        .function("clearScreen", &Renderer::clear_screen)
        .function("drawFilledRectangle", &Renderer::draw_filled_rectangle)
        .function("drawOutlinedRectangle", &Renderer::draw_outlined_rectangle)
        .function("drawFilledCircle", &Renderer::draw_filled_circle)
        .function("drawOutlinedCircle", &Renderer::draw_outlined_circle)
        .function("drawLine", &Renderer::draw_line)
        .function("drawTextString", &Renderer::draw_text_string)
        .function("beginPathDrawing", &Renderer::begin_path_drawing)
        .function("movePathTo", &Renderer::move_path_to)
        .function("drawLineTo", &Renderer::draw_line_to)
        .function("strokeCurrentPath", &Renderer::stroke_current_path)
        .function("fillCurrentPath", &Renderer::fill_current_path)
        .function("saveDrawingState", &Renderer::save_drawing_state)
        .function("restoreDrawingState", &Renderer::restore_drawing_state)
        .function("applyTranslation", &Renderer::apply_translation)
        .function("applyRotation", &Renderer::apply_rotation)
        .function("applyScaling", &Renderer::apply_scaling)
        .function("setGlobalAlphaValue", &Renderer::set_global_alpha_value);

    class_<KeyboardInput>("KeyboardInput")
        .function("isKeyCurrentlyHeld", &KeyboardInput::is_key_currently_held)
        .function("wasKeyPressedThisFrame", &KeyboardInput::was_key_pressed_this_frame)
        .function("wasKeyReleasedThisFrame", &KeyboardInput::was_key_released_this_frame);

    class_<MouseInput>("MouseInput")
        .function("getCursorXPosition", &MouseInput::get_cursor_x_position)
        .function("getCursorYPosition", &MouseInput::get_cursor_y_position)
        .function("isMouseButtonHeld", &MouseInput::is_mouse_button_held)
        .function("wasMouseButtonPressedThisFrame", &MouseInput::was_mouse_button_pressed_this_frame)
        .function("wasMouseButtonReleasedThisFrame", &MouseInput::was_mouse_button_released_this_frame);

    class_<Engine>("Engine")
        .constructor<int, int>()
        .property("deltaTimeSinceLastFrame", &Engine::delta_time_since_last_frame)
        .function("getRenderer", &Engine::get_renderer, allow_raw_pointers())
        .function("getKeyboardInput", &Engine::get_keyboard_input, allow_raw_pointers())
        .function("getMouseInput", &Engine::get_mouse_input, allow_raw_pointers())
        .function("run", &Engine::run)
        .function("stop", &Engine::stop)
        .function("setUpdateCallback", &Engine::set_update_callback)
        .function("setDrawCallback", &Engine::set_draw_callback);
}
