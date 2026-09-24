#include <cstdlib>
#include <iostream>
#include "../../firmware/UniversalDehumidifier/controller.h"
static int failures=0;
#define CHECK(x) do{if(!(x)){std::cerr<<"FAIL line "<<__LINE__<<" " #x "\n";++failures;}}while(0)
static SensorSnapshot goodSensors(float rh=60.0f,float evap=10.0f){SensorSnapshot s; s.roomRh=rh;s.evapTempC=evap;s.roomValid=true;s.evapValid=true;return s;}
static InputSnapshot goodInputs(){InputSnapshot i;i.configValid=true;i.requiredI2cOk=true;return i;}
int main(){
  ControlConfig cfg; DehumidifierController c(cfg); c.reset(0);
  CHECK(!c.update(179999,goodSensors(),goodInputs()).compressor);
  c.update(180000,goodSensors(),goodInputs()); CHECK(c.update(185000,goodSensors(),goodInputs()).compressor);
  auto in=goodInputs();in.bucketFull=true;auto out=c.update(185001,goodSensors(),in);CHECK(!out.compressor&&out.alarm);
  DehumidifierController c2(cfg);c2.reset(0);in=goodInputs();in.requiredI2cOk=false;out=c2.update(999999,goodSensors(),in);CHECK(!out.compressor&&out.alarm);
  CompressorGuard g(cfg);std::uint32_t start=0xFFFFFF00u;g.reset(start);CHECK(!g.update(true,false,start+1000u));CHECK(g.update(true,false,start+180000u));CHECK(!g.update(true,true,start+180001u));
  ControlConfig pcfg;pcfg.pumpEnabled=true;DehumidifierController p(pcfg);p.reset(0);in=goodInputs();in.pumpCall=true;out=p.update(180000,goodSensors(45.0f),in);CHECK(out.pump&&!out.compressor);
  in.secondaryHigh=true;out=p.update(180001,goodSensors(),in);CHECK(!out.compressor&&!out.fan&&!out.pump&&out.alarm);
  if(failures)return EXIT_FAILURE;std::cout<<"host safety tests passed\n";return EXIT_SUCCESS;
}
