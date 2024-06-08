
package sifive.freedom.everywhere.e300artydevkit


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
import chisel3.experimental.{IntParam, BaseModule}
import freechips.rocketchip.amba.axi4._
import freechips.rocketchip.subsystem.BaseSubsystem
import freechips.rocketchip.config.{Parameters, Field, Config}
import freechips.rocketchip.regmapper.{HasRegMap, RegField}
import freechips.rocketchip.util.UIntIsOneOf



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

object Sub {
  val ByteTable: Array[Int] = Array(
    0x63, 0x7c, 0x77, 0x7b, 0xf2, 0x6b, 0x6f, 0xc5, 0x30, 0x01, 0x67, 0x2b, 0xfe, 0xd7, 0xab, 0x76,
    0xca, 0x82, 0xc9, 0x7d, 0xfa, 0x59, 0x47, 0xf0, 0xad, 0xd4, 0xa2, 0xaf, 0x9c, 0xa4, 0x72, 0xc0,
    0xb7, 0xfd, 0x93, 0x26, 0x36, 0x3f, 0xf7, 0xcc, 0x34, 0xa5, 0xe5, 0xf1, 0x71, 0xd8, 0x31, 0x15,
    0x04, 0xc7, 0x23, 0xc3, 0x18, 0x96, 0x05, 0x9a, 0x07, 0x12, 0x80, 0xe2, 0xeb, 0x27, 0xb2, 0x75,
    0x09, 0x83, 0x2c, 0x1a, 0x1b, 0x6e, 0x5a, 0xa0, 0x52, 0x3b, 0xd6, 0xb3, 0x29, 0xe3, 0x2f, 0x84,
    0x53, 0xd1, 0x00, 0xed, 0x20, 0xfc, 0xb1, 0x5b, 0x6a, 0xcb, 0xbe, 0x39, 0x4a, 0x4c, 0x58, 0xcf,
    0xd0, 0xef, 0xaa, 0xfb, 0x43, 0x4d, 0x33, 0x85, 0x45, 0xf9, 0x02, 0x7f, 0x50, 0x3c, 0x9f, 0xa8,
    0x51, 0xa3, 0x40, 0x8f, 0x92, 0x9d, 0x38, 0xf5, 0xbc, 0xb6, 0xda, 0x21, 0x10, 0xff, 0xf3, 0xd2,
    0xcd, 0x0c, 0x13, 0xec, 0x5f, 0x97, 0x44, 0x17, 0xc4, 0xa7, 0x7e, 0x3d, 0x64, 0x5d, 0x19, 0x73,
    0x60, 0x81, 0x4f, 0xdc, 0x22, 0x2a, 0x90, 0x88, 0x46, 0xee, 0xb8, 0x14, 0xde, 0x5e, 0x0b, 0xdb,
    0xe0, 0x32, 0x3a, 0x0a, 0x49, 0x06, 0x24, 0x5c, 0xc2, 0xd3, 0xac, 0x62, 0x91, 0x95, 0xe4, 0x79,
    0xe7, 0xc8, 0x37, 0x6d, 0x8d, 0xd5, 0x4e, 0xa9, 0x6c, 0x56, 0xf4, 0xea, 0x65, 0x7a, 0xae, 0x08,
    0xba, 0x78, 0x25, 0x2e, 0x1c, 0xa6, 0xb4, 0xc6, 0xe8, 0xdd, 0x74, 0x1f, 0x4b, 0xbd, 0x8b, 0x8a,
    0x70, 0x3e, 0xb5, 0x66, 0x48, 0x03, 0xf6, 0x0e, 0x61, 0x35, 0x57, 0xb9, 0x86, 0xc1, 0x1d, 0x9e,
    0xe1, 0xf8, 0x98, 0x11, 0x69, 0xd9, 0x8e, 0x94, 0x9b, 0x1e, 0x87, 0xe9, 0xce, 0x55, 0x28, 0xdf,
    0x8c, 0xa1, 0x89, 0x0d, 0xbf, 0xe6, 0x42, 0x68, 0x41, 0x99, 0x2d, 0x0f, 0xb0, 0x54, 0xbb, 0x16
  )

