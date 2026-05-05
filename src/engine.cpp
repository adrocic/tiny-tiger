#include "engine.h"

#include <emscripten.h>
#include <emscripten/html5.h>

#include <set>
#include <string>

static const int number_of_trackable_mouse_buttons = 8;
static const float initial_delta_time_estimate_in_seconds = 0.016f;

static Engine* global_engine_pointer = nullptr;

static std::set<std::string> currently_held_keys_set;
static std::set<std::string> keys_pressed_in_current_frame;
static std::set<std::string> keys_released_in_current_frame;

static float current_mouse_cursor_x_position = 0.0f;
static float current_mouse_cursor_y_position = 0.0f;
static bool mouse_button_is_currently_held_array[number_of_trackable_mouse_buttons] = {};
static bool mouse_button_was_just_pressed_array[number_of_trackable_mouse_buttons] = {};
static bool mouse_button_was_just_released_array[number_of_trackable_mouse_buttons] = {};

EM_JS(float, js_get_canvas_left_edge_position, (), {
    const canvasElement = document.getElementById('gameCanvas');
    if (!canvasElement) return 0;
    return canvasElement.getBoundingClientRect().left;
});

EM_JS(float, js_get_canvas_top_edge_position, (), {
    const canvasElement = document.getElementById('gameCanvas');
    if (!canvasElement) return 0;
    return canvasElement.getBoundingClientRect().top;
});

static EM_BOOL handle_keydown_event(int event_type, const EmscriptenKeyboardEvent* keyboard_event_data, void* user_data) {
    std::string pressed_key_name = keyboard_event_data->key;
    bool key_was_already_held = currently_held_keys_set.count(pressed_key_name) > 0;
    if (!key_was_already_held) {
        keys_pressed_in_current_frame.insert(pressed_key_name);
    }
    currently_held_keys_set.insert(pressed_key_name);
    return EM_TRUE;
}

static EM_BOOL handle_keyup_event(int event_type, const EmscriptenKeyboardEvent* keyboard_event_data, void* user_data) {
    std::string released_key_name = keyboard_event_data->key;
    currently_held_keys_set.erase(released_key_name);
    keys_released_in_current_frame.insert(released_key_name);
    return EM_TRUE;
}

static EM_BOOL handle_mousemove_event(int event_type, const EmscriptenMouseEvent* mouse_event_data, void* user_data) {
    float canvas_left_edge = js_get_canvas_left_edge_position();
    float canvas_top_edge = js_get_canvas_top_edge_position();
    current_mouse_cursor_x_position = (float)mouse_event_data->clientX - canvas_left_edge;
    current_mouse_cursor_y_position = (float)mouse_event_data->clientY - canvas_top_edge;
    return EM_TRUE;
}

static EM_BOOL handle_mousedown_event(int event_type, const EmscriptenMouseEvent* mouse_event_data, void* user_data) {
    int pressed_button_index = mouse_event_data->button;
    if (pressed_button_index >= 0 && pressed_button_index < number_of_trackable_mouse_buttons) {
        mouse_button_is_currently_held_array[pressed_button_index] = true;
        mouse_button_was_just_pressed_array[pressed_button_index] = true;
    }
    return EM_TRUE;
}

static EM_BOOL handle_mouseup_event(int event_type, const EmscriptenMouseEvent* mouse_event_data, void* user_data) {
    int released_button_index = mouse_event_data->button;
    if (released_button_index >= 0 && released_button_index < number_of_trackable_mouse_buttons) {
        mouse_button_is_currently_held_array[released_button_index] = false;
        mouse_button_was_just_released_array[released_button_index] = true;
    }
    return EM_TRUE;
}

static void global_main_loop_step_callback() {
    if (global_engine_pointer != nullptr) {
        global_engine_pointer->execute_one_game_loop_iteration();
    }
}

EM_JS(void, js_initialize_canvas, (int canvas_width, int canvas_height), {
    let canvasElement = document.getElementById('gameCanvas');
    if (!canvasElement) {
        canvasElement = document.createElement('canvas');
        canvasElement.id = 'gameCanvas';
        document.body.appendChild(canvasElement);
    }
    canvasElement.width = canvas_width;
    canvasElement.height = canvas_height;
    window._tinyTigerCanvasContext = canvasElement.getContext('2d');
});

EM_JS(void, js_clear_canvas_with_color, (int red, int green, int blue, int alpha), {
    const canvasContext = window._tinyTigerCanvasContext;
    canvasContext.fillStyle = `rgba(${red}, ${green}, ${blue}, ${alpha / 255.0})`;
    canvasContext.fillRect(0, 0, canvasContext.canvas.width, canvasContext.canvas.height);
});

