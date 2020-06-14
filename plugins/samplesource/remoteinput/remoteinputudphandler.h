///////////////////////////////////////////////////////////////////////////////////
// Copyright (C) 2016 Edouard Griffiths, F4EXB                                   //
//                                                                               //
// This program is free software; you can redistribute it and/or modify          //
// it under the terms of the GNU General Public License as published by          //
// the Free Software Foundation as version 3 of the License, or                  //
// (at your option) any later version.                                           //
//                                                                               //
// This program is distributed in the hope that it will be useful,               //
// but WITHOUT ANY WARRANTY; without even the implied warranty of                //
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the                  //
// GNU General Public License V3 for more details.                               //
//                                                                               //
// You should have received a copy of the GNU General Public License             //
// along with this program. If not, see <http://www.gnu.org/licenses/>.          //
///////////////////////////////////////////////////////////////////////////////////

#ifndef PLUGINS_SAMPLESOURCE_REMOTEINPUT_REMOTEINPUTUDPHANDLER_H_
#define PLUGINS_SAMPLESOURCE_REMOTEINPUT_REMOTEINPUTUDPHANDLER_H_

#include <QObject>
#include <QUdpSocket>
#include <QHostAddress>
#include <QMutex>
#include <QElapsedTimer>

#include "remoteinputbuffer.h"

#define REMOTEINPUT_THROTTLE_MS 50

class SampleSinkFifo;
class MessageQueue;
class QTimer;
class DeviceAPI;

class RemoteInputUDPHandler : public QObject
{
	Q_OBJECT
public:
	RemoteInputUDPHandler(SampleSinkFifo* sampleFifo, DeviceAPI *deviceAPI);
	~RemoteInputUDPHandler();
	void setMessageQueueToGUI(MessageQueue *queue) { m_outputMessageQueueToGUI = queue; }
	void start();
	void stop();
	void configureUDPLink(const QString& address, quint16 port);
	void getRemoteAddress(QString& s) const { s = m_remoteAddress.toString(); }
    int getNbOriginalBlocks() const { return RemoteNbOrginalBlocks; }
    bool isStreaming() const { return m_masterTimerConnected; }
    int getSampleRate() const { return m_samplerate; }
    int getCenterFrequency() const { return m_centerFrequency; }
    int getBufferGauge() const { return m_remoteInputBuffer.getBufferGauge(); }
    uint64_t getTVmSec() const { return m_tv_msec; }
    int getMinNbBlocks() { return m_remoteInputBuffer.getMinNbBlocks(); }
    int getMaxNbRecovery() { return m_remoteInputBuffer.getMaxNbRecovery(); }
public slots:
	void dataReadyRead();

private:
	DeviceAPI *m_deviceAPI;
	const QTimer& m_masterTimer;
	bool m_masterTimerConnected;
	bool m_running;
    uint32_t m_rateDivider;
	RemoteInputBuffer m_remoteInputBuffer;
	QUdpSocket *m_dataSocket;
	QHostAddress m_dataAddress;
	QHostAddress m_remoteAddress;
	quint16 m_dataPort;
	bool m_dataConnected;
	char *m_udpBuf;
	qint64 m_udpReadBytes;
	SampleSinkFifo *m_sampleFifo;
	uint32_t m_samplerate;
	uint64_t m_centerFrequency;
	uint64_t m_tv_msec;
	MessageQueue *m_outputMessageQueueToGUI;
	uint32_t m_tickCount;
	std::size_t m_samplesCount;
    QTimer *m_timer;

	QElapsedTimer m_elapsedTimer;
	int m_throttlems;
    int32_t m_readLengthSamples;
    uint32_t m_readLength;
    int32_t *m_converterBuffer;
    uint32_t m_converterBufferNbSamples;
    bool m_throttleToggle;
    bool m_autoCorrBuffer;

	void connectTimer();
    void disconnectTimer();
	void processData();

private slots:
	void tick();
};



#endif /* PLUGINS_SAMPLESOURCE_REMOTEINPUT_REMOTEINPUTUDPHANDLER_H_ */
