/*
 * DeviceControlWidget - Widget for managing USB/serial devices.
 */

#include <QList>
#include <QMessageBox>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QSettings>
#include <QStandardItem>
#include <QStringList>
#include <QTabWidget>
#include <QWidget>
#include <thread>
#include "dialog/adddevicedialog.h"
#include "dialog/cueinterpreterdialog.h"
#include "dialog/preferencesdialog.h"
#include "dialog/sectionmapdialog.h"
#include "devicecontrolwidget.h"
#include "ui_devicecontrolwidget.h"
#include "controller/serialdevicecontroller.h"
#include "controller/serialdevicethreadcontroller.h"

namespace PixelMaestroStudio {
	DeviceControlWidget::DeviceControlWidget(QWidget *parent) :
			QWidget(parent),
			ui(new Ui::DeviceControlWidget),
			maestro_control_widget_(*dynamic_cast<MaestroControlWidget*>(parent)) {
		ui->setupUi(this);

		// Block certain Cues from firing
		// TODO: Customizable and per-device blocks
		block_cue(CueController::Handler::SectionCueHandler, static_cast<uint8_t>(SectionCueHandler::Action::SetDimensions));
		block_cue(CueController::Handler::SectionCueHandler, static_cast<uint8_t>(SectionCueHandler::Action::SetBrightness));
		block_cue(CueController::Handler::CanvasCueHandler,
static_cast<uint8_t>(CanvasCueHandler::Action::DrawText));
		//block_cue(CueController::Handler::MaestroCueHandler, static_cast<uint8_t>(MaestroCueHandler::Action::SetShow));
		//block_cue(CueController::Handler::ShowCueHandler, static_cast<uint8_t>(ShowCueHandler::Action::SetEvents));

		// Add saved serial devices to device list.
		QSettings settings;
		int num_serial_devices = settings.beginReadArray(PreferencesDialog::devices);
		for (int device = 0; device < num_serial_devices; device++) {
			settings.setArrayIndex(device);

			QString device_name = settings.value(PreferencesDialog::device_port).toString();
			serial_devices_.push_back(SerialDeviceController(device_name));

			// If the device is set to auto-connect, try connecting
			SerialDeviceController& serial_device = serial_devices_.last();
			if (serial_device.get_autoconnect()) {
				serial_device.connect();
			}

		}
		settings.endArray();

		refresh_device_list();
	}

	/**
	 * Adds a Cue that should be blocked from execution to the list of blocked Cues.
	 * @param handler The CueHandler responsible for this Cue.
	 * @param action The Cue's action index.
	 */
	void DeviceControlWidget::block_cue(CueController::Handler handler, uint8_t action) {
		blocked_cues_.append({handler, action});
	}

	/**
	 * Returns the Maestro Cuefile.
	 * @return Maestro Cuefile.
	 */
	QByteArray* DeviceControlWidget::get_maestro_cue() {
		return &maestro_cue_;
	}

	void DeviceControlWidget::on_addDeviceButton_clicked() {
		AddDeviceDialog dialog(&serial_devices_, nullptr, this);
		dialog.exec();

		refresh_device_list();
	}

	void DeviceControlWidget::on_editDeviceButton_clicked() {
		SerialDeviceController* device = nullptr;
		int selected_device = ui->serialOutputListWidget->currentRow();
		if (selected_device >= 0) {
			device = &serial_devices_[selected_device];
		}

		AddDeviceDialog dialog(&serial_devices_, device, this);
		dialog.exec();

		refresh_device_list();
	}

	/**
	 * Connects to the selected device.
	 */
	void DeviceControlWidget::on_connectPushButton_clicked() {
		int selected = ui->serialOutputListWidget->currentRow();
		if (selected < 0) return;

		SerialDeviceController device = serial_devices_.at(selected);
		if (!device.get_device()->isOpen()) {
			if (device.connect()) {
				refresh_device_list();
			}
			else {
				QMessageBox::warning(this, "Unable to Connect", QString("Unable to connect to device on port " + device.get_port_name() + ": " + device.get_device()->errorString()));
			}
		}
		else {
			QMessageBox::information(this, "Device Already Connected", "This device is already connected.");
		}
	}


	/**
	 * Opens the CueInterpreter dialog for the selected device.
	 */
	void DeviceControlWidget::on_previewButton_clicked() {
		CueInterpreterDialog dialog(this,
									reinterpret_cast<uint8_t*>(maestro_cue_.data()),
									static_cast<uint16_t>(maestro_cue_.size()));
		dialog.exec();
	}

	/**
	 * Disconnects from the selected device.
	 */
	void DeviceControlWidget::on_disconnectPushButton_clicked() {
		int selected_index = ui->serialOutputListWidget->currentRow();
		if (selected_index < 0) return;

		SerialDeviceController device = serial_devices_.at(selected_index);

		if (device.disconnect()) {
			refresh_device_list();
		}
		else {
			QMessageBox::warning(this, "Unable to Disconnect", QString("Unable to disconnect device on port " + device.get_port_name() + ": " + device.get_device()->errorString()));
		}
	}

