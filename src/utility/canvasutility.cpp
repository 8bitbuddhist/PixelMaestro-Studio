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
	void CanvasUtility::load_image(const QString& filename, Canvas *canvas, MaestroControlWidget* maestro_control) {
		QImageReader image(filename, filename.right(3).toLocal8Bit());
		QSize canvas_size(canvas->get_section()->get_dimensions()->x, canvas->get_section()->get_dimensions()->y);
		image.setScaledSize(canvas_size);

		/*
		 * Get the number of frames.
		 * imageCount() = 0 if the image isn't animated, so we set it to 1 here to run the loop at least once.
		 */
		int framecount = 1;
		if (image.imageCount() > 0) {
			framecount = image.imageCount();
		}

		maestro_control->run_cue(
			maestro_control->canvas_handler->set_num_frames(
					maestro_control->section_control_widget_->get_section_index(),
					maestro_control->section_control_widget_->get_layer_index(),
					framecount));

		// Read each frame
		for (uint16_t i = 0; i < framecount; i++) {
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

			if (image.currentImageNumber() == 0) {

				// Set the frame rate. Note that this doesn't account for images with different timings per frame.
				if (framecount > 1) {
					maestro_control->run_cue(
								maestro_control->canvas_handler->set_frame_timer(
									maestro_control->section_control_widget_->get_section_index(),
									maestro_control->section_control_widget_->get_layer_index(),
									static_cast<uint16_t>(image.nextImageDelay())));
				}

				/*
				 * Copy the color table into a temporary RGB array so we can translate it into a Cue.
				 * Note that this only takes the color table from the first frame, so if other frames use different color palettes, they won't come out as expected.
				 */
				QVector<Colors::RGB> color_table_rgb(color_table.size());
				for (uint8_t color = 0; color < color_table.size() - 1; color++) {
					color_table_rgb[color].r = static_cast<uint8_t>(qRed(color_table.at(color)));
					color_table_rgb[color].g = static_cast<uint8_t>(qGreen(color_table.at(color)));
					color_table_rgb[color].b = static_cast<uint8_t>(qBlue(color_table.at(color)));
				}

				Palette palette(&color_table_rgb[0], static_cast<uint8_t>(color_table.size()));
				maestro_control->run_cue(
							maestro_control->canvas_handler->set_palette(
								maestro_control->section_control_widget_->get_section_index(),
								maestro_control->section_control_widget_->get_layer_index(),
								palette));
			}

			/*
			 * Get the index of each pixel's color in the frame's palette.
			 * Then, copy each index into a temporary frame.
			 * Finally, draw the frame to the Canvas.
			 */
			uint8_t color_table_indices[canvas_size.width() * canvas_size.height()];
			for (uint16_t y = 0; y < canvas_size.height(); y++) {
				for (uint16_t x = 0; x < canvas_size.width(); x++) {
					if (canvas->get_section()->get_dimensions()->in_bounds(x, y)) {
						QColor pix_color = frame.pixelColor(x, y);
						color_table_indices[canvas->get_section()->get_dimensions()->get_inline_index(x, y)] = static_cast<uint8_t>(color_table.indexOf(pix_color.rgb()));
					}
				}
			}

			maestro_control->run_cue(
				maestro_control->canvas_handler->draw_frame(
					maestro_control->section_control_widget_->get_section_index(),
					maestro_control->section_control_widget_->get_layer_index(),
					canvas_size.width(),
					canvas_size.height(),
					&color_table_indices[0]
				)
			);

			// If this is an animated image, move to the next frame.
			if (i < (framecount - 1)) {
				image.jumpToNextImage();
				maestro_control->run_cue(
					maestro_control->canvas_handler->next_frame(
						maestro_control->section_control_widget_->get_section_index(),
						maestro_control->section_control_widget_->get_layer_index()));
			}
		}
	}
}
