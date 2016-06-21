#include "MainWindow.hpp"
#include <unordered_map>
#include <QComboBox>
#include <QCoreApplication>
#include <QFileDialog>
#include <QFontDatabase>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMenuBar>
#include <QMessageBox>
#include <QPushButton>
#include <QSerialPort>
#include <QToolBar>
#include <QDebug>
#include <marnav/nmea/nmea.hpp>
#include <marnav/nmea/checksum.hpp>
#include <marnav/nmea/string.hpp>
#include <marnav/nmea/gga.hpp>
#include <marnav/nmea/mwv.hpp>
#include <marnav/nmea/rmc.hpp>

namespace marnav_example
{
namespace detail
{
template <typename T>
static QString render(const marnav::utils::optional<T> & t)
{
	if (!t)
		return "-";
	return marnav::nmea::to_string(*t).c_str();
}

static QString render(const marnav::utils::optional<marnav::nmea::time> & t)
{
	if (!t)
		return "-";
	return QString{"%1:%2:%3"}
		.arg(t->hour(), 2, 10, QLatin1Char('0'))
		.arg(t->minutes(), 2, 10, QLatin1Char('0'))
		.arg(t->seconds(), 2, 10, QLatin1Char('0'));
}

static QString render(const marnav::utils::optional<marnav::geo::latitude> & t)
{
	if (!t)
		return "-";
	return QString{" %1°%2'%3%4"}
		.arg(t->degrees(), 2, 10, QLatin1Char('0'))
		.arg(t->minutes(), 2, 10, QLatin1Char('0'))
		.arg(t->seconds(), 2, 'f', 1, QLatin1Char('0'))
		.arg(to_string(t->hem()).c_str());
}

static QString render(const marnav::utils::optional<marnav::geo::longitude> & t)
{
	if (!t)
		return "-";
	return QString{"%1°%2'%3%4"}
		.arg(t->degrees(), 3, 10, QLatin1Char('0'))
		.arg(t->minutes(), 2, 10, QLatin1Char('0'))
		.arg(t->seconds(), 2, 'f', 1, QLatin1Char('0'))
		.arg(to_string(t->hem()).c_str());
}

static QString details_rmc(const marnav::nmea::sentence * s)
{
	const auto t = marnav::nmea::sentence_cast<marnav::nmea::rmc>(s);
	QString result;
	result += "\nTime UTC : " + render(t->get_time_utc());
	result += "\nStatus   : " + render(t->get_status());
	result += "\nLatitude : " + render(t->get_latitude());
	result += "\nLongitude: " + render(t->get_longitude());
	result += "\nSOG      : " + render(t->get_sog());
	result += "\nHeading  : " + render(t->get_heading());
	result += "\nDate     : " + render(t->get_date());
	result += "\nMagn Dev : " + render(t->get_mag());
	result += "\nMagn Hem : " + render(t->get_mag_hem());
	result += "\nMode Ind : " + render(t->get_mode_indicator());
	return result;
}

static QString details_mwv(const marnav::nmea::sentence * s)
{
	const auto t = marnav::nmea::sentence_cast<marnav::nmea::mwv>(s);
	QString result;
	result += "\nAngle     : " + render(t->get_angle());
	result += "\nAngle Ref : " + render(t->get_angle_ref());
	result += "\nSpeed     : " + render(t->get_speed());
	result += "\nSpeed Unit: " + render(t->get_speed_unit());
	result += "\nData Valid: " + render(t->get_data_valid());
	return result;
}

static QString details_gga(const marnav::nmea::sentence * s)
{
	const auto t = marnav::nmea::sentence_cast<marnav::nmea::gga>(s);
	QString result;
	result += "\nTime            : " + render(t->get_time());
	result += "\nLatitude        : " + render(t->get_latitude());
	result += "\nLongitude       : " + render(t->get_longitude());
	result += "\nQuality Ind     : " + render(t->get_quality_indicator());
	result += "\nNum Satellites  : " + render(t->get_n_satellites());
	result += "\nHoriz Dilution  : " + render(t->get_hor_dilution());
	result += "\nAltitude        : " + render(t->get_altitude());
	result += "\nAltitude Unit   : " + render(t->get_altitude_unit());
	result += "\nGeodial Sep     : " + render(t->get_geodial_separation());
	result += "\nGeodial Sep Unit: " + render(t->get_geodial_separation_unit());
	result += "\nDGPS Age        : " + render(t->get_dgps_age());
	result += "\nDGPS Ref        : " + render(t->get_dgps_ref());
	return result;
}
}

static QString get_details(const marnav::nmea::sentence * s)
{
	using container =
	std::unordered_map<marnav::nmea::sentence_id,
		std::function<QString(const marnav::nmea::sentence *)>>;
	static const container
		details = {
			{marnav::nmea::sentence_id::GGA, detail::details_gga},
			{marnav::nmea::sentence_id::MWV, detail::details_mwv},
			{marnav::nmea::sentence_id::RMC, detail::details_rmc},
		};

	auto i = std::find_if(begin(details), end(details),
		[s](const container::value_type & entry) { return entry.first == s->id(); });

	return (i == end(details)) ? QString{"unknown"} : i->second(s);
}

MainWindow::MainWindow()
{
	setWindowTitle(QCoreApplication::instance()->applicationName());

	create_actions();
	create_menus();
	setup_ui();

	port = new QSerialPort(this);
}

MainWindow::~MainWindow() { on_close(); }

void MainWindow::create_menus()
{
	auto menu_file = menuBar()->addMenu(tr("&File"));
	menu_file->addAction(action_open_file);
	menu_file->addSeparator();
	menu_file->addAction(action_open_port);
	menu_file->addAction(action_close_port);
	menu_file->addSeparator();
	menu_file->addAction(action_exit);

	auto menu_help = menuBar()->addMenu(tr("&Help"));
	menu_help->addAction(action_about);
	menu_help->addAction(action_about_qt);
}

void MainWindow::create_actions()
{
	action_exit = new QAction(tr("E&xit"), this);
	action_exit->setStatusTip(tr("Quits the application"));
	action_exit->setShortcut(tr("Ctrl+Q"));
	connect(action_exit, &QAction::triggered, this, &MainWindow::close);

	action_about = new QAction(tr("About ..."), this);
	action_about->setStatusTip(tr("Shows the About dialog"));
	connect(action_about, &QAction::triggered, this, &MainWindow::on_about);

	action_about_qt = new QAction(tr("About Qt ..."), this);
	action_about_qt->setStatusTip(tr("Shows information about Qt"));
	connect(action_about_qt, &QAction::triggered, this, &MainWindow::on_about_qt);

	action_open_port = new QAction(tr("Open Port"), this);
	action_open_port->setStatusTip(tr("Opens Communication Port to read data"));
	connect(action_open_port, &QAction::triggered, this, &MainWindow::on_open);

	action_close_port = new QAction(tr("Close Port"), this);
	action_close_port->setStatusTip(tr("Closes Communication Port"));
	connect(action_close_port, &QAction::triggered, this, &MainWindow::on_close);
	action_close_port->setEnabled(false);

	action_open_file = new QAction(tr("Open File"), this);
	action_open_file->setStatusTip(tr("Opens file containing NMEA sentences"));
	connect(action_open_file, &QAction::triggered, this, &MainWindow::on_open_file);
}

void MainWindow::setup_ui()
{
	QWidget * center = new QWidget(this);
	center->setMinimumWidth(1024);
	center->setMinimumHeight(600);

	sentence_list = new QListWidget(this);
	sentence_list->setSelectionMode(QAbstractItemView::SingleSelection);
	connect(sentence_list, &QListWidget::itemSelectionChanged, this,
		&MainWindow::on_sentence_selection);

	sentence_desc = new QLabel("", this);
	sentence_desc->setTextFormat(Qt::PlainText);
	sentence_desc->setAlignment(Qt::AlignLeft | Qt::AlignTop);

	QFont font{"Monospace"};
	font.setStyleHint(QFont::TypeWriter);

	sentence_list->setFont(font);
	sentence_desc->setFont(font);

	port_name = new QLineEdit(center);
	port_name->setText("/dev/ttyUSB0");
	cb_baudrate = new QComboBox(center);
	cb_baudrate->setEditable(false);
	cb_baudrate->addItems({"4800", "38400"});

	auto l1 = new QHBoxLayout;
	l1->addWidget(sentence_list, 2);
	l1->addWidget(sentence_desc, 1);

	auto l0 = new QGridLayout;
	l0->addWidget(new QLabel(tr("Port:"), this), 0, 0);
	l0->addWidget(port_name, 0, 1);
	l0->addWidget(cb_baudrate, 0, 2);
	l0->addLayout(l1, 1, 0, 1, 3);

	center->setLayout(l0);
	setCentralWidget(center);
}

void MainWindow::on_open_file()
{
	auto filename
		= QFileDialog::getOpenFileName(this, tr("Open File"), ".", tr("Text Files (*.txt)"));

	if (filename.size() == 0)
		return;

	sentence_list->clear();
	sentence_desc->setText("");

	QFile file{filename};
	if (!file.open(QFile::ReadOnly)) {
		QMessageBox::critical(this, "Error", "Unable to open file: " + filename);
		return;
	}

	QTextStream ifs(&file);
	do {
		QString s = ifs.readLine();
		if (s.isNull())
			break;
		if (s.isEmpty())
			continue;
		if (s.startsWith("#"))
			continue;
		sentence_list->addItem(s);
	} while (true);
}

void MainWindow::on_about()
{
	QCoreApplication * app = QCoreApplication::instance();
	QMessageBox::about(this, app->applicationName(), app->applicationName()
			+ ": example of Qt using marnav\n\nVersion: " + app->applicationVersion()
			+ "\n\nSee file: LICENSE");
}

void MainWindow::on_about_qt()
{
	QCoreApplication * app = QCoreApplication::instance();
	QMessageBox::aboutQt(this, app->applicationName());
}

void MainWindow::on_sentence_selection()
{
	auto items = sentence_list->selectedItems();
	if (items.empty())
		return;

	auto item = items.front();
	try {
		auto s = marnav::nmea::make_sentence(item->text().toStdString());
		sentence_desc->setText(QString{"Tag: %1\nTalker: %2\n\n%3\n"}
								   .arg(s->tag().c_str())
								   .arg(s->talker().c_str())
								   .arg(get_details(s.get())));
	} catch (marnav::nmea::unknown_sentence &) {
		sentence_desc->setText("Unknown Sentence");
		item->setForeground(Qt::red);
	} catch (marnav::nmea::checksum_error &) {
		sentence_desc->setText("Checksum Error");
		item->setForeground(Qt::red);
	} catch (std::invalid_argument & error) {
		sentence_desc->setText("Error: " + QString::fromStdString(error.what()));
		item->setForeground(Qt::red);
	}
}

void MainWindow::on_open()
{
	action_open_port->setEnabled(false);
	action_close_port->setEnabled(true);
	action_open_file->setEnabled(false);
	cb_baudrate->setEnabled(false);
	port_name->setEnabled(false);

	port->setPortName(port_name->text());
	port->setBaudRate(cb_baudrate->currentText().toInt());
	port->setParity(QSerialPort::NoParity);
	port->setDataBits(QSerialPort::Data8);
	port->setStopBits(QSerialPort::OneStop);

	connect(port, &QSerialPort::readyRead, this, &MainWindow::on_data_ready);

	if (!port->open(QIODevice::ReadOnly)) {
		on_close();
		QMessageBox::critical(this, "Error", "Unable to open port: " + port->portName());
	}
}

void MainWindow::on_close()
{
	action_open_port->setEnabled(true);
	action_close_port->setEnabled(true);
	action_open_file->setEnabled(true);
	cb_baudrate->setEnabled(true);
	port_name->setEnabled(true);

	disconnect(port, &QSerialPort::readyRead, this, &MainWindow::on_data_ready);

	port->close();
}

void MainWindow::process_nmea()
{
	try {
		auto sentence = marnav::nmea::make_sentence(received_data);

		// sentence is ok, for now: just show the received data
		// text->appendPlainText(received_data.c_str());

		// TODO: print sentence specific data
		//       ... not shown here
	} catch (...) {
		// ignore
	}
}

void MainWindow::on_data_ready()
{
	while (true) {
		char raw;
		auto rc = port->read(&raw, sizeof(raw));
		if (rc == 0) {
			// no more data for now
			return;
		}
		if (rc < 0) {
			// an error has ocurred
			on_close();
			return;
		}

		switch (raw) {
			case '\r':
				break;
			case '\n':
				process_nmea();
				received_data.clear();
				break;
			default:
				if (received_data.size() > marnav::nmea::sentence::max_length) {
					// error ocurred, discard data
					received_data.clear();
				} else {
					received_data += raw;
				}
				break;
		}
	}
}
}
