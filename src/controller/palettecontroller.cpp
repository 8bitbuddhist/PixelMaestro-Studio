#include <QSettings>
#include <QStringList>
#include "colorpresets.h"
#include "core/colors.h"
#include "dialog/preferencesdialog.h"
#include "palettecontroller.h"

namespace PixelMaestroStudio {
	PaletteController::PaletteController() {
		load_palettes();
	}

	/**
	 * Adds a palette to the list.
	 * @param name Palette name.
	 * @param colors Colors in the palette.
	 * @param num_colors Number of colors in the palette.
	 * @param type The type of Palette.
	 * @param mirror Whether to mirror the palette.
	 * @return New palette.
	 */
	PaletteController::PaletteWrapper* PaletteController::add_palette(QString name, Colors::RGB* colors, uint8_t num_colors, PaletteType type, const Colors::RGB& base_color, const Colors::RGB& target_color, bool mirror) {
		palettes_.emplace_back(PaletteWrapper(name, colors, num_colors, type, base_color, target_color, mirror));
		return &palettes_[palettes_.size() - 1];
	}

	/**
	 * Checks the given string against the existing list of Palettes.
	 * If a Palette is already using the name, a number is appended to the end of the name.
	 * @param name Name to test.
	 * @return Same name if the name is unused, or a new name if it's in use.
	 */
	QString PaletteController::check_palette_name(QString name) {
		int iteration = 0;
		while (get_palette(name) != nullptr) {
			int trail = iteration + 2;
			if (iteration == 0) {
				name = name.append(" #" + QString::number(trail));
			}
			else {
				name = name.replace(name.length() - 1, 1, QString::number(trail));
			}
			iteration++;
		}

		return name;
	}

	/**
	 * Converts a serialized color into an RGB object.
	 * @param string Serialized color.
	 * @return Deserialized color.
	 */
	Colors::RGB PaletteController::deserialize_color(const QString& string) {
		QStringList values = string.split(PreferencesDialog::sub_delimiter);

		if (values.size() < 3) return ColorPresets::Black;
		return Colors::RGB(values.at(0).toInt(), values.at(1).toInt(), values.at(2).toInt());
	}

