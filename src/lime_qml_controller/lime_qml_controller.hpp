#pragma once

#include "structs.hpp"

#include <QObject>
#include <QString>
#include <QVariantList>

#include <atomic>
#include <mutex>
#include <thread>

class LimeQmlController : public QObject
{
	Q_OBJECT

	Q_PROPERTY(QString hostname READ hostname NOTIFY statsChanged)
	Q_PROPERTY(QString uptime READ uptime NOTIFY statsChanged)
	Q_PROPERTY(QString battery READ battery NOTIFY statsChanged)
	Q_PROPERTY(QString kernelVersion READ kernelVersion NOTIFY statsChanged)
	Q_PROPERTY(float cpuUsage READ cpuUsage NOTIFY statsChanged)
	Q_PROPERTY(QVariantList perCoreUsage READ perCoreUsage NOTIFY statsChanged)
	Q_PROPERTY(int cpuTemp READ cpuTemp NOTIFY statsChanged)
	Q_PROPERTY(int cpuFreqMhz READ cpuFreqMhz NOTIFY statsChanged)

public:
	explicit LimeQmlController(QObject *parent = nullptr);
	~LimeQmlController() override;

	QString hostname() const { return m_hostname; }
	QString uptime() const { return m_uptime; }
	QString battery() const { return m_battery; }
	QString kernelVersion() const { return m_kernelVersion; }
	float cpuUsage() const { return m_cpuUsage; }
	QVariantList perCoreUsage() const { return m_perCoreUsage; }
	int cpuTemp() const { return m_cpuTemp; }
	int cpuFreqMhz() const { return m_cpuFreqMhz; }

public slots:
	void refresh();

signals:
	void statsChanged();

private:
	void applyState(const STATE &s);

	STATE m_state;
	std::mutex m_mutex;
	std::atomic<bool> m_run{true};
	std::thread m_worker;

	QString m_hostname;
	QString m_uptime;
	QString m_battery;
	QString m_kernelVersion;
	float m_cpuUsage = 0;
	QVariantList m_perCoreUsage;
	int m_cpuTemp = 0;
	int m_cpuFreqMhz = 0;
};
