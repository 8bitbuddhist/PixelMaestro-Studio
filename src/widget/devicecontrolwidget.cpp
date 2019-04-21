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
		block_cue(CueController::Handler::SectionCueHandler, static_cast<uint8_t>(SectionCueHandler::Action::SetDimensions));

		// Disable device buttons by default
		set_device_controls_enabled(false);

		// Add available serial devices to combo box
		populate_serial_devices();

		// Add saved serial devices to output selection box.
		QSettings settings;
		int num_serial_devices = settings.beginReadArray(PreferencesDialog::devices);
		for (int device = 0; device < num_serial_devices; device++) {
			settings.setArrayIndex(device);

			QString device_name = settings.value(PreferencesDialog::device_port).toString();
			serial_devices_.push_back(SerialDeviceController(device_name));

			// If the saved port is available, try connecting to it
			if (serial_devices_.last().connect()) {
				QListWidgetItem* item = new QListWidgetItem(device_name);
				ui->serialOutputListWidget->addItem(item);
			}
		}
		settings.endArray();
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
	 * Compares the Cuefile size to the selected device's ROM capacity, and highlights the Cuesize textbox accordingly.
	 */
	void DeviceControlWidget::check_device_rom_capacity() {
		if (ui->serialOutputListWidget->currentRow() > -1) {
			int capacity = serial_devices_[ui->serialOutputListWidget->currentRow()].get_capacity();
			int capacity_75 = static_cast<int>(capacity * 0.75);
			if (this->maestro_cue_.size() >= capacity) {
				ui->fileSizeLineEdit->setStyleSheet("border: 1px solid red");
			}
			else if (this->maestro_cue_.size() >= capacity_75) {
				ui->fileSizeLineEdit->setStyleSheet("border: 1px solid orange");
			}
			else {
				ui->fileSizeLineEdit->setStyleSheet("border: 1px solid green");
			}
		}
	}

	/**
	 * Returns the Maestro Cuefile.
	 * @return Maestro Cuefile.
	 */
	QByteArray* DeviceControlWidget::get_maestro_cue() {
		return &maestro_cue_;
	}

	/**
	 * Connects to the selected device.
	 */
	void DeviceControlWidget::on_connectPushButton_clicked() {
		// Check to make sure the entry isn't blank or already in the list
		if (!ui->serialOutputComboBox->currentText().isEmpty() && ui->serialOutputListWidget->findItems(ui->serialOutputComboBox->currentText(), Qt::MatchFixedString).count() >= 0) {

			// Initialize and try connecting to the device.
			SerialDeviceController device(ui->serialOutputComboBox->currentText());
			bool connected = device.connect();
			if (connected) {
				ui->serialOutputListWidget->addItem(device.get_port_name());
				serial_devices_.push_back(device);
				save_devices();

				// Update tab icon
				QTabWidget* tabWidget = dynamic_cast<QTabWidget*>(this->parentWidget()->parentWidget()->parentWidget());
				QWidget* tab = tabWidget->findChild<QWidget*>("deviceTab");
				tabWidget->setTabIcon(tabWidget->indexOf(tab), QIcon(":/icon_connected.png"));

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
	 * Saves the new capacity value to settings.
	 */
	void DeviceControlWidget::on_capacityLineEdit_editingFinished() {
		serial_devices_[ui->serialOutputListWidget->currentRow()].set_capacity(ui->capacityLineEdit->text().toInt());
		save_devices();
		check_device_rom_capacity();
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
	 * Toggles real-time serial updates.
	 * @param arg1 If checked, update the device in real-time.
	 */
	void DeviceControlWidget::on_realTimeCheckBox_stateChanged(int arg1) {
		serial_devices_[ui->serialOutputListWidget->currentRow()].set_real_time_update(arg1);
		ui->sectionMapButton->setEnabled(arg1);
		save_devices();
	}

	/**
	 * Disconnects from the selected device.
	 */
	void DeviceControlWidget::on_disconnectPushButton_clicked() {
		if (ui->serialOutputListWidget->selectedItems().count() > 0) {
			int device_index = ui->serialOutputListWidget->currentRow();
			ui->serialOutputListWidget->takeItem(device_index);
			serial_devices_.removeAt(device_index);
			save_devices();

			// If no devices are connected, hide the tab icon
			if (ui->serialOutputListWidget->count() == 0) {
				QTabWidget* tabWidget = dynamic_cast<QTabWidget*>(this->parentWidget()->parentWidget()->parentWidget());
				QWidget* tab = tabWidget->findChild<QWidget*>("deviceTab");
				tabWidget->setTabIcon(tabWidget->indexOf(tab), QIcon(""));
			}
		}
	}

	void DeviceControlWidget::on_refreshButton_clicked() {
		populate_serial_devices();
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

	void DeviceControlWidget::on_sectionMapButton_clicked() {
		SectionMapDialog dialog(
			serial_devices_[ui->serialOutputListWidget->currentRow()],
			this
		);
		dialog.exec();
	}

	void DeviceControlWidget::on_serialOutputComboBox_editTextChanged(const QString &arg1) {
		ui->connectPushButton->setEnabled(arg1.length() > 0);
	}

	void DeviceControlWidget::on_serialOutputListWidget_currentRowChanged(int currentRow) {
		ui->disconnectPushButton->setEnabled(currentRow > -1);
		set_device_controls_enabled(currentRow > -1);
		ui->uploadProgressBar->setValue(0);

		if (currentRow > -1) {
			ui->capacityLineEdit->setText(locale_.toString(serial_devices_[currentRow].get_capacity()));
			ui->realTimeCheckBox->setChecked(serial_devices_[currentRow].get_real_time_refresh_enabled());
			check_device_rom_capacity();
		}
	}

	/// Displays all available serial devices in the serial output combobox.
	void DeviceControlWidget::populate_serial_devices() {
		ui->serialOutputComboBox->clear();
		QList<QSerialPortInfo> ports = QSerialPortInfo::availablePorts();
		for (const QSerialPortInfo& port : ports) {
			ui->serialOutputComboBox->addItem(port.systemLocation());
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
			if (cue[(uint8_t)CueController::Byte::PayloadByte] == (uint8_t)blocked.handler) {
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

				if (action_byte_index == blocked.action) return;
			}
		}

		CueController* controller = &this->maestro_control_widget_.get_maestro_controller()->get_maestro().get_cue_controller();

		for (SerialDeviceController device : serial_devices_) {
			// TODO: Move to separate thread

			/*
			 * If the device has a Section map saved, apply it to the Cue.
			 * This only applies to real-time updates, not Cuefiles.
			 */
			SectionMapModel* model = device.section_map_model;
			if (model && device.get_real_time_refresh_enabled()) {
				// Only check Section-based Cues
				if (!(cue[(uint8_t)CueController::Byte::PayloadByte] == static_cast<uint8_t>(CueController::Handler::MaestroCueHandler) ||
					cue[(uint8_t)CueController::Byte::PayloadByte] == static_cast<uint8_t>(CueController::Handler::ShowCueHandler))) {

					/*
					 * Grab the local section ID from the buffer.
					 * Then, grab the remote section ID from the map.
					 * If they're different, replace the local with the remote ID in the buffer.
					 */

					// SectionByte is the same location for all Section-related handlers (as of v0.30)
					uint8_t target_section_id = cue[(uint8_t)SectionCueHandler::Byte::SectionByte];

					// Make sure the cell actually exists in the model before swapping.
					QStandardItem* cell = model->item(target_section_id, 1);
					if (!cell->text().isEmpty()) {
						int remote_section_id = cell->text().toInt();
						if (remote_section_id != target_section_id) {
							/*
							 * We have a match. Swap the values and reassmble the Cue.
							 *	Although this changes the original Cue,	we perform this check again for each device.
							 *	This way, we don't have to worry about a wrong mapping.
							 *	...Assuming each device has a map.
							 */
							cue[(uint8_t)SectionCueHandler::Byte::SectionByte] = remote_section_id;
							cue[(uint8_t)CueController::Byte::ChecksumByte] = controller->checksum(cue, size);
						}
					}
				}
			}

			if (device.get_device()->isOpen() && device.get_real_time_refresh_enabled()) {
				write_to_device(device,
								reinterpret_cast<char*>(cue),
								size,
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
		settings.beginWriteArray(PreferencesDialog::devices);
		for (int i = 0; i < ui->serialOutputListWidget->count(); i++) {
			settings.setArrayIndex(i);

			SerialDeviceController* device = &serial_devices_[i];
			settings.setValue(PreferencesDialog::device_capacity, device->get_capacity());
			settings.setValue(PreferencesDialog::device_port, device->get_port_name());
			settings.setValue(PreferencesDialog::device_real_time_refresh, device->get_real_time_refresh_enabled());

			// Save the device's model.
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

	void DeviceControlWidget::set_device_controls_enabled(bool enabled) {
		ui->capacityLineEdit->setEnabled(enabled);
		ui->uploadProgressBar->setEnabled(enabled);
		ui->realTimeCheckBox->setEnabled(enabled);
		ui->uploadButton->setEnabled(enabled);
		ui->sectionMapButton->setEnabled(enabled && ui->realTimeCheckBox->isChecked());
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
	 */
	void DeviceControlWidget::update_cuefile_size() {
		// Calculate and display the size of the current Maestro configuration
		QDataStream datastream(&maestro_cue_, QIODevice::Truncate);

		MaestroController* controller = maestro_control_widget_.get_maestro_controller();

		// Generate the Cuefile
		controller->save_maestro_to_datastream(datastream);

		ui->fileSizeLineEdit->setText(locale_.toString(maestro_cue_.size()));
		check_device_rom_capacity();
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

		// FIXME: Using start(), the device pointer magically becomes invalid when executing the thread. But when using run(), the thread becomes blocking
		// Maybe convert threads to QFuture and use QFutureWatcher to update progress? https://doc.qt.io/qt-5/qfuturewatcher.html#details

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
