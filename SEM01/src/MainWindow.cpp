#include "MainWindow.hpp"
#include "ui_MainWindow.h"

#define BLUE_COLOR    QColor::fromRgb(0, 0, 255)
#define RED_COLOR     QColor::fromRgb(255, 0, 0)
#define GREEN_COLOR   QColor::fromRgb(0, 100, 0)

#define MAX(container) \
	*std::max_element(container.begin(), container.end())

#define INIT_PLOT(plot_ptr, name, color) \
	plot_ptr->clearGraphs(); \
	plot_ptr->addGraph()->setName(name); \
	plot_ptr->xAxis->setLabel("Počet replikácií"); \
	plot_ptr->yAxis->setLabel("Úspešnosť zaparkovania"); \
	plot_ptr->plotLayout()->insertRow(0); \
	plot_ptr->plotLayout()->addElement(0, 0, new QCPPlotTitle(plot_ptr, name)); \
	plot_ptr->graph(0)->setPen(QPen(color))

#define PLOT_DATA(plot_ptr, xval, yval) \
	plot_ptr->graph(0)->addData(xval, yval); \
	plot_ptr->rescaleAxes(true); \
	plot_ptr->replot()

#define CLEAR_PLOT(plot_ptr) \
	plot_ptr->graph(0)->clearData(); \
	plot_ptr->replot()

#define INIT_HISTO(histo_ptr, qcpbars_ptr, name, color) \
	histo_ptr->clearGraphs(); \
	histo_ptr->plotLayout()->insertRow(0); \
	histo_ptr->plotLayout()->addElement(0, 0, new QCPPlotTitle(histo_ptr, name)); \
	qcpbars_ptr = new QCPBars(histo_ptr->xAxis, histo_ptr->yAxis); \
	qcpbars_ptr->setName(name); \
	qcpbars_ptr->setBrush(QBrush(color))

#define PLOT_HISTO(histo_ptr, qcpbars_ptr, xmax, ymax, ticks, labels, data) \
	histo_ptr->xAxis->setRange(0, xmax); \
	histo_ptr->yAxis->setRange(0, ymax); \
	histo_ptr->xAxis->setAutoTicks(false); \
	histo_ptr->xAxis->setAutoTickLabels(false); \
	histo_ptr->xAxis->setTickVector(ticks); \
	histo_ptr->xAxis->setTickVectorLabels(labels); \
	qcpbars_ptr->setData(ticks, data); \
	histo_ptr->replot()

#include <limits>

MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent),
	_ui(new Ui::MainWindow),
	_str1_histo(nullptr), _str2_histo(nullptr), _str3_histo(nullptr),
	_sim(new sim_core_parking), _skip_replications(0)
{
	_ui->setupUi(this);
	_ui->spinBoxCustomSeed->setMaximum(std::numeric_limits<int>::max());
	_ui->labelUsedSeedValue->setTextInteractionFlags(Qt::TextSelectableByMouse);
	_prepare_plot();
	QObject::connect(_sim, SIGNAL(replication_finished(int,double,double,double)), this, SLOT(replication_finished(int,double,double,double)));
	QObject::connect(_sim, SIGNAL(simulation_finished()), this, SLOT(simulation_finished()));
}

MainWindow::~MainWindow()
{
	if (_sim->get_state() == sim_core_parking::state::RUNNING || _thr.joinable())
	{
		_sim->stop();
		_thr.join();
	}
	delete _sim;
	if (_str1_histo) delete _str1_histo;
	if (_str2_histo) delete _str2_histo;
	if (_str3_histo) delete _str3_histo;
	delete _ui;
}

void MainWindow::replication_finished(int replication, double result1, double result2, double result3)
{
	_ui->statusBar->showMessage(QString("Beží replikácia: %1").arg(replication));
	_ui->labelStrategy1Result->setText(QString::asprintf("%.6f", result1));
	_ui->labelStrategy2Result->setText(QString::asprintf("%.6f", result2));
	_ui->labelStrategy3Result->setText(QString::asprintf("%.6f", result3));
	if (replication >= _skip_replications)
	{
		PLOT_DATA(_ui->plot1, replication, result1);
		PLOT_DATA(_ui->plot2, replication, result2);
		PLOT_DATA(_ui->plot3, replication, result3);
	}
}