EM_JS(void, js_draw_filled_rectangle, (float x_pos, float y_pos, float rect_width, float rect_height, int red, int green, int blue, int alpha), {
    const canvasContext = window._tinyTigerCanvasContext;
    canvasContext.fillStyle = `rgba(${red}, ${green}, ${blue}, ${alpha / 255.0})`;
    canvasContext.fillRect(x_pos, y_pos, rect_width, rect_height);
});

EM_JS(void, js_draw_outlined_rectangle, (float x_pos, float y_pos, float rect_width, float rect_height, int red, int green, int blue, int alpha, float thickness), {
    const canvasContext = window._tinyTigerCanvasContext;
    canvasContext.strokeStyle = `rgba(${red}, ${green}, ${blue}, ${alpha / 255.0})`;
    canvasContext.lineWidth = thickness;
    canvasContext.strokeRect(x_pos, y_pos, rect_width, rect_height);
});

EM_JS(void, js_draw_filled_circle, (float center_x, float center_y, float radius, int red, int green, int blue, int alpha), {
    const canvasContext = window._tinyTigerCanvasContext;
    canvasContext.beginPath();
    canvasContext.arc(center_x, center_y, radius, 0, Math.PI * 2);
    canvasContext.fillStyle = `rgba(${red}, ${green}, ${blue}, ${alpha / 255.0})`;
    canvasContext.fill();
});

EM_JS(void, js_draw_outlined_circle, (float center_x, float center_y, float radius, int red, int green, int blue, int alpha, float thickness), {
    const canvasContext = window._tinyTigerCanvasContext;
    canvasContext.beginPath();
    canvasContext.arc(center_x, center_y, radius, 0, Math.PI * 2);
    canvasContext.strokeStyle = `rgba(${red}, ${green}, ${blue}, ${alpha / 255.0})`;
    canvasContext.lineWidth = thickness;
    canvasContext.stroke();
});

EM_JS(void, js_draw_line_segment, (float x1, float y1, float x2, float y2, int red, int green, int blue, int alpha, float thickness), {
    const canvasContext = window._tinyTigerCanvasContext;
    canvasContext.beginPath();
    canvasContext.moveTo(x1, y1);
    canvasContext.lineTo(x2, y2);
    canvasContext.strokeStyle = `rgba(${red}, ${green}, ${blue}, ${alpha / 255.0})`;
    canvasContext.lineWidth = thickness;
    canvasContext.stroke();
});

EM_JS(void, js_draw_text_string, (const char* text_chars_ptr, float x_pos, float y_pos, float font_size, int red, int green, int blue, int alpha), {
    const canvasContext = window._tinyTigerCanvasContext;
    const text_string_value = UTF8ToString(text_chars_ptr);
    canvasContext.font = `${font_size}px sans-serif`;
    canvasContext.fillStyle = `rgba(${red}, ${green}, ${blue}, ${alpha / 255.0})`;
    canvasContext.fillText(text_string_value, x_pos, y_pos);
});

EM_JS(void, js_begin_path, (), {
    window._tinyTigerCanvasContext.beginPath();
});

EM_JS(void, js_move_to_position, (float x_pos, float y_pos), {
    window._tinyTigerCanvasContext.moveTo(x_pos, y_pos);
});

EM_JS(void, js_line_to_position, (float x_pos, float y_pos), {
    window._tinyTigerCanvasContext.lineTo(x_pos, y_pos);
});

EM_JS(void, js_stroke_current_path, (int red, int green, int blue, int alpha, float thickness), {
    const canvasContext = window._tinyTigerCanvasContext;
    canvasContext.strokeStyle = `rgba(${red}, ${green}, ${blue}, ${alpha / 255.0})`;
    canvasContext.lineWidth = thickness;
    canvasContext.stroke();
});

EM_JS(void, js_fill_current_path, (int red, int green, int blue, int alpha), {
    const canvasContext = window._tinyTigerCanvasContext;
    canvasContext.fillStyle = `rgba(${red}, ${green}, ${blue}, ${alpha / 255.0})`;
    canvasContext.fill();
});

EM_JS(void, js_save_canvas_state, (), {
    window._tinyTigerCanvasContext.save();
});

EM_JS(void, js_restore_canvas_state, (), {
    window._tinyTigerCanvasContext.restore();
});

EM_JS(void, js_apply_translation, (float x_offset, float y_offset), {
    window._tinyTigerCanvasContext.translate(x_offset, y_offset);
});

