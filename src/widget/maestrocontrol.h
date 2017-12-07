/*
 * MaestroControl - Widget for interacting with a MaestroController.
 */

#ifndef MAESTROCONTROL_H
#define MAESTROCONTROL_H

#include <memory>
#include <QButtonGroup>
#include <QColor>
#include <QLocale>
#include <QSerialPort>
#include <QSharedPointer>
#include <QTimer>
#include <QVector>
#include <QWidget>
#include "controller/cueinterpreter.h"
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
#include "window/simpledrawingareadialog.h"

namespace Ui {
	class MaestroControl;
}

using namespace PixelMaestro;

class MaestroControl : public QWidget {
	Q_OBJECT

	public:
		/// The Section currently being controlled.
		Section* active_section_ = nullptr;

		/// The controller for managing Palettes.
		PaletteController palette_controller_;

		// Cue components
		CueInterpreter cue_interpreter_;
		CueController* cue_controller_ = nullptr;
		AnimationCueHandler* animation_handler = nullptr;
		CanvasCueHandler* canvas_handler = nullptr;
		MaestroCueHandler* maestro_handler = nullptr;
		SectionCueHandler* section_handler = nullptr;
		ShowCueHandler* show_handler = nullptr;

		explicit MaestroControl(QWidget* parent, MaestroController* maestro_controller);
		~MaestroControl();
		void enable_show_edit_mode(bool enable);
		int16_t get_layer_index();
		uint8_t get_layer_index(Section* section);
		int16_t get_section_index();
		uint8_t get_section_index(Section* section);
		void read_from_file(QString filename);
		void run_cue(uint8_t* cue);
		void save_to_file(QString filename);

	private:
		Ui::MaestroControl *ui;

		/// Extra animation controls
		std::unique_ptr<QWidget> animation_extra_control_widget_;

		/// Temporary storage for the Canvas drawing brush.
		QColor canvas_color_ = QColor::fromRgb(0, 0, 0);

		/// Conversion from canvas_color_ into PixelMaestro color.
		Colors::RGB canvas_rgb_color_;

		/// Group for Canvas shape radio buttons
		QButtonGroup canvas_shape_type_group_;

		/// Separate Maestro DrawingArea
		std::unique_ptr<SimpleDrawingAreaDialog> drawing_area_dialog_;

		/// History of actions performed in the editor. Each entry contains a copy of the Event's Cue.
		QVector<QVector<uint8_t>> event_history_;

		/// Locale for formatting numbers (specifically the program runtime).
		QLocale locale_ = QLocale::system();

		/// MaestroController that this widget is controlling.
		MaestroController* maestro_controller_ = nullptr;

		/// List of enabled output devices.
		QVector<QString> output_devices_;

		/// List of enabled USB devices.
		QVector<QSharedPointer<QSerialPort>> serial_devices_;

		/// Controller for managing Shows.
		ShowController* show_controller_ = nullptr;

		/// Updates the Maestro time in the Show tab.
		QTimer show_timer_;

		/**
		 * Prevents actions from affecting the Maestro.
		 * Used when customizing Shows.
		 */
		bool show_mode_enabled_ = false;

		uint8_t get_num_layers(Section* section);
		void add_cue_to_history(uint8_t* cue);
		void initialize();
		void initialize_palettes();
		void on_section_resize(uint16_t x, uint16_t y);
		void populate_layer_combobox();
		void save_maestro_settings(QDataStream* datastream);
		void save_section_settings(QDataStream* datastream, uint8_t section_id, uint8_t layer_id);
		void set_active_section(Section* section);
		void set_canvas_controls_enabled(bool enabled, CanvasType::Type type);
		void set_offset();

		// Canvas control handling methods
		void set_circle_controls_enabled(bool enabled);
		void set_line_controls_enabled(bool enabled);
		void set_rect_controls_enabled(bool enabled);
		void set_text_controls_enabled(bool enabled);
		void set_triangle_controls_enabled(bool enabled);

		void set_scroll();
		void set_layer_controls_enabled(bool enabled);
		void set_show_controls_enabled(bool enabled);
		void set_speed();
		void show_extra_controls(Animation* animation);
		void write_cue_to_stream(QDataStream* stream, uint8_t* cue);

	private slots:
		void on_alphaSpinBox_valueChanged(int arg1);
		void on_animationComboBox_currentIndexChanged(int index);
		void on_canvasComboBox_currentIndexChanged(int index);
		void on_colorComboBox_currentIndexChanged(int index);
		void on_columnsSpinBox_editingFinished();
		void on_cycleSlider_valueChanged(int value);
		void on_fadeCheckBox_toggled(bool checked);
		void on_mix_modeComboBox_currentIndexChanged(int index);
		void on_orientationComboBox_currentIndexChanged(int index);
		void on_paletteControlButton_clicked();
		void on_reverse_animationCheckBox_toggled(bool checked);
		void on_rowsSpinBox_editingFinished();
		void on_cycleSpinBox_editingFinished();
		void on_enableShowCheckBox_toggled(bool checked);
		void on_pauseSlider_valueChanged(int value);
		void on_pauseSpinBox_valueChanged(int arg1);
		void on_layerComboBox_currentIndexChanged(int index);
		void on_sectionComboBox_currentIndexChanged(int index);
		void on_layerSpinBox_editingFinished();
		void on_offsetXSpinBox_editingFinished();
		void on_offsetYSpinBox_editingFinished();
		void on_toggleShowModeCheckBox_clicked(bool checked);
		void on_addEventButton_clicked();
		void on_frameCountSpinBox_editingFinished();
		void on_toggleCanvasModeCheckBox_toggled(bool checked);
		void on_currentFrameSpinBox_editingFinished();
		void on_frameRateSpinBox_editingFinished();
		void on_circleRadioButton_toggled(bool checked);
		void on_lineRadioButton_toggled(bool checked);
		void on_triangleRadioButton_toggled(bool checked);
		void on_textRadioButton_toggled(bool checked);
		void on_rectRadioButton_toggled(bool checked);
		void on_selectColorButton_clicked();
		void on_loadImageButton_clicked();
		void on_clearButton_clicked();
		void on_drawButton_clicked();
		void on_scrollXSpinBox_editingFinished();
		void on_scrollYSpinBox_editingFinished();
		void on_removeEventButton_clicked();
		void on_timingMethodComboBox_currentIndexChanged(int index);
		void on_loopCheckBox_toggled(bool checked);

		void update_maestro_last_time();

		void on_showPauseButton_clicked();
};

#endif // MAESTROCONTROL_H
