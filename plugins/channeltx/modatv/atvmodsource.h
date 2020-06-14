///////////////////////////////////////////////////////////////////////////////////
// Copyright (C) 2017 Edouard Griffiths, F4EXB                                   //
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

#ifndef PLUGINS_CHANNELTX_MODATV_ATVMODSOURCE_H_
#define PLUGINS_CHANNELTX_MODATV_ATVMODSOURCE_H_

#include <vector>

#include <QObject>
#include <QMutex>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/videoio.hpp>

#include <stdint.h>

#include "dsp/channelsamplesource.h"
#include "dsp/nco.h"
#include "dsp/interpolator.h"
#include "util/movingaverage.h"
#include "dsp/fftfilt.h"
#include "util/message.h"

#include "atvmodsettings.h"

class MessageQueue;

class ATVModSource : public ChannelSampleSource
{
public:
    ATVModSource();
    ~ATVModSource();

    virtual void pull(SampleVector::iterator begin, unsigned int nbSamples);
    virtual void pullOne(Sample& sample);
    virtual void prefetch(unsigned int nbSamples);

    int getEffectiveSampleRate() const { return m_tvSampleRate; };
    double getMagSq() const { return m_movingAverage.asDouble(); }
    void getCameraNumbers(std::vector<int>& numbers);

    void setMessageQueueToGUI(MessageQueue *messageQueue) { m_messageQueueToGUI = messageQueue; }
    void getLevels(qreal& rmsLevel, qreal& peakLevel, int& numSamples) const
    {
        rmsLevel = m_rmsLevel;
        peakLevel = m_peakLevelOut;
        numSamples = m_levelNbSamples;
    }

    void applyChannelSettings(int channelSampleRate, int channelFrequencyOffset, bool force = false);
    void applySettings(const ATVModSettings& settings, bool force = false);
    void openImage(const QString& fileName);
    void openVideo(const QString& fileName);
    void seekVideoFileStream(int seekPercentage);
    void reportVideoFileSourceStreamTiming();
    void configureCameraIndex(int index);
    void configureCameraData(uint32_t index, float mnaualFPS, bool manualFPSEnable);

    static void getBaseValues(int outputSampleRate, int linesPerSecond, int& sampleRateUnits, uint32_t& nbPointsPerRateUnit);
    static float getRFBandwidthDivisor(ATVModSettings::ATVModulation modulation);

private:
    class ATVCamera
    {
    public:
    	cv::VideoCapture m_camera;    //!< camera object
        cv::Mat m_videoframeOriginal; //!< camera non resized image
        cv::Mat m_videoFrame;         //!< displayable camera frame
    	int m_cameraNumber;           //!< camera device number
        float m_videoFPS;             //!< camera FPS rate
        float m_videoFPSManual;       //!< camera FPS rate manually set
        bool m_videoFPSManualEnable;  //!< Enable camera FPS rate manual set value
        int m_videoWidth;             //!< camera frame width
        int m_videoHeight;            //!< camera frame height
        float m_videoFx;              //!< camera horizontal scaling factor
        float m_videoFy;              //!< camera vertictal scaling factor
        float m_videoFPSq;            //!< camera FPS sacaling factor
        float m_videoFPSqManual;      //!< camera FPS sacaling factor manually set
        float m_videoFPSCount;        //!< camera FPS fractional counter
        int m_videoPrevFPSCount;      //!< camera FPS previous integer counter

        ATVCamera() :
        	m_cameraNumber(-1),
			m_videoFPS(25.0f),
			m_videoFPSManual(20.0f),
			m_videoFPSManualEnable(false),
        	m_videoWidth(1),
			m_videoHeight(1),
			m_videoFx(1.0f),
			m_videoFy(1.0f),
			m_videoFPSq(1.0f),
			m_videoFPSqManual(1.0f),
		    m_videoFPSCount(0.0f),
		    m_videoPrevFPSCount(0)
        {}

