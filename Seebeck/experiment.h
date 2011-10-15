#ifndef EXPERIMENT_H
#define EXPERIMENT_H

#include <math.h>
#include <QObject>
#include <QTimer>

#include "../msdptool/src/include/msdp2xxx.h"
#include "../QSCPIDev/qscpidev.h"
#include "../QCSVWriter/qcsvwriter.h"

class Experiment : public QObject
{
    Q_OBJECT
public:
    explicit Experiment(QObject *parent = 0);

    double furnaceT();
    double sampleHeatI();

    void close();
    // When changing prototype, change numeric suffix.
    // This is simple hack to keep definition clear even for function
    // which takes as parameters jus bunch of strings.
    bool open_00(const QString &eurothermPort, const QString &hp34970Port,
              const QString &msdpPort, const QString &dataDir);
    void setFurnaceT(double T, double straggling = NAN);
    void setFurnaceTStraggling(double straggling);
    void setFurnaceStabilizationTime(double t);
    void setSampleHeatI(double I);
    void setSampleTStraggling(double straggling);
    void setSampleStabilizationTime(double t);
    void start();
    void stop();

protected:
    typedef enum {
        STATE_STOP = 0,
        STATE_STABILIZE,
        STATE_COOLDOWN
    } State_t;
    State_t state;

    QSCPIDev hp34970;
    /** Timer for measurement. */
    QTimer timer;
    /** Timing for timer. */
    static const double timerDwell;

private:
    double furnaceWantedTf;
    double furnaceWantedTStragglingf;
    double sampleHeatIf;
    double sampleTStragglingf;

    void doCoolDown();
    void doStabilize();
    void doStop();

signals:
    void fatalError(const QString &errorShort, const QString &errorLong);

    void furnaceTMeasured(double T);
    void furnaceTStateChanged(bool stabilized);

    void sampleTMeasured(double T1, double T2, double T3, double T4);
    void sampleTStateChanged(bool stabilized);

private slots:
    void on_timer_timeout();

};

#endif // EXPERIMENT_H
