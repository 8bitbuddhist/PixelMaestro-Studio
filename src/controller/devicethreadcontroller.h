#ifndef SERIALDEVICETHREAD_H
#define SERIALDEVICETHREAD_H

#include <QByteArray>
#include <QThread>
#include "devicecontroller.h"

namespace PixelMaestroStudio {
	class DeviceThreadController : public QThread {
		Q_OBJECT

		public:
			DeviceThreadController(DeviceController& device, const char* out, int size);
			void run() override;

		signals:
			void progress_changed(int progress);

		private:
			/// Size of each chunk in bytes
			const uint8_t CHUNK_SIZE = 64;

			/// Time in milliseconds between chunks. Disabled by default.
			const uint8_t SLEEP_INTERVAL = 0;

			DeviceController& device_;
			QByteArray output_;
	};
}

#endif // SERIALDEVICETHREAD_H