EM_JS(void, js_apply_rotation, (float angle_in_radians), {
    window._tinyTigerCanvasContext.rotate(angle_in_radians);
});

EM_JS(void, js_apply_scaling, (float x_scale, float y_scale), {
    window._tinyTigerCanvasContext.scale(x_scale, y_scale);
});

EM_JS(void, js_set_global_alpha, (float alpha_value), {
    window._tinyTigerCanvasContext.globalAlpha = alpha_value;
});

Renderer::Renderer(int width, int height) {
    canvas_width_in_pixels = width;
    canvas_height_in_pixels = height;
    js_initialize_canvas(width, height);
}

void Renderer::clear_screen(Color background_color) {
    js_clear_canvas_with_color(
        background_color.red_channel_value,
        background_color.green_channel_value,
        background_color.blue_channel_value,
        background_color.alpha_channel_value
    );
}

void Renderer::draw_filled_rectangle(float x_position, float y_position, float rectangle_width, float rectangle_height, Color fill_color) {
    js_draw_filled_rectangle(
        x_position, y_position, rectangle_width, rectangle_height,
        fill_color.red_channel_value,
        fill_color.green_channel_value,
        fill_color.blue_channel_value,
        fill_color.alpha_channel_value
    );
}

void Renderer::draw_outlined_rectangle(float x_position, float y_position, float rectangle_width, float rectangle_height, Color border_color, float border_thickness) {
    js_draw_outlined_rectangle(
        x_position, y_position, rectangle_width, rectangle_height,
        border_color.red_channel_value,
        border_color.green_channel_value,
        border_color.blue_channel_value,
        border_color.alpha_channel_value,
        border_thickness
    );
}

void Renderer::draw_filled_circle(float center_x_position, float center_y_position, float circle_radius, Color fill_color) {
    js_draw_filled_circle(
        center_x_position, center_y_position, circle_radius,
        fill_color.red_channel_value,
        fill_color.green_channel_value,
        fill_color.blue_channel_value,
        fill_color.alpha_channel_value
    );
}

void Renderer::draw_outlined_circle(float center_x_position, float center_y_position, float circle_radius, Color border_color, float border_thickness) {
    js_draw_outlined_circle(
        center_x_position, center_y_position, circle_radius,
        border_color.red_channel_value,
        border_color.green_channel_value,
        border_color.blue_channel_value,
        border_color.alpha_channel_value,
        border_thickness
    );
}

void Renderer::draw_line(float start_x_position, float start_y_position, float end_x_position, float end_y_position, Color line_color, float line_thickness) {
    js_draw_line_segment(
        start_x_position, start_y_position,
        end_x_position, end_y_position,
        line_color.red_channel_value,
        line_color.green_channel_value,
        line_color.blue_channel_value,
        line_color.alpha_channel_value,
        line_thickness
    );
}

void Renderer::draw_text_string(std::string text_to_display, float x_position, float y_position, float font_size_in_pixels, Color text_color) {
    js_draw_text_string(
        text_to_display.c_str(),
        x_position, y_position, font_size_in_pixels,
        text_color.red_channel_value,
        text_color.green_channel_value,
        text_color.blue_channel_value,
        text_color.alpha_channel_value
    );
}

void Renderer::begin_path_drawing() {
    js_begin_path();
}

void Renderer::move_path_to(float x_position, float y_position) {
    js_move_to_position(x_position, y_position);
}

void Renderer::draw_line_to(float x_position, float y_position) {
    js_line_to_position(x_position, y_position);
}

void Renderer::stroke_current_path(Color stroke_color, float stroke_width) {
    js_stroke_current_path(
        stroke_color.red_channel_value,
        stroke_color.green_channel_value,
        stroke_color.blue_channel_value,
        stroke_color.alpha_channel_value,
        stroke_width
    );
}

void Renderer::fill_current_path(Color fill_color) {
    js_fill_current_path(
        fill_color.red_channel_value,
        fill_color.green_channel_value,
        fill_color.blue_channel_value,
        fill_color.alpha_channel_value
    );
}

void Renderer::save_drawing_state() {
    js_save_canvas_state();
}

void Renderer::restore_drawing_state() {
    js_restore_canvas_state();
}

void Renderer::apply_translation(float x_offset, float y_offset) {
    js_apply_translation(x_offset, y_offset);
}

void Renderer::apply_rotation(float rotation_angle_in_radians) {
    js_apply_rotation(rotation_angle_in_radians);
}

void Renderer::apply_scaling(float x_scale_factor, float y_scale_factor) {
    js_apply_scaling(x_scale_factor, y_scale_factor);
}

