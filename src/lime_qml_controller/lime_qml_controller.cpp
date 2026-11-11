#include "lime_qml_controller.hpp"
#include "lime_core.hpp"

#include <QTimer>

LimeQmlController::LimeQmlController(QObject *parent)
    : QObject(parent)
{
	m_worker = std::thread([this]() { gather_data(m_state, m_mutex, m_run, 1000); });

	auto *timer = new QTimer(this);
	connect(timer, &QTimer::timeout, this, &LimeQmlController::refresh);
	timer->start(500);
	refresh();
}

LimeQmlController::~LimeQmlController()
{
	m_run = false;
	if (m_worker.joinable())
		m_worker.join();
}

void LimeQmlController::refresh()
{
	STATE snapshot;
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		snapshot = m_state;
	}
	applyState(snapshot);
	emit statsChanged();
}

void LimeQmlController::applyState(const STATE &s)
{
	m_hostname = QString::fromStdString(s.sys.hostname);
	m_kernelVersion = QString::fromStdString(s.sys.kver);
	m_uptime = QString("%1:%2:%3")
	               .arg(s.proc.uptimeH, 2, 10, QChar('0'))
	               .arg(s.proc.uptimeM, 2, 10, QChar('0'))
	               .arg(s.proc.uptimeS, 2, 10, QChar('0'));
	m_cpuUsage = s.cpud.smoothUsage;
	m_cpuTemp = s.cpuTemp;

	m_perCoreUsage.clear();
	m_cpuFreqMhz = 0;
	for (const auto &core : s.cpud.cores)
	{
		m_perCoreUsage.push_back(core.usage);
		if (core.freqMhz > m_cpuFreqMhz)
			m_cpuFreqMhz = core.freqMhz;
	}

	if (s.battery.available)
	{
		m_battery = QString("BAT %1% %2%3")
		                .arg(s.battery.capacity)
		                .arg(QString::fromStdString(s.battery.status))
		                .arg(s.battery.acOnline ? " AC" : "");
	}
	else
	{
		m_battery = QStringLiteral("BAT N/A");
	}
}