void MainWindow::simulation_finished()
{
	_thr.join();
	_toggle_controls_enabled(true);

	QVector<double> ticks(_ui->spinBoxParkingPlaces->value() + 2);
	std::iota(ticks.begin(), ticks.end(), 0);

	QVector<QString> labels(_ui->spinBoxParkingPlaces->value() + 2);
	for (int i = 0; i < labels.size(); ++i)
		labels[i] = QString::number(i);
	labels.back() = QString::number(2 * _ui->spinBoxParkingPlaces->value());

	auto data1 = _sim->get_strategy_1_data();
	auto data2 = _sim->get_strategy_2_data();
	auto data3 = _sim->get_strategy_3_data();

	PLOT_HISTO(_ui->histo1, _str1_histo, ticks.size(), MAX(data1), ticks, labels, data1);
	PLOT_HISTO(_ui->histo2, _str2_histo, ticks.size(), MAX(data2), ticks, labels, data2);
	PLOT_HISTO(_ui->histo3, _str3_histo, ticks.size(), MAX(data3), ticks, labels, data3);

	if (_sim->get_state() == sim_core_parking::state::STOPPED)
	{
		_ui->statusBar->showMessage("Simulácia zastavená", 5000);
	}
	else
	{
		_ui->statusBar->showMessage("Simulácia ukončená", 5000);
	}
}

void MainWindow::on_radioButtonCustomSeed_toggled(bool checked)
{
	_ui->spinBoxCustomSeed->setEnabled(checked);
}

void MainWindow::on_pushButtonStartSimulation_clicked()
{
	_toggle_controls_enabled(false);
	_clear_plot_data();
	_skip_replications = _ui->spinBoxFilterReplications->value();
	_sim->send_signal_at_replication(_ui->spinBoxRecordEvery->value());
	int replications = _ui->spinBoxReplications->value();
	int parking_places = _ui->spinBoxParkingPlaces->value();
	if (_ui->radioButtonCustomSeed->isChecked())
	{
		int seed = _ui->spinBoxCustomSeed->value();
		_sim->init(parking_places, seed);
	}
	else
	{
		_sim->init(parking_places);
	}
	_ui->labelUsedSeedValue->setText(QString::number(_sim->get_seed()));
	_thr = std::thread(&sim_core_parking::simulate, _sim, replications);
}

void MainWindow::on_pushButtonStopSimulation_clicked()
{
	_sim->stop();
}

void MainWindow::_prepare_plot()
{
	INIT_PLOT(_ui->plot1, "Stratégia 1", BLUE_COLOR);
	INIT_PLOT(_ui->plot2, "Stratégia 2", RED_COLOR);
	INIT_PLOT(_ui->plot3, "Stratégia 3", GREEN_COLOR);

	INIT_HISTO(_ui->histo1, _str1_histo, "Stratégia 1", BLUE_COLOR);
	INIT_HISTO(_ui->histo2, _str2_histo, "Stratégia 2", RED_COLOR);
	INIT_HISTO(_ui->histo3, _str3_histo, "Stratégia 3", GREEN_COLOR);
}

void MainWindow::_clear_plot_data()
{
	CLEAR_PLOT(_ui->plot1);
	CLEAR_PLOT(_ui->plot2);
	CLEAR_PLOT(_ui->plot3);
}

void MainWindow::_toggle_controls_enabled(bool enabled)
{
	_ui->groupBoxSeed->setEnabled(enabled);
	_ui->groupBoxParameters->setEnabled(enabled);
	_ui->pushButtonStartSimulation->setEnabled(enabled);
	_ui->pushButtonStopSimulation->setEnabled(!enabled);
}