        ATVCamera(const ATVCamera& camera) :
            m_camera(camera.m_camera),
            m_videoframeOriginal(camera.m_videoframeOriginal),
            m_videoFrame(camera.m_videoFrame),
            m_cameraNumber(camera.m_cameraNumber),
            m_videoFPS(camera.m_videoFPS),
            m_videoFPSManual(camera.m_videoFPSManual),
            m_videoFPSManualEnable(camera.m_videoFPSManualEnable),
            m_videoWidth(camera.m_videoWidth),
            m_videoHeight(camera.m_videoHeight),
            m_videoFx(camera.m_videoFx),
            m_videoFy(camera.m_videoFy),
            m_videoFPSq(camera.m_videoFPSq),
            m_videoFPSqManual(camera.m_videoFPSqManual),
            m_videoFPSCount(camera.m_videoFPSCount),
            m_videoPrevFPSCount(camera.m_videoPrevFPSCount)
        {}
    };

    int m_channelSampleRate;
    int m_channelFrequencyOffset;
    ATVModSettings m_settings;

    NCO m_carrierNco;
    Complex m_modSample;
    float m_modPhasor; //!< For FM modulation
    Interpolator m_interpolator;
    Real m_interpolatorDistance;
    Real m_interpolatorDistanceRemain;
    int      m_tvSampleRate;     //!< sample rate for generating signal
    uint32_t m_pointsPerLine;    //!< Number of points per full line
    int      m_pointsPerSync;    //!< number of line points for the horizontal sync
    int      m_pointsPerBP;      //!< number of line points for the back porch
    int      m_pointsPerImgLine; //!< number of line points for the image line
    uint32_t m_pointsPerFP;      //!< number of line points for the front porch
    int      m_pointsPerFSync;   //!< number of line points for the field first sync
    uint32_t m_pointsPerHBar;    //!< number of line points for a bar of the bar chart
    uint32_t m_linesPerVBar;     //!< number of lines for a bar of the bar chart
    uint32_t m_pointsPerTU;      //!< number of line points per time unit
    int      m_nbLines;          //!< number of lines per complete frame
    int      m_nbLines2;         //!< same number as above (non interlaced) or half the number above (interlaced)
    uint32_t m_nbImageLines;     //!< number of image lines excluding synchronization lines
    uint32_t m_nbImageLines2;    //!< same number as above (non interlaced) or half the number above (interlaced)
    int      m_nbHorizPoints;    //!< number of line points per horizontal line
    int      m_nbSyncLinesHeadE; //!< number of header sync lines on even frame
    int      m_nbSyncLinesHeadO; //!< number of header sync lines on odd frame
    int      m_nbSyncLinesBottom;//!< number of sync lines at bottom
    int      m_nbLongSyncLines;  //!< number of whole long sync lines for vertical synchronization
    int      m_nbHalfLongSync;   //!< number of half long sync / equalization lines
    int      m_nbWholeEqLines;   //!< number of whole equalizing lines
    bool     m_singleLongSync;   //!< single or double long sync per long sync line
    int      m_nbBlankLines;     //!< number of lines in a frame (full or half) that are blanked (black) at the top of the image
    float    m_blankLineLvel;    //!< video level of blank lines
    float    m_hBarIncrement;    //!< video level increment at each horizontal bar increment
    float    m_vBarIncrement;    //!< video level increment at each vertical bar increment
    bool     m_interleaved;      //!< true if image is interlaced (2 half frames per frame)
    bool     m_evenImage;        //!< in interlaced mode true if this is an even image
    int      m_horizontalCount;  //!< current point index on line
    int      m_lineCount;        //!< current line index in frame
    float    m_fps;              //!< resulting frames per second

    MovingAverageUtil<double, double, 16> m_movingAverage;
    quint32 m_levelCalcCount;
    qreal m_rmsLevel;
    qreal m_peakLevelOut;
    Real m_peakLevel;
    Real m_levelSum;