  val InvByteTable: Array[Int] = Array(
    0x52, 0x09, 0x6a, 0xd5, 0x30, 0x36, 0xa5, 0x38, 0xbf, 0x40, 0xa3, 0x9e, 0x81, 0xf3, 0xd7, 0xfb,
    0x7c, 0xe3, 0x39, 0x82, 0x9b, 0x2f, 0xff, 0x87, 0x34, 0x8e, 0x43, 0x44, 0xc4, 0xde, 0xe9, 0xcb,
    0x54, 0x7b, 0x94, 0x32, 0xa6, 0xc2, 0x23, 0x3d, 0xee, 0x4c, 0x95, 0x0b, 0x42, 0xfa, 0xc3, 0x4e,
    0x08, 0x2e, 0xa1, 0x66, 0x28, 0xd9, 0x24, 0xb2, 0x76, 0x5b, 0xa2, 0x49, 0x6d, 0x8b, 0xd1, 0x25,
    0x72, 0xf8, 0xf6, 0x64, 0x86, 0x68, 0x98, 0x16, 0xd4, 0xa4, 0x5c, 0xcc, 0x5d, 0x65, 0xb6, 0x92,
    0x6c, 0x70, 0x48, 0x50, 0xfd, 0xed, 0xb9, 0xda, 0x5e, 0x15, 0x46, 0x57, 0xa7, 0x8d, 0x9d, 0x84,
    0x90, 0xd8, 0xab, 0x00, 0x8c, 0xbc, 0xd3, 0x0a, 0xf7, 0xe4, 0x58, 0x05, 0xb8, 0xb3, 0x45, 0x06,
    0xd0, 0x2c, 0x1e, 0x8f, 0xca, 0x3f, 0x0f, 0x02, 0xc1, 0xaf, 0xbd, 0x03, 0x01, 0x13, 0x8a, 0x6b,
    0x3a, 0x91, 0x11, 0x41, 0x4f, 0x67, 0xdc, 0xea, 0x97, 0xf2, 0xcf, 0xce, 0xf0, 0xb4, 0xe6, 0x73,
    0x96, 0xac, 0x74, 0x22, 0xe7, 0xad, 0x35, 0x85, 0xe2, 0xf9, 0x37, 0xe8, 0x1c, 0x75, 0xdf, 0x6e,
    0x47, 0xf1, 0x1a, 0x71, 0x1d, 0x29, 0xc5, 0x89, 0x6f, 0xb7, 0x62, 0x0e, 0xaa, 0x18, 0xbe, 0x1b,
    0xfc, 0x56, 0x3e, 0x4b, 0xc6, 0xd2, 0x79, 0x20, 0x9a, 0xdb, 0xc0, 0xfe, 0x78, 0xcd, 0x5a, 0xf4,
    0x1f, 0xdd, 0xa8, 0x33, 0x88, 0x07, 0xc7, 0x31, 0xb1, 0x12, 0x10, 0x59, 0x27, 0x80, 0xec, 0x5f,
    0x60, 0x51, 0x7f, 0xa9, 0x19, 0xb5, 0x4a, 0x0d, 0x2d, 0xe5, 0x7a, 0x9f, 0x93, 0xc9, 0x9c, 0xef,
    0xa0, 0xe0, 0x3b, 0x4d, 0xae, 0x2a, 0xf5, 0xb0, 0xc8, 0xeb, 0xbb, 0x3c, 0x83, 0x53, 0x99, 0x61,
    0x17, 0x2b, 0x04, 0x7e, 0xba, 0x77, 0xd6, 0x26, 0xe1, 0x69, 0x14, 0x63, 0x55, 0x21, 0x0c, 0x7d
  )
}


object RTable {
  val Constants: Array[Int] = Array(0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x1b, 0x36)
  }

object Rcon {
  val RconTable = Wire(Vec(10, UInt(8.W)))
  RconTable := VecInit(RTable.Constants.map(_&0xff).map(_.U(8.W)))
  def apply(x: UInt):UInt = RconTable(x)
}

