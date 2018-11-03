/*
 * MaestroControl - Widget for interacting with a MaestroController.
 */

#ifndef MAESTROCONTROLWIDGET_H
#define MAESTROCONTROLWIDGET_H

#include <memory>
#include <QButtonGroup>
#include <QColor>
#include <QLocale>
#include <QSerialPort>
#include <QSharedPointer>
#include <QTimer>
#include <QVector>
#include <QWidget>
#include "utility/cueinterpreter.h"
#include "controller/maestrocontroller.h"
#include "controller/palettecontroller.h"
#include "controller/showcontroller.h"
#include "core/colors.h"
#include "core/maestro.h"
#include "cue/animationcuehandler.h"
#include "cue/canvascuehandler.h"
#include "cue/maestrocuehandler.h"
#include "cue/sectioncuehandler.h"
#include "cue/showcuehandler.h"
#include "dialog/maestrodrawingareadialog.h"
#include "widget/devicecontrolwidget.h"
#include "widget/animationcontrolwidget.h"
#include "widget/sectioncontrolwidget.h"
#include "widget/showcontrolwidget.h"

namespace Ui {
	class MaestroControlWidget;
}

using namespace PixelMaestro;

namespace PixelMaestroStudio {
	class AnimationControlWidget;
	class DeviceControlWidget;
	class MaestroController;
	class MaestroDrawingAreaDialog;
	class SectionControlWidget;
	class ShowController;
	class ShowControlWidget;
	class MaestroControlWidget : public QWidget {
		Q_OBJECT

		public:
			/**
			 * Sets whether we're in the middle of loading a Cue.
			 * Used to flag whether or not to update the total Cue size in DeviceControlWidget.
			 */
			bool loading_cue_ = false;

			/// The controller for managing Palettes.
			/// TODO: Move to MaestroController (since Palettes are Maestro-wide)
			PaletteController palette_controller_;

			// Cue components
			CueController* cue_controller_ = nullptr;
			AnimationCueHandler* animation_handler = nullptr;
			CanvasCueHandler* canvas_handler = nullptr;
			MaestroCueHandler* maestro_handler = nullptr;
			SectionCueHandler* section_handler = nullptr;
			ShowCueHandler* show_handler = nullptr;

			// Control subwidgets
			QSharedPointer<AnimationControlWidget> animation_control_widget_;
			QSharedPointer<DeviceControlWidget> device_control_widget_;
			QSharedPointer<SectionControlWidget> section_control_widget_;
			QSharedPointer<ShowControlWidget> show_control_widget_;

			explicit MaestroControlWidget(QWidget* parent);
			~MaestroControlWidget();
			void edit_palettes(QString palette);
			uint8_t get_canvas_color_index() const;
			bool get_canvas_painting_enabled();
			MaestroController* get_maestro_controller();
			void load_cuefile(QByteArray byte_array);
			void refresh();
			void refresh_maestro_settings();
			void run_cue(uint8_t* cue, bool remote_only = false);
			void set_active_section(Section* section);
			void set_canvas_origin(Point* coordinates);
			void set_maestro_controller(MaestroController* maestro_controller);

		protected:
			bool eventFilter(QObject *watched, QEvent *event);

		private:
			Ui::MaestroControlWidget *ui;

			/// Color index storage for Canvases.
			uint8_t canvas_color_index_ = 255;

			/// Conversion from canvas_color_ into PixelMaestro color.
			Colors::RGB canvas_rgb_color_;

			/// Group for Canvas shape radio buttons
			QButtonGroup canvas_shape_type_group_;

			/// Separate Maestro DrawingArea
			std::unique_ptr<MaestroDrawingAreaDialog> drawing_area_dialog_;

			/// MaestroController that this widget is controlling.
			MaestroController* maestro_controller_ = nullptr;

			/// Updates the Maestro time in the Show tab.
			QTimer show_timer_;

			void initialize();
			void initialize_palettes();
			void populate_palette_canvas_color_selection(PaletteController::PaletteWrapper* palette_wrapper);
			void set_canvas_controls_enabled(bool enabled);

			// Canvas control handling methods
			void set_circle_controls_enabled(bool enabled);
			void set_line_controls_enabled(bool enabled);
			void set_paint_controls_enabled(bool enabled);
			void set_rect_controls_enabled(bool enabled);
			void set_text_controls_enabled(bool enabled);
			void set_triangle_controls_enabled(bool enabled);
			void set_canvas_frame_interval();

		private slots:
			void on_frameCountSpinBox_editingFinished();
			void on_currentFrameSpinBox_editingFinished();
			void on_frameIntervalSpinBox_editingFinished();
			void on_loadImageButton_clicked();
			void on_clearButton_clicked();
			void on_drawButton_clicked();

			void on_canvas_color_clicked();
			void on_canvasEditPaletteButton_clicked();
			void on_canvasPaletteComboBox_currentIndexChanged(int index);
			void on_circleToolButton_toggled(bool checked);
			void on_lineToolButton_toggled(bool checked);
			void on_triangleToolButton_toggled(bool checked);
			void on_textToolButton_toggled(bool checked);
			void on_rectToolButton_toggled(bool checked);
			void on_canvasPlaybackStartStopToolButton_toggled(bool checked);
			void on_canvasPlaybackNextToolButton_clicked();
			void on_canvasPlaybackBackToolButton_clicked();
			void on_paintToolButton_toggled(bool checked);
			void on_frameIntervalSlider_valueChanged(int value);
			void on_canvasEnableCheckBox_toggled(bool checked);
	};
}

#endif // MAESTROCONTROLWIDGET_H
