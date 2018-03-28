#ifndef PALETTECONTROLLER_H
#define PALETTECONTROLLER_H

#include "core/colors.h"
#include "core/palette.h"
#include <QString>
#include <QVector>
#include <vector>

using namespace PixelMaestro;

namespace PixelMaestroStudio {
	class PaletteController {
		public:

			enum class PaletteType : uint8_t {
				Blank,
				Scaling,
				Random
			};

			/**
			 * Wrapper class for PixelMaestro Palettes.
			 */
			struct PaletteWrapper {
				Colors::RGB base_color;
				Colors::RGB target_color;
				bool mirror;
				QString name = "";
				QVector<Colors::RGB> colors;
				PaletteType type = PaletteType::Blank;
				Palette palette = Palette(nullptr, 0, false);

				bool operator==(Colors::RGB* section_colors) {
					bool match = true;
					for (uint8_t i = 0; i < this->colors.size(); i++) {
						if (this->colors[i] != section_colors[i]) {
							match = false;
						}
					}
					return match;
				}

				/**
				 * Constructor.
				 * Provides storage for colors (which is why we pass colors directly instead of a Palette object.)
				 * @param new_name Palette name.
				 * @param new_colors Palette colors.
				 * @param num_colors Number of colors in the Palette.
				 * @param type The type of Palette.
				 * @param base_color The Palette's initial color.
				 * @param target_color The Palette's target color.
				 * @param mirror For scaling Palettes, whether to mirror the colors.
				 */
				PaletteWrapper(QString new_name, Colors::RGB* new_colors, uint8_t num_colors, PaletteType type, Colors::RGB base_color, Colors::RGB target_color, bool mirror) {
					this->base_color = base_color;
					this->target_color = target_color;
					this->mirror = mirror;
					this->name = new_name;
					this->type = type;
					for (uint8_t i = 0; i < num_colors; i++) {
						this->colors.push_back(new_colors[i]);
					}

					palette.set_colors(&colors[0], colors.size());
				}
			};

			PaletteController();
			PaletteWrapper* add_palette(QString name, Colors::RGB* colors, uint8_t num_colors, PaletteType type, Colors::RGB base_color, Colors::RGB target_color, bool mirror = false);
			int find(Colors::RGB* search_palette);
			PaletteWrapper* get_palette(uint8_t index);
			PaletteWrapper* get_palette(QString name);
			std::vector<PaletteWrapper>* get_palettes();
			void initialize_palettes();
			void remove_palette(uint8_t index);

		private:
			std::vector<PaletteWrapper> palettes_;
	};
}

#endif // PALETTECONTROLLER_H
