#include "cueinterpreterdialog.h"
#include "ui_cueinterpreterdialog.h"
#include "core/maestro.h"

namespace PixelMaestroStudio {
	CueInterpreterDialog::CueInterpreterDialog(QWidget *parent, uint8_t* cuefile, uint16_t size) : QDialog(parent), ui(new Ui::CueInterpreterDialog) {
		ui->setupUi(this);
		this->cuefile_ = cuefile;

		Section sections[] = {
			Section(10, 10)
		};
		Maestro maestro = Maestro(sections, 1);
		CueController* controller = maestro.set_cue_controller();
		controller->enable_animation_cue_handler();
		controller->enable_canvas_cue_handler();
		controller->enable_maestro_cue_handler();
		controller->enable_section_cue_handler();
		controller->enable_show_cue_handler();

		for (int i = 0; i < size; i++) {
			if (controller->read(cuefile[i])) {
				ui->interpretedCuePlainTextEdit->appendPlainText(interpreter_.interpret_cue(controller->get_buffer()));
			}
		}
	}

	CueInterpreterDialog::~CueInterpreterDialog() {
		delete ui;
	}
}