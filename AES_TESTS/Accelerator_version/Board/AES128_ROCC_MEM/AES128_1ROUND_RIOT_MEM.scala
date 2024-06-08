
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



//========================================================================================================///
class AES_Module(opcodes: OpcodeSet, c: AES_Config = new AES_128)(implicit p: Parameters) extends LazyRoCC(opcodes) {
  override lazy val module = new AES_ModuleImp(this, c)
}

class AES_ModuleImp(outer: AES_Module, c: AES_Config)(implicit p: Parameters) extends LazyRoCCModuleImp(outer)
    with HasCoreParameters {
  

  // ------------> Alias <-------------
  val inputData1 = io.cmd.bits.rs1
  val inputData2 = io.cmd.bits.rs2
  val funct = io.cmd.bits.inst.funct
  // ----------------------------------

  val ReceiveKey_0 = funct === 0.U
  val ReceiveKey_1 = funct === 1.U
  val ReadKey_4 = funct === 4.U
  val Set_en_5 = funct === 5.U
  val Set_de_6 = funct === 6.U
  val ReceiveSize_7 = funct === 7.U
//=====================AES VALUES==================


  val Cipher = Module(new Cipher_Module(c))

  val data = Wire(UInt(32.W))   
  data := DontCare
  val key =  RegInit(VecInit(Seq.fill(c.KeyLength)(0.U(32.W))))
  val plaintext =  RegInit(VecInit(Seq.fill(4)(1.U(32.W))))  // 32/4= 8 blocks
  val size = RegInit(0.U(16.W))
  val start_delayed =  RegInit(false.B)
  val start =  RegInit(false.B)
  val load =  RegInit(false.B)
  val store =  RegInit(false.B)
  val en =  RegInit(false.B)
  val de =  RegInit(false.B)
  val ON =  RegInit(false.B)
//=====================AES VALUES==================

  val busy = RegInit(VecInit(Seq.fill(4){false.B}))
  val addr = RegInit(0.U(2.W))
  val max_addr = RegInit(3.U(2.W))
  val memRespTag = io.mem.resp.bits.tag(log2Up(4)-1,0)
  val address_load = RegInit(0.U(40.W))
  val address_store = RegInit(0.U(40.W))
  val addend = RegInit(0.U(40.W))
  val mem_flag =  RegInit(false.B)
  val mem_cmd = RegInit(0.U(4.W))
  
  for (i <- 0 until c.KeyLength) {
     Cipher.io.in(i) := key(i) 
  }
  for (i <- 0 until 4) {
    Cipher.io.message(i) := Cat(plaintext(i)(7,0),plaintext(i)(15,8),plaintext(i)(23,16),plaintext(i)(31,24))
  }
  start_delayed := start
  Cipher.io.start := start_delayed
  Cipher.io.en := en
  Cipher.io.de := de

  when (io.cmd.fire()){
    when(ReceiveKey_0) {
      key(0) := Cat(inputData1(7,0),inputData1(15,8),inputData1(23,16),inputData1(31,24))
      key(1) := Cat(inputData2(7,0),inputData2(15,8),inputData2(23,16),inputData2(31,24))
    }
     when(ReceiveKey_1) {
      key(2) := Cat(inputData1(7,0),inputData1(15,8),inputData1(23,16),inputData1(31,24))
      key(3) := Cat(inputData2(7,0),inputData2(15,8),inputData2(23,16),inputData2(31,24))
    }
    when(ReadKey_4) {
      data := key(inputData1)
    }
    when(Set_en_5) {  
      address_load := inputData1 
      address_store := inputData2   
      en := true.B
      de := false.B
      ON := true.B
      load := true.B
    }
    when(Set_de_6) {
      address_load := inputData1 
      address_store := inputData2     
      en := false.B
      de := true.B
      ON := true.B
      load := true.B
    }
    when(ReceiveSize_7) {
      size := inputData1
    }
  }

  when(ON){
    when(size >= 4.U){
      ///LOAD 
      when(load && !start && !store){
        mem_cmd := M_XRD  
        mem_flag := true.B
        addend := address_load
      ///ENCRYPT / DECRYPT
      }.elsewhen(!load && start && !store){
        when(Cipher.io.start && Cipher.io.ready){
          load  := false.B
          start := false.B
          store := true.B
          for (i <- 0 until 4) {
            plaintext(i.U) := Cat(Cipher.io.out(i)(7,0),Cipher.io.out(i)(15,8),Cipher.io.out(i)(23,16),Cipher.io.out(i)(31,24))
          }
        }
      ///STORE
      }.elsewhen(!load && !start && store){
        mem_cmd := M_XWR  
        addend := address_store
        mem_flag := true.B
        max_addr := 3.U
      }
    //OFF
    }.elsewhen(size === 0.U){
        max_addr := 3.U
        load  := false.B
        start := false.B
        store := false.B
        ON := false.B
    //PADDING
    }.otherwise{
      when((size % 4.U) =/= 0.U){
        plaintext(max_addr) := 0.U
        size := size + 1.U
        max_addr := max_addr - 1.U
      }
    }  
  }

  when (io.mem.resp.valid) {
    //MEMORY LOAD
    when(mem_cmd === M_XRD){
      plaintext(memRespTag) := io.mem.resp.bits.data
      busy(memRespTag) := false.B
      addr := addr + 1.U
      when(addr ===  max_addr){
        mem_flag := false.B
        load  := false.B
        start := true.B
        store := false.B
        addr := 0.U
        }
    //MEMORY STORE
    }.elsewhen(mem_cmd === M_XWR){
      busy(memRespTag) := false.B
      addr := addr + 1.U
      when(addr ===  max_addr){
        mem_flag := false.B
        load  := true.B
        start := false.B
        store := false.B
        address_store := address_store + 16.U
        size := size - 4.U
      }
    }
  }

  //control
  when (io.mem.req.fire()) {
    busy(addr) := true.B
  }

  val doResp = io.cmd.bits.inst.xd
  val stallReg = busy(addr)
  val stallMem = mem_flag && !io.mem.req.ready
  val stallResp = doResp && !io.resp.ready
 
  io.cmd.ready := !stallReg && !stallMem && !stallResp && !ON
  io.resp.valid := io.cmd.valid && doResp && !stallReg && !stallMem && !ON
  io.resp.bits.rd := io.cmd.bits.inst.rd
  io.resp.bits.data := data
  io.busy := io.cmd.valid || busy.reduce(_||_)
  io.interrupt := false.B

  io.mem.req.valid := mem_flag && !stallReg && !stallResp && !start
  io.mem.req.bits.addr := (addend + addr*4.U) 
  io.mem.req.bits.tag := addr
  io.mem.req.bits.cmd := mem_cmd // perform a load (M_XWR for stores) M_XRD load
  io.mem.req.bits.data := plaintext(addr) 
  io.mem.req.bits.phys := Bool(false)
  io.mem.req.bits.typ := MT_W // D = 8 bytes, W = 4, H = 2, B = 1 0x010
}
