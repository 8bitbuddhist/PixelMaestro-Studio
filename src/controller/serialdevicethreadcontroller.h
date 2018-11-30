#ifndef SERIALDEVICETHREAD_H
#define SERIALDEVICETHREAD_H

#include <QByteArray>
#include <QThread>
#include "serialdevicecontroller.h"

namespace PixelMaestroStudio {
	class SerialDeviceThreadController : public QThread {
		Q_OBJECT

		public:
			SerialDeviceThreadController(SerialDeviceController* serial_device, const char* out, int size);
			void run() override;

		signals:
			void progress_changed(int progress);

		private:
			SerialDeviceController* serial_device_ = nullptr;
			QByteArray output_;
	};
}

#endif // SERIALDEVICETHREAD_H
