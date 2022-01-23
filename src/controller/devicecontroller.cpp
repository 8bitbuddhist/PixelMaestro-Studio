/*
 * SerialDevice - Utility class for managing devices connected via USB/Bluetooth.
 */

#ifndef NO_SERIALPORT
#include <QSerialPort>
#endif

#include <QRegularExpression>
#include <QSettings>
#include <QTcpSocket>
#include "dialog/preferencesdialog.h"
#include "devicecontroller.h"
#include "widget/maestrocontrolwidget.h"

namespace PixelMaestroStudio {
	/**
	 * Constructor.
	 * @param port_name The full path name to the device.
	 */
	DeviceController::DeviceController(const QString& port_name) {
		this->port_name_ = port_name;

		/*
		 * FIXME: Use the device_type supplied by the user instead of assuming
		 * Check whether the port name is an IP address.
		 * If so, initialize a TCP socket.
		 * Otherwise, assume a serial device.
		 * The only exception is if we're using a device that doesn't support serial devices.
		 */
		QRegularExpression exp("^(?:[0-9]{1,3}.){3}[0-9]{1,3}");
		bool is_network_device = exp.match(port_name).hasMatch();
		if (is_network_device) {
			this->device_ = QSharedPointer<QTcpSocket>(new QTcpSocket());
			this->device_type_ = DeviceType::TCP;
		}
#ifndef NO_SERIALPORT
		else {
			this->device_ = QSharedPointer<QSerialPort>(new QSerialPort());
			this->device_type_ = DeviceType::Serial;
		}
#endif

		// Look up the device in settings
		QSettings settings;
		int num_devices = settings.beginReadArray(PreferencesDialog::devices);
		for (int device	= 0; device < num_devices; device++) {
			settings.setArrayIndex(device);
			QString comp_name = settings.value(PreferencesDialog::device_port).toString();
			if (port_name == comp_name) {
				set_real_time_update(settings.value(PreferencesDialog::device_real_time_refresh).toBool());
				set_autoconnect(settings.value(PreferencesDialog::device_autoconnect).toBool());

				// Load Section Map model (if it exists)
				int num_maps = settings.beginReadArray(PreferencesDialog::section_map);
				if (num_maps > 0) {
					section_map_model = new SectionMapModel();
					for (int row = 0; row < num_maps; row++) {
						settings.setArrayIndex(row);
						section_map_model->add_section();

						QString remote_section = settings.value(PreferencesDialog::section_map_remote).toString();
						section_map_model->item(row, 1)->setText(remote_section);
					}
				}

				settings.endArray();
				break;
			}
		}
		settings.endArray();
	}

	/**
	 * Connects to the device.
	 * @return True if a connection was established.
	 */
	bool DeviceController::connect() {
		if (device_type_ == DeviceType::Serial) {
#ifndef NO_SERIALPORT
			QSerialPort* serial_device = dynamic_cast<QSerialPort*>(device_.data());

			if (!serial_device) return false;

			serial_device->setPortName(port_name_);
			serial_device->setBaudRate(baud_rate_);

			// Set comm settings
			serial_device->setFlowControl(QSerialPort::FlowControl::NoFlowControl);
			serial_device->setParity(QSerialPort::Parity::NoParity);
			serial_device->setDataBits(QSerialPort::DataBits::Data8);
			serial_device->setStopBits(QSerialPort::StopBits::OneStop);

			return (serial_device->open(QIODevice::WriteOnly));
#endif
		}
		else if (device_type_ == DeviceType::TCP) {
			// Extract the IP address and port number
			QRegularExpression address_re("^(?:[0-9]{1,3}\\.){3}[0-9]{1,3}");
			QRegularExpression port_re("[^:][0-9]+$");

			QString address = address_re.match(port_name_).captured(0);
			QString port = port_re.match(port_name_).captured(0);

			// If no port number is found, use the default
			uint16_t port_num = static_cast<uint16_t>(port.toUInt());
			if (port_num == 0) {
				QSettings settings;
				port_num = PORT_NUM;
			}

			QTcpSocket* tcp_device = dynamic_cast<QTcpSocket*>(device_.data());

			if (!tcp_device) return false;

			tcp_device->connectToHost(address, port_num);

			// Wait for a connection
			// TODO: Make non-blocking
			return tcp_device->waitForConnected(TIMEOUT);
		}

		return false;
	}

