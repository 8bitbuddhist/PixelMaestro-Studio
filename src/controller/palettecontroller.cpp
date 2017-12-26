#include "colorpresets.h"
#include "core/colors.h"
#include "palettecontroller.h"

namespace PixelMaestroStudio {
	PaletteController::PaletteController() {
		initialize_palettes();
	}

	/**
	 * Adds a palette to the list.
	 * @param name Palette name.
	 * @param colors Colors in the palette.
	 * @param num_colors Number of colors in the palette.
	 * @return New palette.
	 */
	PaletteController::Palette* PaletteController::add_palette(QString name, Colors::RGB* colors, uint8_t num_colors) {
		palettes_.push_back(Palette(name, colors, num_colors));
		return &palettes_[palettes_.size() - 1];
	}

	/**
	 * Searches for the palette in the PaletteControl.
	 * @param palette Palette to search for.
	 * @return Index of the palette, or -1 if not found.
	 */
	int PaletteController::find(Colors::RGB *search_palette) {
		for (uint16_t i = 0; i < palettes_.size(); i++) {
			PaletteController::Palette* palette = &palettes_.at(i);
			if (*palette == search_palette) {
				return i;
			}
		}

		return -1;
	}

	/**
	 * Returns the palette at the specified index.
	 * @param index Palette index.
	 * @return Palette.
	 */
	PaletteController::Palette* PaletteController::get_palette(uint8_t index) {
		return &palettes_[index];
	}

	/**
	 * Gets the palette with the specified name.
	 * @param name Palette name.
	 * @return Palette.
	 */
	PaletteController::Palette* PaletteController::get_palette(QString name) {
		for (uint8_t i = 0; i < palettes_.size(); i++) {
			if (palettes_[i].name == name) {
				return &palettes_[i];
			}
		}
		return nullptr;
	}

	/**
	 * Gets the full list of palettes.
	 * @return Palette list.
	 */
	std::vector<PaletteController::Palette>* PaletteController::get_palettes() {
		return &palettes_;
	}

	/**
	 * Resets the palette list to defaults.
	 */
	void PaletteController::initialize_palettes() {
		uint8_t num_colors = 14;
		Colors::RGB colors[num_colors];

		palettes_.clear();

		Colors::generate_scaling_color_array(colors, &ColorPresets::RED, &ColorPresets::YELLOW, num_colors, true);
		palettes_.push_back(Palette("Fire", colors, num_colors));

		Colors::generate_scaling_color_array(colors, &ColorPresets::BLUE, &ColorPresets::GREEN, num_colors, true);
		palettes_.push_back(Palette("Deep Sea", colors, num_colors));

		palettes_.push_back(Palette("Color Wheel", ColorPresets::COLORWHEEL, 12));
	}

	/**
	 * Removes the palette at the specified index.
	 * @param index Palette index.
	 */
	void PaletteController::remove_palette(uint8_t index) {
		palettes_.erase(palettes_.begin() + index);
	}
}