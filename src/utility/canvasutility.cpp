/*
 * CanvasUtility - Tools for managing Canvases.
 */

#include <QColor>
#include <QImage>
#include <QImageReader>
#include "canvasutility.h"
#include "canvas/animationcanvas.h"
#include "canvas/colorcanvas.h"
#include "canvas/palettecanvas.h"
#include "core/point.h"

namespace PixelMaestroStudio {
	/**
	 * Copies all frames to or from an AnimationCanvas.
	 * @param canvas AnimationCanvas to copy from.
	 * @param target Target frameset.
	 * @param target_x Target width.
	 * @param target_y Target height.
	 */
	void CanvasUtility::copy_from_canvas(AnimationCanvas *canvas, bool** target, uint16_t target_x, uint16_t target_y) {
		for (uint16_t frame = 0; frame < canvas->get_num_frames(); frame++) {
			Point target_bounds(target_x, target_y);
			for (uint16_t y = 0; y < canvas->get_section()->get_dimensions()->y; y++) {
				for (uint16_t x = 0; x < canvas->get_section()->get_dimensions()->x; x++) {
					if (x <= target_x && y <= target_y) {
						target[frame][target_bounds.get_inline_index(x, y)] = canvas->get_frame(frame)[canvas->get_section()->get_dimensions()->get_inline_index(x, y)];
					}
				}
			}
		}
	}

	void CanvasUtility::copy_to_canvas(AnimationCanvas *canvas, bool **source, uint16_t target_x, uint16_t target_y, MaestroControl* maestro_control) {
		for (uint16_t frame = 0; frame < canvas->get_num_frames(); frame++) {
			maestro_control->run_cue(maestro_control->canvas_handler->draw_frame(maestro_control->get_section_index(), maestro_control->get_layer_index(), target_x, target_y, source[frame]));
			maestro_control->run_cue(maestro_control->canvas_handler->next_frame(maestro_control->get_section_index(), maestro_control->get_layer_index()));
		}
	}

	/**
	 * Copies all frames from a ColorCanvas.
	 * @param canvas ColorCanvas to copy from.
	 * @param target Target frameset.
	 * @param target_x Target width.
	 * @param target_y Target height.
	 * @param copy_from_canvas If true, copy from the Canvas to the target. Otherwise, copy from the target to the Canvas.
	 * @param maestro_control If copying to Canvas and serial is enabled, rebuild the Canvas on the serial device.
	 */
	void CanvasUtility::copy_from_canvas(ColorCanvas *canvas, Colors::RGB** target, uint16_t target_x, uint16_t target_y) {
		for (uint16_t frame = 0; frame < canvas->get_num_frames(); frame++) {
			Point target_bounds(target_x, target_y);
			for (uint16_t y = 0; y < canvas->get_section()->get_dimensions()->y; y++) {
				for (uint16_t x = 0; x < canvas->get_section()->get_dimensions()->x; x++) {
					if (x <= target_x && y <= target_y) {
						target[frame][target_bounds.get_inline_index(x, y)] = canvas->get_frame(frame)[canvas->get_section()->get_dimensions()->get_inline_index(x, y)];
					}
				}
			}
		}
	}

	void CanvasUtility::copy_to_canvas(ColorCanvas *canvas, Colors::RGB **source, uint16_t target_x, uint16_t target_y, MaestroControl* maestro_control) {
		for (uint16_t frame = 0; frame < canvas->get_num_frames(); frame++) {
			maestro_control->run_cue(maestro_control->canvas_handler->draw_frame(maestro_control->get_section_index(), maestro_control->get_layer_index(), target_x, target_y, source[frame]));
			maestro_control->run_cue(maestro_control->canvas_handler->next_frame(maestro_control->get_section_index(), maestro_control->get_layer_index()));
		}
	}

	/**
	 * Loads an image into the Canvas.
	 * @param filename Image location.
	 * @param canvas Canvas to load the image into.
	 * @param maestro_control Optional MaestroControl for generating Cues.
	 */
	void CanvasUtility::load_image(QString filename, Canvas *canvas, MaestroControl* maestro_control) {
		QImageReader image(filename, filename.right(3).toLocal8Bit());
		QSize canvas_size(canvas->get_section()->get_dimensions()->x, canvas->get_section()->get_dimensions()->y);
		image.setScaledSize(canvas_size);

		maestro_control->run_cue(maestro_control->canvas_handler->set_num_frames(maestro_control->get_section_index(), maestro_control->get_layer_index(), image.imageCount()));

		// For animated images, set the frame rate
		if (image.imageCount() > 1) {
			maestro_control->run_cue(maestro_control->canvas_handler->set_frame_timing(maestro_control->get_section_index(), maestro_control->get_layer_index(), image.nextImageDelay()));
		}

		Point cursor(0, 0);
		for (uint16_t i = 0; i < image.imageCount(); i++) {
			QImage frame = image.read();

			// For PaletteCanvases, convert the image into an 8-bit analogue
			// FIXME: This runs multiple times for animations when it should only run once
			if (canvas->get_type() == CanvasType::PaletteCanvas) {
				frame = frame.convertToFormat(QImage::Format_Indexed8);
			}

			// Extract the image's color table (for PaletteCanvases)
			QVector<QRgb> color_table = frame.colorTable();
			if (color_table.size() == 256) {
				color_table.removeLast();
			}

			// For PaletteCanvases, set the Canvas' palette before continuing.
			if (canvas->get_type() == CanvasType::PaletteCanvas) {

				// Copy the color table into a temporary RGB array so we can Cue it
				Colors::RGB color_table_rgb[color_table.size()];
				for (uint8_t color = 0; color < color_table.size() - 1; color++) {
					color_table_rgb[color].r = qRed(color_table.at(color));
					color_table_rgb[color].g = qGreen(color_table.at(color));
					color_table_rgb[color].b = qBlue(color_table.at(color));
				}

				maestro_control->run_cue(maestro_control->canvas_handler->set_colors(maestro_control->get_section_index(), maestro_control->get_layer_index(), &color_table_rgb[0], color_table.size()));
			}

			// Iterate over each pixel and the frame and re-draw it
			for (uint16_t y = 0; y < canvas_size.height(); y++) {
				for (uint16_t x = 0; x < canvas_size.width(); x++) {
					cursor.set(x, y);
					if (canvas->in_bounds(cursor.x, cursor.y)) {
						QColor pix_color = frame.pixelColor(x, y);
						Colors::RGB color(pix_color.red(), pix_color.green(), pix_color.blue());

						switch (canvas->get_type()) {
							case CanvasType::AnimationCanvas:
								{
									// Only draw if the Pixel is not completely black
									if (color != Colors::RGB {0, 0, 0}) {
										maestro_control->run_cue(maestro_control->canvas_handler->draw_point(maestro_control->get_section_index(), maestro_control->get_layer_index(), x, y));
									}
								}
								break;
							case CanvasType::ColorCanvas:
								maestro_control->run_cue(maestro_control->canvas_handler->draw_point(maestro_control->get_section_index(), maestro_control->get_layer_index(), color, x, y));
								break;
							case CanvasType::PaletteCanvas:
								maestro_control->run_cue(maestro_control->canvas_handler->draw_point(maestro_control->get_section_index(), maestro_control->get_layer_index(), color_table.indexOf(pix_color.rgb()), x, y));
								break;
						}
					}
				}
			}
			image.jumpToNextImage();
			maestro_control->run_cue(maestro_control->canvas_handler->next_frame(maestro_control->get_section_index(), maestro_control->get_layer_index()));
		}
	}
}