class Cipher_Module(c: AES_Config)(implicit p: Parameters) extends Module{ //32bit decoder implementation without shifts. Compiler doesn't accept shifts of more than 20bits
  

  //=====================INPUTS OUTPUT========================//
  val io = IO(new Bundle {
  val message = Input(Vec(c.BlockSize, UInt(32.W)))
  val in = Input(Vec(c.KeyLength, UInt(32.W)))
  val start = Input(Bool())
  val en = Input(Bool())
  val de = Input(Bool())
  val out = Output(Vec(c.BlockSize, UInt(32.W)))
  val ready = Output(Bool())
  val reset = Output(Bool())
  })
  //==============================================================///

  val Sbox1 = Wire(Vec(256, UInt(8.W)))
  Sbox1 := VecInit(Sub.ByteTable.map(_&0xff).map(_.U(8.W)))

  val Sbox2 = Wire(Vec(256, UInt(8.W)))
  Sbox2 := VecInit(Sub.InvByteTable.map(_&0xff).map(_.U(8.W)))
//========================================================///

  //==================================================FUCTIONS DEFINES==================================================//  
  def SubShiftRow (r0:UInt, r1:UInt, r2:UInt, r3:UInt): UInt = Cat(Sbox1(r0(31,24)), Sbox1(r1(23,16)), Sbox1(r2(15,8)), Sbox1(r3(7,0)))
  def InvSubShiftRow (r0:UInt, r1:UInt, r2:UInt, r3:UInt): UInt = Cat(Sbox2(r0(31,24)), Sbox2(r1(23,16)), Sbox2(r2(15,8)), Sbox2(r3(7,0)))
   
  def x02(x:UInt):UInt = Mux(x(7), Cat(x(6,0), 0.U(1.W)) ^ 0x1b.U(8.W), Cat(x(6,0), 0.U(1.W)))
  def x03(x:UInt):UInt = x02(x) ^ x
  def x09(x:UInt):UInt = x02(x02(x02(x))) ^ x
  def x0b(x:UInt):UInt = x02(x02(x02(x))) ^ x02(x) ^ x
  def x0d(x:UInt):UInt = x02(x02(x02(x))) ^ x02(x02(x)) ^ x
  def x0e(x:UInt):UInt = x02(x02(x02(x))) ^ x02(x02(x)) ^ x02(x)

  def En_Line0(w:UInt):UInt = x02(w(31,24)) ^ x03(w(23,16)) ^ w(15,8)      ^ w(7,0)
  def En_Line1(w:UInt):UInt = w(31,24)      ^ x02(w(23,16)) ^ x03(w(15,8)) ^ w(7,0)
  def En_Line2(w:UInt):UInt = w(31,24)      ^ w(23,16)      ^ x02(w(15,8)) ^ x03(w(7,0))
  def En_Line3(w:UInt):UInt = x03(w(31,24)) ^ w(23,16)      ^ w(15,8)      ^ x02(w(7,0))

  def De_Line0(w:UInt):UInt = x0e(w(31,24)) ^ x0b(w(23,16)) ^ x0d(w(15,8)) ^ x09(w(7,0))
  def De_Line1(w:UInt):UInt = x09(w(31,24)) ^ x0e(w(23,16)) ^ x0b(w(15,8)) ^ x0d(w(7,0))
  def De_Line2(w:UInt):UInt = x0d(w(31,24)) ^ x09(w(23,16)) ^ x0e(w(15,8)) ^ x0b(w(7,0))
  def De_Line3(w:UInt):UInt = x0b(w(31,24)) ^ x0d(w(23,16)) ^ x09(w(15,8)) ^ x0e(w(7,0))

  def SubWord (w:UInt): UInt = Cat(Sbox1(w(31,24)), Sbox1(w(23,16)), Sbox1(w(15,8)), Sbox1(w(7,0)))
  def SubRotateWord (w:UInt): UInt = Cat(Sbox1(w(23,16)), Sbox1(w(15,8)), Sbox1(w(7,0)), Sbox1(w(31,24)))
  def RconWord (y:UInt): UInt = Cat(Rcon(y), Zeros)
  //=====================REGISTERS========================//