    cv::Mat m_imageFromFile;     //!< original image not resized not overlaid by text
    cv::Mat m_imageOriginal;     //!< original not resized image
    cv::Mat m_image;             //!< resized image for transmission at given rate
    bool m_imageOK;

    cv::VideoCapture m_video;    //!< current video capture
    cv::Mat m_videoframeOriginal; //!< current frame from video
    cv::Mat m_videoFrame;        //!< current displayable video frame
    float m_videoFPS;            //!< current video FPS rate
    int m_videoWidth;            //!< current video frame width
    int m_videoHeight;           //!< current video frame height
    float m_videoFx;             //!< current video horizontal scaling factor
    float m_videoFy;             //!< current video vertictal scaling factor
    float m_videoFPSq;           //!< current video FPS sacaling factor
    float m_videoFPSCount;       //!< current video FPS fractional counter
    int m_videoPrevFPSCount;     //!< current video FPS previous integer counter
    int m_videoLength;           //!< current video length in frames
    bool m_videoEOF;             //!< current video has reached end of file
    bool m_videoOK;

    std::vector<ATVCamera> m_cameras; //!< vector of available cameras
    int m_cameraIndex;           //!< curent camera index in list of available cameras

    std::string m_overlayText;
    QString m_imageFileName;
    QString m_videoFileName;

    // Used for standard SSB
    fftfilt* m_SSBFilter;
    Complex* m_SSBFilterBuffer;
    int m_SSBFilterBufferIndex;

    // Used for vestigial SSB with asymmetrical filtering (needs double sideband scheme)
    fftfilt* m_DSBFilter;
    Complex* m_DSBFilterBuffer;
    int m_DSBFilterBufferIndex;

    MessageQueue *m_messageQueueToGUI;

    static const int m_ssbFftLen;
    static const float m_blackLevel;
    static const float m_spanLevel;
    static const int m_levelNbSamples;
    static const int m_nbBars; //!< number of bars in bar or chessboard patterns
    static const int m_cameraFPSTestNbFrames; //!< number of frames for camera FPS test

    void pullFinalize(Complex& ci, Sample& sample);
    void pullVideo(Real& sample);
    void calculateLevel(Real& sample);
    void modulateSample();
    Complex& modulateSSB(Real& sample);
    Complex& modulateVestigialSSB(Real& sample);
    void applyStandard(const ATVModSettings& settings);
    void resizeImage();
    void calculateVideoSizes();
    void resizeVideo();
    void scanCameras();
    void releaseCameras();
    void calculateCamerasSizes();
    void resizeCameras();
    void resizeCamera();
    void mixImageAndText(cv::Mat& image);

    MessageQueue *getMessageQueueToGUI() { return m_messageQueueToGUI; }

