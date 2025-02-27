#include "imgui.h"
#include <iostream>
#include <vector>
#include <sstream>
#include "./Species.h"
#include "./Numerical.h"
#include "./settings.h"


using namespace std;

void config_board_size_species() {
    int board_width_slider = board_width;
    if (ImGui::SliderInt("Board Width", &board_width_slider, 1, BOARD_LIMIT)) {
        if (board_width_slider > board_width) {
            // add board_width_slider - board_width els to every arr
            for (auto & i : board) {
                for (int j = 0; j < board_width_slider-board_width; j++) {
                    i.emplace_back(15,0);
                }
            }
        } else {
            // remove board_width - board_width_slider elements
            for (auto & i : board) {
                for (int j = 0; j < board_width-board_width_slider; j++) {
                    i.pop_back();
                }
            }
        }
        board_width = board_width_slider;
    }
    int board_height_slider = board_height;
    if (ImGui::SliderInt("Board Height", &board_height_slider, 1, BOARD_LIMIT)) {
        if (board_height_slider > board_height) {
            // add board_height_slider - board_height rows
            for (int j = 0; j < board_height_slider-board_height; j++) {
                board.emplace_back(board_width, vector<int>(15,0));
            }
        } else {
            // remove board_height - board_height_slider rows
            for (int j = 0; j < board_height_slider-board_height; j++) {
                board.pop_back();
            }
        }
        board_height = board_height_slider;
    }

    u_int counter = species.size();
    ImGui::BeginDisabled(counter < 2); // cant get below 1
    if (ImGui::Button("-")) {
        species.pop_back();  // Decrement the counter when the minus button is clicked
    }
    ImGui::EndDisabled();

    ImGui::SameLine();  // Keep the counter value on the same line as the buttons
    ImGui::Text("%d", counter);
    ImGui::SameLine();  // Keep the plus button on the same line
    ImGui::BeginDisabled(counter > 14); // cant get below 1
    if (ImGui::Button("+")) {
        species.push_back(new Species("specie " + std::to_string(counter+1), colors[counter]));  // Increment the counter when the plus button is clicked
    }
    ImGui::EndDisabled();
    ImGui::SameLine();
    ImGui::Text("Number of Species");
}

void config_species_list() {
    for (int i = 0; i < species.size(); i++) {
        // TODO, check to prevent string getting deleted, problem is the last character
        ImGui::PushStyleColor(ImGuiCol_Text, HexToImVec4(colors[i]));
        ImGui::InputText(("##species"+to_string(i)).c_str(), species[i]->name.data(), 128);
        ImGui::PopStyleColor(1);
    }
}

void config_dynamics() {
    ImGui::Text("How does 'column' species affect 'color' one");
    ImGuiStyle& style = ImGui::GetStyle();
    style.CellPadding = ImVec2(1, 1);  // Remove padding between cells

    // Begin a table with species.size() columns, no padding or borders between columns
    if (ImGui::BeginTable("Grid Table", species.size()+1, ImGuiTableFlags_SizingStretchSame | ImGuiTableFlags_NoPadInnerX | ImGuiTableFlags_NoBordersInBody)) {
        ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(1, 1));  // Ensure no padding between cells

        for (int i = 0; i < species.size(); i++) {
            ImGui::TableSetupColumn(species[i]->name.c_str());  // Use species name as column header
        }
        ImGui::TableSetupColumn("Dispersion");
        ImGui::TableHeadersRow();


        for (int i = 0; i < species.size(); i++) {
            ImGui::TableNextRow();
            ImGui::PushStyleColor(ImGuiCol_Text, HexToImVec4(colors[i]));

            for (int j = 0; j < species.size(); j++) {
                ImGui::TableSetColumnIndex(j);  // Move to the correct column

                ImGui::PushItemWidth(-1);
                ImGui::InputFloat(("##hidden"+to_string(i*species.size()+j)).c_str(), &coefficients[i][j], 0.0f, 0.0f, "%.2f", ImGuiInputTextFlags_CharsDecimal);
                ImGui::PopItemWidth();
            }

            ImGui::TableSetColumnIndex(species.size());  // Move to the correct column
            ImGui::PushItemWidth(-1);

            // input dispersion limited in range between [0:0.5]
            ImGui::InputFloat(("##hidden"+to_string(i*species.size())+"dispersion").c_str(), &dispersion_coefficients[i], 0.0f, 0.0f, "%.5f", ImGuiInputTextFlags_CharsDecimal);
            dispersion_coefficients[i] = std::clamp(dispersion_coefficients[i], 0.0f, 0.4f);

            ImGui::PopItemWidth();
            ImGui::PopStyleColor(1);
        }

        ImGui::PopStyleVar();  // Restore default padding settings
        ImGui::EndTable();
    }

    // TODO set max 2000 for later
    // set min = 1 and max = 2000
    ImGui::InputInt("Timesteps", &number_steps_t);
    number_steps_t = std::clamp(number_steps_t, 1, 2000);
    if (ImGui::Button("Simulate")) {
        prepareCalculations();
        current = SIMULATION;
    }
}

void board_render() {
    // width height == 500
    double size_x = (BOARD_WINDOW_WIDTH-65) / board_width;
    double size_y = (BOARD_WINDOW_WIDTH-65) / board_height;

    ImGuiStyle& style = ImGui::GetStyle();
    ImVec2 originalCellPadding = style.CellPadding;     // Save original cell padding
    // set bot to 0 to remove padding between rows
    style.CellPadding = ImVec2(1, 1);

    // ImGuiTableFlags_NoPadInnerX to remove padding between columns
    if (ImGui::BeginTable("Grid Table", board_width, ImGuiTableFlags_SizingFixedFit )) {
        ImVec2 buttonSize = ImVec2(size_x, size_y);  // Customize the button size here

        for (int row = 0; row < board_height; row++) {
            ImGui::TableNextRow();  // Move to the next row
            for (int col = 0; col < board_width; col++) {
                ImGui::TableSetColumnIndex(col);  // Set the current column

                ostringstream oss;

                // if (selected_box != -1 && row == selected_box/board_width && col == selected_box%board_width) {
                    for (size_t i = 0; i < species.size(); ++i) {
                        oss << board[row][col][i];
                        if (i < species.size()-1) {
                            oss<<",";
                        }
                    }
                // }
                oss << " ##Button"+to_string(row * board_width + col);
                if (ImGui::Button(oss.str().c_str(), buttonSize)) {
                    selected_box = row*board_width+col;
                }

            }
        }
        ImGui::EndTable();
        ImGui::Spacing();
        int x = selected_box%board_width;
        int y = selected_box/board_width;
        if (selected_box != -1) {
            ImGui::Text(("Population inside square X: "
                    + to_string(x) + ", Y: "
                    + to_string(y)).c_str());
        } else {
            ImGui::Text("Population inside square");
        }
        ImGui::BeginDisabled(selected_box == -1);
        for (int i = 0; i < species.size(); i++) {
            ImGui::PushStyleColor(ImGuiCol_Text, HexToImVec4(colors[i]));
            // checks needed so that in case of resizing, doesnt crash
            if (selected_box != -1 && y < board.size() && x < board[0].size()) {
                ImGui::InputInt(species[i]->name.c_str(), &board[y][x][i]);
            } else {
                int myInt = 0;
                ImGui::InputInt(species[i]->name.c_str(), &myInt);
            }
            ImGui::PopStyleColor(1);
        }
        ImGui::EndDisabled();

    }
    style.CellPadding = originalCellPadding;

}

