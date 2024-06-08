package chipyard

import chisel3._
import chisel3.util._
import chisel3.util.log2Ceil
import chisel3.util.Cat 

import freechips.rocketchip.config.{Config}
import freechips.rocketchip.config._
import freechips.rocketchip.devices.debug._
import freechips.rocketchip.devices.tilelink._
import freechips.rocketchip.diplomacy._
import freechips.rocketchip.rocket._
import freechips.rocketchip.tile._
import freechips.rocketchip.tilelink._
import freechips.rocketchip.util._



//============================================================================================================//

//============================================================================================================//
  // length of the key in 32-bit words: 4 words for AES-128, 6 words for AES-192, and 8 words for AES-256
  // rounds: 128 bits key => 10 Rounds, 192 bits key => 12 Rounds, 256 bits key => 14 Rounds
  // round keys needed: 11 round keys for AES-128, 13 keys for AES-192, and 15 keys for AES-256
  
  // Nk=4: Nr=10, 44*32 (11*128)
  // Nk=6, Nr=12, 52*32 (13*128)
  // Nk=8: Nr=14, 60*32 (15*128)
  
case class AES_Config(
  KeySize : Int,
  KeyLength : Int,
  BlockSize : Int,
  NumberRounds : Int,
  KeyBSize : Int,
)

class AES_128 extends AES_Config (
  KeySize = 128,
  KeyLength = 4,
  BlockSize = 4,
  NumberRounds = 10,
  KeyBSize = 44,
)


//========================================================================================================///
class AES_Module(opcodes: OpcodeSet, c: AES_Config = new AES_128)(implicit p: Parameters) extends LazyRoCC(opcodes) {
  override lazy val module = new AES_ModuleImp(this, c)
}

class AES_ModuleImp(outer: AES_Module, c: AES_Config)(implicit p: Parameters) extends LazyRoCCModuleImp(outer)
    with HasCoreParameters {
   val regfile = Mem(4, UInt(32.W))
  val busy = RegInit(VecInit(Seq.fill(4){false.B}))

  val funct = io.cmd.bits.inst.funct
  val addr = io.cmd.bits.rs2(log2Up(4)-1,0)
  val doWrite = funct === 0.U
  val doRead = funct === 1.U
  val doLoad = funct === 2.U
  val doAccum = funct === 3.U
  val doStore = funct === 4.U
  val memRespTag = io.mem.resp.bits.tag(log2Up(4)-1,0)

  // datapath
  val addend = io.cmd.bits.rs1
  val accum = regfile(addr)
  val mem_cmd = Wire(UInt(4.W))   
  mem_cmd := M_XRD
    when(doLoad) {
      mem_cmd := M_XRD
    }
    when(doStore) {
      mem_cmd := M_XWR
    }
  

  when (io.mem.resp.valid) {
    regfile(memRespTag) := io.mem.resp.bits.data
    busy(memRespTag) := false.B
  }

  // control
  when (io.mem.req.fire()) {
    busy(addr) := true.B
  }

  val doResp = io.cmd.bits.inst.xd
  val stallReg = busy(addr)
  val stallLoad = doLoad && !io.mem.req.ready
  val stallResp = doResp && !io.resp.ready

  io.cmd.ready := !stallReg && !stallLoad && !stallResp
    // command resolved if no stalls AND not issuing a load that will need a request

  // PROC RESPONSE INTERFACE
  io.resp.valid := io.cmd.valid && doResp && !stallReg && !stallLoad
    // valid response if valid command, need a response, and no stalls
  io.resp.bits.rd := io.cmd.bits.inst.rd
    // Must respond with the appropriate tag or undefined behavior
  io.resp.bits.data := accum
    // Semantics is to always send out prior accumulator register value

  io.busy := io.cmd.valid || busy.reduce(_||_)
    // Be busy when have pending memory requests or committed possibility of pending requests
  io.interrupt := false.B
    // Set this true to trigger an interrupt on the processor (please refer to supervisor documentation)

  // MEMORY REQUEST INTERFACE
  io.mem.req.valid := io.cmd.valid && (doLoad || doStore) && !stallReg && !stallResp
  io.mem.req.bits.addr := addend
  io.mem.req.bits.tag := addr
  io.mem.req.bits.cmd := mem_cmd // perform a load (M_XWR for stores) M_XRD load
  io.mem.req.bits.size := log2Ceil(8).U
  io.mem.req.bits.signed := false.B
  io.mem.req.bits.data := 1.U // we're not performing any stores...
  io.mem.req.bits.phys := false.B
  io.mem.req.bits.dprv := io.cmd.bits.status.dprv

}