#include <plugin/plugininstancegui.h>
#include "dsp/devicesamplesource.h"
#include "dsp/devicesamplesink.h"
#include "dsp/devicesamplemimo.h"
#include "plugin/plugininterface.h"


void PluginInterface::deleteSampleSourcePluginInstanceGUI(PluginInstanceGUI *ui)
{
    if (ui) { ui->destroy(); }
}

void PluginInterface::deleteSampleSourcePluginInstanceInput(DeviceSampleSource *source)
{
    if (source) { source->destroy(); }
}

void PluginInterface::deleteSampleSinkPluginInstanceGUI(PluginInstanceGUI *ui)
{
    if (ui) { ui->destroy(); }
}

void PluginInterface::deleteSampleSinkPluginInstanceOutput(DeviceSampleSink *sink)
{
    if (sink) { sink->destroy(); }
}

void PluginInterface::deleteSampleMIMOPluginInstanceGUI(PluginInstanceGUI *ui)
{
    if (ui) { ui->destroy(); }
}

void PluginInterface::deleteSampleMIMOPluginInstanceMIMO(DeviceSampleMIMO *mimo)
{
    if (mimo) { mimo->destroy(); }
}
