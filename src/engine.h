#pragma once

#include <string>
#include <cmath>
#include <emscripten/val.h>

struct Color {
    int red_channel_value;
    int green_channel_value;
    int blue_channel_value;
    int alpha_channel_value;

    Color() {
        red_channel_value = 0;
        green_channel_value = 0;
        blue_channel_value = 0;
        alpha_channel_value = 255;
    }

    Color(int red, int green, int blue) {
        red_channel_value = red;
        green_channel_value = green;
        blue_channel_value = blue;
        alpha_channel_value = 255;
    }

    Color(int red, int green, int blue, int alpha) {
        red_channel_value = red;
        green_channel_value = green;
        blue_channel_value = blue;
        alpha_channel_value = alpha;
    }
};

struct Vector2 {
    float x_position;
    float y_position;

    Vector2() {
        x_position = 0.0f;
        y_position = 0.0f;
    }

    Vector2(float x, float y) {
        x_position = x;
        y_position = y;
    }

    Vector2 add(Vector2 other_vector) {
        Vector2 result_vector;
        result_vector.x_position = x_position + other_vector.x_position;
        result_vector.y_position = y_position + other_vector.y_position;
        return result_vector;
    }

    Vector2 subtract(Vector2 other_vector) {
        Vector2 result_vector;
        result_vector.x_position = x_position - other_vector.x_position;
        result_vector.y_position = y_position - other_vector.y_position;
        return result_vector;
    }

    Vector2 scale(float scale_amount) {
        Vector2 result_vector;
        result_vector.x_position = x_position * scale_amount;
        result_vector.y_position = y_position * scale_amount;
        return result_vector;
    }

    float get_length() {
        float length_squared = x_position * x_position + y_position * y_position;
        return std::sqrt(length_squared);
    }
};

class Renderer {
public:
    int canvas_width_in_pixels;
    int canvas_height_in_pixels;

    Renderer(int width, int height);

    void clear_screen(Color background_color);
    void draw_filled_rectangle(float x_position, float y_position, float rectangle_width, float rectangle_height, Color fill_color);
    void draw_outlined_rectangle(float x_position, float y_position, float rectangle_width, float rectangle_height, Color border_color, float border_thickness);
    void draw_filled_circle(float center_x_position, float center_y_position, float circle_radius, Color fill_color);
    void draw_outlined_circle(float center_x_position, float center_y_position, float circle_radius, Color border_color, float border_thickness);
    void draw_line(float start_x_position, float start_y_position, float end_x_position, float end_y_position, Color line_color, float line_thickness);
    void draw_text_string(std::string text_to_display, float x_position, float y_position, float font_size_in_pixels, Color text_color);
    void begin_path_drawing();
    void move_path_to(float x_position, float y_position);
    void draw_line_to(float x_position, float y_position);
    void stroke_current_path(Color stroke_color, float stroke_width);
    void fill_current_path(Color fill_color);
    void save_drawing_state();
    void restore_drawing_state();
    void apply_translation(float x_offset, float y_offset);
    void apply_rotation(float rotation_angle_in_radians);
    void apply_scaling(float x_scale_factor, float y_scale_factor);
    void set_global_alpha_value(float alpha_value);
};

class KeyboardInput {
public:
    bool is_key_currently_held(std::string key_name);
    bool was_key_pressed_this_frame(std::string key_name);
    bool was_key_released_this_frame(std::string key_name);
    void clear_per_frame_key_state();
};

class MouseInput {
public:
    float get_cursor_x_position();
    float get_cursor_y_position();
    bool is_mouse_button_held(int button_index);
    bool was_mouse_button_pressed_this_frame(int button_index);
    bool was_mouse_button_released_this_frame(int button_index);
    void clear_per_frame_mouse_button_state();
};

class Engine {
public:
    Renderer* renderer_instance;
    KeyboardInput* keyboard_input_handler;
    MouseInput* mouse_input_handler;
    bool engine_is_currently_running;
    double time_of_last_frame_in_milliseconds;
    float delta_time_since_last_frame;

    emscripten::val on_update_callback_function;
    emscripten::val on_draw_callback_function;

    Engine(int canvas_width, int canvas_height);
    ~Engine();

    Renderer* get_renderer() { return renderer_instance; }
    KeyboardInput* get_keyboard_input() { return keyboard_input_handler; }
    MouseInput* get_mouse_input() { return mouse_input_handler; }

    void run();
    void stop();
    void set_update_callback(emscripten::val update_callback_function);
    void set_draw_callback(emscripten::val draw_callback_function);
    void execute_one_game_loop_iteration();
};
