///////////////////////////////////////////////////////////////////////////////////
// Copyright (C) 2016 Edouard Griffiths, F4EXB                                   //
//                                                                               //
// This program is free software; you can redistribute it and/or modify          //
// it under the terms of the GNU General Public License as published by          //
// the Free Software Foundation as version 3 of the License, or                  //
//                                                                               //
// This program is distributed in the hope that it will be useful,               //
// but WITHOUT ANY WARRANTY; without even the implied warranty of                //
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the                  //
// GNU General Public License V3 for more details.                               //
//                                                                               //
// You should have received a copy of the GNU General Public License             //
// along with this program. If not, see <http://www.gnu.org/licenses/>.          //
///////////////////////////////////////////////////////////////////////////////////

#ifndef INCLUDE_SDRDAEMONSOURCEGUI_H
#define INCLUDE_SDRDAEMONSOURCEGUI_H

#include <sys/time.h>

#include <QTimer>
#include <QWidget>
#include <QNetworkRequest>

#include "plugin/plugininstancegui.h"
#include "util/messagequeue.h"

#include "sdrdaemonsourceinput.h"

class DeviceUISet;
class QNetworkAccessManager;
class QNetworkReply;
class QJsonObject;

namespace Ui {
	class SDRdaemonSourceGui;
}

class SDRdaemonSourceGui : public QWidget, public PluginInstanceGUI {
	Q_OBJECT

public:
	explicit SDRdaemonSourceGui(DeviceUISet *deviceUISet, QWidget* parent = 0);
	virtual ~SDRdaemonSourceGui();
	virtual void destroy();

	void setName(const QString& name);
	QString getName() const;

	void resetToDefaults();
	QByteArray serialize() const;
	bool deserialize(const QByteArray& data);
	virtual qint64 getCenterFrequency() const;
	virtual void setCenterFrequency(qint64 centerFrequency);
	virtual MessageQueue *getInputMessageQueue() { return &m_inputMessageQueue; }
	virtual bool handleMessage(const Message& message);

private:
	Ui::SDRdaemonSourceGui* ui;

	DeviceUISet* m_deviceUISet;
    SDRdaemonSourceSettings m_settings;        //!< current settings
	SDRdaemonSourceInput* m_sampleSource;
    bool m_acquisition;
    int m_streamSampleRate;          //!< Sample rate of received stream
    quint64 m_streamCenterFrequency; //!< Center frequency of received stream
	QTimer m_updateTimer;
	QTimer m_statusTimer;
    int m_lastEngineState;
    MessageQueue m_inputMessageQueue;

    //	int m_sampleRate;
    //	quint64 m_centerFrequency;
	struct timeval m_startingTimeStamp;
	int m_framesDecodingStatus;
	bool m_allBlocksReceived;
	float m_bufferLengthInSecs;
    int32_t m_bufferGauge;
    int m_minNbBlocks;
    int m_minNbOriginalBlocks;
    int m_maxNbRecovery;
    float m_avgNbBlocks;
    float m_avgNbOriginalBlocks;
    float m_avgNbRecovery;
    int m_nbOriginalBlocks;
    int m_nbFECBlocks;

	int m_samplesCount;
	std::size_t m_tickCount;

	bool m_addressEdited;
	bool m_dataPortEdited;

    uint32_t m_countUnrecoverable;
    uint32_t m_countRecovered;
    QTime m_eventsTime;

	bool m_doApplySettings;
    bool m_forceSettings;
    double m_txDelay;

    QPalette m_paletteGreenText;
    QPalette m_paletteWhiteText;

    QNetworkAccessManager *m_networkManager;
    QNetworkRequest m_networkRequest;

    void blockApplySettings(bool block);
	void displaySettings();
	void displayTime();
    void sendSettings();
	void updateWithAcquisition();
	void updateWithStreamTime();
	void updateSampleRateAndFrequency();
	void displayEventCounts();
    void displayEventTimer();
    void analyzeApiReply(const QJsonObject& jsonObject);

private slots:
    void handleInputMessages();
	void on_apiApplyButton_clicked(bool checked);
    void on_dataApplyButton_clicked(bool checked);
	void on_dcOffset_toggled(bool checked);
	void on_iqImbalance_toggled(bool checked);
	void on_apiAddress_returnPressed();
	void on_apiPort_returnPressed();
    void on_dataAddress_returnPressed();
	void on_dataPort_returnPressed();
	void on_startStop_toggled(bool checked);
    void on_record_toggled(bool checked);
    void on_eventCountsReset_clicked(bool checked);
    void updateHardware();
	void updateStatus();
	void networkManagerFinished(QNetworkReply *reply);
};

#endif // INCLUDE_SDRDAEMONSOURCEGUI_H
