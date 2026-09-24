#include "controller.h"

DehumidifierController::DehumidifierController(const ControlConfig& cfg) : cfg_(cfg), guard_(cfg) {}
void DehumidifierController::reset(std::uint32_t nowMs) { guard_.reset(nowMs); state_=OperatingState::STARTUP_LOCKOUT; stateSinceMs_=nowMs; demandLatched_=false; pumpRunning_=false; pumpSinceMs_=nowMs; }

bool DehumidifierController::criticalFault(const SensorSnapshot& s,const InputSnapshot& i) const {
  return !i.configValid || !i.requiredI2cOk || i.secondaryHigh || i.interlockOpen || !s.roomValid || (cfg_.requireEvapSensor && !s.evapValid);
}
bool DehumidifierController::humidityDemand(const SensorSnapshot& s) const {
  if (!s.roomValid) return false;
  return demandLatched_ ? s.roomRh > (cfg_.targetRh-cfg_.hysteresisRh) : s.roomRh >= (cfg_.targetRh+cfg_.hysteresisRh);
}

OutputRequest DehumidifierController::update(std::uint32_t nowMs,const SensorSnapshot& s,const InputSnapshot& i) {
  OutputRequest out{};
  if (criticalFault(s,i)) { guard_.update(false,true,nowMs); state_=OperatingState::FAULT; out.alarm=true; return out; }
  if (i.bucketFull) { guard_.update(false,true,nowMs); state_=OperatingState::BUCKET_FULL; out.alarm=true; return out; }
  if (cfg_.pumpEnabled && i.pumpCall) {
    if (!pumpRunning_) { pumpRunning_=true; pumpSinceMs_=nowMs; }
    if ((nowMs-pumpSinceMs_) > cfg_.pumpMaxRunMs) { pumpRunning_=false; guard_.update(false,true,nowMs); state_=OperatingState::FAULT; out.alarm=true; return out; }
    out.pump=true;
  } else pumpRunning_=false;
  const bool demand=humidityDemand(s); demandLatched_=demand;
  const bool freezeTrip=cfg_.requireEvapSensor && s.evapValid && s.evapTempC<=cfg_.freezeThresholdC;
  if (state_==OperatingState::STARTUP_LOCKOUT) {
    guard_.update(false,freezeTrip,nowMs);
    if ((nowMs-stateSinceMs_)<cfg_.startupLockoutMs) return out;
    state_=demand && !freezeTrip ? OperatingState::FAN_PRERUN : OperatingState::IDLE; stateSinceMs_=nowMs; out.fan=state_==OperatingState::FAN_PRERUN; return out;
  }
  if (!demand || freezeTrip) {
    const bool wasRunning=guard_.running(); const bool running=guard_.update(false,freezeTrip,nowMs); out.compressor=running;
    if (running) { out.fan=true; state_=OperatingState::DEHUMIDIFY; return out; }
    if (wasRunning || state_==OperatingState::DEHUMIDIFY) { state_=OperatingState::FAN_POSTRUN; stateSinceMs_=nowMs; }
    if (state_==OperatingState::FAN_POSTRUN && (nowMs-stateSinceMs_)<cfg_.fanPostrunMs) out.fan=true; else state_=OperatingState::IDLE;
    return out;
  }
  if (state_==OperatingState::IDLE || state_==OperatingState::FAN_POSTRUN) { state_=OperatingState::FAN_PRERUN; stateSinceMs_=nowMs; }
  if (state_==OperatingState::FAN_PRERUN) { out.fan=true; guard_.update(false,false,nowMs); if ((nowMs-stateSinceMs_)<cfg_.fanPrerunMs) return out; }
  out.compressor=guard_.update(true,false,nowMs); out.fan=true; state_=out.compressor?OperatingState::DEHUMIDIFY:OperatingState::FAN_PRERUN; return out;
}