    inline void pullImageLine(Real& sample, bool noHSync = false)
    {
        if (m_horizontalCount < m_pointsPerSync) // sync pulse
        {
            sample = noHSync ? m_blackLevel : 0.0f; // ultra-black
        }
        else if (m_horizontalCount < m_pointsPerSync + m_pointsPerBP) // back porch
        {
            sample = m_blackLevel; // black
        }
        else if (m_horizontalCount < m_pointsPerSync + m_pointsPerBP + m_pointsPerImgLine)
        {
            int pointIndex = m_horizontalCount - (m_pointsPerSync + m_pointsPerBP);
            int oddity = m_lineCount < m_nbLines2 + 1 ? 0 : 1;
            int iLine = oddity == 0 ? m_lineCount :  m_lineCount - m_nbLines2 - 1;
            int iLineImage = iLine - m_nbBlankLines - (oddity == 0 ? m_nbSyncLinesHeadE : m_nbSyncLinesHeadO);

            switch(m_settings.m_atvModInput)
            {
            case ATVModSettings::ATVModInputHBars:
                sample = (pointIndex / m_pointsPerHBar) * m_hBarIncrement + m_blackLevel;
                break;
            case ATVModSettings::ATVModInputVBars:
                sample = (iLine / m_linesPerVBar) * m_vBarIncrement + m_blackLevel;
                break;
            case ATVModSettings::ATVModInputChessboard:
                sample = (((iLine / m_linesPerVBar)*5 + (pointIndex / m_pointsPerHBar)) % 2) * m_spanLevel * m_settings.m_uniformLevel + m_blackLevel;
                break;
            case ATVModSettings::ATVModInputHGradient:
                sample = (pointIndex / (float) m_pointsPerImgLine) * m_spanLevel + m_blackLevel;
                break;
            case ATVModSettings::ATVModInputVGradient:
                sample = ((iLine -5) / (float) m_nbImageLines2) * m_spanLevel + m_blackLevel;
                break;
            case ATVModSettings::ATVModInputImage:
                if (!m_imageOK || (iLineImage < -oddity) || m_image.empty())
                {
                    sample = m_spanLevel * m_settings.m_uniformLevel + m_blackLevel;
                }
                else
                {
                	unsigned char pixv;

                	if (m_interleaved) {
                        pixv = m_image.at<unsigned char>(2*iLineImage + oddity, pointIndex); // row (y), col (x)
                	} else {
                        pixv = m_image.at<unsigned char>(iLineImage, pointIndex); // row (y), col (x)
                	}

                    sample = (pixv / 256.0f) * m_spanLevel + m_blackLevel;
                }
                break;
            case ATVModSettings::ATVModInputVideo:
                if (!m_videoOK || (iLineImage < -oddity) || m_videoFrame.empty())
                {
                    sample = m_spanLevel * m_settings.m_uniformLevel + m_blackLevel;
                }
                else
                {
                	unsigned char pixv;

                	if (m_interleaved) {
                        pixv = m_videoFrame.at<unsigned char>(2*iLineImage + oddity, pointIndex); // row (y), col (x)
                	} else {
                        pixv = m_videoFrame.at<unsigned char>(iLineImage, pointIndex); // row (y), col (x)
                	}

                    sample = (pixv / 256.0f) * m_spanLevel + m_blackLevel;
                }
            	break;
            case ATVModSettings::ATVModInputCamera:
                if ((iLineImage < -oddity) || (m_cameraIndex < 0))
                {
                    sample = m_spanLevel * m_settings.m_uniformLevel + m_blackLevel;
                }
                else
                {
                    ATVCamera& camera = m_cameras[m_cameraIndex];

                    if (camera.m_videoFrame.empty())
                    {
                        sample = m_spanLevel * m_settings.m_uniformLevel + m_blackLevel;
                    }
                    else
                    {
                        unsigned char pixv;

                        if (m_interleaved) {
                            pixv = camera.m_videoFrame.at<unsigned char>(2*iLineImage + oddity, pointIndex); // row (y), col (x)
                        } else {
                            pixv = camera.m_videoFrame.at<unsigned char>(iLineImage, pointIndex); // row (y), col (x)
                        }

                        sample = (pixv / 256.0f) * m_spanLevel + m_blackLevel;
                    }
                }
                break;
            case ATVModSettings::ATVModInputUniform:
            default:
                sample = m_spanLevel * m_settings.m_uniformLevel + m_blackLevel;
            }
        }
        else // front porch
        {
            sample = m_blackLevel; // black
        }
    }

    inline void pullVSyncLineLongPulses(Real& sample)
    {
        int halfIndex = m_horizontalCount % (m_nbHorizPoints/2);

        if (halfIndex < (m_nbHorizPoints/2) - m_pointsPerSync) // ultra-black
        {
            sample = 0.0f;
        }
        else // black
        {
            if (m_singleLongSync && (m_horizontalCount < m_nbHorizPoints/2)) {
                sample = 0.0f;
            } else {
                sample = m_blackLevel;
            }
        }
    }

