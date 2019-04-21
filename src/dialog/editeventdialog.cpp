#include "editeventdialog.h"
#include "ui_editeventdialog.h"
#include "utility/cueinterpreter.h"

namespace PixelMaestroStudio {
	EditEventDialog::EditEventDialog(Event& event, QWidget *parent) : QDialog(parent), ui(new Ui::EditEventDialog), event_(event) {
		ui->setupUi(this);

		ui->cueLineEdit->setText(CueInterpreter::interpret_cue(event.get_cue()));
		ui->timeSpinBox->setValue(event.get_time());

		ui->timeSpinBox->setFocus();
	}

	void EditEventDialog::on_buttonBox_accepted() {
		uint32_t new_time = ui->timeSpinBox->value();

		if (new_time != event_.get_time()) {
			event_.set_time(new_time);
		}
	}

	EditEventDialog::~EditEventDialog() {
		delete ui;
	}
}
