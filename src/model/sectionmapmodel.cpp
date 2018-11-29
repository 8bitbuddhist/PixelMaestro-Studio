#include "sectionmapmodel.h"
#include <QCheckBox>
#include <QStandardItemModel>
#include <QStringList>

namespace PixelMaestroStudio {
	SectionMapModel::SectionMapModel(Maestro* maestro) : QStandardItemModel() {
		this->maestro_ = maestro;

		// Build header. Column count is based on the number of Maestro Sections.
		QStringList header_labels("Remote Section #");
		for (uint8_t i; i < maestro->get_num_sections(); i++) {
			header_labels.append("Section "	+ QString::number(i));
		}
		setHorizontalHeaderLabels(header_labels);

		// Add one default Section
		add_section();
	}

	/**
	 * Adds a Section to the model.
	 */
	void SectionMapModel::add_section() {
		current_index_++;

		QList<QStandardItem*> items;
		QStandardItem* section_num = new QStandardItem(QString::number(current_index_));
		section_num->setEnabled(false);
		section_num->setTextAlignment(Qt::AlignCenter);
		items.append(section_num);

		// Add a checkbox for each Section in the Maestro.
		for (uint8_t index = 0; index < maestro_->get_num_sections(); index++) {
			QStandardItem* checkbox = new QStandardItem();
			checkbox->setTextAlignment(Qt::AlignCenter);
			checkbox->setEditable(false);
			checkbox->setCheckable(true);
			items.append(checkbox);
		}

		insertRow(current_index_, items);
	}

	/**
	 * Removes the last Section from the model.
	 */
	void SectionMapModel::remove_section() {
		removeRow(current_index_);
		current_index_--;
	}
}
