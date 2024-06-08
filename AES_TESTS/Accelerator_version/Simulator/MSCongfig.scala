package chipyard

import Chisel._
import chisel3.util.log2Ceil

import freechips.rocketchip.config.{Config}
import freechips.rocketchip.config._
import freechips.rocketchip.devices.debug._
import freechips.rocketchip.devices.tilelink._
import freechips.rocketchip.diplomacy._
import freechips.rocketchip.rocket._
import freechips.rocketchip.tile._
import freechips.rocketchip.tilelink._
import freechips.rocketchip.util._
import freechips.rocketchip.diplomacy.{AsynchronousCrossing}


// DOC include start: AESTLRocketConfig
class MS32MMIOConfig128Round1 extends Config(
  new freechips.rocketchip.subsystem.WithRV32 ++       
  new chipyard.WithAES() ++          // Use GCD Chisel, connect Tilelink
  new freechips.rocketchip.subsystem.WithNBigCores(1) ++
  new chipyard.config.AbstractConfig)
// DOC include end: AESTLRocketConfig

/*
class WithChamelIoT extends Config((site, here, up) => {
  case BuildRoCC => Seq(
    (p: Parameters) => {
        val aes = LazyModule(new AES_Module(OpcodeSet.custom0)(p))
        aes
    }
    )
})


 class MS32RoccConfig128Round9 extends Config(
 new WithChamelIoT ++
 new freechips.rocketchip.subsystem.WithRV32 ++           // set RocketTiles to be 32-bit
 new freechips.rocketchip.system.WithJtagDTMSystem ++
 new freechips.rocketchip.subsystem.WithNBigCores(1) ++
 new chipyard.config.AbstractConfig)

 class MS32RoccConfig128Round1 extends Config(
 new WithChamelIoT ++
 new freechips.rocketchip.subsystem.WithRV32 ++           // set RocketTiles to be 32-bit
 new freechips.rocketchip.system.WithJtagDTMSystem ++
 new freechips.rocketchip.subsystem.WithNBigCores(1) ++
 new chipyard.config.AbstractConfig)

 class MS32RoccConfig128Round1Mem extends Config(
 new WithChamelIoT ++
 new freechips.rocketchip.subsystem.WithRV32 ++           // set RocketTiles to be 32-bit
 new freechips.rocketchip.system.WithJtagDTMSystem ++
 new freechips.rocketchip.subsystem.WithNBigCores(1) ++
 new chipyard.config.AbstractConfig)

 class MS32RoccConfigTEST extends Config(
 new WithChamelIoT ++
 new freechips.rocketchip.subsystem.WithRV32 ++           // set RocketTiles to be 32-bit
 new freechips.rocketchip.system.WithJtagDTMSystem ++
 new freechips.rocketchip.subsystem.WithNBigCores(1) ++
 new chipyard.config.AbstractConfig)
*/
