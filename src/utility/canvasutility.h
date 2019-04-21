/*
 * CanvasUtility - Tools for managing Canvases.
 */

#ifndef CANVASUTILITY_H
#define CANVASUTILITY_H

#include <QByteArray>
#include <QString>
#include "canvas/canvas.h"
#include "core/colors.h"
#include "widget/maestrocontrolwidget.h"

using namespace PixelMaestro;

namespace PixelMaestroStudio {
	class CanvasUtility {
		public:
			/**
			 * Copies frames from a Canvas.
			 * @param canvas Canvas to copy from.
			 * @param target Target frameset.
			 * @param target_x Target width.
			 * @param target_y Target height.
			 */
			static void copy_from_canvas(Canvas& canvas, uint8_t** target, uint16_t target_x, uint16_t target_y) {
				Point target_bounds(target_x, target_y);
				Point canvas_bounds(canvas.get_section().get_dimensions().x, canvas.get_section().get_dimensions().y);

				for (uint16_t frame = 0; frame < canvas.get_num_frames(); frame++) {
					for (uint16_t y = 0; y < canvas_bounds.y; y++) {
						for (uint16_t x = 0; x < canvas_bounds.x; x++) {
							if (x <= target_x && y <= target_y) {
								target[frame][target_bounds.get_inline_index(x, y)] = canvas.get_frame(frame)[canvas_bounds.get_inline_index(x, y)];
							}
						}
					}
				}
			}

			static void copy_to_canvas(Canvas& canvas, uint8_t** source, uint16_t target_x, uint16_t target_y, MaestroControlWidget& maestro_control) {
				for (uint16_t frame = 0; frame < canvas.get_num_frames(); frame++) {
					maestro_control.run_cue(
						maestro_control.canvas_handler->set_current_frame_index(
							maestro_control.section_control_widget_->get_section_index(),
							maestro_control.section_control_widget_->get_layer_index(),
							frame
						)
					);

					maestro_control.run_cue(
						maestro_control.canvas_handler->draw_frame(
							maestro_control.section_control_widget_->get_section_index(),
							maestro_control.section_control_widget_->get_layer_index(),
							target_x,
							target_y,
							source[frame]
						)
					);
				}
			}
			static void load_image(const QString& filename, Canvas& canvas, MaestroControlWidget* maestro_control = nullptr);
	};
}

#endif // CANVASUTILITY_H
