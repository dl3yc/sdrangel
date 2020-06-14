///////////////////////////////////////////////////////////////////////////////////
// Copyright (C) 2016-2020 F4EXB                                                 //
// written by Edouard Griffiths                                                  //
//                                                                               //
// OpenGL interface modernization.                                               //
// See: http://doc.qt.io/qt-5/qopenglshaderprogram.html                          //
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

#include "gui/glspectrumgui.h"
#include "dsp/fftwindow.h"
#include "dsp/spectrumvis.h"
#include "gui/glspectrum.h"
#include "util/simpleserializer.h"
#include "ui_glspectrumgui.h"

GLSpectrumGUI::GLSpectrumGUI(QWidget* parent) :
	QWidget(parent),
	ui(new Ui::GLSpectrumGUI),
	m_spectrumVis(nullptr),
	m_glSpectrum(nullptr),
    m_doApplySettings(true)
{
	ui->setupUi(this);
	on_linscale_toggled(false);

    ui->refLevel->clear();
	for(int ref = 0; ref >= -110; ref -= 5) {
		ui->refLevel->addItem(QString("%1").arg(ref));
    }

	ui->levelRange->clear();
	for(int range = 100; range >= 5; range -= 5) {
		ui->levelRange->addItem(QString("%1").arg(range));
    }

	connect(&m_messageQueue, SIGNAL(messageEnqueued()), this, SLOT(handleInputMessages()));
    displaySettings();
	setAveragingCombo();
    applySettings();
}

GLSpectrumGUI::~GLSpectrumGUI()
{
	delete ui;
}

void GLSpectrumGUI::setBuddies(SpectrumVis* spectrumVis, GLSpectrum* glSpectrum)
{
	m_spectrumVis = spectrumVis;
	m_glSpectrum = glSpectrum;
	m_glSpectrum->setMessageQueueToGUI(&m_messageQueue);
    m_spectrumVis->setMessageQueueToGUI(&m_messageQueue);
}

void GLSpectrumGUI::resetToDefaults()
{
    m_settings.resetToDefaults();
    displaySettings();
	applySettings();
}

QByteArray GLSpectrumGUI::serialize() const
{
    return m_settings.serialize();
}

bool GLSpectrumGUI::deserialize(const QByteArray& data)
{
    if (m_settings.deserialize(data))
    {
        displaySettings();
        applySettings();
        return true;
    }
    else
    {
        resetToDefaults();
        return false;
    }
}

void GLSpectrumGUI::displaySettings()
{
    blockApplySettings(true);
	ui->refLevel->setCurrentIndex(-m_settings.m_refLevel / 5);
	ui->levelRange->setCurrentIndex((100 - m_settings.m_powerRange) / 5);
	ui->decay->setSliderPosition(m_settings.m_decay);
	ui->decayDivisor->setSliderPosition(m_settings.m_decayDivisor);
	ui->stroke->setSliderPosition(m_settings.m_histogramStroke);
	ui->waterfall->setChecked(m_settings.m_displayWaterfall);
	ui->maxHold->setChecked(m_settings.m_displayMaxHold);
	ui->current->setChecked(m_settings.m_displayCurrent);
	ui->histogram->setChecked(m_settings.m_displayHistogram);
	ui->invert->setChecked(m_settings.m_invert);
	ui->grid->setChecked(m_settings.m_displayGrid);
	ui->gridIntensity->setSliderPosition(m_settings.m_displayGridIntensity);

	ui->decay->setToolTip(QString("Decay: %1").arg(m_settings.m_decay));
	ui->decayDivisor->setToolTip(QString("Decay divisor: %1").arg(m_settings.m_decayDivisor));
	ui->stroke->setToolTip(QString("Stroke: %1").arg(m_settings.m_histogramStroke));
	ui->gridIntensity->setToolTip(QString("Grid intensity: %1").arg(m_settings.m_displayGridIntensity));
	ui->traceIntensity->setToolTip(QString("Trace intensity: %1").arg(m_settings.m_displayTraceIntensity));

	ui->fftWindow->blockSignals(true);
	ui->averaging->blockSignals(true);
	ui->averagingMode->blockSignals(true);
	ui->linscale->blockSignals(true);

	ui->fftWindow->setCurrentIndex(m_settings.m_fftWindow);

	for (int i = 0; i < 6; i++)
	{
		if (m_settings.m_fftSize == (1 << (i + 7)))
		{
			ui->fftSize->setCurrentIndex(i);
			break;
		}
	}

	ui->averaging->setCurrentIndex(m_settings.m_averagingIndex);
	ui->averagingMode->setCurrentIndex((int) m_settings.m_averagingMode);
	ui->linscale->setChecked(m_settings.m_linear);
	setAveragingToolitp();

	ui->fftWindow->blockSignals(false);
	ui->averaging->blockSignals(false);
	ui->averagingMode->blockSignals(false);
	ui->linscale->blockSignals(false);
    blockApplySettings(false);
}

