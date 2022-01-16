#include "cueinterpreterdialog.h"
#include "ui_cueinterpreterdialog.h"
#include "core/maestro.h"
#include "dialog/preferencesdialog.h"
#include "utility/cueinterpreter.h"
#include <QClipboard>
#include <QSettings>
#include <QString>

/**
 *	NOTE: Maybe add the ability to edit Cues
 *  - Allow users to load and save Cuefiles from the dialog
 *  - Users can delete Cues (e.g. remove a line)
 */
namespace PixelMaestroStudio {
	CueInterpreterDialog::CueInterpreterDialog(QWidget *parent, uint8_t* cuefile, uint32_t size) : QDialog(parent), ui(new Ui::CueInterpreterDialog), model_(cuefile, size) {
		ui->setupUi(this);

		setWindowIcon(QIcon("qrc:/../../../docsrc/images/logo.png"));

		ui->interpretedCueTableView->setModel(&model_);
		ui->interpretedCueTableView->resizeColumnToContents(0);
		ui->interpretedCueTableView->setTextElideMode(Qt::ElideRight);
		ui->interpretedCueTableView->horizontalHeader()->setStretchLastSection(true);
		ui->interpretedCueTableView->setWordWrap(false);
		ui->interpretedCueTableView->resizeRowsToContents();

		// Show or hide C++ code column, depending on user's settings
		QSettings settings;
		bool show_code = settings.value(PreferencesDialog::show_cue_code, false).toBool();
		ui->showCueCodeCheckBox->setChecked(show_code);
		if (!show_code) {
			ui->interpretedCueTableView->hideColumn(2);
		}

		// Restore window geometry
		restoreGeometry(settings.value(geometry_str).toByteArray());

		ui->interpretedCueTableView->show();
	}

	void CueInterpreterDialog::on_closeButton_clicked() {
		QSettings settings;
		settings.setValue(geometry_str, saveGeometry());

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

		if (list.size() > 0) {
			for (int i = 0; i < list.size(); i++) {
				QModelIndex item = list.at(i);
				text.append(item.data().toString()).append("\n");
			}
		}
		else { // No items selected - copy everything
			for (int i = 0; i < model_.rowCount(); i++) {
				QStandardItem* item = model_.item(i, 2);
				text.append(item->text()).append("\n");
			}
		}
		clipboard->setText(text);
	}

	void CueInterpreterDialog::on_showCueCodeCheckBox_toggled(bool checked) {
		if (checked) {
			ui->interpretedCueTableView->showColumn(2);
		}
		else {
			ui->interpretedCueTableView->hideColumn(2);
		}
	}

	CueInterpreterDialog::~CueInterpreterDialog() {
		QSettings settings;
		settings.setValue(PreferencesDialog::show_cue_code, ui->showCueCodeCheckBox->isChecked());
		delete ui;
	}
}
