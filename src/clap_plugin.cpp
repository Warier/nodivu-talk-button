// Only CLAP ABI / validation. Algorithm: dsp.h. Presentation: plugin.json.
#include <clap/clap.h>
#include <atomic>
#include <cstdio>
#include <cstring>
#include <new>
#include "dsp.h"
namespace {
constexpr uint32_t count=sizeof(plugin::parameters)/sizeof(plugin::parameters[0]);
const char* features[]={CLAP_PLUGIN_FEATURE_AUDIO_EFFECT,CLAP_PLUGIN_FEATURE_UTILITY,nullptr};
const clap_plugin_descriptor_t descriptor={CLAP_VERSION_INIT,"org.nodivu.talk-button","Botão de fala","Nodivu contributors","https://github.com/Warier/nodivu-talk-button","","","1.0.0","Sempre aberto, segurar para falar ou alternar o microfone.",features};
struct Instance {
 clap_plugin_t plugin{};plugin::Dsp dsp;std::atomic<double> values[count];bool active=false,processing=false;
 Instance(){for(uint32_t i=0;i<count;++i)values[i].store(plugin::parameters[i].initial);}
};
Instance* state(const clap_plugin_t* p){return static_cast<Instance*>(p->plugin_data);}
void set(Instance* s,clap_id id,double value){
 if(id>=count||!std::isfinite(value))return;const auto& p=plugin::parameters[id];
 if(value<p.min||value>p.max||(p.stepped&&std::floor(value)!=value))return;
 s->values[id].store(value,std::memory_order_relaxed);s->dsp.configure(id,value);
}
bool CLAP_ABI init(const clap_plugin_t*){return true;}
void CLAP_ABI destroy(const clap_plugin_t* p){delete state(p);}
bool CLAP_ABI activate(const clap_plugin_t* p,double rate,uint32_t min,uint32_t max){if(rate!=48000||!min||max>480||min>max)return false;state(p)->active=true;return true;}
void CLAP_ABI deactivate(const clap_plugin_t* p){state(p)->active=false;}
bool CLAP_ABI start(const clap_plugin_t* p){state(p)->processing=state(p)->active;return state(p)->processing;}
void CLAP_ABI stop(const clap_plugin_t* p){state(p)->processing=false;}
void CLAP_ABI reset(const clap_plugin_t* p){auto*s=state(p);s->dsp.reset();for(uint32_t i=0;i<count;++i)s->dsp.configure(i,s->values[i].load(std::memory_order_relaxed));}
void CLAP_ABI main_thread(const clap_plugin_t*){}
clap_process_status CLAP_ABI process(const clap_plugin_t* p,const clap_process_t* a){
 auto*s=state(p);if(!s->processing||!a||!a->frames_count||a->frames_count>480||a->audio_inputs_count!=(plugin::sidechain?2u:1u)||a->audio_outputs_count!=1||!a->audio_inputs||!a->audio_outputs)return CLAP_PROCESS_ERROR;
 for(uint32_t i=0;i<a->audio_inputs_count;++i)if(a->audio_inputs[i].channel_count!=2||!a->audio_inputs[i].data32||!a->audio_inputs[i].data32[0]||!a->audio_inputs[i].data32[1])return CLAP_PROCESS_ERROR;
 if(a->audio_outputs[0].channel_count!=2||!a->audio_outputs[0].data32||!a->audio_outputs[0].data32[0]||!a->audio_outputs[0].data32[1])return CLAP_PROCESS_ERROR;
 uint32_t event=0,total=a->in_events?a->in_events->size(a->in_events):0;if(total>32)return CLAP_PROCESS_ERROR;
 for(uint32_t i=0;i<a->frames_count;++i){
  while(event<total){auto*h=a->in_events->get(a->in_events,event);if(!h)return CLAP_PROCESS_ERROR;if(h->time>i)break;
   if(h->space_id==CLAP_CORE_EVENT_SPACE_ID&&h->type==CLAP_EVENT_PARAM_VALUE&&h->size>=sizeof(clap_event_param_value_t)){auto*v=reinterpret_cast<const clap_event_param_value_t*>(h);set(s,v->param_id,v->value);}++event;}
  float in[2]={a->audio_inputs[0].data32[0][i],a->audio_inputs[0].data32[1][i]},key[2]={0,0},out[2];
  if(plugin::sidechain){key[0]=a->audio_inputs[1].data32[0][i];key[1]=a->audio_inputs[1].data32[1][i];}
  if(!std::isfinite(in[0])||!std::isfinite(in[1])||!std::isfinite(key[0])||!std::isfinite(key[1]))return CLAP_PROCESS_ERROR;
  s->dsp.sample(in,key,out);for(uint32_t c=0;c<2;++c)a->audio_outputs[0].data32[c][i]=out[c];
 }
 return CLAP_PROCESS_CONTINUE;
}
uint32_t CLAP_ABI port_count(const clap_plugin_t*,bool input){return input&&plugin::sidechain?2:1;}
bool CLAP_ABI port_info(const clap_plugin_t* p,uint32_t index,bool input,clap_audio_port_info_t* info){
 if(!info||index>=port_count(p,input))return false;*info={};info->id=input?10+index:20;
 std::snprintf(info->name,sizeof(info->name),"%s",index?"Referencia":"Audio");info->flags=index?0:CLAP_AUDIO_PORT_IS_MAIN;
 info->channel_count=2;info->port_type=CLAP_PORT_STEREO;info->in_place_pair=CLAP_INVALID_ID;return true;
}
const clap_plugin_audio_ports_t ports={port_count,port_info};
uint32_t CLAP_ABI param_count(const clap_plugin_t*){return count;}
bool CLAP_ABI param_info(const clap_plugin_t*,uint32_t i,clap_param_info_t* info){if(i>=count||!info)return false;*info={};const auto& s=plugin::parameters[i];info->id=i;info->flags=CLAP_PARAM_IS_AUTOMATABLE|(s.stepped?CLAP_PARAM_IS_STEPPED:0);std::snprintf(info->name,sizeof(info->name),"%s",s.name);info->min_value=s.min;info->max_value=s.max;info->default_value=s.initial;return true;}
bool CLAP_ABI value(const clap_plugin_t* p,clap_id id,double* v){if(id>=count||!v)return false;*v=state(p)->values[id].load(std::memory_order_relaxed);return true;}
bool CLAP_ABI text(const clap_plugin_t*,clap_id,double v,char* out,uint32_t size){return out&&size&&std::snprintf(out,size,"%.2f",v)>0;}
bool CLAP_ABI parse(const clap_plugin_t*,clap_id,const char*,double*){return false;}
void CLAP_ABI flush(const clap_plugin_t*p,const clap_input_events_t*in,const clap_output_events_t*){if(!in)return;for(uint32_t i=0;i<std::min(in->size(in),32u);++i){auto*h=in->get(in,i);if(h&&h->space_id==CLAP_CORE_EVENT_SPACE_ID&&h->type==CLAP_EVENT_PARAM_VALUE&&h->size>=sizeof(clap_event_param_value_t)){auto*v=reinterpret_cast<const clap_event_param_value_t*>(h);set(state(p),v->param_id,v->value);}}}
const clap_plugin_params_t params={param_count,param_info,value,text,parse,flush};
uint32_t CLAP_ABI latency(const clap_plugin_t*){return 0;}
const clap_plugin_latency_t latency_ext={latency};
const void* CLAP_ABI extension(const clap_plugin_t*,const char* id){if(!std::strcmp(id,CLAP_EXT_AUDIO_PORTS))return &ports;if(!std::strcmp(id,CLAP_EXT_PARAMS))return &params;if(!std::strcmp(id,CLAP_EXT_LATENCY))return &latency_ext;return nullptr;}
uint32_t CLAP_ABI plugin_count(const clap_plugin_factory_t*){return 1;}
const clap_plugin_descriptor_t* CLAP_ABI describe(const clap_plugin_factory_t*,uint32_t i){return i?nullptr:&descriptor;}
const clap_plugin_t* CLAP_ABI create(const clap_plugin_factory_t*,const clap_host_t*host,const char*id){if(!host||!clap_version_is_compatible(host->clap_version)||!id||std::strcmp(id,descriptor.id))return nullptr;auto*s=new(std::nothrow)Instance;if(!s)return nullptr;s->plugin={&descriptor,s,init,destroy,activate,deactivate,start,stop,reset,process,extension,main_thread};return &s->plugin;}
const clap_plugin_factory_t factory={plugin_count,describe,create};
bool CLAP_ABI entry_init(const char*){return true;}void CLAP_ABI entry_deinit(){}
const void* CLAP_ABI factory_get(const char*id){return id&&!std::strcmp(id,CLAP_PLUGIN_FACTORY_ID)?&factory:nullptr;}
}
extern "C" CLAP_EXPORT const clap_plugin_entry_t clap_entry={CLAP_VERSION_INIT,entry_init,entry_deinit,factory_get};
