#pragma once
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
struct Parameter { const char* name; double min,max,initial; bool stepped; };
// Independent DSP; the host renews the momentary control with a bounded lease.
namespace plugin {
constexpr bool sidechain=false;
constexpr Parameter parameters[]={
 {"Modo",0,2,1,true},{"Aberto (alternar)",0,1,0,true},
 {"Segurar para falar",0,1,0,true},{"Transição (ms)",1,50,5,false}};
struct Dsp {
 int mode=1;bool opened=false,held=false;float gain=0,increment=1.f/240;
 void reset(){gain=0;held=false;}
 void configure(uint32_t id,double v){if(id==0)mode=static_cast<int>(v);if(id==1)opened=v!=0;if(id==2)held=v!=0;if(id==3)increment=static_cast<float>(1./(48.*v));}
 void sample(const float in[2],const float[2],float out[2]){
  const float target=(mode==0||(mode==1&&held)||(mode==2&&opened))?1.f:0.f;
  gain=target>gain?std::min(target,gain+increment):std::max(target,gain-increment);
  out[0]=in[0]*gain;out[1]=in[1]*gain;
 }
};
}