	/**
	 * Disconnects the device.
	 * @return True if the disconnection was successful.
	 */
	bool DeviceController::disconnect() {
		if (device_type_ == DeviceType::Serial && device_) {
#ifndef NO_SERIALPORT
			bool flushed = dynamic_cast<QSerialPort*>(device_.data())->flush();
			device_->close();
			return flushed;
#endif
		}
		else if (device_type_ == DeviceType::TCP && device_) {
			QTcpSocket* tcp_device = dynamic_cast<QTcpSocket*>(device_.data());
			tcp_device->flush();
			tcp_device->disconnectFromHost();
			return true;
		}

		return false;
	}

	/**
	 * Returns whether autoconnect is enabled.
	 * @return True if enabled.
	 */
	bool DeviceController::get_autoconnect() const {
		return autoconnect_;
	}

	/**
	 * Returns the actual device object.
	 * @return Device.
	 */
	QIODevice* DeviceController::get_device() const {
		return device_.data();
	}

	/**
	 * @brief Return the connection type.
	 * @return Connection type.
	 */
	DeviceController::DeviceType DeviceController::get_device_type() const {
		return device_type_;
	}

	/**
	 * Returns whether the device is connected and writeable.
	 * @return True if the device is connected.
	 */
	bool DeviceController::get_open() const {
		if (device_) {
			switch (device_type_) {
				case DeviceType::Serial:
#ifndef NO_SERIALPORT
					return dynamic_cast<QSerialPort*>(device_.data())->isOpen();
#else
					return false;
#endif
				case DeviceType::TCP:
					return dynamic_cast<QTcpSocket*>(device_.data())->state() == QAbstractSocket::ConnectedState;
			}
		}

		return false;
	}

	/**
	 * Returns the device's port.
	 * @return Device port.
	 */
	QString DeviceController::get_port_name() const {
		return port_name_;
	}

	/**
	 * Returns whether real-time refreshing is enabled for this device.
	 * @return True if enabled.
	 */
	bool DeviceController::get_real_time_refresh_enabled() const {
		return real_time_updates_;
	}

	/**
	 * Writes any buffered data to the device.
	 */
	void DeviceController::flush() {
		switch (device_type_) {
			case DeviceType::Serial:
#ifndef NO_SERIALPORT
				dynamic_cast<QSerialPort*>(device_.data())->flush();
#endif
				break;
			case DeviceType::TCP:
				dynamic_cast<QTcpSocket*>(device_.data())->flush();
				break;
		}
	}

	/**
	 * Sets whether to automatically connect to the device on startup.
	 * @param autoconnect If true, autoconnect to the device.
	 */
	void DeviceController::set_autoconnect(bool autoconnect) {
		this->autoconnect_ = autoconnect;
	}

	/**
	 * @brief Sets the current device type.
	 * @param device_type Type of device.
	 */
	void DeviceController::set_device_type(const DeviceController::DeviceType device_type) {
		this->device_type_ = device_type;
	}

	/**
	 * Sets the device's address.
	 * @param port_name The URI of the device (can be a port name or IP address).
	 */
	void DeviceController::set_port_name(const QString &port_name) {
		this->port_name_ = port_name;
	}

	/**
	 * Sets whether real-time refreshing is enabled for this device.
	 * Real-time refresh sends Cues to this device as they're performed.
	 * @param enabled Whether real-time refresh is enabled.
	 */
	void DeviceController::set_real_time_update(bool enabled) {
		this->real_time_updates_ = enabled;
	}
}
