#include <QCloseEvent>
#include <QMessageBox>
#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    config(),
    configUI(),
    experiment(this),
    ui(new Ui::MainWindow)
{
    experiment.setObjectName("experiment");
    ui->setupUi(this);
    QObject::connect(&configUI, SIGNAL(accepted()), this, SLOT(show()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::close()
{
    experiment.close();
    hide();
    configUI.show();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (configUI.isHidden() && configUI.result() == QDialog::Accepted) {
        event->ignore();

        close();
        return;
    }

    QMainWindow::closeEvent(event);
}

void MainWindow::on_experiment_fatalError(const QString &errorShort, const QString &errorLong)
{
    QString title("Fatal error in experiment: ");
    QString text("%1:\n\n%2");

    text = text.arg(errorShort).arg(errorLong);
    title.append(errorShort);
    QMessageBox::critical(this, title, text);
    close();
}

void MainWindow::on_autoMeasFromSpinBox_editingFinished()
{
    int measFrom(ui->autoMeasFromSpinBox->value());

    if (ui->autoMeasToSpinBox->value() < measFrom) {
        ui->autoMeasToSpinBox->setValue(measFrom);
    }
}

void MainWindow::on_autoMeasToSpinBox_editingFinished()
{
    int measFrom(ui->autoMeasFromSpinBox->value());

    if (ui->autoMeasToSpinBox->value() < measFrom) {
        ui->autoMeasToSpinBox->setValue(measFrom);
    }
}

void MainWindow::on_experiment_furnaceTMeasured(int T, double Tstraggling)
{
    ui->furnaceTSpinBox->setValue(T);
    ui->furnaceTStragglingDoubleSpinBox->setValue(Tstraggling);
}

void MainWindow::on_experimentOffRadioButton_toggled(bool checked)
{
    if (checked) {
        if (experiment.isRunning() || experiment.isSetup()) {
            if (!experiment.abort()) {
                on_experiment_fatalError("Failed to stop experiment",
                                         experiment.errorString());
                return;
            }
        }
    }

    if (!checked) {
        Experiment::SetupParams params;

        params.experimentator = ui->experimentatorLineEdit->text();
        params.resistivityI = ui->resistivityIDoubleSpinBox->value();
        params.sample.l1 = ui->sampleL1DoubleSpinBox->value();
        params.sample.l2 = ui->sampleL2DoubleSpinBox->value();
        params.sample.l3 = ui->sampleL3DoubleSpinBox->value();
        params.sample.S = ui->sampleSDubleSpinBox->value();
        params.sampleId = ui->sampleIdLineEdit->text();

        if (!experiment.setup(params)) {
            QString errTitle("Failed to setup experiment");

            if (experiment.error() == Experiment::ERR_VALUE) {
                QMessageBox::critical(this, errTitle, experiment.errorString());
                ui->experimentOffRadioButton->setChecked(true);
                return;
            }
            on_experiment_fatalError(errTitle, experiment.errorString());
            return;
        }
    }

    ui->automatedTab->setEnabled(ui->experimentAutoRadioButton->isChecked());
    ui->manualTab->setEnabled(ui->experimentManualRadioButton->isChecked());
    ui->settingsOtherGroupBox->setEnabled(checked);
    ui->settingsSampleGroupBox->setEnabled(checked);
}

void MainWindow::on_experiment_sampleHeatingUIMeasured(double I, double U)
{
    ui->sampleHeatingIDoubleSpinBox->setValue(I);
    ui->sampleHeatingUDoubleSpinBox->setValue(U);
    ui->sampleHeatingPDoubleSpinBox->setValue(I*U);
}

void MainWindow::on_experiment_sampleTMeasured(double T1, double T2, double T3, double T4)
{
    ui->sampleT1DoubleSpinBox->setValue(T1);
    ui->sampleT2DoubleSpinBox->setValue(T2);
    ui->sampleT3DoubleSpinBox->setValue(T3);
    ui->sampleT4DoubleSpinBox->setValue(T4);
}

void MainWindow::on_experiment_sampleUMeasured(double U12, double U23, double U34, double U41)
{
    ui->sampleU12DoubleSpinBox->setValue(U12);
    ui->sampleU23DoubleSpinBox->setValue(U23);
    ui->sampleU34DoubleSpinBox->setValue(U34);
    ui->sampleU41DoubleSpinBox->setValue(U41);
}

void MainWindow::show()
{
    Experiment::OpenParams openParams;

    openParams.dataDirName = config.dataDir();
    openParams.eurothermPort = config.eurothermPort();
    openParams.eurothermSlave = config.eurothermSlave();
    openParams.hp34970Port = config.hp34970Port();
    openParams.msdpPort = config.msdpPort();

    if (!experiment.open(openParams)) {
        QMessageBox::critical(this, "Experiment open failed.", experiment.errorString());
        close();
        return;
    }

    Experiment::RunParams params(experiment.runParams());
    ui->manualFurnaceTSpinBox->setValue(params.furnaceT);

    // turn experiment off -> experiment is set up at star (manual or automatic)
    ui->experimentOffRadioButton->setChecked(true);

    int Tmin, Tmax;
    if (!experiment.furnaceTRange(&Tmin, &Tmax)) {
        on_experiment_fatalError("Eurotherm operation error",
                                 "Failed to get furnace T operation range.");
        return;
    }
    ui->manualFurnaceTSpinBox->setRange(Tmin, Tmax);
    ui->autoMeasFromSpinBox->setRange(Tmin, Tmax);
    ui->autoMeasToSpinBox->setRange(Tmin, Tmax);

    QWidget::show();
}

void MainWindow::startApp()
{
    configUI.show();
}

void MainWindow::on_manualApplyFurnacePushButton_clicked()
{
    Experiment::RunParams params(experiment.runParams());
    params.furnaceT = ui->manualFurnaceTSpinBox->value();
    params.furnaceSettleTime = ui->manualSettleTimeSpinBox->value();

    if (!experiment.run(params))
    {
        on_experiment_fatalError("Failed to start experiment", experiment.errorString());
    }
}

void MainWindow::on_manualApplySamplePushButton_clicked()
{
    Experiment::RunParams params(experiment.runParams());
    params.sampleHeatingI = ui->manualSampleIDoubleSpinBox->value();
    if (!experiment.run(params))
    {
        on_experiment_fatalError("Failed to start experiment", experiment.errorString());
    }
}

void MainWindow::on_manualStartPushButton_clicked()
{
    experiment.sampleMeasure();
}

void MainWindow::on_sampleSPushButton_clicked()
{
    ui->sampleSDubleSpinBox->setValue(ui->sampleWidthDoubleSpinBox->value() *
                                      ui->sampleHeightDoubleSpinBox->value());
}