	void DeviceControlWidget::on_removeDeviceButton_clicked() {
		int selected = ui->serialOutputListWidget->currentRow();
		if (selected < 0) return;

		QMessageBox::StandardButton confirm;
		confirm = QMessageBox::question(this, "Remove Device", "Are you sure you want to remove this device from your saved devices?", QMessageBox::Yes | QMessageBox::No);
		if (confirm == QMessageBox::Yes) {
			serial_devices_.removeAt(selected);
			refresh_device_list();
		}
	}

	/**
	 * Transmits the Maestro's Cuefile to the selected device.
	 */
	void DeviceControlWidget::on_uploadButton_clicked() {
		/*
		 * "ROMBEG" indicates the start of the Cuefile.
		 * "ROMEND" indicates the end of the Cuefile.
		 */
		QByteArray out = QByteArray("ROMBEG") +
						 maestro_cue_ +
						 QByteArray("ROMEND");

		write_to_device(serial_devices_[ui->serialOutputListWidget->currentRow()],
				static_cast<const char*>(out),
				out.size(),
				true);
	}

	void DeviceControlWidget::on_serialOutputListWidget_currentRowChanged(int currentRow) {
		if (currentRow < 0) return;

		SerialDeviceController device = serial_devices_.at(currentRow);

		bool connected = device.get_device()->isOpen();

		ui->connectPushButton->setEnabled(!connected);
		ui->disconnectPushButton->setEnabled(connected);
		ui->uploadButton->setEnabled(connected);
		ui->uploadProgressBar->setValue(0);
	}

	void DeviceControlWidget::refresh_device_list() {
		ui->serialOutputListWidget->clear();
		for (SerialDeviceController device : serial_devices_) {
			QListWidgetItem* item = new QListWidgetItem(device.get_port_name());
			if (device.get_device()->isOpen()) {
				item->setTextColor(Qt::white);
			}
			else {
				item->setTextColor(Qt::gray);
			}
			ui->serialOutputListWidget->addItem(item);
		}

		int selected = ui->serialOutputListWidget->currentRow();
		if (selected >= 0) {
			ui->uploadButton->setEnabled(serial_devices_[selected].get_device()->isOpen());
		}
		else {
			ui->uploadButton->setEnabled(false);
			ui->connectPushButton->setEnabled(false);
			ui->disconnectPushButton->setEnabled(false);
		}
	}

	/**
	 * Updates the Maestro's Cuefile.
	 * This also sends the Cue to all connected devices.
	 * @param cue The Cue to execute.
	 * @param size The size of the Cue.
	 */
	void DeviceControlWidget::run_cue(uint8_t *cue, int size) {
		// Check the Cue against the block list
		for (BlockedCue blocked : blocked_cues_) {
			if (cue[(uint8_t)CueController::Byte::PayloadByte] == (char)blocked.handler) {
				int action_byte_index = -1;
				switch (blocked.handler) {
					case CueController::Handler::AnimationCueHandler:
						action_byte_index = (uint8_t)AnimationCueHandler::Byte::ActionByte;
						break;
					case CueController::Handler::CanvasCueHandler:
						action_byte_index = (uint8_t)CanvasCueHandler::Byte::ActionByte;
						break;
					case CueController::Handler::MaestroCueHandler:
						action_byte_index = (uint8_t)MaestroCueHandler::Byte::ActionByte;
						break;
					case CueController::Handler::SectionCueHandler:
						action_byte_index = (uint8_t)SectionCueHandler::Byte::ActionByte;
						break;
					case CueController::Handler::ShowCueHandler:
						action_byte_index = (uint8_t)ShowCueHandler::Byte::ActionByte;
						break;
				}

				if (cue[action_byte_index] == (char)blocked.action) {
					return;
				}
			}
		}

		CueController* controller = &this->maestro_control_widget_.get_maestro_controller()->get_maestro().get_cue_controller();

		for (SerialDeviceController device : serial_devices_) {
			// TODO: Move to separate thread

			// Copy the Cue for each device
			QByteArray out = QByteArray(reinterpret_cast<const char*>(cue), size);

			/*
			 * If the device has a Section map saved, apply it to the Cue.
			 * This only applies to real-time updates, not Cuefiles.
			 */
			SectionMapModel* model = device.section_map_model;
			if (model != nullptr && device.get_real_time_refresh_enabled()) {

				// Only check Section-based Cues
				if (!(out[(uint8_t)CueController::Byte::PayloadByte] == static_cast<char>(CueController::Handler::MaestroCueHandler) ||
					out[(uint8_t)CueController::Byte::PayloadByte] == static_cast<char>(CueController::Handler::ShowCueHandler))) {

					/*
					 * Grab the local section ID from the buffer.
					 * Then, grab the remote section ID from the map.
					 * If they're different, replace the local with the remote ID in the buffer.
					 */

					// SectionByte is the same location for all Section-related handlers (as of v0.30)
					uint8_t local_section_id = out[(uint8_t)SectionCueHandler::Byte::SectionByte];

					// Make sure the cell actually exists in the model before swapping.
					QStandardItem* cell = model->item(local_section_id, 1);
					if (cell && !cell->text().isEmpty()) {
						// If the remote section number is negative, exit immediately
						int remote_section_id = cell->text().toInt();
						if (remote_section_id < 0) return;
						if (remote_section_id != local_section_id) {
							// We have a match. Swap the values and reassmble the Cue.
							out[(uint8_t)SectionCueHandler::Byte::SectionByte] = remote_section_id;
							out[(uint8_t)CueController::Byte::ChecksumByte] = controller->checksum(reinterpret_cast<uint8_t*>(out.data()), out.size());
						}
					}
				}
			}

			if (device.get_device()->isOpen() && device.get_real_time_refresh_enabled()) {
				write_to_device(device,
								out.data(),
								out.size(),
								false);
			}
		}

		update_cuefile_size();
	}