  val message = RegInit(VecInit(Seq.fill(c.BlockSize)(0.U(32.W))))
  val key = RegInit(VecInit(Seq.fill(c.KeyBSize)(0.U(32.W))))
  val Zeros = RegInit(0.U(24.W))
  //=====================WIRES========================//
  val state = Wire(Vec(4,UInt(32.W)))  
  val state_d = Wire(Vec(4,UInt(32.W)))  

  //=====================MODULES========================//
  val round1 = RegInit(VecInit(Seq.fill(4)(0.U(32.W))))    
  //====================================================//
  for (i <- 0 until c.KeyLength) {
      key(i) := io.in(i)
    }
  for (i <- c.KeyLength until c.KeyBSize) {
    if((i % c.KeyLength) == 0) {
    key(i) := key(i-c.KeyLength) ^SubRotateWord(key(i-1)) ^ RconWord(((i/c.KeyLength)-1).asUInt)
    }
    else {
      key(i) := key(i-c.KeyLength) ^ key(i-1)
    }
  }

  for (i <- 0 until c.BlockSize) {
    message(i) := io.message(i)
    state(i) := message(i) ^ key(i)
    state_d(i) := message(i) ^ key((c.NumberRounds*4)+i)
  }
  

  //==============================================================///

  
  val E_round = RegInit(0.U(5.W))
  val D_round = RegInit(c.NumberRounds.U(5.W))
  when(io.start && (io.en && !io.de)){
    when(E_round === 0.U){
      for (i <- 0 until c.BlockSize) {
        round1(i) := state(i)
      }                  
      E_round := E_round + 1.U
      io.ready := false.B
      io.reset := false.B
      io.out(0) := 0.U 
      io.out(1) := 0.U 
      io.out(2) := 0.U 
      io.out(3) := 0.U 
    }.elsewhen((E_round =/= 0.U) && (E_round =/= c.NumberRounds.U)){
      
      round1(0) := Cat(En_Line0(SubShiftRow(round1(0), round1(1), round1(2), round1(3))), 
                        En_Line1(SubShiftRow(round1(0), round1(1), round1(2), round1(3))), 
                        En_Line2(SubShiftRow(round1(0), round1(1), round1(2), round1(3))), 
                        En_Line3(SubShiftRow(round1(0), round1(1), round1(2), round1(3)))) ^ key((E_round*4.U))
      round1(1) := Cat(En_Line0(SubShiftRow(round1(1), round1(2), round1(3), round1(0))), 
                        En_Line1(SubShiftRow(round1(1), round1(2), round1(3), round1(0))), 
                        En_Line2(SubShiftRow(round1(1), round1(2), round1(3), round1(0))), 
                        En_Line3(SubShiftRow(round1(1), round1(2), round1(3), round1(0)))) ^ key((E_round*4.U)+1.U)
      round1(2) := Cat(En_Line0(SubShiftRow(round1(2), round1(3), round1(0), round1(1))), 
                        En_Line1(SubShiftRow(round1(2), round1(3), round1(0), round1(1))), 
                        En_Line2(SubShiftRow(round1(2), round1(3), round1(0), round1(1))), 
                        En_Line3(SubShiftRow(round1(2), round1(3), round1(0), round1(1)))) ^ key((E_round*4.U)+2.U)
      round1(3) := Cat(En_Line0(SubShiftRow(round1(3), round1(0), round1(1), round1(2))), 
                        En_Line1(SubShiftRow(round1(3), round1(0), round1(1), round1(2))), 
                        En_Line2(SubShiftRow(round1(3), round1(0), round1(1), round1(2))), 
                        En_Line3(SubShiftRow(round1(3), round1(0), round1(1), round1(2)))) ^ key((E_round*4.U)+3.U)             
      E_round := E_round + 1.U
      io.ready := false.B
      io.reset := false.B
      io.out(0) := 0.U 
      io.out(1) := 0.U 
      io.out(2) := 0.U 
      io.out(3) := 0.U 
    }.otherwise{
      io.out(0) := Cat(SubShiftRow(round1(0), round1(1), round1(2), round1(3)), 
                      SubShiftRow(round1(0), round1(1), round1(2), round1(3)), 
                      SubShiftRow(round1(0), round1(1), round1(2), round1(3)), 
                      SubShiftRow(round1(0), round1(1), round1(2), round1(3))) ^ key(c.NumberRounds*4)
      io.out(1) := Cat(SubShiftRow(round1(1), round1(2), round1(3), round1(0)), 
                        SubShiftRow(round1(1), round1(2), round1(3), round1(0)), 
                        SubShiftRow(round1(1), round1(2), round1(3), round1(0)), 
                        SubShiftRow(round1(1), round1(2), round1(3), round1(0))) ^ key((c.NumberRounds*4)+1)
      io.out(2) := Cat(SubShiftRow(round1(2), round1(3), round1(0), round1(1)), 
                        SubShiftRow(round1(2), round1(3), round1(0), round1(1)), 
                        SubShiftRow(round1(2), round1(3), round1(0), round1(1)), 
                        SubShiftRow(round1(2), round1(3), round1(0), round1(1))) ^ key((c.NumberRounds*4)+2)
      io.out(3) := Cat(SubShiftRow(round1(3), round1(0), round1(1), round1(2)), 
                        SubShiftRow(round1(3), round1(0), round1(1), round1(2)), 
                        SubShiftRow(round1(3), round1(0), round1(1), round1(2)), 
                        SubShiftRow(round1(3), round1(0), round1(1), round1(2))) ^ key((c.NumberRounds*4)+3)    
      io.ready := true.B
      io.reset := false.B
    }
  }.elsewhen(io.start && (!io.en && io.de)){
    when(D_round === c.NumberRounds.U){         
      for (i <- 0 until c.BlockSize) {
        round1(i) := state_d(i)
      }                          
      D_round := D_round - 1.U
      io.ready := false.B
      io.reset := false.B
      io.out(0) := 0.U 
      io.out(1) := 0.U 
      io.out(2) := 0.U 
      io.out(3) := 0.U 
    }.elsewhen((D_round =/= c.NumberRounds.U) && (D_round =/= 0.U)){
                         
      round1(0) :=  Cat(De_Line0(InvSubShiftRow(round1(0), round1(3), round1(2), round1(1)) ^ key(D_round*4.U)), 
                        De_Line1(InvSubShiftRow(round1(0), round1(3), round1(2), round1(1)) ^ key(D_round*4.U)), 
                        De_Line2(InvSubShiftRow(round1(0), round1(3), round1(2), round1(1)) ^ key(D_round*4.U)), 
                        De_Line3(InvSubShiftRow(round1(0), round1(3), round1(2), round1(1)) ^ key(D_round*4.U))) 
      round1(1) :=  Cat(De_Line0(InvSubShiftRow(round1(1), round1(0), round1(3), round1(2)) ^ key((D_round*4.U)+1.U)), 
                        De_Line1(InvSubShiftRow(round1(1), round1(0), round1(3), round1(2)) ^ key((D_round*4.U)+1.U)), 
                        De_Line2(InvSubShiftRow(round1(1), round1(0), round1(3), round1(2)) ^ key((D_round*4.U)+1.U)), 
                        De_Line3(InvSubShiftRow(round1(1), round1(0), round1(3), round1(2)) ^ key((D_round*4.U)+1.U))) 
      round1(2) :=  Cat(De_Line0(InvSubShiftRow(round1(2), round1(1), round1(0), round1(3)) ^ key((D_round*4.U)+2.U)), 
                        De_Line1(InvSubShiftRow(round1(2), round1(1), round1(0), round1(3)) ^ key((D_round*4.U)+2.U)), 
                        De_Line2(InvSubShiftRow(round1(2), round1(1), round1(0), round1(3)) ^ key((D_round*4.U)+2.U)), 
                        De_Line3(InvSubShiftRow(round1(2), round1(1), round1(0), round1(3)) ^ key((D_round*4.U)+2.U))) 
      round1(3) :=  Cat(De_Line0(InvSubShiftRow(round1(3), round1(2), round1(1), round1(0)) ^ key((D_round*4.U)+3.U)), 
                        De_Line1(InvSubShiftRow(round1(3), round1(2), round1(1), round1(0)) ^ key((D_round*4.U)+3.U)), 
                        De_Line2(InvSubShiftRow(round1(3), round1(2), round1(1), round1(0)) ^ key((D_round*4.U)+3.U)), 
                        De_Line3(InvSubShiftRow(round1(3), round1(2), round1(1), round1(0)) ^ key((D_round*4.U)+3.U))) 
      D_round := D_round - 1.U
      io.ready := false.B
      io.reset := false.B
      io.out(0) := 0.U 
      io.out(1) := 0.U 
      io.out(2) := 0.U 
      io.out(3) := 0.U 
    }.otherwise{  
      io.out(0) :=  Cat(InvSubShiftRow(round1(0), round1(3), round1(2), round1(1)) ^ key(0), 
                        InvSubShiftRow(round1(0), round1(3), round1(2), round1(1)) ^ key(0), 
                        InvSubShiftRow(round1(0), round1(3), round1(2), round1(1)) ^ key(0), 
                        InvSubShiftRow(round1(0), round1(3), round1(2), round1(1)) ^ key(0)) 
      io.out(1) :=  Cat(InvSubShiftRow(round1(1), round1(0), round1(3), round1(2)) ^ key(1), 
                        InvSubShiftRow(round1(1), round1(0), round1(3), round1(2)) ^ key(1), 
                        InvSubShiftRow(round1(1), round1(0), round1(3), round1(2)) ^ key(1), 
                        InvSubShiftRow(round1(1), round1(0), round1(3), round1(2)) ^ key(1)) 
      io.out(2) :=  Cat(InvSubShiftRow(round1(2), round1(1), round1(0), round1(3)) ^ key(2), 
                        InvSubShiftRow(round1(2), round1(1), round1(0), round1(3)) ^ key(2), 
                        InvSubShiftRow(round1(2), round1(1), round1(0), round1(3)) ^ key(2), 
                        InvSubShiftRow(round1(2), round1(1), round1(0), round1(3)) ^ key(2)) 
      io.out(3) :=  Cat(InvSubShiftRow(round1(3), round1(2), round1(1), round1(0)) ^ key(3), 
                        InvSubShiftRow(round1(3), round1(2), round1(1), round1(0)) ^ key(3), 
                        InvSubShiftRow(round1(3), round1(2), round1(1), round1(0)) ^ key(3), 
                        InvSubShiftRow(round1(3), round1(2), round1(1), round1(0)) ^ key(3)) 
      io.ready := true.B
      io.reset := false.B
  }
  }.otherwise{
    E_round := 0.U
    D_round := c.NumberRounds.U
    io.reset := true.B
    io.ready := false.B
    io.out(0) := 0.U 
    io.out(1) := 0.U 
    io.out(2) := 0.U 
    io.out(3) := 0.U 
  }
}

