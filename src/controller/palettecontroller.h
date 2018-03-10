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
			/**
			 * Wrapper class for PixelMaestro Palettes.
			 */
			struct PaletteWrapper {
				QString name = "";
				QVector<Colors::RGB> colors;
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
				 */
				PaletteWrapper(QString new_name, Colors::RGB* new_colors, uint8_t num_colors) {
					this->name = new_name;
					for (uint8_t i = 0; i < num_colors; i++) {
						this->colors.push_back(new_colors[i]);
					}

					palette.set_colors(&colors[0], colors.size());
				}
			};

			PaletteController();
			PaletteWrapper* add_palette(QString name, Colors::RGB* colors, uint8_t num_colors);
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
