/*
 * SerialDevice - Utility class for managing devices connected via USB/Bluetooth.
 */

#include <QSerialPort>
#include <QSettings>
#include "dialog/preferencesdialog.h"
#include "serialdevice.h"

namespace PixelMaestroStudio {
	SerialDevice::SerialDevice() { }

	/**
	 * Constructor.
	 * @param port_name The full path name to the device.
	 * @param real_time_updates If true, send Cues to this device as they're created.
	 */
	SerialDevice::SerialDevice(QString port_name, bool real_time_updates) {
		this->port_name_ = port_name;
		this->real_time_updates_ = real_time_updates;
	}

	/**
	 * Returns the total capacity of the device's ROM.
	 * @return ROM capacity in bytes.
	 */
	int SerialDevice::get_capacity() const {
		return capacity_;
	}

	/**
	 * Connects to the device.
	 * @return True if a connection was established.
	 */
	bool SerialDevice::connect() {
		serial_device_->setPortName(port_name_);
		serial_device_->setBaudRate(9600);

		// Set comm settings
		serial_device_->setFlowControl(QSerialPort::FlowControl::NoFlowControl);
		serial_device_->setParity(QSerialPort::Parity::NoParity);
		serial_device_->setDataBits(QSerialPort::DataBits::Data8);
		serial_device_->setStopBits(QSerialPort::StopBits::OneStop);

		if (serial_device_->open(QIODevice::WriteOnly)) {
			return true;
		}
		else {
			return false;
		}
	}

	/**
	 * Disconnects the device.
	 * @return True if the disconnection was successful.
	 */
	bool SerialDevice::disconnect() {
		bool flushed = serial_device_->flush();
		serial_device_->close();
		return flushed;
	}

	/**
	 * Returns the device.
	 * @return Device object.
	 */
	QSerialPort* SerialDevice::get_device() {
		return serial_device_.data();
	}

	/**
	 * Returns the device's port.
	 * @return Device port.
	 */
	QString SerialDevice::get_port_name() const {
		return port_name_;
	}

	/**
	 * Returns whether real-time refreshing is enabled for this device.
	 * @return True if enabled.
	 */
	bool SerialDevice::get_real_time_refresh_enabled() const {
		return real_time_updates_;
	}

	/**
	 * Sets the device's new capacity.
	 * @param capacity New capacity.
	 */
	void SerialDevice::set_capacity(int capacity) {
		this->capacity_ = capacity;
	}

	/**
	 * Sets whether real-time refreshing is enabled for this device.
	 * Real-time refresh sends Cues to this device as they're performed.
	 * @param enabled Whether real-time refresh is enabled.
	 */
	void SerialDevice::set_real_time_refresh_enabled(bool enabled) {
		this->real_time_updates_ = enabled;
	}

	/**
	 * Writes data to the device.
	 * @param out Data to send.
	 * @param size Size of the data.
	 */
	void SerialDevice::write(const char *out, int size) {
		serial_device_->write(out, size);
		serial_device_->flush();
	}
}