// DOC include start: AES params
case class AESParams(
  address: BigInt = 0x2000,
  width: Int = 32)
// DOC include end: AES params


// DOC include start: AES key
case object AESKey extends Field[Option[AESParams]](None)
// DOC include end: AES key

class AESIO() extends Bundle {
  val clock = Input(Clock())
  val reset = Input(Bool())
}

trait AESTopIO extends Bundle {
  val aes_busy = Output(Bool())
}

trait HasAESIO extends BaseModule {
  val io = IO(new AESIO())
}



// DOC include start: AES chisel
class AESMMIOChiselModule() extends Module
  with HasAESIO
{
}
// DOC include end: AES chisel



// DOC include start: AES instance regmap

trait AESModule extends HasRegMap {
  val io: AESTopIO
  val c: AES_Config = new AES_128

  implicit val p: Parameters
  def params: AESParams
  val clock: Clock
  val reset: chisel3.core.Reset
  val input_ready = Wire(Bool())
  val input_valid = Wire(Bool())
  val output_valid = Wire(Bool())
  
  val key = RegInit(VecInit(Seq.fill(c.KeyLength)(0.U(32.W))))
  val message = RegInit(VecInit(Seq.fill(4)(0.U(32.W))))
  val status = Wire(UInt(2.W))
  val En_De = Wire(new DecoupledIO(UInt(32.W)))

  
  val s_idle :: s_run :: Nil = Enum(2)
  val state = RegInit(s_idle)
  val Cipher = Module(new Cipher_Module(c))
  val impl = Module(new AESMMIOChiselModule())

