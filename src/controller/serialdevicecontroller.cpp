/*
 * SerialDevice - Utility class for managing devices connected via USB/Bluetooth.
 */

#include <QSerialPort>
#include <QSettings>
#include "dialog/preferencesdialog.h"
#include "serialdevicecontroller.h"

namespace PixelMaestroStudio {
	/**
	 * Constructor.
	 * @param port_name The full path name to the device.
	 */
	SerialDeviceController::SerialDeviceController(const QString& port_name) {
		this->port_name_ = port_name;

		// Look up the device in settings
		QSettings settings;
		int num_devices = settings.beginReadArray(PreferencesDialog::devices);
		for (int device	= 0; device < num_devices; device++) {
			settings.setArrayIndex(device);
			QString comp_name = settings.value(PreferencesDialog::device_port).toString();
			if (port_name == comp_name) {
				set_capacity(settings.value(PreferencesDialog::device_capacity).toInt());
				set_real_time_update(settings.value(PreferencesDialog::device_real_time_refresh).toBool());

				// Load Section Map Model
				int num_maps = settings.beginReadArray(PreferencesDialog::section_map);
				if (num_maps > 0) {
					section_map_model->clear();
					for (int map = 0; map < num_maps; map++) {
						settings.setArrayIndex(map);
						section_map_model->add_section();

						QString maps = settings.value(PreferencesDialog::section_map_mapped_sections).toString();
						QStringList map_indices = maps.split(PreferencesDialog::delimiter);
						for (int index = 1; index < map_indices.length(); index++) {
							int check_state = QString(map_indices.at(index)).toInt();
							section_map_model->item(map, index)->setCheckState(Qt::CheckState(check_state));
						}
					}
				}
				settings.endArray();
				break;
			}
		}
		settings.endArray();
	}

	/**
	 * Returns the total capacity of the device's ROM.
	 * @return ROM capacity in bytes.
	 */
	int SerialDeviceController::get_capacity() const {
		return capacity_;
	}

	/**
	 * Connects to the device.
	 * @return True if a connection was established.
	 */
	bool SerialDeviceController::connect() {
		serial_device_->setPortName(port_name_);
		serial_device_->setBaudRate(9600);

		// Set comm settings
		serial_device_->setFlowControl(QSerialPort::FlowControl::NoFlowControl);
		serial_device_->setParity(QSerialPort::Parity::NoParity);
		serial_device_->setDataBits(QSerialPort::DataBits::Data8);
		serial_device_->setStopBits(QSerialPort::StopBits::OneStop);

		return (serial_device_->open(QIODevice::WriteOnly));
	}

	/**
	 * Disconnects the device.
	 * @return True if the disconnection was successful.
	 */
	bool SerialDeviceController::disconnect() {
		bool flushed = serial_device_->flush();
		serial_device_->close();
		return flushed;
	}

	/**
	 * Returns the device.
	 * @return Device object.
	 */
	QSerialPort* SerialDeviceController::get_device() {
		return serial_device_.data();
	}

	/**
	 * Returns the device's port.
	 * @return Device port.
	 */
	QString SerialDeviceController::get_port_name() const {
		return port_name_;
	}

	/**
	 * Returns whether real-time refreshing is enabled for this device.
	 * @return True if enabled.
	 */
	bool SerialDeviceController::get_real_time_refresh_enabled() const {
		return real_time_updates_;
	}

	/**
	 * Sets the device's new capacity.
	 * @param capacity New capacity.
	 */
	void SerialDeviceController::set_capacity(int capacity) {
		this->capacity_ = capacity;
	}

	/**
	 * Sets whether real-time refreshing is enabled for this device.
	 * Real-time refresh sends Cues to this device as they're performed.
	 * @param enabled Whether real-time refresh is enabled.
	 */
	void SerialDeviceController::set_real_time_update(bool enabled) {
		this->real_time_updates_ = enabled;
	}
}
