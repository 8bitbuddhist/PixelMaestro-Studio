/*
 * CanvasUtility - Tools for managing Canvases.
 */

#include <QColor>
#include <QImage>
#include <QImageReader>
#include "canvasutility.h"
#include "canvas/canvas.h"
#include "core/point.h"

namespace PixelMaestroStudio {
	/**
	 * Loads an image into the Canvas.
	 * @param filename Image location.
	 * @param canvas Canvas to load the image into.
	 * @param maestro_control Optional MaestroControl for generating Cues.
	 */
	void CanvasUtility::load_image(QString filename, Canvas *canvas, MaestroControlWidget* maestro_control) {
		QImageReader image(filename, filename.right(3).toLocal8Bit());
		QSize canvas_size(canvas->get_section()->get_dimensions()->x, canvas->get_section()->get_dimensions()->y);
		image.setScaledSize(canvas_size);

		maestro_control->run_cue(
			maestro_control->canvas_handler->set_num_frames(
					maestro_control->get_section_index(),
					maestro_control->get_layer_index(),
					static_cast<uint16_t>(image.imageCount())));

		/*
		 * Handle animated images by iterating over each frame and extracting color values.
		 */
		if (image.imageCount() >= 1) {
			for (uint16_t i = 0; i < image.imageCount(); i++) {
				QImage frame = image.read();

				frame = frame.convertToFormat(QImage::Format_Indexed8);

				/*
				 * Extract the frame's color table.
				 * We do this for every frame to ensure each Pixel is assigned a color index, even though we only use the first frame's color table as the Palette.
				 * We also make sure the color table is at most 255 colors to avoid transparency issues.
				 */
				QVector<QRgb> color_table = frame.colorTable();
				while (color_table.size() >= 256) {
					color_table.removeLast();
				}

				// Handle one-time actions
				if (image.currentImageNumber() == 0) {

					// Set the frame rate
					maestro_control->run_cue(
						maestro_control->canvas_handler->set_frame_timer(
							maestro_control->get_section_index(),
							maestro_control->get_layer_index(),
							static_cast<uint16_t>(image.nextImageDelay())));

					// FIXME: Optimize this section
					// Copy the color table into a temporary RGB array so we can translate it into a Cue
					QVector<Colors::RGB> color_table_rgb(color_table.size());
					for (uint8_t color = 0; color < color_table.size() - 1; color++) {
						color_table_rgb[color].r = static_cast<uint8_t>(qRed(color_table.at(color)));
						color_table_rgb[color].g = static_cast<uint8_t>(qGreen(color_table.at(color)));
						color_table_rgb[color].b = static_cast<uint8_t>(qBlue(color_table.at(color)));
					}

					Palette palette(&color_table_rgb[0], static_cast<uint8_t>(color_table.size()));
					maestro_control->run_cue(
								maestro_control->canvas_handler->set_palette(
									maestro_control->get_section_index(),
									maestro_control->get_layer_index(),
									&palette));
				}

				/*
				 * Convert the frame's color table into a Canvas-supported one.
				 * Iterate over each Pixel and add its index to a temporary frame, then draw the frame to the Canvas.
				 */
				uint8_t color_table_indices[canvas_size.width() * canvas_size.height()];
				for (uint16_t y = 0; y < canvas_size.height(); y++) {
					for (uint16_t x = 0; x < canvas_size.width(); x++) {
						if (canvas->in_bounds(x, y)) {
							QColor pix_color = frame.pixelColor(x, y);
							color_table_indices[canvas->get_section()->get_dimensions()->get_inline_index(x, y)] = static_cast<uint8_t>(color_table.indexOf(pix_color.rgb()));
						}
					}
				}

				maestro_control->run_cue(
					maestro_control->canvas_handler->draw_frame(
						maestro_control->get_section_index(),
						maestro_control->get_layer_index(),
						canvas_size.width(),
						canvas_size.height(),
						&color_table_indices[0]
					)
				);

				image.jumpToNextImage();
				maestro_control->run_cue(
					maestro_control->canvas_handler->next_frame(
						maestro_control->get_section_index(),
						maestro_control->get_layer_index()));
			}
		}
	}
}
