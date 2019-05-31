/*
 * CueModel.h - Class for representing a Cue in different data types.
 */

#ifndef CUEMODEL_H
#define CUEMODEL_H

#include <QByteArray>
#include <QStandardItemModel>
#include <QString>

namespace PixelMaestroStudio {
	class CueModel : public QStandardItemModel {
		public:
			CueModel(uint8_t* cue, uint16_t size);

		private:
			int add_cue(uint8_t* cue, uint16_t size);
	};
}

#endif // CUEMODEL_H
