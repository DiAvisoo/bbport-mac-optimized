#!/usr/bin/env python3
"""Regression harness for the native 0.5 loading deadlock.

Compiles the actual sceGnmDingDong function from the baseline and working checkout
against a small simulated GPU queue. A graphics job cannot retire until compute
has been submitted. The baseline must time out; the repaired function must pass
both completion modes, wrapped rings and empty/invalid submissions. This checks
submission ordering, not graphics-driver execution; copied-save launches cover that.
"""
from pathlib import Path
import subprocess, tempfile
import argparse
parser=argparse.ArgumentParser(description="Exercise the native compute submission function against a cross-queue dependency.")
parser.add_argument('--native-checkout',type=Path,default=Path(__file__).resolve().parents[2])
parser.add_argument('--baseline',default='57b2dbde88')
args=parser.parse_args()
native=args.native_checkout.resolve()
name='gpu/shadps4/core/libraries/gnmdriver/gnmdriver.cpp'
old=subprocess.check_output(['git','show',args.baseline+':'+name],cwd=native,text=True)
new=(native/name).read_text()
prefix=r'''
#include <array>
#include <vector>
#include <span>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <chrono>
#include <stdexcept>
#include <iostream>
#include <cassert>
#include <cstdint>
using u32=uint32_t;using u64=uint64_t;
#define PS4_SYSV_ABI
#define HLE_TRACE
#define LOG_DEBUG(...)
std::mutex m_submit_lock,completion_lock;
std::condition_variable work_cv,completion_cv;
bool decode=false,queued=false,retired=false;
unsigned waits=0;
u32 asc_next_offs_dw[1]={};u64 frames_submitted=0;
std::array<u32,8> ring{};
std::vector<std::vector<u32>> submissions;
bool SubmitLockOnDecode(){return decode;}
void WaitGpuIdle(){
 ++waits;std::unique_lock lock(completion_lock);
 if(!completion_cv.wait_for(lock,std::chrono::milliseconds(250),[]{return retired;}))
  throw std::runtime_error("graphics is waiting for compute, but the compute submit is blocked");
}
void WaitPreviousFrameAtSubmit(){WaitGpuIdle();}
namespace Gpu {
 struct Queue {uintptr_t map_addr;u32 ring_size_dw;};
 Queue AscQueue(u32){return {reinterpret_cast<uintptr_t>(ring.data()),ring.size()};}
 void SubmitAsc(u32,std::span<const u32> s){
  submissions.emplace_back(s.begin(),s.end());
  for(u32 word:s)if(word==0xc0ffee){std::lock_guard lock(completion_lock);queued=true;work_cv.notify_all();}
 }
}
namespace DebugStateType{
 enum class QueueType {acb};
 struct Dump {QueueType type;u32 submit_num,num2;std::vector<u32> data;uintptr_t base_addr;};
}
struct Debug {
 bool ShouldPauseInSubmit(){return false;}void PauseGuestThreads(){}
 bool DumpingCurrentFrame(){return false;}void PushQueueDump(DebugStateType::Dump){}
} DebugState;
enum class PM4ItOpcode {IndirectBuffer};
struct PM4CmdIndirectBuffer {
 struct {PM4ItOpcode opcode;}header;u32 ib_size;
 template<class T>T* Address()const{return reinterpret_cast<T*>(ring.data());}
};
'''
suffix=r'''
int main(){try{
 for(bool decode_mode:{false,true})for(bool wrap:{false,true}){
  decode=decode_mode;queued=false;retired=false;waits=0;submissions.clear();ring.fill(0);
  asc_next_offs_dw[0]=wrap?6:0;ring[1]=0xc0ffee;
  std::jthread gpu([]{
   std::unique_lock lock(completion_lock);
   if(work_cv.wait_for(lock,std::chrono::milliseconds(400),[]{return queued;})){
    retired=true;completion_cv.notify_all();
   }
  });
  sceGnmDingDong(1,2);
  assert(retired&&waits==1&&asc_next_offs_dw[0]==2);
  assert(submissions.size()==(wrap?2:1));
  if(wrap)assert(submissions[0].size()==2);
  assert(submissions.back().size()==2&&submissions.back()[1]==0xc0ffee);
  gpu.join();
  submissions.clear();waits=0;
  sceGnmDingDong(1,2);assert(submissions.empty()&&waits==1);
  waits=0;sceGnmDingDong(0,0);assert(waits==0);
 }
 std::cout<<"PASS: cross-queue dependency, ring wrap, both completion modes, empty and invalid submissions\n";
 return 0;
}catch(const std::exception&e){std::cerr<<e.what()<<'\n';return 1;}}
'''
with tempfile.TemporaryDirectory(prefix='bb-submit-order-') as d:
 for label,source,expected in [('before',old,1),('after',new,0)]:
  start=source.index('void PS4_SYSV_ABI sceGnmDingDong(u32')
  end=source.index('\nvoid PS4_SYSV_ABI sceGnmDingDongForWorkload',start)
  cpp=Path(d)/(label+'.cpp');exe=Path(d)/label
  cpp.write_text(prefix+source[start:end]+suffix)
  subprocess.run(['clang++','-std=c++20','-O2','-isysroot','/Library/Developer/CommandLineTools/SDKs/MacOSX26.5.sdk',str(cpp),'-o',str(exe)],check=True)
  result=subprocess.run([str(exe)],text=True,capture_output=True,timeout=5)
  print(label+': '+result.stdout+result.stderr,end='')
  assert result.returncode==expected,(label,result.returncode)