  val start =  RegInit(false.B)
  impl.io.clock := clock
  impl.io.reset := reset
  Cipher.io.start := start

  input_ready := state === s_idle
  output_valid := state === s_run

  when (state === s_idle && input_valid){
    start := true.B
    state := s_run
  } .elsewhen (state === s_run && Cipher.io.ready) {
    state := s_idle
    start := false.B
  } 
  
  for (i <- 0 until c.KeyLength) {
  Cipher.io.in(i) := Cat(key(i)(7,0),key(i)(15,8),key(i)(23,16),key(i)(31,24))
  }
  for (i <- 0 until 4) {
  Cipher.io.message(i) := Cat(message(i)(7,0),message(i)(15,8),message(i)(23,16),message(i)(31,24))
  }
  Cipher.io.en := En_De.bits(0)
  Cipher.io.de := En_De.bits(1)

  input_valid := En_De.valid
  En_De.ready := input_ready
  status := Cat(input_ready, output_valid)

  when(Cipher.io.start && Cipher.io.ready){
    for (i <- 0 until 4) {
      message(i) := Cat(Cipher.io.out(i)(7,0),Cipher.io.out(i)(15,8),Cipher.io.out(i)(23,16),Cipher.io.out(i)(31,24))
    }
  }

  io.aes_busy := state =/= s_idle