void Renderer::set_global_alpha_value(float alpha_value) {
    js_set_global_alpha(alpha_value);
}

bool KeyboardInput::is_key_currently_held(std::string key_name) {
    return currently_held_keys_set.count(key_name) > 0;
}

bool KeyboardInput::was_key_pressed_this_frame(std::string key_name) {
    return keys_pressed_in_current_frame.count(key_name) > 0;
}

bool KeyboardInput::was_key_released_this_frame(std::string key_name) {
    return keys_released_in_current_frame.count(key_name) > 0;
}

void KeyboardInput::clear_per_frame_key_state() {
    keys_pressed_in_current_frame.clear();
    keys_released_in_current_frame.clear();
}

float MouseInput::get_cursor_x_position() {
    return current_mouse_cursor_x_position;
}

float MouseInput::get_cursor_y_position() {
    return current_mouse_cursor_y_position;
}

bool MouseInput::is_mouse_button_held(int button_index) {
    if (button_index < 0 || button_index >= number_of_trackable_mouse_buttons) {
        return false;
    }
    return mouse_button_is_currently_held_array[button_index];
}

bool MouseInput::was_mouse_button_pressed_this_frame(int button_index) {
    if (button_index < 0 || button_index >= number_of_trackable_mouse_buttons) {
        return false;
    }
    return mouse_button_was_just_pressed_array[button_index];
}

bool MouseInput::was_mouse_button_released_this_frame(int button_index) {
    if (button_index < 0 || button_index >= number_of_trackable_mouse_buttons) {
        return false;
    }
    return mouse_button_was_just_released_array[button_index];
}

void MouseInput::clear_per_frame_mouse_button_state() {
    for (int button_index = 0; button_index < number_of_trackable_mouse_buttons; button_index++) {
        mouse_button_was_just_pressed_array[button_index] = false;
        mouse_button_was_just_released_array[button_index] = false;
    }
}

Engine::Engine(int canvas_width, int canvas_height)
    : on_update_callback_function(emscripten::val::undefined()),
      on_draw_callback_function(emscripten::val::undefined()) {

    renderer_instance = new Renderer(canvas_width, canvas_height);
    keyboard_input_handler = new KeyboardInput();
    mouse_input_handler = new MouseInput();
    engine_is_currently_running = false;
    time_of_last_frame_in_milliseconds = 0.0;
    delta_time_since_last_frame = initial_delta_time_estimate_in_seconds;

    emscripten_set_keydown_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, nullptr, EM_TRUE, handle_keydown_event);
    emscripten_set_keyup_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, nullptr, EM_TRUE, handle_keyup_event);
    emscripten_set_mousemove_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, nullptr, EM_TRUE, handle_mousemove_event);
    emscripten_set_mousedown_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, nullptr, EM_TRUE, handle_mousedown_event);
    emscripten_set_mouseup_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, nullptr, EM_TRUE, handle_mouseup_event);

    global_engine_pointer = this;
}

Engine::~Engine() {
    delete renderer_instance;
    delete keyboard_input_handler;
    delete mouse_input_handler;
    global_engine_pointer = nullptr;
}

void Engine::set_update_callback(emscripten::val update_callback_function) {
    on_update_callback_function = update_callback_function;
}

void Engine::set_draw_callback(emscripten::val draw_callback_function) {
    on_draw_callback_function = draw_callback_function;
}

void Engine::run() {
    engine_is_currently_running = true;
    time_of_last_frame_in_milliseconds = emscripten_get_now();
    emscripten_set_main_loop(global_main_loop_step_callback, 0, 0);
}

void Engine::stop() {
    engine_is_currently_running = false;
    emscripten_cancel_main_loop();
}

void Engine::execute_one_game_loop_iteration() {
    double current_time_in_milliseconds = emscripten_get_now();
    double elapsed_time_in_milliseconds = current_time_in_milliseconds - time_of_last_frame_in_milliseconds;
    delta_time_since_last_frame = (float)(elapsed_time_in_milliseconds / 1000.0);
    time_of_last_frame_in_milliseconds = current_time_in_milliseconds;

    bool update_callback_is_set = !on_update_callback_function.isUndefined() && !on_update_callback_function.isNull();
    if (update_callback_is_set) {
        on_update_callback_function(delta_time_since_last_frame);
    }

    bool draw_callback_is_set = !on_draw_callback_function.isUndefined() && !on_draw_callback_function.isNull();
    if (draw_callback_is_set) {
        on_draw_callback_function();
    }

    keyboard_input_handler->clear_per_frame_key_state();
    mouse_input_handler->clear_per_frame_mouse_button_state();
}
