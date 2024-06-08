// See LICENSE for license details.
package sifive.freedom.everywhere.e300artydevkit


import sifive.blocks.devices.mockaon._
import sifive.blocks.devices.gpio._
import sifive.blocks.devices.pwm._
import sifive.blocks.devices.spi._
import sifive.blocks.devices.uart._
import sifive.blocks.devices.i2c._

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
import freechips.rocketchip.subsystem._
import freechips.rocketchip.diplomacy.{DTSModel, DTSTimebase}
import freechips.rocketchip.system._
import freechips.rocketchip.diplomacy.{AsynchronousCrossing}

class WithChamelIoT extends Config((site, here, up) => {
  case BuildRoCC => Seq(
    (p: Parameters) => {
        val aes_module = LazyModule(new AES_Module(OpcodeSet.custom0)(p))
        aes_module
    }
    )
})

/*
class WithChamelIoT extends Config((site, here, up) => {
  case BuildRoCC => Seq(
    (p: Parameters) => {
        val chameliot = LazyModule(new ChamelIoT(
          OpcodeSet.custom2,
          new ChamelIoTConfig( 
            MaxPriority = 8,
            MaxThreads = 8,
          )
          )(p))
        chameliot
    },

    (p: Parameters) => {
        val accumulator = LazyModule(new AccumulatorExample(OpcodeSet.custom0, n = 4)(p))
        accumulator
    }
    )
})
*/


// Default FreedomEConfig
class DefaultFreedomEConfig extends Config (

  new WithNBreakpoints(8)        ++
  new WithNExtTopInterrupts(0)   ++
  new WithJtagDTM                ++
  new WithL1ICacheWays(2)        ++
  new WithL1ICacheSets(128)      ++
  new WithDefaultBtb             ++
  new WithL1DCacheSets(1024)    ++
  new WithChamelIoT ++
  //new sifive.freedom.everywhere.e300artydevkit.WithAES() ++   
  new TinyConfig
) 

// Freedom E300 Arty Dev Kit Peripherals
class E300DevKitPeripherals extends Config((site, here, up) => {
  case PeripheryGPIOKey => List(
    GPIOParams(address = 0x10012000, width = 32, includeIOF = true))
  case PeripheryPWMKey => List(
    PWMParams(address = 0x10015000, cmpWidth = 8),
    PWMParams(address = 0x10025000, cmpWidth = 16),
    PWMParams(address = 0x10035000, cmpWidth = 16))
  case PeripherySPIKey => List(
    SPIParams(csWidth = 4, rAddress = 0x10024000, defaultSampleDel = 3),
    SPIParams(csWidth = 1, rAddress = 0x10034000, defaultSampleDel = 3))
  case PeripherySPIFlashKey => List(
    SPIFlashParams(
      fAddress = 0x20000000,
      rAddress = 0x10014000,
      defaultSampleDel = 3))
  case PeripheryUARTKey => List(
    UARTParams(address = 0x10013000),
    UARTParams(address = 0x10023000))
  case PeripheryI2CKey => List(
    I2CParams(address = 0x10016000))
  case PeripheryMockAONKey =>
    MockAONParams(address = 0x10000000)
  case PeripheryMaskROMKey => List(
    MaskROMParams(address = 0x10000, name = "BootROM"))
})

// Freedom E300 Arty Dev Kit Peripherals
class E300ArtyDevKitConfig extends Config(
  new E300DevKitPeripherals    ++
  new DefaultFreedomEConfig().alter((site,here,up) => {
    case DTSTimebase => BigInt(32768)
    case JtagDTMKey => new JtagDTMConfig (
      idcodeVersion = 2,
      idcodePartNum = 0x000,
      idcodeManufId = 0x489,
      debugIdleCycles = 5)
    case RocketTilesKey => up(RocketTilesKey, site) map { r =>
      r.copy(icache = r.icache.map(_.copy(itimAddr = Some(0x8000000L))))
    }
  })
)

