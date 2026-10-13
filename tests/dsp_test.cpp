#include "../src/dsp.h"
#include <cassert>
#include <cstdio>
int main(){plugin::Dsp d;float in[2]={0.5f,-0.25f},key[2]={0,0},out[2];
 d.sample(in,key,out);assert(out[0]==0);d.configure(2,1);for(int i=0;i<480;++i)d.sample(in,key,out);assert(out[0]==0.5f&&out[1]==-0.25f);
 d.configure(2,0);float previous=out[0];for(int i=0;i<480;++i){d.sample(in,key,out);assert(out[0]<=previous);assert(previous-out[0]<=0.003f);previous=out[0];}assert(out[0]==0);
 d.configure(0,2);d.configure(1,1);for(int i=0;i<480;++i)d.sample(in,key,out);assert(out[0]==in[0]);d.configure(1,0);for(int i=0;i<480;++i)d.sample(in,key,out);assert(out[0]==0);
 d.configure(0,0);for(int i=0;i<480;++i)d.sample(in,key,out);assert(out[0]==in[0]);d.reset();assert(d.gain==0&&!d.held);
 plugin::Dsp other;other.sample(in,key,out);assert(out[0]==0);

 std::puts("PASS: independent DSP, stereo, state, controls, reset");}
