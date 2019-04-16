#include "cueinterpreterdialog.h"
#include "ui_cueinterpreterdialog.h"
#include "core/maestro.h"
#include "utility/cueinterpreter.h"

namespace PixelMaestroStudio {
	CueInterpreterDialog::CueInterpreterDialog(QWidget *parent, uint8_t* cuefile, uint16_t size) : QDialog(parent), ui(new Ui::CueInterpreterDialog) {
		ui->setupUi(this);

		/*
		 * Create a dummy Maestro to run the Cuefile.
		 * We use the Maestro's CueController to read valid cues, which we then forward to the interpreter.
		 */
		Maestro virtual_maestro = Maestro(nullptr, 0);
		CueController& cue_controller = virtual_maestro.set_cue_controller(UINT16_MAX);

		for (int i = 0; i < size; i++) {
			if (cue_controller.read(cuefile[i])) {
				ui->interpretedCuePlainTextEdit->appendPlainText(CueInterpreter::interpret_cue(cue_controller.get_buffer()));
			}
		}
	}

	CueInterpreterDialog::~CueInterpreterDialog() {
		delete ui;
	}
}
