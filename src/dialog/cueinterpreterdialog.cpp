#include "cueinterpreterdialog.h"
#include "ui_cueinterpreterdialog.h"
#include "core/maestro.h"
#include "utility/cueinterpreter.h"
#include <QClipboard>

namespace PixelMaestroStudio {
	CueInterpreterDialog::CueInterpreterDialog(QWidget *parent, uint8_t* cuefile, uint16_t size) : QDialog(parent), ui(new Ui::CueInterpreterDialog), model_(cuefile, size) {
		ui->setupUi(this);

		ui->interpretedCueTableView->setModel(&model_);
		ui->interpretedCueTableView->resizeColumnToContents(0);
		ui->interpretedCueTableView->setTextElideMode(Qt::ElideRight);
		ui->interpretedCueTableView->horizontalHeader()->setStretchLastSection(true);
		ui->interpretedCueTableView->setWordWrap(false);
		ui->interpretedCueTableView->resizeRowsToContents();

		ui->interpretedCueTableView->show();
	}

	void CueInterpreterDialog::on_closeButton_clicked() {
		this->close();
	}


	/**
	 * Copies the string representation of the Cue to clipboard.
	 */
	void CueInterpreterDialog::on_copyButton_clicked() {
		QClipboard* clipboard = QApplication::clipboard();

		// Copy selected cells to clipboard. It's important that we preserve the order of selected Cues
		QString text("");
		QModelIndexList list = ui->interpretedCueTableView->selectionModel()->selectedIndexes();
		for (int i = 0; i < list.size(); i++) {
			QModelIndex item = list.at(i);
			text.append(item.data().toString()).append("\n");
		}
		clipboard->setText(text);
	}

	CueInterpreterDialog::~CueInterpreterDialog() {
		delete ui;
	}
}
