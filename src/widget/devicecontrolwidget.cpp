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
#include "controller/devicecontroller.h"
#include "controller/devicethreadcontroller.h"

namespace PixelMaestroStudio {
	DeviceControlWidget::DeviceControlWidget(QWidget *parent) :
			QWidget(parent),
			ui(new Ui::DeviceControlWidget),
			maestro_control_widget_(*dynamic_cast<MaestroControlWidget*>(parent)) {
		ui->setupUi(this);

		// Add saved serial devices to device list.
		QSettings settings;
		int num_serial_devices = settings.beginReadArray(PreferencesDialog::devices);
		for (int device = 0; device < num_serial_devices; device++) {
			settings.setArrayIndex(device);

			QString device_name = settings.value(PreferencesDialog::device_port).toString();
			serial_devices_.push_back(DeviceController(device_name));

			// If the device is set to auto-connect, try connecting
			DeviceController& serial_device = serial_devices_.last();
			if (serial_device.get_autoconnect()) {
				serial_device.connect();
			}

		}
		settings.endArray();

		refresh_device_list();
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
		DeviceController* device = nullptr;
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

		DeviceController device = serial_devices_.at(selected);
		if (!device.get_open()) {
			if (device.connect()) {
				refresh_device_list();
			}
			else {
				QString error = "Unable to connect to device at " + device.get_port_name();
				if (device.get_device()) error += ": " + device.get_device()->errorString();

				QMessageBox::warning(this, "Unable to Connect", error);
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
									static_cast<uint32_t>(maestro_cue_.size()));
		dialog.exec();
	}

	/**
	 * Disconnects from the selected device.
	 */
	void DeviceControlWidget::on_disconnectPushButton_clicked() {
		int selected_index = ui->serialOutputListWidget->currentRow();
		if (selected_index < 0) return;

		DeviceController device = serial_devices_.at(selected_index);

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
		// "ROMEND" flags to the Arduino that we're done transmitting the Cuefile
		QByteArray out = maestro_cue_.append("ROMEND");
		write_to_device(serial_devices_[ui->serialOutputListWidget->currentRow()],
				static_cast<const char*>(maestro_cue_),
				maestro_cue_.size(),
				true);
	}

	void DeviceControlWidget::on_serialOutputListWidget_currentRowChanged(int currentRow) {
		if (currentRow < 0) return;

		DeviceController device = serial_devices_.at(currentRow);

		bool connected = device.get_open();

		ui->connectPushButton->setEnabled(!connected);
		ui->disconnectPushButton->setEnabled(connected);
		ui->uploadButton->setEnabled(connected);
		ui->uploadProgressBar->setValue(0);

		ui->editDeviceButton->setEnabled(currentRow >= 0);
		ui->removeDeviceButton->setEnabled(currentRow >= 0);
	}

	void DeviceControlWidget::refresh_device_list() {
		ui->serialOutputListWidget->clear();
		bool connected_devices = false;
		for (DeviceController device : serial_devices_) {
			QListWidgetItem* item = new QListWidgetItem(device.get_port_name());
			if (device.get_open()) {
				item->setTextColor(Qt::white);
				connected_devices = true;
			}
			else {
				item->setTextColor(Qt::gray);
			}
			ui->serialOutputListWidget->addItem(item);
		}

		// Display icon in tab
		QTabWidget* tab_widget = maestro_control_widget_.findChild<QTabWidget*>("tabWidget");
		QWidget* tab = tab_widget->findChild<QWidget*>("deviceTab");
		if (connected_devices) {
			tab_widget->setTabIcon(tab_widget->indexOf(tab), QIcon(":/icon_connected.png"));
		}
		else {
			tab_widget->setTabIcon(tab_widget->indexOf(tab), QIcon());
		}

		int selected = ui->serialOutputListWidget->currentRow();
		if (selected >= 0) {
			ui->uploadButton->setEnabled(serial_devices_[selected].get_open());
			ui->serialOutputListWidget->setCurrentRow(selected);
		}
		else {
			ui->uploadButton->setEnabled(false);
			ui->connectPushButton->setEnabled(false);
			ui->disconnectPushButton->setEnabled(false);
		}

		ui->editDeviceButton->setEnabled(selected >= 0);
		ui->removeDeviceButton->setEnabled(selected >= 0);
	}

	/**
	 * Updates the Maestro's Cuefile.
	 * This also sends the Cue to all connected devices.
	 * @param cue The Cue to execute.
	 * @param size The size of the Cue.
	 */
	void DeviceControlWidget::run_cue(uint8_t *cue, int size) {
		CueController* controller = &this->maestro_control_widget_.get_maestro_controller()->get_maestro().get_cue_controller();

		for (DeviceController device : serial_devices_) {
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

			if (device.get_open() && device.get_real_time_refresh_enabled()) {
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

			DeviceController* device = &serial_devices_[i];
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
	void DeviceControlWidget::write_to_device(DeviceController& device, const char *out, const int size, bool progress) {
		DeviceThreadController* thread = new DeviceThreadController(device,
														  out,
														  size);

		connect(thread, &DeviceThreadController::finished, thread, &DeviceThreadController::deleteLater);

		if (progress) {
			connect(thread, &DeviceThreadController::progress_changed, this, &DeviceControlWidget::set_progress_bar);
		}

		/*
		 * NOTE: Device pointer becomes invalid when using Thread::start(), likely due to invalid memory access.
		 *		Need to make the DeviceThreadController and output thread-safe before sending it to a separate thread.
		 *
		 *		Maybe convert threads to QFuture and use QFutureWatcher to update progress? https://doc.qt.io/qt-5/qfuturewatcher.html#details
		 *
		 */

		//thread->start();
		thread->run();
	}

	DeviceControlWidget::~DeviceControlWidget() {
		for (DeviceController& device : serial_devices_) {
			device.disconnect();
		}
		delete ui;
	}
}
