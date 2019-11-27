#include "window/mainwindow.h"
#include <QApplication>
#include <QFontDatabase>
#include <QFormLayout>
#include <QMessageBox>
#include <QStyleFactory>
#include <QVBoxLayout>

int main(int argc, char* argv[]) {
	// Set application parameters for QSettings
	QCoreApplication::setOrganizationName("PixelMaestro");
	QCoreApplication::setApplicationName("PixelMaestro Studio");

	QApplication app(argc, argv);
	PixelMaestroStudio::MainWindow w;

	// Set global color palette (https://gist.github.com/QuantumCD/6245215)
	app.setStyle(QStyleFactory::create("Fusion"));

	QPalette palette;
	palette.setColor(QPalette::Window, QColor(53,53,53));
	palette.setColor(QPalette::WindowText, Qt::white);
	palette.setColor(QPalette::Base, QColor(15,15,15));
	palette.setColor(QPalette::AlternateBase, QColor(53,53,53));
	palette.setColor(QPalette::ToolTipBase, QColor(15,15,15));
	palette.setColor(QPalette::ToolTipText, Qt::white);
	palette.setColor(QPalette::Text, Qt::white);
	palette.setColor(QPalette::Button, QColor(53,53,53));
	palette.setColor(QPalette::ButtonText, Qt::white);
	palette.setColor(QPalette::BrightText, Qt::red);
	palette.setColor(QPalette::Highlight, QColor(142,45,197).lighter());
	palette.setColor(QPalette::HighlightedText, Qt::black);
	app.setPalette(palette);

	// Change color of QToolButtons when checked
	//app.setStyleSheet(QString(".QToolButton:checked { background-color: rgb(142, 45, 197); }"));

	// Set application font
	int id = QFontDatabase::addApplicationFont(":/FiraSans-Regular.ttf");
	app.setFont(QFont(QFontDatabase::applicationFontFamilies(id).at(0), 10));

	// Verify main layout is present
	QVBoxLayout *main_layout = w.findChild<QVBoxLayout*>("mainLayout");
	Q_ASSERT(main_layout);

	// Enable high DPI support
	app.setAttribute(Qt::ApplicationAttribute::AA_EnableHighDpiScaling, true);

	w.show();

	try {
		return app.exec();
	}
	catch (std::exception& ex) {
		QMessageBox::critical(&w, QString("Unhandled Exception"), QString("A critical error has occurred. The application will now close. Error: ") + QString::fromLatin1(ex.what()));
		return 1;
	}
}