void GLSpectrumGUI::blockApplySettings(bool block)
{
    m_doApplySettings = !block;
}

void GLSpectrumGUI::applySettings()
{
    if (!m_doApplySettings) {
        return;
    }

    if (m_glSpectrum)
    {
        m_glSpectrum->setDisplayWaterfall(m_settings.m_displayWaterfall);
        m_glSpectrum->setInvertedWaterfall(m_settings.m_invertedWaterfall);
        m_glSpectrum->setDisplayMaxHold(m_settings.m_displayMaxHold);
        m_glSpectrum->setDisplayCurrent(m_settings.m_displayCurrent);
        m_glSpectrum->setDisplayHistogram(m_settings.m_displayHistogram);
        m_glSpectrum->setDecay(m_settings.m_decay);
        m_glSpectrum->setDecayDivisor(m_settings.m_decayDivisor);
        m_glSpectrum->setHistoStroke(m_settings.m_histogramStroke);
        m_glSpectrum->setInvertedWaterfall(m_settings.m_invert);
        m_glSpectrum->setDisplayGrid(m_settings.m_displayGrid);
        m_glSpectrum->setDisplayGridIntensity(m_settings.m_displayGridIntensity);
        m_glSpectrum->setDisplayTraceIntensity(m_settings.m_displayTraceIntensity);

        if ((m_settings.m_averagingMode == GLSpectrumSettings::AvgModeFixed) || (m_settings.m_averagingMode == GLSpectrumSettings::AvgModeMax)) {
            m_glSpectrum->setTimingRate(getAveragingValue(m_settings.m_averagingIndex, m_settings.m_averagingMode) == 0 ?
                1 :
                getAveragingValue(m_settings.m_averagingIndex, m_settings.m_averagingMode));
        } else {
            m_glSpectrum->setTimingRate(1);
        }

        Real refLevel = m_settings.m_linear ? pow(10.0, m_settings.m_refLevel/10.0) : m_settings.m_refLevel;
        Real powerRange = m_settings.m_linear ? pow(10.0, m_settings.m_refLevel/10.0) :  m_settings.m_powerRange;
        qDebug("GLSpectrumGUI::applySettings: refLevel: %e powerRange: %e", refLevel, powerRange);
        m_glSpectrum->setReferenceLevel(refLevel);
        m_glSpectrum->setPowerRange(powerRange);
        m_glSpectrum->setLinear(m_settings.m_linear);
    }

    if (m_spectrumVis)
    {
	    m_spectrumVis->configure(
            m_settings.m_fftSize,
            m_settings.m_refLevel,
            m_settings.m_powerRange,
            m_settings.m_fftOverlap,
            getAveragingValue(m_settings.m_averagingIndex, m_settings.m_averagingMode),
            (SpectrumVis::AvgMode) m_settings.m_averagingMode,
            (FFTWindow::Function) m_settings.m_fftWindow,
            m_settings.m_linear
		);
    }
}

void GLSpectrumGUI::on_fftWindow_currentIndexChanged(int index)
{
	qDebug("GLSpectrumGUI::on_fftWindow_currentIndexChanged: %d", index);
	m_settings.m_fftWindow = (FFTWindow::Function) index;
	applySettings();
}

void GLSpectrumGUI::on_fftSize_currentIndexChanged(int index)
{
	qDebug("GLSpectrumGUI::on_fftSize_currentIndexChanged: %d", index);
	m_settings.m_fftSize = 1 << (7 + index);
	applySettings();
	setAveragingToolitp();
}

