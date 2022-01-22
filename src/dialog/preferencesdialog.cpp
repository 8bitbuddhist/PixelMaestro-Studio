#include <QMessageBox>
#include <QSettings>
#include <QTime>
#include "preferencesdialog.h"
#include "ui_preferencesdialog.h"

namespace PixelMaestroStudio {

	QString PreferencesDialog::delimiter = QStringLiteral("|");
	QString PreferencesDialog::sub_delimiter = QStringLiteral(",");

	// "Files" section
	QString PreferencesDialog::last_cuefile_directory = QStringLiteral("Files/LastCuefileDirectory");

	// "Displays" section
	QString PreferencesDialog::separate_window_option = QStringLiteral("Displays/SeparateWindow");
	QString PreferencesDialog::main_window_option = QStringLiteral("Displays/MainWindow");

	// "Maestro" section
	QString PreferencesDialog::num_sections = QStringLiteral("Maestro/NumSections");
	QString PreferencesDialog::num_sections_per_row = QStringLiteral("Maestro/NumSectionsPerRow");
	QString PreferencesDialog::refresh_rate = QStringLiteral("Maestro/Refresh");

	// "Interface" section
	QString PreferencesDialog::pause_on_start = QStringLiteral("Interface/PauseOnStart");
	QString PreferencesDialog::pixel_shape = QStringLiteral("Interface/Shape");
	QString PreferencesDialog::save_session = QStringLiteral("Interface/SaveSessionOnClose");
	QString PreferencesDialog::last_session = QStringLiteral("Interface/LastSession");
	QString PreferencesDialog::show_cue_code = QStringLiteral("Interface/ShowCueCode");
	QString PreferencesDialog::splitter_position = QStringLiteral("Interface/SplitterPosition");
	QString PreferencesDialog::window_geometry = QStringLiteral("Interface/WindowGeometry");
	QString PreferencesDialog::window_state = QStringLiteral("Interface/WindowState");

	// "Devices" section
	QString PreferencesDialog::devices = QStringLiteral("Devices");
	QString PreferencesDialog::device_autoconnect = QStringLiteral("Autoconnect");
	QString PreferencesDialog::device_capacity = QStringLiteral("Capacity");
	QString PreferencesDialog::device_connectiontype = QStringLiteral("ConnectionType");
	QString PreferencesDialog::device_port = QStringLiteral("Port");
	QString PreferencesDialog::device_real_time_refresh = QStringLiteral("RealTimeRefresh");

	// Device section map
	QString PreferencesDialog::section_map = QStringLiteral("SectionMap");
	QString PreferencesDialog::section_map_local = QStringLiteral("Local");
	QString PreferencesDialog::section_map_remote = QStringLiteral("Remote");

	// "Palettes" section
	QString PreferencesDialog::palettes = QStringLiteral("Palettes");
	QString PreferencesDialog::palette_base_color = QStringLiteral("BaseColor");
	QString PreferencesDialog::palette_colors = QStringLiteral("Colors");
	QString PreferencesDialog::palette_mirror = QStringLiteral("Mirror");
	QString PreferencesDialog::palette_name = QStringLiteral("Name");
	QString PreferencesDialog::palette_num_colors = QStringLiteral("ColorCount");
	QString PreferencesDialog::palette_target_color = QStringLiteral("TargetColor");
	QString PreferencesDialog::palette_type = QStringLiteral("Type");
	QString PreferencesDialog::palette_start = QStringLiteral("Start");
	QString PreferencesDialog::palette_length = QStringLiteral("Length");
	QString PreferencesDialog::palette_thumbnail = QStringLiteral("Thumbnail");

	// "Show" section
	QString PreferencesDialog::event_history_max = QStringLiteral("Interface/EventHistoryMax");
	QString PreferencesDialog::events_trigger_device_updates = QStringLiteral("Interface/EventsTriggerDeviceUpdates");
	QString PreferencesDialog::cue_server_enabled = QStringLiteral("Interface/CueServerEnabled");
	QString PreferencesDialog::cue_server_port = QStringLiteral("Interface/CueServerPort");

