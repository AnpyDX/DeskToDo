#pragma once

#define CONFIG_TOML_PATH "./appdata/config.toml"
#define HISTORY_TOML_PATH "./appdata/history.toml"

#include <string>
#include <vector>
#include <ranges>
#include <fstream>
#include <stdexcept>
#include <toml.hpp>

struct Config {
    struct {
        std::string default_content;
    } settings;

    struct {
        struct {
            std::string background;
        } window;

        struct {
            std::string width;
            std::string border_radius;
            std::string background;
            std::string background_hover;
            std::string background_pressed;
        } scrollbar;

        struct {
            std::string font_size;
            std::string font_color;
            std::string border_radius;
            std::string background;
            std::string background_hover;
            std::string background_pressed;
        } button;

        struct {
            std::string font_size;
            std::string font_color;
            std::string font_color_completed;
            std::string border_radius;
            std::string background;
            std::string checker_empty_color_0;
            std::string checker_hover_color_0;
            std::string checker_hover_color_1;
            std::string checker_hover_color_2;
            std::string checker_done_color_0;
            std::string checker_done_color_1;
            std::string checker_done_color_2;
        } item;
    } theme;
};

struct TaskRecord {
    bool completed;
    std::string content;
};

/**
 * @brief Read config info from file (Singleton Pattern).
 */
static const Config& GetConfig() {
    static Config config {};
    static bool isLoaded = false;

    if (isLoaded) return config;

    auto data = toml::parse(CONFIG_TOML_PATH);

    std::vector<std::pair<std::string_view, std::string&>> attributes {
        { "settings.default-content", config.settings.default_content },

        { "theme.window.background", config.theme.window.background },
        
        { "theme.scrollbar.width", config.theme.scrollbar.width },
        { "theme.scrollbar.border-radius", config.theme.scrollbar.border_radius },
        { "theme.scrollbar.background", config.theme.scrollbar.background },
        { "theme.scrollbar.background-hover", config.theme.scrollbar.background_hover },
        { "theme.scrollbar.background-pressed", config.theme.scrollbar.background_pressed },

        { "theme.button.font-size", config.theme.button.font_size },
        { "theme.button.font-color", config.theme.button.font_color },
        { "theme.button.border-radius", config.theme.button.border_radius },
        { "theme.button.background", config.theme.button.background },
        { "theme.button.background-hover", config.theme.button.background_hover },
        { "theme.button.background-pressed", config.theme.button.background_pressed },

        { "theme.item.font-size", config.theme.item.font_size },
        { "theme.item.font-color", config.theme.item.font_color },
        { "theme.item.font-color-completed", config.theme.item.font_color_completed },
        { "theme.item.border-radius", config.theme.item.border_radius },
        { "theme.item.background", config.theme.item.background },
        { "theme.item.checker-empty-color-0", config.theme.item.checker_empty_color_0 },
        { "theme.item.checker-hover-color-0", config.theme.item.checker_hover_color_0 },
        { "theme.item.checker-hover-color-1", config.theme.item.checker_hover_color_1 },
        { "theme.item.checker-hover-color-2", config.theme.item.checker_hover_color_2 },
        { "theme.item.checker-done-color-0", config.theme.item.checker_done_color_0 },
        { "theme.item.checker-done-color-1", config.theme.item.checker_done_color_1},
        { "theme.item.checker-done-color-2", config.theme.item.checker_done_color_2 },
    };

    for (auto& [name, attr] : attributes) {
        auto parts = name | std::ranges::views::split('.');
        auto* value = &data;
        for (auto&& key : parts) {
            value = &value->at({key.begin(), key.end()});
            if (value->is_string()) {
                attr = value->as_string();
                break;
            }
        }
    }

    isLoaded = true;

    return config;
}

/**
 * @brief Load tasks from local history file.
 */
static std::vector<TaskRecord> GetHistory() {
    auto data = toml::parse(HISTORY_TOML_PATH);
    auto& records = data.at("tasks").as_array();

    std::vector<TaskRecord> result {};
    for (auto& rec : records) {
        auto& task = rec.as_table();
        result.emplace_back(
            task["completed"].as_boolean(),
            task["content"].as_string()
        );
    }

    return result;
}

/**
 * @brief Save tasks into local history file. 
 */
static void SaveHistory(const std::vector<TaskRecord>& records) {
    std::ofstream file(HISTORY_TOML_PATH, std::ios::out | std::ios::trunc);

    if (!file.is_open()) {
        throw std::runtime_error(std::format("failed to open history.toml ({})", HISTORY_TOML_PATH));
    }

    for (auto task : records) {
        file << "[[tasks]]\n";
        file << "completed = " << (task.completed ? "true\n" : "false\n");
        file << "content = \"" << task.content << "\"\n\n";
    }

    file.close();
}