void GLSpectrumGUI::on_averagingMode_currentIndexChanged(int index)
{
	qDebug("GLSpectrumGUI::on_averagingMode_currentIndexChanged: %d", index);
    m_settings.m_averagingMode = index < 0 ?
        GLSpectrumSettings::AvgModeNone :
        index > 3 ?
            GLSpectrumSettings::AvgModeMax :
            (GLSpectrumSettings::AveragingMode) index;

    setAveragingCombo();
	applySettings();
}

void GLSpectrumGUI::on_averaging_currentIndexChanged(int index)
{
	qDebug("GLSpectrumGUI::on_averaging_currentIndexChanged: %d", index);
    m_settings.m_averagingIndex = index;
	applySettings();
    setAveragingToolitp();
}

void GLSpectrumGUI::on_linscale_toggled(bool checked)
{
	qDebug("GLSpectrumGUI::on_averaging_currentIndexChanged: %s", checked ? "lin" : "log");
    m_settings.m_linear = checked;
	applySettings();
}

void GLSpectrumGUI::on_refLevel_currentIndexChanged(int index)
{
	m_settings.m_refLevel = 0 - index * 5;
    applySettings();
}

void GLSpectrumGUI::on_levelRange_currentIndexChanged(int index)
{
	m_settings.m_powerRange = 100 - index * 5;
    applySettings();
}

void GLSpectrumGUI::on_decay_valueChanged(int index)
{
	m_settings.m_decay = index;
	ui->decay->setToolTip(QString("Decay: %1").arg(m_settings.m_decay));
    applySettings();
}

void GLSpectrumGUI::on_decayDivisor_valueChanged(int index)
{
	m_settings.m_decayDivisor = index;
	ui->decayDivisor->setToolTip(QString("Decay divisor: %1").arg(m_settings.m_decayDivisor));
    applySettings();
}

void GLSpectrumGUI::on_stroke_valueChanged(int index)
{
	m_settings.m_histogramStroke = index;
	ui->stroke->setToolTip(QString("Stroke: %1").arg(m_settings.m_histogramStroke));
    applySettings();
}

void GLSpectrumGUI::on_waterfall_toggled(bool checked)
{
	m_settings.m_displayWaterfall = checked;
    applySettings();
}

void GLSpectrumGUI::on_histogram_toggled(bool checked)
{
	m_settings.m_displayHistogram = checked;
    applySettings();
}

void GLSpectrumGUI::on_maxHold_toggled(bool checked)
{
	m_settings.m_displayMaxHold = checked;
    applySettings();
}

void GLSpectrumGUI::on_current_toggled(bool checked)
{
	m_settings.m_displayCurrent = checked;
    applySettings();
}

void GLSpectrumGUI::on_invert_toggled(bool checked)
{
	m_settings.m_invert = checked;
    applySettings();
}

void GLSpectrumGUI::on_grid_toggled(bool checked)
{
	m_settings.m_displayGrid = checked;
    applySettings();
}

void GLSpectrumGUI::on_gridIntensity_valueChanged(int index)
{
	m_settings.m_displayGridIntensity = index;
	ui->gridIntensity->setToolTip(QString("Grid intensity: %1").arg(m_settings.m_displayGridIntensity));
    applySettings();
}

void GLSpectrumGUI::on_traceIntensity_valueChanged(int index)
{
	m_settings.m_displayTraceIntensity = index;
	ui->traceIntensity->setToolTip(QString("Trace intensity: %1").arg(m_settings.m_displayTraceIntensity));
    applySettings();
}

void GLSpectrumGUI::on_clearSpectrum_clicked(bool checked)
{
    (void) checked;

	if (m_glSpectrum) {
	    m_glSpectrum->clearSpectrumHistogram();
	}
}

int GLSpectrumGUI::getAveragingMaxScale(GLSpectrumSettings::AveragingMode averagingMode)
{
    if (averagingMode == GLSpectrumSettings::AvgModeMoving) {
        return 2;
    } else {
        return 5;
    }
}

int GLSpectrumGUI::getAveragingIndex(int averagingValue, GLSpectrumSettings::AveragingMode averagingMode)
{
    if (averagingValue <= 1) {
        return 0;
    }

    int v = averagingValue;
    int j = 0;

    for (int i = 0; i <= getAveragingMaxScale(averagingMode); i++)
    {
        if (v < 20)
        {
            if (v < 2) {
                j = 0;
            } else if (v < 5) {
                j = 1;
            } else if (v < 10) {
                j = 2;
            } else {
                j = 3;
            }

            return 3*i + j;
        }

        v /= 10;
    }

    return 3 * getAveragingMaxScale(averagingMode) + 3;
}

