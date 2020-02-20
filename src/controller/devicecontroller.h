/*
 * SerialDevice - Utility class for managing devices connected via USB/Bluetooth.
 */

#ifndef SERIALDEVICE_H
#define SERIALDEVICE_H

#include <QIODevice>
#include <QSharedPointer>
#include <QString>
#include "model/sectionmapmodel.h"

namespace PixelMaestroStudio {
	class DeviceController {
		public:
			enum DeviceType {
				Serial,
				TCP
			};

			/// Default connect/disconnect timeout to 10 seconds
			static const uint16_t TIMEOUT = 10000;

			DeviceController() = default;
			explicit DeviceController(const QString& port_name);
			bool connect();
			bool disconnect();
			int get_capacity() const;
			QIODevice* get_device() const;
			QString get_error() const;
			bool get_open() const;
			QString get_port_name() const;
			bool get_autoconnect() const;
			bool get_real_time_refresh_enabled() const;
			void flush();
			void set_autoconnect(const bool autoconnect);
			void set_capacity(const int capacity);
			void set_port_name(const QString &port_name);
			void set_real_time_update(const bool enabled);
			void write(const QByteArray &array);

			/// Custom mapping of local Sections to remote Sections. Made public because of weird pointer issues. Fix later.
			SectionMapModel* section_map_model = nullptr;

		private:
			/// Whether to connect to the device on startup.
			bool autoconnect_ = false;

			/// The baud rate.
			const int baud_rate_ = 9600;

			/// The maximum number of bytes the device's ROM can hold.
			int capacity_ = 1024;

			/// The actual device type.
			QSharedPointer<QIODevice> device_;

			/// The type of connected device.
			DeviceType device_type_	= DeviceType::Serial;

			/// The full path to the device (QSerialPortInfo::systemLocation()).
			QString port_name_;

			/// If true, commands will be sent to the device in real-time.
			bool real_time_updates_ = false;
	};
}

#endif // SERIALDEVICE_H
