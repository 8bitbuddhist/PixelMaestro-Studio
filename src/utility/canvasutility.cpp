/*
 * CanvasUtility - Tools for managing Canvases.
 */

#include <QColor>
#include <QImage>
#include <QImageReader>
#include "canvasutility.h"
#include "canvas/palettecanvas.h"
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

		maestro_control->run_cue(maestro_control->canvas_handler->set_num_frames(maestro_control->get_section_index(), maestro_control->get_layer_index(), image.imageCount()));

		// For animated images, set the frame rate
		if (image.imageCount() > 1) {
			maestro_control->run_cue(maestro_control->canvas_handler->set_frame_timer(maestro_control->get_section_index(), maestro_control->get_layer_index(), image.nextImageDelay()));
		}

		Point cursor(0, 0);
		for (uint16_t i = 0; i < image.imageCount(); i++) {
			QImage frame = image.read();

			frame = frame.convertToFormat(QImage::Format_Indexed8);

			// Extract the image's color table
			QVector<QRgb> color_table = frame.colorTable();

			/*
			 * Set the Canvas' palette before continuing.
			 * Pare down the frame's palette so it fits in the Canvas' palette.
			 */
			while (color_table.size() >= 256) {
				color_table.removeLast();
			}

			// Copy the color table into a temporary RGB array so we can Cue it
			Colors::RGB color_table_rgb[color_table.size()];
			for (uint8_t color = 0; color < color_table.size() - 1; color++) {
				color_table_rgb[color].r = qRed(color_table.at(color));
				color_table_rgb[color].g = qGreen(color_table.at(color));
				color_table_rgb[color].b = qBlue(color_table.at(color));
			}

			maestro_control->run_cue(maestro_control->canvas_handler->set_palette(maestro_control->get_section_index(), maestro_control->get_layer_index(), new Palette(&color_table_rgb[0], color_table.size())));

			// Iterate over each pixel and the frame and re-draw it
			for (uint16_t y = 0; y < canvas_size.height(); y++) {
				for (uint16_t x = 0; x < canvas_size.width(); x++) {
					cursor.set(x, y);
					if (canvas->in_bounds(cursor.x, cursor.y)) {
						QColor pix_color = frame.pixelColor(x, y);
						maestro_control->run_cue(maestro_control->canvas_handler->draw_point(maestro_control->get_section_index(), maestro_control->get_layer_index(), color_table.indexOf(pix_color.rgb()), x, y));
					}
				}
			}
			image.jumpToNextImage();
			maestro_control->run_cue(maestro_control->canvas_handler->next_frame(maestro_control->get_section_index(), maestro_control->get_layer_index()));
		}
	}
}