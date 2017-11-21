/*
 * MaestroControl - Widget for interacting with a MaestroController.
 */

#ifndef MAESTROCONTROL_H
#define MAESTROCONTROL_H

#include <memory>
#include <QSerialPort>
#include <QTextStream>
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
#include "widget/showcontrol.h"
#include "window/virtualserialdevicedialog.h"

namespace Ui {
	class MaestroControl;
}

using namespace PixelMaestro;

class MaestroController;

class ShowControl;

class ShowController;

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

		/// Connection to an Arduino or other device.
		QSerialPort serial_port_;

		explicit MaestroControl(QWidget* parent, MaestroController* maestro_controller);
		~MaestroControl();
		void enable_show_edit_mode(bool enable);
		void execute_cue(uint8_t* cue);
		int16_t get_overlay_index();
		uint8_t get_overlay_index(Section* section);
		int16_t get_section_index();
		uint8_t get_section_index(Section* section);
		void read_from_file(QString filename);
		void save_to_file(QString filename);

	private:
		Ui::MaestroControl *ui;

		/// Extra animation controls
		std::unique_ptr<QWidget> animation_extra_control_widget_;

		/// Canvas controls
		std::unique_ptr<QWidget> canvas_control_widget_;

		/// MaestroController that this widget is controlling.
		MaestroController* maestro_controller_ = nullptr;

		/// Show controls
		std::unique_ptr<QWidget> show_control_widget_;

		/// Controller for managing Shows.
		ShowController* show_controller_ = nullptr;

		/**
		 * Prevents actions from affecting the Maestro.
		 * Used when customizing Shows.
		 */
		bool show_mode_enabled_ = false;

		/// Virtual device for testing Cue commands.
		std::unique_ptr<VirtualSerialDeviceDialog> virtual_device_dialog_;

		uint8_t get_num_overlays(Section* section);
		void initialize();
		void initialize_palettes();
		void on_section_resize(uint16_t x, uint16_t y);
		void populate_overlay_combobox();
		void save_maestro_settings(QDataStream* datastream);
		void save_section_settings(QDataStream* datastream, uint8_t section_id, uint8_t overlay_id);
		void set_active_section(Section* section);
		void set_center();
		void set_overlay_controls_visible(bool visible);
		void set_speed();
		void show_extra_controls(Animation* animation);
		void show_canvas_controls(bool visible);
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
		void on_overlayComboBox_currentIndexChanged(int index);
		void on_sectionComboBox_currentIndexChanged(int index);
		void on_overlaySpinBox_editingFinished();
		void on_offsetResetButton_clicked();
		void on_offsetXSpinBox_editingFinished();
		void on_offsetYSpinBox_editingFinished();
};

#endif // MAESTROCONTROL_H
