#ifndef SECTIONMAPMODEL_H
#define SECTIONMAPMODEL_H

#include "core/maestro.h"
#include <QStandardItemModel>

using namespace PixelMaestro;

namespace PixelMaestroStudio {
	class SectionMapModel : public QStandardItemModel {
		public:
			SectionMapModel(Maestro* maestro);
			~SectionMapModel() = default;
			void add_section();
	};
}

#endif // SECTIONMAPMODEL_H