	/**
	 * Searches for the palette in the PaletteControl.
	 * @param palette Palette to search for.
	 * @return Index of the palette, or -1 if not found.
	 */
	int PaletteController::find(Colors::RGB *search_palette) {
		for (uint16_t i = 0; i < palettes_.size(); i++) {
			PaletteController::PaletteWrapper* palette = &palettes_.at(i);
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
	PaletteController::PaletteWrapper* PaletteController::get_palette(uint8_t index) {
		return &palettes_[index];
	}

	/**
	 * Gets the palette with the specified name.
	 * @param name Palette name.
	 * @return Palette.
	 */
	PaletteController::PaletteWrapper* PaletteController::get_palette(const QString& name) {
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
	std::vector<PaletteController::PaletteWrapper>* PaletteController::get_palettes() {
		return &palettes_;
	}

	/**
	 * Resets the palette list to defaults.
	 */
	void PaletteController::initialize_palettes() {
		uint8_t num_colors = 14;
		Colors::RGB colors[num_colors];

		palettes_.clear();

		Colors::generate_scaling_color_array(&colors[0], &ColorPresets::Red, &ColorPresets::Yellow, num_colors, true);
		palettes_.emplace_back(PaletteWrapper("Fire", &colors[0], num_colors, PaletteType::Scaling, ColorPresets::Red, ColorPresets::Yellow, true));

		Colors::generate_scaling_color_array(&colors[0], &ColorPresets::Blue, &ColorPresets::Green, num_colors, true);
		palettes_.emplace_back(PaletteWrapper("Deep Sea", &colors[0], num_colors, PaletteType::Scaling, ColorPresets::Blue, ColorPresets::Green, true));

		palettes_.emplace_back(PaletteWrapper("Color Wheel", &ColorPresets::Colorwheel[0], 12, PaletteType::Random, Colors::RGB(0, 0, 0), Colors::RGB(0, 0, 0), false));
	}

	/**
	 * Loads custom palettes from settings
	 */
	void PaletteController::load_palettes() {
		QSettings settings;

		/*
		 * Check if settings contains stored Palettes.
		 * If not, initialize a new set of Palettes.
		 */
		if (!settings.childGroups().contains(PreferencesDialog::palettes) || settings.value(PreferencesDialog::save_session) == false) {
			initialize_palettes();
			return;
		}

		int num_palettes = settings.beginReadArray(PreferencesDialog::palettes);
		for (int palette_index = 0; palette_index < num_palettes; palette_index++) {
			settings.setArrayIndex(palette_index);

			Colors::RGB base_color = deserialize_color(settings.value(PreferencesDialog::palette_base_color).toString());
			bool mirror = settings.value(PreferencesDialog::palette_mirror, false).toBool();
			QString name = settings.value(PreferencesDialog::palette_name, QString("Custom Palette") + QString::number(palette_index + 1)).toString();
			int num_colors = settings.value(PreferencesDialog::palette_num_colors, 0).toInt();
			Colors::RGB target_color = deserialize_color(settings.value(PreferencesDialog::palette_target_color).toString());
			PaletteType type = (PaletteType)settings.value(PreferencesDialog::palette_type, 0).toInt();

			// Build color array
			QString color_string = settings.value(PreferencesDialog::palette_colors).toString();
			QVector<Colors::RGB> color_array;
			QStringList color_string_list = color_string.split(PreferencesDialog::delimiter);
			for (const QString& color_string : color_string_list) {
				color_array.push_back(deserialize_color(color_string));
			}

			palettes_.emplace_back(PaletteWrapper(name, color_array.data(), num_colors, type, base_color, target_color, mirror));
		}
		settings.endArray();
	}

	/**
	 * Removes the palette at the specified index.
	 * @param index Palette index.
	 */
	void PaletteController::remove_palette(uint8_t index) {
		palettes_.erase(palettes_.begin() + index);
	}

	/**
	 * Saves Palettes to settings.
	 */
	void PaletteController::save_palettes() {
		QSettings settings;
		settings.beginWriteArray(PreferencesDialog::palettes);
		for (uint16_t i = 0; i < palettes_.size(); i++) {
			settings.setArrayIndex(i);

			PaletteWrapper* palette_wrapper = &palettes_.at(i);

			// Set color array
			QStringList color_list;
			Colors::RGB* colors = palette_wrapper->palette.get_colors();
			for (int color_index = 0; color_index < palette_wrapper->palette.get_num_colors(); color_index++) {
				color_list.append(serialize_color(&colors[color_index]));
			};
			settings.setValue(PreferencesDialog::palette_colors, color_list.join(PreferencesDialog::delimiter));

			settings.setValue(PreferencesDialog::palette_base_color, serialize_color(&palette_wrapper->base_color));
			settings.setValue(PreferencesDialog::palette_mirror, palette_wrapper->mirror);
			settings.setValue(PreferencesDialog::palette_name, palette_wrapper->name);
			settings.setValue(PreferencesDialog::palette_num_colors, palette_wrapper->palette.get_num_colors());
			settings.setValue(PreferencesDialog::palette_target_color, serialize_color(&palette_wrapper->target_color));
			settings.setValue(PreferencesDialog::palette_type, (uint8_t)palette_wrapper->type);
		}
		settings.endArray();
	}

	/**
	 * Converts a color into a string for serialization.
	 * @param color Color to serialize.
	 * @return Serialized string.
	 */
	QString PaletteController::serialize_color(Colors::RGB *color) {
		return QString::number(color->r) + PreferencesDialog::sub_delimiter +
				QString::number(color->g) + PreferencesDialog::sub_delimiter +
				QString::number(color->b);
	}

	PaletteController::~PaletteController() {
		// If the user chose to save their session, save Palettes to settings
		QSettings settings;
		if (settings.value(PreferencesDialog::save_session) == true) {
			save_palettes();
		}
	}
}
