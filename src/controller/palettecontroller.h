#ifndef PALETTECONTROLLER_H
#define PALETTECONTROLLER_H

#include "core/colors.h"
#include "core/palette.h"
#include <QString>
#include <QVector>

using namespace PixelMaestro;

namespace PixelMaestroStudio {
	class PaletteController {
		public:

			enum class PaletteType : uint8_t {
				Blank,
				Scaling,
				Random,
				Comet
			};

			/**
			 * Wrapper class for PixelMaestro Palettes.
			 */
			struct PaletteWrapper {
				Colors::RGB base_color;
				Colors::RGB target_color;
				bool mirror;
				QString name = "";
				uint8_t start = 0;
				uint8_t length = 0;

				PaletteType type = PaletteType::Blank;
				Palette palette;

				bool operator==(Colors::RGB* section_colors) {
					bool match = true;
					for (uint8_t i = 0; i < this->palette.get_num_colors(); i++) {
						if (this->palette.get_colors()[i] != section_colors[i]) {
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
				 * @param start For comets, where the body of the comet starts.
				 * @param length For comets, where the tail of the comet ends.
				 */
				PaletteWrapper(QString new_name, Colors::RGB new_colors[], uint8_t num_colors, PaletteType type, const Colors::RGB& base_color, const Colors::RGB& target_color, bool mirror, uint8_t start, uint8_t length) : palette(new_colors, num_colors) {
					this->base_color = base_color;
					this->target_color = target_color;
					this->mirror = mirror;
					this->name = new_name;
					this->type = type;
					this->start = start;
					this->length = length;
				}
			};

			PaletteController();
			~PaletteController();
			PaletteWrapper& add_palette(QString name, Colors::RGB colors[], uint8_t num_colors, PaletteType type, const Colors::RGB& base_color, const Colors::RGB& target_color, bool mirror = false, uint8_t start = 0, uint8_t length = 0);
			QString check_palette_name(QString name);
			Colors::RGB deserialize_color(const QString& string);
			int find(Colors::RGB* colors, int num_colors);
			PaletteWrapper& get_palette(uint8_t index);
			PaletteWrapper* get_palette(const QString& name);
			std::vector<PaletteWrapper>* get_palettes();
			void initialize_palettes();
			void load_palettes();
			void remove_palette(uint8_t index);
			void save_palettes();
			QString serialize_color(Colors::RGB& color);

		private:
			std::vector<PaletteWrapper> palettes_;	// Deliberately kept as std::vector instead of QVector
	};
}

#endif // PALETTECONTROLLER_H
