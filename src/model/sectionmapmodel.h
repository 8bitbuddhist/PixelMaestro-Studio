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
			void remove_section();

		private:
			Maestro* maestro_ = nullptr;

			/// Index of the highest remote Section.
			int current_index_ = -1;
	};
}

#endif // SECTIONMAPMODEL_H
