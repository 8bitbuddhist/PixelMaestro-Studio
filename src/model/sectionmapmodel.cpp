#include "sectionmapmodel.h"
#include <QList>
#include <QStandardItem>
#include <QStandardItemModel>

namespace PixelMaestroStudio {
	SectionMapModel::SectionMapModel(Maestro& maestro) : QStandardItemModel() {
		QStringList header_labels;
		header_labels.append("Local Section");
		header_labels.append("Remote Section");
		setHorizontalHeaderLabels(header_labels);

		// Add Sections
		for (uint8_t sections = 0; sections < maestro.get_num_sections(); sections++) {
			add_section();
		}
	}

	/**
	 * Adds a Section to the model.
	 */
	void SectionMapModel::add_section() {
		int current_index = rowCount(QModelIndex());

		QList<QStandardItem*> items;

		QStandardItem* local_section_num = new QStandardItem(QString::number(current_index));
		local_section_num->setEnabled(false);
		local_section_num->setTextAlignment(Qt::AlignCenter);
		items.append(local_section_num);

		QStandardItem* remote_section_num = new QStandardItem(QString::number(current_index));
		remote_section_num->setTextAlignment(Qt::AlignCenter);
		items.append(remote_section_num);

		insertRow(current_index, items);
	}
}
