/*
 * CueModel.h - Class for representing a Cue in different data types.
 */

#include "cuemodel.h"
#include "utility/cueinterpreter.h"
#include "core/maestro.h"
#include <QModelIndex>
#include <QList>
#include <QLocale>
#include <QStandardItem>

using namespace PixelMaestro;

// TODO: Too much c++ code can cause performance issues. Maybe hide by default and allow user to show in preferences.
namespace PixelMaestroStudio {
	CueModel::CueModel(uint8_t* cue, uint32_t size) : QStandardItemModel() {
		QStringList header_labels;
		header_labels.append("Text");
		header_labels.append("Size");
		header_labels.append("Code (C++)");
		setHorizontalHeaderLabels(header_labels);

		// Interpret Cue by creating a dummy Maestro to run it, then pass valid Cues to a CueInterpreter.
		Maestro virtual_maestro = Maestro(nullptr, 0);
		CueController& cue_controller = virtual_maestro.set_cue_controller(UINT16_MAX);

		for (uint32_t i = 0; i < size; i++) {
			if (cue_controller.read(cue[i])) {
				add_cue(cue_controller.get_buffer(), cue_controller.get_cue_size());
			}
		}
	}

	int CueModel::add_cue(uint8_t *cue, uint32_t size) {
		QList<QStandardItem*> items;

		// Add interpreted text
		QStandardItem* interpreted_text_item = new QStandardItem(CueInterpreter::interpret_cue(cue));
		interpreted_text_item->setTextAlignment(Qt::AlignLeft);
		items.append(interpreted_text_item);

		// Add size
		QLocale locale = QLocale::system();
		QStandardItem* size_item = new QStandardItem(locale.toString(size));
		size_item->setTextAlignment(Qt::AlignRight);
		items.append(size_item);

		// Add C++ Code
		int current_row = rowCount(QModelIndex());
		QString cue_num = QString("cue") + QString::number(current_row);
		QString byte_string_prefix = QString("uint8_t " + cue_num + "[") + QString::number(size + 1) + QString("] = ");
		QStandardItem* byte_array_item = new QStandardItem(byte_string_prefix + CueInterpreter::convert_cue_to_byte_array_string(cue, size) + ";");
		byte_array_item->setTextAlignment(Qt::AlignLeft);
		items.append(byte_array_item);

		insertRow(current_row, items);

		return current_row;
	}
}