  regmap(
    0x00 -> Seq(RegField.r(2, status)), // read-only, gcd.ready is set on read
    0x04 -> Seq(RegField(32, key(0))),
    0x08 -> Seq(RegField(32, key(1))),
    0x0C -> Seq(RegField(32, key(2))),
    0x10 -> Seq(RegField(32, key(3))),
    0x24 -> Seq(RegField(32, message(0))),
    0x28 -> Seq(RegField(32, message(1))),
    0x2C -> Seq(RegField(32, message(2))),
    0x30 -> Seq(RegField(32, message(3))),
    0x34 -> Seq(RegField.w(32, En_De))) 
  
}
// DOC include end: AES instance regmap


// DOC include start: AES router
class AESTL(params: AESParams, beatBytes: Int)(implicit p: Parameters)
  extends TLRegisterRouter(
    params.address, "aes", Seq("ucbbar,aes"),
    beatBytes = beatBytes)(
      new TLRegBundle(params, _) with AESTopIO)(
      new TLRegModule(params, _, _) with AESModule)
// DOC include end: AES router

// DOC include start: AES lazy trait
trait CanHavePeripheryAES { this: BaseSubsystem =>
  private val portName = "aes"
  val aes = p(AESKey) match {
    case Some(params) => {
        val aes = LazyModule(new AESTL(params, pbus.beatBytes)(p))
        pbus.toVariableWidthSlave(Some(portName)) { aes.node }
        Some(aes)
    }
    case None => None
  }
}
// DOC include end: AES lazy trait

// DOC include start: AES imp trait
trait CanHavePeripheryAESModuleImp extends LazyModuleImp {
  val outer: CanHavePeripheryAES
  val aes_busy = outer.aes match {
    case Some(aes) => {
      val busy = IO(Output(Bool()))
      busy := aes.module.io.aes_busy
      Some(busy)
    }
    case None => None
  }
}

// DOC include end: AES imp trait

// DOC include start: AES config fragment
class WithAES() extends Config((site, here, up) => {
  case AESKey => Some(AESParams())
})
// DOC include end: AES config fragment

