//
//  Generated file. Do not edit.
//

#include "generated_plugin_registrant.h"

#include <agora_rtc_engine/agora_rtc_engine_plugin.h>
#include <agora_rtc_rawdata/agora_rtc_rawdata_plugin.h>

void fl_register_plugins(FlPluginRegistry* registry) {
  g_autoptr(FlPluginRegistrar) agora_rtc_engine_registrar =
      fl_plugin_registry_get_registrar_for_plugin(registry, "AgoraRtcEnginePlugin");
  agora_rtc_engine_plugin_register_with_registrar(agora_rtc_engine_registrar);
  g_autoptr(FlPluginRegistrar) agora_rtc_rawdata_registrar =
      fl_plugin_registry_get_registrar_for_plugin(registry, "AgoraRtcRawdataPlugin");
  agora_rtc_rawdata_plugin_register_with_registrar(agora_rtc_rawdata_registrar);
}