	/**
	 * Saves the device list to settings.
	 */
	void DeviceControlWidget::save_devices() {
		// Save devices
		QSettings settings;
		settings.remove(PreferencesDialog::devices);
		settings.beginWriteArray(PreferencesDialog::devices);
		for (int i = 0; i < ui->serialOutputListWidget->count(); i++) {
			settings.setArrayIndex(i);

			SerialDeviceController* device = &serial_devices_[i];
			settings.setValue(PreferencesDialog::device_port, device->get_port_name());
			settings.setValue(PreferencesDialog::device_real_time_refresh, device->get_real_time_refresh_enabled());

			// Save the device's model
			SectionMapModel* model = device->section_map_model;
			if (model != nullptr) {
				settings.beginWriteArray(PreferencesDialog::section_map);

				QModelIndex parent = QModelIndex();
				for (int row = 0; row < model->rowCount(parent); row++) {
					settings.setArrayIndex(row);

					QStandardItem* local_section = model->item(row, 0);
					settings.setValue(PreferencesDialog::section_map_local, local_section->text().toInt());

					QStandardItem* remote_section = model->item(row, 1);
					settings.setValue(PreferencesDialog::section_map_remote, remote_section->text().toInt());
				}
				settings.endArray();
			}
		}
		settings.endArray();
	}

	/**
	 * Sets the state of the progress bar.
	 * @param val Value to set the progress to.
	 */
	void DeviceControlWidget::set_progress_bar(int val) {
		ui->uploadProgressBar->setValue(val);
		// Disable upload button while sending data
		ui->uploadButton->setEnabled(!(val > 0 && val < 100));
	}

	/**
	 * Regenerates the Maestro Cuefile and updates the size in the UI.
	 *
	 * TODO: Move to separate thread
	 */
	void DeviceControlWidget::update_cuefile_size() {
		// Calculate and display the size of the current Maestro configuration
		QDataStream datastream(&maestro_cue_, QIODevice::Truncate);

		MaestroController* controller = maestro_control_widget_.get_maestro_controller();

		// Generate the Cuefile
		controller->save_maestro_to_datastream(datastream);

		ui->fileSizeLineEdit->setText(locale_.toString(maestro_cue_.size()));
	}

	/**
	 * Sends serial output to device in a separate thread.
	 * @param device Device to send output to.
	 * @param out Data to send.
	 * @param size Size of data to send.
	 */
	void DeviceControlWidget::write_to_device(SerialDeviceController& device, const char *out, const int size, bool progress) {
		SerialDeviceThreadController* thread = new SerialDeviceThreadController(device,
														  out,
														  size);

		connect(thread, &SerialDeviceThreadController::finished, thread, &SerialDeviceThreadController::deleteLater);

		if (progress) {
			connect(thread, &SerialDeviceThreadController::progress_changed, this, &DeviceControlWidget::set_progress_bar);
		}

		/*
		 * FIXME: Device pointer becomes invalid when using Thread::run(), likely due to invalid memory access.
		 *		Need to make the DeviceThreadController and output thread-safe before sending it to a separate thread.
		 *
		 *		Maybe convert threads to QFuture and use QFutureWatcher to update progress? https://doc.qt.io/qt-5/qfuturewatcher.html#details
		 *
		 */

		//thread->start();
		thread->run();
	}

	DeviceControlWidget::~DeviceControlWidget() {
		for (SerialDeviceController& device : serial_devices_) {
			device.disconnect();
		}
		delete ui;
	}
}
