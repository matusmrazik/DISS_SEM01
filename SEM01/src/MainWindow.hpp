#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include <QMainWindow>
#include <thread>
#include <qcustomplot.h>

#include "sim/sim_core_parking.hpp"

namespace Ui {
	class MainWindow;
}

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	explicit MainWindow(QWidget *parent = nullptr);
	~MainWindow();

private slots:
	void replication_finished(int replication, double result1, double result2, double result3);
	void simulation_finished();
	void on_radioButtonCustomSeed_toggled(bool checked);
	void on_pushButtonStartSimulation_clicked();
	void on_pushButtonStopSimulation_clicked();

private:
	void _prepare_plot();
	void _clear_plot_data();
	void _toggle_controls_enabled(bool enabled);

private:
	Ui::MainWindow *_ui;
	QCPBars *_str1_histo, *_str2_histo, *_str3_histo;
	sim_core_parking *_sim;
	std::thread _thr;

	int _skip_replications;
};

#endif // MAINWINDOW_HPP