    inline void pullVSyncLineEqualizingPulses(Real& sample)
    {
        if (m_horizontalCount < m_pointsPerSync)
        {
            sample = 0.0f; // ultra-black
        }
        else if (m_horizontalCount < (m_nbHorizPoints/2))
        {
            sample = m_blackLevel; // black
        }
        else if (m_horizontalCount < (m_nbHorizPoints/2) + m_pointsPerFSync)
        {
            sample = 0.0f; // ultra-black
        }
        else
        {
            sample = m_blackLevel; // black
        }
    }

    inline void pullVSyncLineEqualizingThenLongPulses(Real& sample)
    {
        if (m_horizontalCount < m_pointsPerSync)
        {
            sample = 0.0f; // ultra-black
        }
        else if (m_horizontalCount < (m_nbHorizPoints/2))
        {
            sample = m_blackLevel; // black
        }
        else if (m_horizontalCount < m_nbHorizPoints - m_pointsPerSync)
        {
            sample = 0.0f; // ultra-black
        }
        else
        {
            sample = m_blackLevel; // black
        }
    }

    inline void pullVSyncLineLongThenEqualizingPulses(Real& sample)
    {
        if (m_horizontalCount < (m_nbHorizPoints/2) - m_pointsPerSync)
        {
            sample = 0.0f; // ultra-black
        }
        else if (m_horizontalCount < (m_nbHorizPoints/2))
        {
            sample = m_blackLevel; // black
        }
        else if (m_horizontalCount < (m_nbHorizPoints/2) + m_pointsPerFSync)
        {
            sample = 0.0f; // ultra-black
        }
        else
        {
            sample = m_blackLevel; // black
        }
    }

    inline void pullVSyncLine(Real& sample)
    {
        if (m_lineCount < m_nbLines2 + 1) // even
        {
            int fieldLine = m_lineCount;

            if (fieldLine < m_nbLongSyncLines) // 0,1: Whole line "long" pulses
            {
                pullVSyncLineLongPulses(sample);
            }
            else if (fieldLine < m_nbLongSyncLines + m_nbHalfLongSync) // long pulse then equalizing pulse
            {
                pullVSyncLineLongThenEqualizingPulses(sample);
            }
            else if (fieldLine < m_nbLongSyncLines + m_nbHalfLongSync + m_nbWholeEqLines) // Whole line equalizing pulses
            {
                pullVSyncLineEqualizingPulses(sample);
            }
            else if (fieldLine > m_nbLines2 - m_nbHalfLongSync) // equalizing pulse then long pulse
            {
                pullVSyncLineEqualizingThenLongPulses(sample);
            }
            else if (fieldLine > m_nbLines2 - m_nbHalfLongSync - m_nbWholeEqLines) // Whole line equalizing pulses
            {
                pullVSyncLineEqualizingPulses(sample);
            }
            else // black images
            {
                if (m_horizontalCount < m_pointsPerSync)
                {
                    sample = 0.0f;
                }
                else
                {
                    sample = m_blankLineLvel;
                }
            }
        }
        else // odd
        {
            int fieldLine = m_lineCount - m_nbLines2 - 1;

            if (fieldLine < m_nbLongSyncLines) // 0,1: Whole line "long" pulses
            {
                pullVSyncLineLongPulses(sample);
            }
            else if (fieldLine < m_nbLongSyncLines + m_nbWholeEqLines) // Whole line equalizing pulses
            {
                pullVSyncLineEqualizingPulses(sample);
            }
            else if (fieldLine > m_nbLines2 - 1 - m_nbWholeEqLines - m_nbHalfLongSync) // Whole line equalizing pulses
            {
                pullVSyncLineEqualizingPulses(sample);
            }
            else // black images
            {
                if (m_horizontalCount < m_pointsPerSync)
                {
                    sample = 0.0f;
                }
                else
                {
                    sample = m_blankLineLvel;
                }
            }
        }
    }
};


#endif /* PLUGINS_CHANNELTX_MODATV_ATVMOD_H_ */
