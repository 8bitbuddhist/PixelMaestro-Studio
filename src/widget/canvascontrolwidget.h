#ifndef CANVASCONTROLWIDGET_H
#define CANVASCONTROLWIDGET_H

#include <QButtonGroup>
#include <QWidget>
#include "controller/palettecontroller.h"
#include "core/point.h"
#include "widget/maestrocontrolwidget.h"

namespace Ui {
	class CanvasControlWidget;
}

namespace PixelMaestroStudio {
	class CanvasControlWidget : public QWidget {
		Q_OBJECT

		public:
			explicit CanvasControlWidget(QWidget *parent = nullptr);
			~CanvasControlWidget();

			uint8_t get_selected_color_index() const;
			bool get_painting_enabled() const;
			bool get_replace_enabled() const;
			void initialize();
			void refresh();
			void refresh_palettes();
			void set_canvas_origin(Point& coordinates);

		public slots:
			void on_drawButton_clicked();

		protected:
			bool eventFilter(QObject *watched, QEvent *event);

		private slots:
			void on_canvas_color_clicked();
			void on_enableCheckBox_toggled(bool checked);

			void on_loadImageButton_clicked();

			void on_circleToolButton_toggled(bool checked);

			void on_lineToolButton_toggled(bool checked);

			void on_triangleToolButton_toggled(bool checked);

			void on_rectToolButton_toggled(bool checked);

			void on_textToolButton_toggled(bool checked);

			void on_editPaletteButton_clicked();

			void on_playbackPreviousToolButton_clicked();

			void on_playbackStartStopToolButton_toggled(bool checked);

			void on_playbackNextToolButton_clicked();

			void on_frameCountSpinBox_editingFinished();

			void on_currentFrameSpinBox_editingFinished();

			void on_frameIntervalSlider_valueChanged(int value);

			void on_clearButton_clicked();

			void on_brushToolButton_toggled(bool checked);

			void on_replaceToolButton_toggled(bool checked);

			void on_paletteComboBox_activated(int index);

			void on_frameTimeEdit_editingFinished();

		private:
			/// Color index storage for Canvases.
			uint8_t selected_color_index_ = 255;

			/// Conversion from canvas_color_ into PixelMaestro color.
			Colors::RGB canvas_rgb_color_;

			/// Group for Palette color buttons
			QButtonGroup canvas_palette_color_group_;

			/// Group for Canvas shape radio buttons
			QButtonGroup canvas_shape_type_group_;

			/// The parent controller widget.
			MaestroControlWidget& maestro_control_widget_;
			Ui::CanvasControlWidget *ui;

			void add_palette_to_selection(const Palette& palette);
			void populate_palette_canvas_color_selection(PaletteController::PaletteWrapper& palette_wrapper);
			void set_controls_enabled(bool enabled);
			void set_frame_interval();
			void set_circle_controls_enabled(bool enabled);
			void set_replace_controls_enabled(bool enabled);
			void set_line_controls_enabled(bool enabled);
			void set_brush_controls_enabled(bool enabled);
			void set_rect_controls_enabled(bool enabled);
			void set_text_controls_enabled(bool enabled);
			void set_triangle_controls_enabled(bool enabled);
	};
}

#endif // CANVASCONTROLWIDGET_H