int GLSpectrumGUI::getAveragingValue(int averagingIndex, GLSpectrumSettings::AveragingMode averagingMode)
{
    if (averagingIndex <= 0) {
        return 1;
    }

    int v = averagingIndex - 1;
    int m = pow(10.0, v/3 > getAveragingMaxScale(averagingMode) ? getAveragingMaxScale(averagingMode) : v/3);
    int x = 1;

    if (v % 3 == 0) {
        x = 2;
    } else if (v % 3 == 1) {
        x = 5;
    } else if (v % 3 == 2) {
        x = 10;
    }

    return x * m;
}

void GLSpectrumGUI::setAveragingCombo()
{
    int index = ui->averaging->currentIndex();
	ui->averaging->blockSignals(true);
    ui->averaging->clear();
    ui->averaging->addItem(QString("1"));

    for (int i = 0; i <= getAveragingMaxScale(m_settings.m_averagingMode); i++)
    {
        QString s;
        int m = pow(10.0, i);
        int x = 2*m;
        setNumberStr(x, s);
        ui->averaging->addItem(s);
        x = 5*m;
        setNumberStr(x, s);
        ui->averaging->addItem(s);
        x = 10*m;
        setNumberStr(x, s);
        ui->averaging->addItem(s);
    }

    ui->averaging->setCurrentIndex(index >= ui->averaging->count() ? ui->averaging->count() - 1 : index);
	ui->averaging->blockSignals(false);
}

void GLSpectrumGUI::setNumberStr(int n, QString& s)
{
    if (n < 1000) {
        s = tr("%1").arg(n);
    } else if (n < 100000) {
        s = tr("%1k").arg(n/1000);
    } else if (n < 1000000) {
        s = tr("%1e5").arg(n/100000);
    } else if (n < 1000000000) {
        s = tr("%1M").arg(n/1000000);
    } else {
        s = tr("%1G").arg(n/1000000000);
    }
}

void GLSpectrumGUI::setNumberStr(float v, int decimalPlaces, QString& s)
{
    if (v < 1e-6) {
        s = tr("%1n").arg(v*1e9, 0, 'f', decimalPlaces);
    } else if (v < 1e-3) {
        s = tr("%1µ").arg(v*1e6, 0, 'f', decimalPlaces);
    } else if (v < 1.0) {
        s = tr("%1m").arg(v*1e3, 0, 'f', decimalPlaces);
    } else if (v < 1e3) {
        s = tr("%1").arg(v, 0, 'f', decimalPlaces);
    } else if (v < 1e6) {
        s = tr("%1k").arg(v*1e-3, 0, 'f', decimalPlaces);
    } else if (v < 1e9) {
        s = tr("%1M").arg(v*1e-6, 0, 'f', decimalPlaces);
    } else {
        s = tr("%1G").arg(v*1e-9, 0, 'f', decimalPlaces);
    }
}

void GLSpectrumGUI::setAveragingToolitp()
{
    if (m_glSpectrum)
    {
        QString s;
        float averagingTime = (m_settings.m_fftSize * (getAveragingValue(m_settings.m_averagingIndex, m_settings.m_averagingMode) == 0 ?
            1 :
            getAveragingValue(m_settings.m_averagingIndex, m_settings.m_averagingMode))) / (float) m_glSpectrum->getSampleRate();
        setNumberStr(averagingTime, 2, s);
        ui->averaging->setToolTip(QString("Number of averaging samples (avg time: %1s)").arg(s));
    }
    else
    {
        ui->averaging->setToolTip(QString("Number of averaging samples"));
    }
}

bool GLSpectrumGUI::handleMessage(const Message& message)
{
    if (GLSpectrum::MsgReportSampleRate::match(message))
    {
        setAveragingToolitp();
        return true;
    }

    return false;
}

void GLSpectrumGUI::handleInputMessages()
{
    Message* message;

    while ((message = m_messageQueue.pop()) != 0)
    {
        qDebug("GLSpectrumGUI::handleInputMessages: message: %s", message->getIdentifier());

        if (handleMessage(*message))
        {
            delete message;
        }
    }
}