	// MessageBox dialogs set to "don't show"
	QString PreferencesDialog::msgbox_hide_clear_canvas = QStringLiteral("MsgBoxHide/ClearCanvas");
	QString PreferencesDialog::msgbox_hide_clear_section_mappings = QStringLiteral("MsgBoxHide/ClearSectionMappings");
	QString PreferencesDialog::msgbox_hide_close_maestro = QStringLiteral("MsgBoxHide/CloseMaestro");
	QString PreferencesDialog::msgbox_hide_delete_palette = QStringLiteral("MsgBoxHide/DeletePalette");
	QString PreferencesDialog::msgbox_hide_remove_device = QStringLiteral("MsgBoxHide/RemoveDevice");
	QString PreferencesDialog::msgbox_hide_reset_palettes = QStringLiteral("MsgBoxHide/ResetPalettes");
	QString PreferencesDialog::msgbox_hide_sync_timers = QStringLiteral("MsgBoxHide/SyncTimers");

	PreferencesDialog::PreferencesDialog(QWidget *parent) : QDialog(parent), ui(new Ui::PreferencesDialog) {

		setWindowIcon(QIcon("qrc:/../../../docsrc/images/logo.png"));

		ui->setupUi(this);

		// Interface settings
		ui->pixelShapeComboBox->setCurrentIndex(settings_.value(pixel_shape, 1).toInt());		// Default to square pixels
		ui->saveSessionCheckBox->setChecked(settings_.value(save_session, true).toBool());		// Default to old session

		// Maestro settings
		ui->numSectionsSpinBox->setValue(settings_.value(num_sections, 1).toInt());				// Default to 1 Section
		ui->refreshTimeEdit->setTime(QTime::fromMSecsSinceStartOfDay(settings_.value(refresh_rate, 50).toInt()));				// Default to 50 ms
		ui->pauseOnStartCheckBox->setChecked(settings_.value(pause_on_start, false).toBool());	// Default to run on start
		ui->gridWidthSpinBox->setValue(settings_.value(num_sections_per_row, 1).toInt());

		// Show settings
		ui->eventHistorySizeSpinBox->setValue(settings_.value(event_history_max, 200).toInt());	// Default to 200
		ui->eventsTriggerDeviceUpdateCheckBox->setChecked(settings_.value(events_trigger_device_updates, false).toBool());	// Default to false

		ui->cueServerCheckBox->setChecked(settings_.value(cue_server_enabled, false).toBool());	// Disable Cue server by default
		ui->cueServerPortSpinBox->setValue(settings_.value(cue_server_port, 8077).toInt());	// Default to port 8077
	}

	void PreferencesDialog::on_buttonBox_accepted() {
		// Save Maestro settings
		settings_.setValue(refresh_rate, ui->refreshTimeEdit->time().msecsSinceStartOfDay());
		settings_.setValue(num_sections, ui->numSectionsSpinBox->value());
		settings_.setValue(pause_on_start, ui->pauseOnStartCheckBox->isChecked());

		// Save interface settings
		settings_.setValue(pixel_shape, ui->pixelShapeComboBox->currentIndex());
		settings_.setValue(save_session, ui->saveSessionCheckBox->isChecked());
		settings_.setValue(num_sections_per_row, ui->gridWidthSpinBox->value());

		// Save Show settings
		settings_.setValue(event_history_max, ui->eventHistorySizeSpinBox->value());
		settings_.setValue(events_trigger_device_updates, ui->eventsTriggerDeviceUpdateCheckBox->isChecked());
		settings_.setValue(cue_server_enabled, ui->cueServerCheckBox->isChecked());
		settings_.setValue(cue_server_port, ui->cueServerPortSpinBox->value());
	}

	void PreferencesDialog::on_cueServerCheckBox_stateChanged(int arg1) {
		ui->cueServerPortLabel->setEnabled(arg1);
		ui->cueServerPortSpinBox->setEnabled(arg1);
	}

	void PreferencesDialog::on_resetConfirmButton_clicked() {
		QMessageBox::StandardButton confirm;
		confirm = QMessageBox::question(this, "Reset Confirmation Messages", "Show all confirmation messages again, even if they've been marked as hidden?", QMessageBox::Yes|QMessageBox::No);

		if (confirm == QMessageBox::Yes) {
			settings_.setValue(this->msgbox_hide_clear_canvas, false);
			settings_.setValue(this->msgbox_hide_clear_section_mappings, false);
			settings_.setValue(this->msgbox_hide_close_maestro, false);
			settings_.setValue(this->msgbox_hide_delete_palette, false);
			settings_.setValue(this->msgbox_hide_remove_device, false);
			settings_.setValue(this->msgbox_hide_reset_palettes, false);
			settings_.setValue(this->msgbox_hide_sync_timers, false);
		}
	}

	PreferencesDialog::~PreferencesDialog() {
		delete ui;
	}

}
